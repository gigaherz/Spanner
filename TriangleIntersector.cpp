#include <math.h>
#include <assert.h>

#include "Spans.h"
#include "Profiler.h"

#define interpolate(a,b,p) (a+p*(b-a))

void SwapInte(int*a,int*b)
{
	int c=*a;
	*a=*b;
	*b=c;
}

void SwapVertex(TriangleVertex**a,TriangleVertex**b)
{
	TriangleVertex *c=*a;
	*a=*b;
	*b=c;
}

void SortVertices(Triangle*t)
{
	if( t->a->pos.Y > t->b->pos.Y )		//check if a>b
		SwapVertex(&(t->a),&(t->b));

	if( t->b->pos.Y > t->c->pos.Y )		//check if second > c
		SwapVertex(&(t->b),&(t->c));

	if( t->a->pos.Y > t->b->pos.Y )		//check if first > second
		SwapVertex(&(t->a),&(t->b));
}

#define def_inter_value(v,s,e) \
	float v##_start = (s); \
	float v##_end   = (e); \

#define def_inter_var(v) \
	float v##_s; \
	float v##_e; \
	float v##_sdiff; \
	float v##_ediff;

#define init_inter_var(v,si,ei,sf,ef,n) \
		v##_s = (si); \
		v##_e = (ei); \
		v##_sdiff  = ((sf)-(si))/(n); \
		v##_ediff  = ((ef)-(ei))/(n);

#define inter_step(v) v##_s+=v##_sdiff; v##_e+=v##_ediff;

#define def_inter_middle(v,comp)	def_inter_value(v,(t.b->##comp),interpolate(t.a->##comp,t.c->##comp, prog))

#define init_inter_vars1(v,comp)    init_inter_var(v,t.a->##comp,t.a->##comp,v##_start,v##_end,(ymid-ystart))
#define init_inter_vars2(v,comp)    init_inter_var(v,t.a->##comp,t.a->##comp,v##_end,v##_start,(ymid-ystart))
#define init_inter_vars3(v,comp)    init_inter_var(v,v##_start,v##_end,t.c->##comp,t.c->##comp,(yend-ymid))
#define init_inter_vars4(v,comp)    init_inter_var(v,v##_end,v##_start,t.c->##comp,t.c->##comp,(yend-ymid))

#define fix_position(v) if(v.W==0) v.W=1; v.X=v.X/v.W; v.Y=v.Y/v.W; v.Z=v.Z/v.W; v.W=v.W/v.W;

void InterpolateTriangle(const Triangle *tri, SpanStorage* spans, ScreenBuffer *scr)
{
	Profiler_FunctionEnter(InterpolateTriangle);

	Triangle t={tri->a,tri->b,tri->c};

	fix_position(t.a->pos);
	fix_position(t.b->pos);
	fix_position(t.c->pos);

	SortVertices(&t); //make sure the 3 vertices are sorted in the Y direction

	//add the coefs
	t.a->C1=0;
	t.a->C2=0;
	t.b->C1=1;
	t.b->C2=0;
	t.c->C1=0;
	t.c->C2=1;

	int ystart=(int)(t.a->pos.Y+0.5);
	int ymid  =(int)(t.b->pos.Y+0.5);
	int yend  =(int)(t.c->pos.Y+0.5);

	float prog = (((t.b->pos.Y)-(t.a->pos.Y)) / ((t.c->pos.Y)-(t.a->pos.Y)));

	//values at the middle point
	def_inter_middle(px,pos.X);
	def_inter_middle(pz,pos.Z);
	def_inter_middle(c1,C1);
	def_inter_middle(c2,C2);

	def_inter_var(px);
	def_inter_var(pz);
	def_inter_var(c2);
	def_inter_var(c1);

	if(px_start<px_end) //b to the left of a->c
	{
		init_inter_vars1(px,pos.X);
		init_inter_vars1(pz,pos.Z);
		init_inter_vars1(c1,C1);
		init_inter_vars1(c2,C2);
	}
	else //if(px_start>px_end) // b to the right of a->c
	{
		init_inter_vars2(px,pos.X);
		init_inter_vars2(pz,pos.Z);
		init_inter_vars2(c1,C1);
		init_inter_vars2(c2,C2);
	}

	for(int y=ystart;y<ymid;y++)
	{
		SpanVertex vs={px_s,pz_s,c1_s,c2_s};
		SpanVertex ve={px_e,pz_e,c1_e,c2_e};
		Span *s=spans->GetNew();

		s->start=vs;
		s->end=ve;
		s->SrcVertex1=t.a;
		s->SrcVertex2=t.b;
		s->SrcVertex3=t.c;

		assert(vs.X<=ve.X);

		if((y>=0)&&(y<scr->height)&&(!s->degenerate()))
			SpanAddToBuffers(spans,&(scr->lines[y]),s,scr->width/scr->chunkwidth,scr->chunkwidth);

		inter_step(px);
		inter_step(pz);
		inter_step(c1);
		inter_step(c2);
	}

	if(px_start<px_end) //b to the left of a->c
	{
		init_inter_vars3(px,pos.X);
		init_inter_vars3(pz,pos.Z);
		init_inter_vars3(c1,C1);
		init_inter_vars3(c2,C2);
	}
	else //if(px_start>px_end) // b to the right of a->c
	{
		init_inter_vars4(px,pos.X);
		init_inter_vars4(pz,pos.Z);
		init_inter_vars4(c1,C1);
		init_inter_vars4(c2,C2);
	}

	for(int y=ymid;y<=yend;y++)
	{
		SpanVertex vs={px_s,pz_s,c1_s,c2_s};
		SpanVertex ve={px_e,pz_e,c1_e,c2_e};
		Span *s=spans->GetNew();

		s->start=vs;
		s->end=ve;
		s->SrcVertex1=t.a;
		s->SrcVertex2=t.b;
		s->SrcVertex3=t.c;

		
		if((y>=0)&&(y<scr->height)&&(!s->degenerate()))
			SpanAddToBuffers(spans,&(scr->lines[y]),s,scr->width/scr->chunkwidth,scr->chunkwidth);

		inter_step(px);
		inter_step(pz);
		inter_step(c1);
		inter_step(c2);
	}

	Profiler_FunctionExit();
}
