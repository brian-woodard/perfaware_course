/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 158
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

typedef uint8_t u8;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int32_t b32;

typedef float f32;
typedef double f64;

#define ArrayCount(Array) (sizeof(Array)/sizeof((Array)[0]))

#include "listing_0125_buffer.cpp"
#include "listing_0137_os_platform.cpp"
#include "listing_0109_pagefault_repetition_tester.cpp"

typedef void ASMFunction(u64 Count, u8 *ReadData, u8 *WriteData, u64 WriteCount);

extern "C" void Test_Cache(u64 Count, u8 *ReadData, u8 *WriteData, u64 WriteCount);
#pragma comment (lib, "listing_0159_non_temporal_store")

struct test_function
{
    char const *Name;
    ASMFunction *Func;
};
test_function TestFunctions[] =
{
    {"Test_Cache", Test_Cache},
};

int main(void)
{
    InitializeOSPlatform();
    
    u64 WriteCount = 128;
    buffer ReadBuffer = AllocateBuffer(1*1024*1024);
    buffer WriteBuffer = AllocateBuffer(1*1024*1024);
    if(IsValid(ReadBuffer) && IsValid(WriteBuffer))
    {
        // NOTE(casey): Because OSes may not map allocated pages until they are written to, we write garbage
        // to the entire buffer to force it to be mapped.
        // for(u64 ByteIndex = 0; ByteIndex < ReadBuffer.Count; ++ByteIndex)
        // {
        //     ReadBuffer.Data[ByteIndex] = (u8)ByteIndex;
        // }
        
        repetition_tester Testers[ArrayCount(TestFunctions)] = {};
        for(;;)
        {
            for(u32 FuncIndex = 0; FuncIndex < ArrayCount(TestFunctions); ++FuncIndex)
            {
                repetition_tester *Tester = &Testers[FuncIndex];
                test_function TestFunc = TestFunctions[FuncIndex];
                
                printf("\n--- %s ---\n", TestFunc.Name);
                NewTestWave(Tester, ReadBuffer.Count, GetCPUTimerFreq());
                
                while(IsTesting(Tester))
                {
                    BeginTime(Tester);
                    TestFunc.Func(ReadBuffer.Count, ReadBuffer.Data, WriteBuffer.Data, WriteCount);
                    EndTime(Tester);
                    CountBytes(Tester, ReadBuffer.Count);
                }
            }
        }
    }
    else
    {
        fprintf(stderr, "Unable to allocate memory buffer for testing");
    }
    
    FreeBuffer(&ReadBuffer);
    FreeBuffer(&WriteBuffer);
    
    return 0;
}
