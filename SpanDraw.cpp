#include "Spans.h"
#include "Profiler.h"

#define interpolate3f(s1,s2,s3,p1,p2) (s1+p1*(s2-s1)+p2*(s3-s1))

#define interpolate3_xyzw(dest,src1,src2,src3,p1,p2) \
	(dest).X = interpolate3f( (src1).X, (src2).X, (src3).X, p1, p2); \
	(dest).Y = interpolate3f( (src1).Y, (src2).Y, (src3).Y, p1, p2); \
	(dest).Z = interpolate3f( (src1).Z, (src2).Z, (src3).Z, p1, p2); \
	(dest).W = interpolate3f( (src1).W, (src2).W, (src3).W, p1, p2)

#define interpolate3_vertex(dest,src1,src2,src3,p1,p2) \
	interpolate3_xyzw((dest).pos,         (src1).pos,         (src2).pos,         (src3).pos,         p1, p2); \
	interpolate3_xyzw((dest).color,       (src1).color,       (src2).color,       (src3).color,       p1, p2); \
	interpolate3_xyzw((dest).color_offset,(src1).color_offset,(src2).color_offset,(src3).color_offset,p1, p2); \
	interpolate3_xyzw((dest).texture,     (src1).texture,     (src2).texture,     (src3).texture,     p1, p2)

#define def_init_inter_var(v,s,e,n) \
	float v##_s = (s); \
	float v##_sdiff  = ((e)-(s))/(n);

#define def_init_inter_var_pix(v,comp)    def_init_inter_var(v,st.##comp,ed.##comp,(xend-xstart))

#define inter_step_pix(v) v##_s+=v##_sdiff;

int tag=0;

float __forceinline min(float x,float y) { return ((x<y)?(x):(y)); }
float __forceinline max(float x,float y) { return ((x>y)?(x):(y)); }

float __forceinline clamp(float a,float m, float n)
{
	return min(max(a,m),n);
}

int Spans;
int Buffers;
int Scanlines;

void __forceinline DrawPixel(Pixel* pix, float cr, float cg, float cb, float ca, float or, float og, float ob, float oa, float tu, float tv)
{
	//TODO: add texture lookups

	Pixel r=(Pixel)clamp(cr*255+or*255,0,255);
	Pixel g=(Pixel)clamp(cg*255+og*255,0,255);
	Pixel b=(Pixel)clamp(cb*255+ob*255,0,255);
	Pixel a=(Pixel)clamp(ca*255+oa*255,0,255);

	*pix = (r<<16) | (g<<8) | (b<<0) | (a<<24);
}

void DrawSpan(Pixel*dest, const Span *src)
{
	//here comes the fun! I have to interpolate 2 TriangleVertex'es from the span ends,
	// then draw the interpolated values from them into the pixel buffer!

	TriangleVertex st;	//final result for point in a-b-c triangle for start vertex
	TriangleVertex ed;	//final result for point in a-b-c triangle for end vertex

	if(src->degenerate())
		return;

	Spans++;

	interpolate3_vertex(st,
		*(src->SrcVertex1),*(src->SrcVertex2),*(src->SrcVertex3),
		src->start.SrcCoef1,src->start.SrcCoef2);

	interpolate3_vertex(ed,
		*(src->SrcVertex1),*(src->SrcVertex2),*(src->SrcVertex3),
		src->end.SrcCoef1,src->end.SrcCoef2);

	int xstart= (int)st.pos.X;
	int xend  = (int)ed.pos.X;

	def_init_inter_var_pix(cr,color.X);
	def_init_inter_var_pix(cg,color.Y);
	def_init_inter_var_pix(cb,color.Z);
	def_init_inter_var_pix(ca,color.W);
	def_init_inter_var_pix(or,color_offset.X);
	def_init_inter_var_pix(og,color_offset.Y);
	def_init_inter_var_pix(ob,color_offset.Z);
	def_init_inter_var_pix(oa,color_offset.W);
	def_init_inter_var_pix(tu,texture.X);
	def_init_inter_var_pix(tv,texture.Y);

	for(int x=xstart; x<=xend; x++)
	{
		DrawPixel(&(dest[x]),cr_s,cg_s,cb_s,ca_s,or_s,og_s,ob_s,oa_s,tu_s,tv_s);
		inter_step_pix(cr);
		inter_step_pix(cg);
		inter_step_pix(cb);
		inter_step_pix(ca);
		inter_step_pix(or);
		inter_step_pix(og);
		inter_step_pix(ob);
		inter_step_pix(oa);
		inter_step_pix(tu);
		inter_step_pix(tv);
	}

}

void DrawSpanBuffer(Pixel *dest, const SpanBuffer *src)
{
	Buffers++;
	for(Span *sp=src->firstspan;sp!=NULL;sp=sp->next)
	{
		DrawSpan(dest,sp);
	}
}

void DrawScanline(Pixel *dest, const Scanline *src)
{
	Scanlines++;
	for(int i=0;i<src->nbuffers;i++)
	{
		DrawSpanBuffer(dest,src->buffers+i);
	}
}

void DrawScreenBuffer(Surface *dest, const ScreenBuffer *src)
{
	Profiler_FunctionEnter(DrawScreenBuffer);

	Scanlines=0;
	Buffers=0;
	Spans=0;
	
	for(int y=0;y<src->height;y++)
	{
		Pixel *line = (Pixel*)((char*)dest->buffer+y*dest->pitch);
		DrawScanline(line,src->lines+y);
	}

	/*
	printf("Frame Drawn. Statistics:\n");
	printf(" * Total Scanlines: %d\n",Scanlines);
	printf(" * Total Buffers: %d\n",Buffers);
	printf(" * Total Spans: %d\n",Spans);
	printf(" * Avg. Spans/Buffer: %g\n",(float)Spans/(float)Buffers);
	printf(" * Avg. Spans/Scanline: %g\n",(float)Spans/(float)Scanlines);
	*/
	Profiler_FunctionExit();
}