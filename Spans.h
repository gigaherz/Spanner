#pragma once

#include <stdio.h> 
#include <memory.h>
#include <malloc.h>
#include <math.h>


struct Vector4f
{
	float X;
	float Y;
	float Z;
	float W;
};

struct TriangleVertex
{
	Vector4f pos;
	Vector4f color;
	Vector4f color_offset;
	Vector4f texture;
	float C1;
	float C2;
};

struct Triangle
{
	TriangleVertex* a;
	TriangleVertex* b;
	TriangleVertex* c;
};


struct SpanVertex
{
	float X;
	float Z;
	float SrcCoef1;
	float SrcCoef2;
};

struct Span
{
	SpanVertex start;
	SpanVertex end;
	TriangleVertex* SrcVertex1;
	TriangleVertex* SrcVertex2;
	TriangleVertex* SrcVertex3;

	//for the rendering list
	Span *next;

	//for the dependency tree
	Span *tree_brother;
	Span *tree_child;

	bool degenerate() const
	{
		return (abs(start.X-end.X)<0.001)||(start.X>end.X);
	}
};

struct SpanHolder
{
	Span *span;
	SpanHolder *next;
};

class SpanBuffer
{
	SpanHolder *spanHolders;
	SpanHolder *firstFree;
	int spansSize; //number of spans allocated in the arrays
	int allocated;

	void MakeBigger()
	{
		int n=spansSize<<1;

		spanHolders = (SpanHolder*) realloc(spanHolders,n*sizeof(SpanHolder));

		for(int i=spansSize;i<n;i++)
		{
			spanHolders[i].span=NULL;
			spanHolders[i].next=spanHolders+i+1;
		}

		spanHolders[n-1].next=NULL;
		firstFree=spanHolders+spansSize;
		spansSize=n;
	}

public:
	SpanHolder *first;
	SpanHolder *last;
	float left;
	float right;

	Span *firstspan;

	SpanBuffer()
	{
		Init();
	}

	void Init()
	{
		int n=1024;

		spanHolders=(SpanHolder*)malloc(n*sizeof(SpanHolder));

		for(int i=0;i<n;i++)
		{
			spanHolders[i].span=NULL;
			spanHolders[i].next=spanHolders+i+1;
		}

		spanHolders[n-1].next=NULL;
		firstFree=spanHolders;
		spansSize=n;

		first=NULL;
		last=NULL;
		allocated=0;
	}

	void AddSpan(Span *s)
	{
		if(firstFree==NULL)
			MakeBigger();

		SpanHolder *h=firstFree;
		firstFree=firstFree->next;
		h->span=s;
		h->next=NULL;

		if(first==NULL)
		{
			first=h;
			last=h;
		}
		else
		{
			last->next=h;
			last=h;
		}

		allocated++;
	}

	void Clear()
	{
		int n=spansSize;

		for(int i=0;i<n;i++)
		{
			spanHolders[i].span=NULL;
			spanHolders[i].next=spanHolders+i+1;
		}

		spanHolders[n-1].next=NULL;
		firstFree=spanHolders;
		first=NULL;
		last=NULL;
		allocated=0;
	}
};

struct SpanArray
{
	Span *spans;
	SpanHolder *spanHolders;
	int size;
};

class SpanStorage
{
private:
	SpanArray *spanArrays;
	SpanHolder *firstFree;
	SpanHolder *lastUsed;

	int nArrays;		//total allocated
	int nArraysFull;	//number of arrays used
	int spansSize;		//number of spans allocated in the last array

	void MakeBigger()
	{
		int n=spansSize<<1;

		int k=nArraysFull++;
		if(k==nArrays)
		{
			nArrays<<=1;
			spanArrays=(SpanArray*)realloc(spanArrays,nArrays*sizeof(SpanArray));
		}

		spanArrays[k].spans       = (Span      *)malloc(n*sizeof(Span));
		spanArrays[k].spanHolders = (SpanHolder*)malloc(n*sizeof(SpanHolder));
		spanArrays[k].size = n;

		for(int i=0;i<n;i++)
		{
			spanArrays[k].spanHolders[i].span=spanArrays[k].spans+i;
			spanArrays[k].spanHolders[i].next=spanArrays[k].spanHolders+i+1;
		}
		spanArrays[k].spanHolders[n-1].next=NULL; //should be null
		firstFree=spanArrays[k].spanHolders;
		spansSize=n;
	}

public:
	SpanStorage(int unused)
	{
		nArrays=16;
		nArraysFull=1;

		spanArrays=(SpanArray*)malloc(nArrays*sizeof(SpanArray));

		Init(4096);
	}

