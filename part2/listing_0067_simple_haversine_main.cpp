/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 67
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

struct haversine_pair
{
    f64 X0, Y0;
    f64 X1, Y1;
};

#include "listing_0065_haversine_formula.cpp"
#include "listing_0068_buffer.cpp"
#include "listing_0069_lookup_json_parser.cpp"
#include "listing_0070_platform_metrics.cpp"

static buffer ReadEntireFile(char *FileName)
{
    buffer Result = {};
        
    FILE *File = fopen(FileName, "rb");
    if(File)
    {
#if _WIN32
        struct __stat64 Stat;
        _stat64(FileName, &Stat);
#else
        struct stat Stat;
        stat(FileName, &Stat);
#endif
        
        Result = AllocateBuffer(Stat.st_size);
        if(Result.Data)
        {
            if(fread(Result.Data, Result.Count, 1, File) != 1)
            {
                fprintf(stderr, "ERROR: Unable to read \"%s\".\n", FileName);
                FreeBuffer(&Result);
            }
        }
    }
    else
    {
        fprintf(stderr, "ERROR: Unable to open \"%s\".\n", FileName);
    }
    
    return Result;
}

static f64 SumHaversineDistances(u64 PairCount, haversine_pair *Pairs)
{
    f64 Sum = 0;
    
    f64 SumCoef = 1 / (f64)PairCount;
    for(u64 PairIndex = 0; PairIndex < PairCount; ++PairIndex)
    {
        haversine_pair Pair = Pairs[PairIndex];
        f64 EarthRadius = 6372.8;
        f64 Dist = ReferenceHaversine(Pair.X0, Pair.Y0, Pair.X1, Pair.Y1, EarthRadius);
        Sum += SumCoef*Dist;
    }
    
    return Sum;
}

int main(int ArgCount, char **Args)
{
    int Result = 1;
    u64 Start = ReadCPUTimer();
    u64 StartupTimer = 0;
    u64 ReadTimer = 0;
    u64 MiscSetupTimer = 0;
    u64 ParseTimer = 0;
    u64 SumTimer = 0;
    u64 MiscOutputTimer = 0;
    u64 End = 0;
    
    if((ArgCount == 2) || (ArgCount == 3))
    {
        StartupTimer = ReadCPUTimer();
        buffer InputJSON = ReadEntireFile(Args[1]);
        ReadTimer = ReadCPUTimer();
        
        u32 MinimumJSONPairEncoding = 6*4;
        u64 MaxPairCount = InputJSON.Count / MinimumJSONPairEncoding;
        if(MaxPairCount)
        {
            buffer ParsedValues = AllocateBuffer(MaxPairCount * sizeof(haversine_pair));
            if(ParsedValues.Count)
            {
                haversine_pair *Pairs = (haversine_pair *)ParsedValues.Data;
                MiscSetupTimer = ReadCPUTimer();
                u64 PairCount = ParseHaversinePairs(InputJSON, MaxPairCount, Pairs);
                ParseTimer = ReadCPUTimer();
                f64 Sum = SumHaversineDistances(PairCount, Pairs);
                SumTimer = ReadCPUTimer();
                
                fprintf(stdout, "Input size: %llu\n", InputJSON.Count);
                fprintf(stdout, "Pair count: %llu\n", PairCount);
                fprintf(stdout, "Haversine sum: %.16f\n", Sum);
                MiscOutputTimer = ReadCPUTimer();
                
                if(ArgCount == 3)
                {
                    buffer AnswersF64 = ReadEntireFile(Args[2]);
                    if(AnswersF64.Count >= sizeof(f64))
                    {
                        f64 *AnswerValues = (f64 *)AnswersF64.Data;
                        
                        fprintf(stdout, "\nValidation:\n");
                        
                        u64 RefAnswerCount = (AnswersF64.Count - sizeof(f64)) / sizeof(f64);
                        if(PairCount != RefAnswerCount)
                        {
                            fprintf(stdout, "FAILED - pair count doesn't match %llu.\n", RefAnswerCount);
                        }
                        
                        f64 RefSum = AnswerValues[RefAnswerCount];
                        fprintf(stdout, "Reference sum: %.16f\n", RefSum);
                        fprintf(stdout, "Difference: %.16f\n", Sum - RefSum);
                        
                        fprintf(stdout, "\n");
                    }
                }
            }
            
            FreeBuffer(&ParsedValues);
        }
        else
        {
            fprintf(stderr, "ERROR: Malformed input JSON\n");
        }

        FreeBuffer(&InputJSON);
        
        Result = 0;
    }
    else
    {
        fprintf(stderr, "Usage: %s [haversine_input.json]\n", Args[0]);
        fprintf(stderr, "       %s [haversine_input.json] [answers.f64]\n", Args[0]);
    }

    u64 CPUFreq = EstimateCPUFreq();
    End = ReadCPUTimer();
    fprintf(stdout, ">>> %llu %llu\n", MiscSetupTimer, ParseTimer);
    fprintf(stdout, "\nTotal time: %.4f ms (CPU freq %llu)\n", (f64)(End-Start)*1000.0/(f64)CPUFreq, CPUFreq);
    fprintf(stdout, "  Startup: %llu (%.2f%%)\n", StartupTimer-Start, (f64)(StartupTimer-Start)*100.0/(f64)(End-Start));
    fprintf(stdout, "  Read: %llu (%.2f%%)\n", ReadTimer-StartupTimer, (f64)(ReadTimer-StartupTimer)*100.0/(f64)(End-Start));
    fprintf(stdout, "  MiscSetup: %llu (%.2f%%)\n", MiscSetupTimer-ReadTimer, (f64)(MiscSetupTimer-ReadTimer)*100.0/(f64)(End-Start));
    fprintf(stdout, "  Parse: %llu (%.2f%%)\n", ParseTimer-MiscSetupTimer, (f64)(ParseTimer-MiscSetupTimer)*100.0/(f64)(End-Start));
    fprintf(stdout, "  Sum: %llu (%.2f%%)\n", SumTimer-ParseTimer, (f64)(SumTimer-ParseTimer)*100.0/(f64)(End-Start));
    fprintf(stdout, "  MiscOutput: %llu (%.2f%%)\n", MiscOutputTimer-SumTimer, (f64)(MiscOutputTimer-SumTimer)*100.0/(f64)(End-Start));

    return Result;
}
