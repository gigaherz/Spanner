#pragma once

int Profiler_Alloc(const char *fname);
void Profiler_Enter(int fid);
void Profiler_Exit(int fid);
void Profiler_Init();
void Profiler_DisplayInfo();

//#define USE_PROFILING

#ifdef USE_PROFILING

#define Profiler_FunctionEnterA(fname) \
	static int profiler_fid=0; \
	if(profiler_fid==0) profiler_fid=Profiler_Alloc(#fname); \
	Profiler_Enter(profiler_fid)

#define Profiler_FunctionEnter(fname) \
	static int profiler_fid=Profiler_Alloc(#fname); \
	Profiler_Enter(profiler_fid)

#define Profiler_FunctionExit() \
	Profiler_Exit(profiler_fid)

#define Profiler_Return(retval) \
	do { \
		Profiler_Exit(profiler_fid); \
		return retval; \
	} while(0)

#else

#define Profiler_FunctionEnterA(fname) /* fname */
#define Profiler_FunctionEnter(fname) /* fname */
#define Profiler_FunctionExit() /* fname */
#define Profiler_Return(retval) return retval;

#endif