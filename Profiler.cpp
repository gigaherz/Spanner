#include <windows.h>
#include <stdio.h>
#include "Profiler.h"

typedef __int64 timestamp;

struct FunctionInfo
{
	const char *fname;
	int calls;
	timestamp totaltime;
	timestamp nochildtime;
};

struct CallInfo
{
	int fid;
	timestamp globalstart;
	timestamp localstart;
};

FunctionInfo funcs[1024];
int used;

CallInfo callStack[4096];
int stackLevel;

timestamp freq;

timestamp Profiler_GetTimestamp()
{
	timestamp t;
	QueryPerformanceCounter((LARGE_INTEGER*)&t);
	return t;
}

void Profiler_Init()
{
	QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
	used=0;
	stackLevel=0;
}

int Profiler_Alloc(const char *fname)
{
	fprintf(stderr,"Allocating function entry for '%s'...\n",fname);
	funcs[used].fname=fname;
	funcs[used].calls=0;
	funcs[used].totaltime=0;
	funcs[used].nochildtime=0;
	return used++;
}

void Profiler_PauseOld()
{
	if(stackLevel==0) return;
	timestamp diff=(Profiler_GetTimestamp()- callStack[stackLevel-1].localstart)*1000000/freq;
	funcs[callStack[stackLevel-1].fid].nochildtime+=diff;
}

void Profiler_ResumeOld()
{
	if(stackLevel==0) return;
	callStack[stackLevel-1].localstart=Profiler_GetTimestamp();
}

void Profiler_Enter(int fid)
{
	Profiler_PauseOld();

	funcs[fid].calls++;
	callStack[stackLevel].fid=fid;
	callStack[stackLevel].globalstart=Profiler_GetTimestamp();
	callStack[stackLevel].localstart=Profiler_GetTimestamp();
	stackLevel++;
}

void Profiler_Exit(int fid)
{
	stackLevel--;

	timestamp et=Profiler_GetTimestamp();

	if(fid!=callStack[stackLevel].fid)
	{
		fprintf(stderr,"ERROR: Function '%s' is missing an Exit call!!!\n",funcs[callStack[stackLevel].fid].fname);
	}
	timestamp ldiff=(et-callStack[stackLevel].localstart)*1000000/freq;
	timestamp gdiff=(et-callStack[stackLevel].globalstart)*1000000/freq;
	funcs[fid].totaltime+=gdiff;
	funcs[fid].nochildtime+=ldiff;

	Profiler_ResumeOld();
}

void Profiler_PrintFunctionInfo(int fid)
{
	fprintf(stderr," * %8d %16I64d %16I64d '%s'\n",funcs[fid].calls,funcs[fid].totaltime,funcs[fid].nochildtime,funcs[fid].fname);
}

void Profiler_DisplayInfo()
{
	fprintf(stderr,"Profiling results:\n");
	fprintf(stderr," * Calls    Total Time       Time W/O Childs  Function Name\n");
	for(int i=0;i<used;i++)
	{
		Profiler_PrintFunctionInfo(i);
	}
}
