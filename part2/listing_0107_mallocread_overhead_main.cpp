/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 107
   ======================================================================== */

/* NOTE(casey): _CRT_SECURE_NO_WARNINGS is here because otherwise we cannot
   call fopen(). If we replace fopen() with fopen_s() to avoid the warning,
   then the code doesn't compile on Linux anymore, since fopen_s() does not
   exist there.
   
   What exactly the CRT maintainers were thinking when they made this choice,
   I have no idea. */
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <climits>
#include <unistd.h>

typedef uint8_t u8;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int32_t b32;

typedef float f32;
typedef double f64;

#define ArrayCount(Array) (sizeof(Array)/sizeof((Array)[0]))

#include "listing_0068_buffer.cpp"
#include "listing_0074_platform_metrics.cpp"
#include "listing_0103_repetition_tester.cpp"
#include "listing_0106_mallocread_overhead_test.cpp"

struct test_function
{
   const char* Name;
   read_overhead_test_func* Func;
};

test_function TestFunctions[] =
{
   {"fread", ReadViaFRead},
   {"read", ReadViaRead},
};

int main(int ArgCount, char **Args)
{
   u64 CPUTimerFreq = EstimateCPUTimerFreq();

   if (ArgCount == 2)
   {
      char* FileName = Args[1];

      struct stat Stat;
      stat(FileName, &Stat);

      read_parameters Params = {};
      Params.Dest = AllocateBuffer(Stat.st_size);
      Params.FileName = FileName;

      if (Params.Dest.Count > 0)
      {
         for (;;)
         {
            repetition_tester Testers[ArrayCount(TestFunctions)][AllocType_Count] = {};

            for(u32 FuncIndex = 0; FuncIndex < ArrayCount(TestFunctions); ++FuncIndex)
            {
               for (u32 AllocType = 0; AllocType < AllocType_Count; ++AllocType)
               {
                  Params.AllocType = (allocation_type)AllocType;

                  repetition_tester* Tester = &Testers[FuncIndex][AllocType];
                  test_function TestFunc = TestFunctions[FuncIndex];

                  printf("\n--- %s%s%s ---\n",
                         DescribeAllocationType(Params.AllocType),
                         Params.AllocType ? " + " : "",
                         TestFunc.Name);
                  NewTestWave(Tester, Params.Dest.Count, CPUTimerFreq);
                  TestFunc.Func(Tester, &Params);
               }
            }
         }
      }
      else
      {
         fprintf(stderr, "ERROR: Test data size must be non-zero\n");
      }
   }

   return 0;
}