	void Init(int initialSize)
	{
		int n=initialSize;

		spanArrays[0].spans      =(Span      *)malloc(n*sizeof(Span));
		spanArrays[0].spanHolders=(SpanHolder*)malloc(n*sizeof(SpanHolder));
		spanArrays[0].size = n;

		for(int i=0;i<n;i++)
		{
			spanArrays[0].spanHolders[i].span=spanArrays[0].spans+i;
			spanArrays[0].spanHolders[i].next=spanArrays[0].spanHolders+i+1;
		}

		spanArrays[0].spanHolders[n-1].next=NULL;
		firstFree=spanArrays[0].spanHolders;
		lastUsed=NULL;
		spansSize=n;
	}

	//Returns a free span
	Span *GetNew()
	{
		if(firstFree==NULL)
			MakeBigger();
		Span *s=firstFree->span;
		SpanHolder *k=firstFree;

		firstFree=k->next;

		k->next=lastUsed;
		lastUsed=k;

		return s;
	}

	//Marks a span as free
	void Free(Span*s)
	{
		SpanHolder *c;
		SpanHolder *l=NULL;
		for(c=lastUsed;c!=NULL;l=c,c=c->next)
		{
			if(c->span==s)
			{
				if(l==NULL)
					lastUsed=c->next;
				else
					l->next=c->next;

				c->next=firstFree;
				firstFree=c;

				return;
			}
		}
		//can't find it? :/
	}

	void Clear()
	{
		int n=spansSize;

		int j=nArraysFull;
		while(j>0)
		{
			j--;
			for(int i=0;i<spanArrays[j].size;i++)
			{
				spanArrays[j].spanHolders[i].next=spanArrays[j].spanHolders+i+1;
			}
			if(j==(nArraysFull-1))
				spanArrays[j].spanHolders[spanArrays[j].size-1].next=NULL;
			else
				spanArrays[j].spanHolders[spanArrays[j].size-1].next=spanArrays[j+1].spanHolders;
		}
		firstFree=spanArrays[0].spanHolders;
		lastUsed=NULL;
	}

};

class Scanline
{
public:
	SpanBuffer *buffers;
	int nbuffers;

	Scanline(int nbuffs,float width)
	{
		Init(nbuffs,width);
	}

	void Init(int nbuffs,float width)
	{
		nbuffers=nbuffs;
		buffers=new SpanBuffer[nbuffers];

		float left=0;
		for(int i=0;i<nbuffers;i++)
		{
			buffers[i].Init();
			buffers[i].left=left;
			left+=width;
			buffers[i].right=left;
		}
	}

	void Clear()
	{
		for(int i=0;i<nbuffers;i++)
		{
			buffers[i].Clear();
		}
	}

};

class ScreenBuffer
{
public:
	int width;
	int height;
	int chunkwidth;
	Scanline *lines;

	ScreenBuffer(int w,int h,int chunk_width)
	{
		width=w;
		height=h;
		chunkwidth=chunk_width;
		lines=(Scanline*)malloc(sizeof(Scanline)*height);
		for(int i=0;i<height;i++)
		{
			lines[i].Init(width/chunk_width,chunk_width);
		}
	}

	void Clear()
	{
		for(int i=0;i<height;i++)
		{
			lines[i].Clear();
		}
	}

};


typedef unsigned __int32 Pixel;

struct Surface
{
	int width;
	int height;
	int pitch;
	int pixel_format; //0=ARGB/XRGB 32
	Pixel *buffer;

	void Clear(Pixel color) //yes yes I totally ignore the clear color!
	{
		memset(buffer,0,pitch*height);
	}
};
//function declarations

bool SpanClip(Span *dest,const Span *src,float left,float right);

void SpanAddToBuffers(SpanStorage *spans, Scanline *line, const Span *s, int nchunks, float chunk_width);
void InterpolateTriangle(const Triangle *tri, SpanStorage* spans, ScreenBuffer *scr);
void DrawScreenBuffer(Surface *dest, const ScreenBuffer *src);

void SortScanline(Scanline *sl);
void SortScreenBuffer(ScreenBuffer *sb);

float __inline Interpolatef(float a,float b,float p);

extern Surface buff;
extern ScreenBuffer scr;
extern SpanStorage spans;
extern TriangleVertex vl[];
extern Triangle tl[];

#define WIDTH 512
#define HEIGHT 256
