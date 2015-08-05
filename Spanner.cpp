// Spanner.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include "Spans.h"
#include "Profiler.h"

#define LX (WIDTH-1)
#define LY (HEIGHT-1)

Pixel screen[WIDTH*HEIGHT];

//vertex list
TriangleVertex vl[]={
//	{{px,py,pz,1},{cr,cg,cb,ca},{or,og,ob,oa},{tu,tv}},
	{{0,0, 5,1},{1,0,0,0},{0,0,0,0},{0,0}},
	{{0,0, 2,1},{0,1,0,0},{0,0,0,0},{0,0}},
	{{0,0, -3,1},{0,0,1,0},{0,0,0,0},{0,0}},

	{{ 0, 0,-20,1},{1,0,0,0},{0,0,0,0},{0,0}},
	{{LX, 0,-20,1},{1,0,0,0},{0,0,0,0},{0,0}},
	{{ 0,LY,-20,1},{1,0,0,0},{0,0,0,0},{0,0}},
	{{LX,LY,-20,1},{1,0,0,0},{0,0,0,0},{0,0}},
	{{LX, 0,-20,1},{1,0,0,0},{0,0,0,0},{0,0}},
	{{ 0,LY,-20,1},{1,0,0,0},{0,0,0,0},{0,0}},

	{{ 0, 0,0,1},{1,0,1,0},{0,0,0,0},{0,0}},
	{{LX, 0,0,1},{1,1,0,0},{0,0,0,0},{0,0}},
	{{ 0,LY,0,1},{0,1,1,0},{0,0,0,0},{0,0}},
	{{LX,LY,0,1},{1,0,1,0},{0,0,0,0},{0,0}},
	{{LX, 0,0,1},{1,1,0,0},{0,0,0,0},{0,0}},
	{{ 0,LY,0,1},{0,1,1,0},{0,0,0,0},{0,0}},

	//end vertex... useful only to avoid errors of extra/missing "," in the rest :P
	{{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0}}
};

//triangle list
Triangle tl[]={
	{&(vl[ 3]),&(vl[ 4]),&(vl[ 5])},
	{&(vl[ 6]),&(vl[ 7]),&(vl[ 8])},
	{&(vl[ 9]),&(vl[10]),&(vl[11])},
	{&(vl[12]),&(vl[13]),&(vl[14])},
	{&(vl[ 0]),&(vl[ 1]),&(vl[ 2])},

	//end marker 
	{0,0,0}
};

Surface buff = {WIDTH, HEIGHT, WIDTH*4, 0, screen};
ScreenBuffer scr(WIDTH,HEIGHT,WIDTH);
SpanStorage spans(0);

int __stdcall RealMain();

int main(int argc, char* argv[])
{
	Profiler_Init();

	Profiler_FunctionEnter(Totals);

	RealMain();

	Profiler_FunctionExit();

	Profiler_DisplayInfo();
	return 0;
}

