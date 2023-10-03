/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 102
   ======================================================================== */


struct read_parameters
{
   buffer Dest;
   char const* FileName;
};

typedef void read_overhead_test_func(repetition_tester* Tester, read_parameters* Params);

static void ReadViaFRead(repetition_tester* Tester, read_parameters* Params)
{
   while (IsTesting(Tester))
   {
      FILE* File = fopen(Params->FileName, "rb");
      if (File)
      {
         buffer DestBuffer = Params->Dest;

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

         close(File);
      }
      else
      {
         Error(Tester, "open failed");
      }
   }
}
