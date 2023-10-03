/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 106
   ======================================================================== */

enum allocation_type
{
   AllocType_none,
   AllocType_malloc,

   AllocType_Count,
};

struct read_parameters
{
   allocation_type AllocType;
   buffer Dest;
   char const* FileName;
};

typedef void read_overhead_test_func(repetition_tester* Tester, read_parameters* Params);

static const char* DescribeAllocationType(allocation_type AllocType)
{
   const char* Result;
   switch (AllocType)
   {
      case AllocType_none: { Result = ""; } break;
      case AllocType_malloc: { Result = "malloc"; } break;
      default: { Result = "UNKNOWN"; } break;
   }

   return Result;
}

static void HandleAllocation(read_parameters* Params, buffer* Buffer)
{
   switch (Params->AllocType)
   {
      case AllocType_none:
         break;

      case AllocType_malloc:
         *Buffer = AllocateBuffer(Params->Dest.Count);
         break;

      default:
         fprintf(stderr, "ERROR: Unrecognized allocation type");
   }
}

static void HandleDeallocation(read_parameters* Params, buffer* Buffer)
{
   switch (Params->AllocType)
   {
      case AllocType_none:
         break;

      case AllocType_malloc:
         FreeBuffer(Buffer);
         break;

      default:
         fprintf(stderr, "ERROR: Unrecognized allocation type");
   }
}

static void ReadViaFRead(repetition_tester* Tester, read_parameters* Params)
{
   while (IsTesting(Tester))
   {
      FILE* File = fopen(Params->FileName, "rb");
      if (File)
      {
         buffer DestBuffer = Params->Dest;
         HandleAllocation(Params, &DestBuffer);

         BeginTime(Tester);
         size_t Result = fread(DestBuffer.Data, DestBuffer.Count, 1, File);
         EndTime(Tester);

         if (Result == 1)
         {
            CountBytes(Tester, DestBuffer.Count);
         }
         else
         {
            Error(Tester, "fread failed");
         }

         HandleDeallocation(Params, &DestBuffer);
         fclose(File);
      }
      else
      {
         Error(Tester, "fopen failed");
      }
   }
}

static void ReadViaRead(repetition_tester* Tester, read_parameters* Params)
{
   while (IsTesting(Tester))
   {
      int File = open(Params->FileName, O_RDONLY);

      if (File != -1)
      {
         buffer DestBuffer = Params->Dest;
         HandleAllocation(Params, &DestBuffer);

         u8* Dest = DestBuffer.Data;
         u64 SizeRemaining = DestBuffer.Count;
         while (SizeRemaining)
         {
            u32 ReadSize = INT_MAX;

            if ((u64)ReadSize > SizeRemaining)
            {
               ReadSize = (u32) SizeRemaining;
            }

            BeginTime(Tester);
            int Result = read(File, Dest, ReadSize);
            EndTime(Tester);

            if (Result == (int)ReadSize)
            {
               CountBytes(Tester, ReadSize);
            }
            else
            {
               Error(Tester, "read failed");
               break;
            }

            SizeRemaining -= ReadSize;
            Dest += ReadSize;
         }

         HandleDeallocation(Params, &DestBuffer);
         close(File);
      }
      else
      {
         Error(Tester, "open failed");
      }
   }
}
