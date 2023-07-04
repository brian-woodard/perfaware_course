
#include "listing_0074_platform_metrics.cpp"

struct ProfileData
{
   ProfileData() {}
   ProfileData(const char* Name, u64 Index) : mName(Name), mIndex(Index), mStart(ReadCPUTimer()), mEnd(0) {}
   ~ProfileData();

   const char* mName;
   u64 mIndex;
   u64 mStart;
   u64 mEnd;
};

struct profiler
{
   profiler() : TotalAnchors(0) {}

   ProfileData Total;
   ProfileData Anchors[10];
   u64         TotalAnchors;
};

profiler my_profiler;

ProfileData::~ProfileData()
{
   mEnd = ReadCPUTimer();
   my_profiler.Anchors[mIndex] = *this;
   my_profiler.TotalAnchors++;
}

void BeginProfile()
{
   my_profiler.Total.mStart = ReadCPUTimer();
}

void EndAndPrintProfile()
{
   my_profiler.Total.mEnd = ReadCPUTimer();
   u64 CPUFreq = EstimateCPUTimerFreq();
   fprintf(stdout, "\nTotal time: %.4f ms (CPU freq %llu)\n", (f64)(my_profiler.Total.mEnd-my_profiler.Total.mStart)*1000.0/(f64)CPUFreq, CPUFreq);

   for (u64 i = 0; i < my_profiler.TotalAnchors; i++)
   {
      ProfileData* prof = &my_profiler.Anchors[i];
      u64 Elapsed = prof->mEnd - prof->mStart;
      u64 TotalTSCElapsed = my_profiler.Total.mEnd - my_profiler.Total.mStart;
      f64 Percent = 100.0 * ((f64)Elapsed / (f64)TotalTSCElapsed);
      printf("  %s: %llu (%.2f%%)\n", prof->mName, Elapsed, Percent);
   }
}

#define TimeFunction ProfileData profile(__FUNCTION__, __COUNTER__)
#define TimeBlock(name) ProfileData profile(name, __COUNTER__)
