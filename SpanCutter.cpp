#include "Spans.h"
#include <assert.h>
#include "Profiler.h"

//interpolates a float value
float __inline Interpolatef(float a, float b, float p)
{
	return (a+p*(b-a));
}

//Interpolates a vertex between two vertices, based on a position value
void SpanInterpolateVertex(SpanVertex *dest, const Span *s, float p)
{
	dest->X=Interpolatef(s->start.X,s->end.X,p);
	dest->Z=Interpolatef(s->start.Z,s->end.Z,p);
	dest->SrcCoef1=Interpolatef(s->start.SrcCoef1,s->end.SrcCoef1,p);
	dest->SrcCoef2=Interpolatef(s->start.SrcCoef2,s->end.SrcCoef2,p);
}

//Clips a span to the specified X range
bool SpanClip(Span *dest,const Span *src,float left,float right)
{
	Profiler_FunctionEnter(SpanClip);

	if(src->end.X<=left)
		Profiler_Return(false);

	if(src->start.X>=right)
		Profiler_Return(false);

	if(src->start.X>=left)
		dest->start=src->start;
	else
		SpanInterpolateVertex(&(dest->start),src,(left-src->start.X)/(src->end.X-src->start.X));

	if(src->end.X<=right)
		dest->end=src->end;
	else
		SpanInterpolateVertex(&(dest->end),src,(right-src->start.X)/(src->end.X-src->start.X));

	dest->SrcVertex1=src->SrcVertex1;
	dest->SrcVertex2=src->SrcVertex2;
	dest->SrcVertex3=src->SrcVertex3;
	Profiler_Return(true);
}

//Checks if two spans intersect, and sets the intersection point if they do
bool SpanIntersect(const Span* first, const Span* second, SpanVertex *ip)
{
	Profiler_FunctionEnter(SpanIntersect);
	float Ax=first->start.X;
	float Ay=first->start.Z;
	float Bx=first->end.X;
	float By=first->end.Z;
	float Cx=second->start.X;
	float Cy=second->start.Z;
	float Dx=second->end.X;
	float Dy=second->end.Z;

	float na = (Ay-Cy)*(Dx-Cx)-(Ax-Cx)*(Dy-Cy);
	float nb = (Ay-Cy)*(Bx-Ax)-(Ax-Cx)*(By-Ay);
    float dn = (Bx-Ax)*(Dy-Cy)-(By-Ay)*(Dx-Cx);

	if(abs(dn)<1e-20) //parallel or coincident
		Profiler_Return(false);

	float ua=na/dn;
	float ub=nb/dn;

	if((ua>0.01)&&(ua<0.99)&&(ub>0.01)&&(ub<0.99)) //might be touching, but not intersecting
	{
		SpanInterpolateVertex(ip,second,ub);
		Profiler_Return(true);
	}
	//changed to if() { do } instead of if(not) { exit }because of -1.#IND000's
	Profiler_Return(false);
}


//Checks the span against all the spans in the buffer, splitting as needed,
//and adds the remaining bits of itself into the buffer after it.
bool SpanAdd(SpanStorage *spans, SpanBuffer *dest, const Span *a, int level=0)
{
	Profiler_FunctionEnter(SpanAdd);
	if(level>20)
	{
		printf("wtf...\n");
	}

	Span *clipped=spans->GetNew();

	SpanClip(clipped,a,dest->left,dest->right); //gets a version of the span clipped inside the buffer range

	SpanVertex ipoint;

	for(SpanHolder *it=dest->first; it!=NULL; it=it->next)
	{
		// current = it->span
		if(SpanIntersect(it->span,clipped,&ipoint))
		{
			Span *lpart=spans->GetNew();
			Span *rpart=spans->GetNew();

			if(!SpanClip(lpart,clipped,dest->left,ipoint.X))
				spans->Free(lpart);
			else if(lpart->degenerate())
				spans->Free(lpart);
			else if(SpanAdd(spans,dest,lpart,level+1))
				spans->Free(lpart);

			if(!SpanClip(rpart,clipped,ipoint.X,dest->right))
				spans->Free(rpart);
			else if(rpart->degenerate())
				spans->Free(rpart);
			else if(SpanAdd(spans,dest,rpart,level+1))
				spans->Free(rpart);

			spans->Free(clipped);
			Profiler_Return(true);
		}
	}

	//if we reach here, we didn't find any intersection for the span, so add it to the buffer

	dest->AddSpan(clipped);
	Profiler_Return(false);
}

void SpanAddToBuffers(SpanStorage *spans, Scanline *line, const Span *s, int nchunks, float chunk_width)
{
	Profiler_FunctionEnter(SpanAddToBuffers);
	//assume start.X<=end.X, so Bs<=Be
	int Bs = (int)floor(s->start.X/32);
	int Be = (int)floor(s->end.X/32);

	if(Bs<0) Bs=0;
	if(Be>19) Be=19;

	//assume 20 buffers of 32 pixels
	for(int i=0;i<nchunks;i++)
	{
		Span *t=spans->GetNew();
		if(SpanClip(t,s,i*chunk_width,(i+1)*chunk_width))
		{
			if(SpanAdd(spans,&(line->buffers[i]),t))
				spans->Free(t);
		}
		else
			spans->Free(t);
	}
	Profiler_FunctionExit();
}
