/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 108
   ======================================================================== */

#if _WIN32

#include <intrin.h>
#include <windows.h>
#include <psapi.h>

struct os_metrics
{
   b32 Initialized;
   HANDLE ProcessHandle;
};
static os_metrics GlobalMetrics;

static u64 GetOSTimerFreq(void)
{
	LARGE_INTEGER Freq;
	QueryPerformanceFrequency(&Freq);
	return Freq.QuadPart;
}

static u64 ReadOSTimer(void)
{
	LARGE_INTEGER Value;
	QueryPerformanceCounter(&Value);
	return Value.QuadPart;
}

static u64 ReadOSPageFaultCount(void)
{
   PROCESS_MEMORY_COUNTERS_EX MemoryCounters = {};
   MemoryCounters.cb = sizeof(MemoryCounters);
   GetProcessMemoryInfo(GloablMetrics.ProcessHandle, (PROCESS_MEMORY_COUNTERS *)&MemoryCounters, sizeof(MemoryCounters));

   u64 Result = MemoryCounters.PageFaultCount;
   return Result;
}

static void InitializeOSMetrics(void)
{
   if (!GlobalMetrics.Initialized)
   {
      GlobalMetrics.Initialized = true;
      GlobalMetrics.ProcessHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, GetCurrentProcessId());
   }
}

#else

#include <x86intrin.h>
#include <sys/time.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <stdint.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/perf_event.h>

struct os_metrics
{
   b32 Initialized;
   int PageFaultFd;
};
static os_metrics GlobalMetrics;

static u64 GetOSTimerFreq(void)
{
#if 1
   return 1000000000;
#else
	return 1000000;
#endif
}

static u64 ReadOSTimer(void)
{
	// NOTE(casey): The "struct" keyword is not necessary here when compiling in C++,
	// but just in case anyone is using this file from C, I include it.
#if 1
	struct timespec tm;
	clock_gettime(CLOCK_MONOTONIC, &tm);
	
	u64 Result = GetOSTimerFreq()*(u64)tm.tv_sec + (u64)tm.tv_nsec;
	return Result;
#else
	struct timeval Value;
	gettimeofday(&Value, 0);
	
	u64 Result = GetOSTimerFreq()*(u64)Value.tv_sec + (u64)Value.tv_usec;
	return Result;
#endif
}

static long perf_event_open(struct perf_event_attr *hw_event,
                            pid_t pid,
                            int cpu,
                            int group_fd,
                            unsigned long flags)
{
  int ret = syscall(__NR_perf_event_open, hw_event, pid, cpu,
                    group_fd, flags);
  return ret;
}

static u64 ReadOSPageFaultCount(void)
{
   u64 Result;

   // Stop counting and read value
   ioctl(GlobalMetrics.PageFaultFd, PERF_EVENT_IOC_DISABLE, 0);
   read(GlobalMetrics.PageFaultFd, &Result, sizeof(Result));

   return Result;
}

static void InitializeOSMetrics(void)
{
   if (!GlobalMetrics.Initialized)
   {
      GlobalMetrics.Initialized = true;
      struct perf_event_attr pe_attr_page_faults;
      memset(&pe_attr_page_faults, 0, sizeof(pe_attr_page_faults));
      pe_attr_page_faults.size = sizeof(pe_attr_page_faults);
      pe_attr_page_faults.type =   PERF_TYPE_SOFTWARE;
      pe_attr_page_faults.config = PERF_COUNT_SW_PAGE_FAULTS;
      pe_attr_page_faults.disabled = 1;
      pe_attr_page_faults.exclude_kernel = 1;
      GlobalMetrics.PageFaultFd = perf_event_open(&pe_attr_page_faults, 0, -1, -1, 0);
      if (GlobalMetrics.PageFaultFd == -1)
      {
         printf("perf_event_open failed for page faults: %s\n", strerror(errno));
         GlobalMetrics.Initialized = false;
         return;
      }
   }

   // Start counting
   ioctl(GlobalMetrics.PageFaultFd, PERF_EVENT_IOC_RESET, 0);
   ioctl(GlobalMetrics.PageFaultFd, PERF_EVENT_IOC_ENABLE, 0);
}

#endif

/* NOTE(casey): This does not need to be "inline", it could just be "static"
   because compilers will inline it anyway. But compilers will warn about 
   static functions that aren't used. So "inline" is just the simplest way 
   to tell them to stop complaining about that. */
inline u64 ReadCPUTimer(void)
{
	// NOTE(casey): If you were on ARM, you would need to replace __rdtsc
	// with one of their performance counter read instructions, depending
	// on which ones are available on your platform.
	
	return __rdtsc();
}

static u64 EstimateCPUTimerFreq(void)
{
	u64 MillisecondsToWait = 100;
	u64 OSFreq = GetOSTimerFreq();

	u64 CPUStart = ReadCPUTimer();
	u64 OSStart = ReadOSTimer();
	u64 OSEnd = 0;
	u64 OSElapsed = 0;
	u64 OSWaitTime = OSFreq * MillisecondsToWait / 1000;
	while(OSElapsed < OSWaitTime)
	{
		OSEnd = ReadOSTimer();
		OSElapsed = OSEnd - OSStart;
	}
	
	u64 CPUEnd = ReadCPUTimer();
	u64 CPUElapsed = CPUEnd - CPUStart;
	
	u64 CPUFreq = 0;
	if(OSElapsed)
	{
		CPUFreq = OSFreq * CPUElapsed / OSElapsed;
	}
	
	return CPUFreq;
}
