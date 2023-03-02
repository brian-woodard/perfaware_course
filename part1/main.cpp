#include <iostream>
#include <fstream>

#define INTEL_MOV_OPCODE 0x22

struct T_OpcodeByte
{
   uint8_t Word        : 1; // if set, use 16-bit data
   uint8_t Destination : 1; // if set, destination is set in REG field
   uint8_t Opcode      : 6;
};

struct T_RegisterByte
{
   uint8_t Rm   : 3;
   uint8_t Reg  : 3;
   uint8_t Mode : 2;
};

const char* RegisterTable[2][8] = 
{
   {
      "al",
      "cl",
      "dl",
      "bl",
      "ah",
      "ch",
      "dh",
      "bh"
   },
   {
      "ax",
      "cx",
      "dx",
      "bx",
      "sp",
      "bp",
      "si",
      "di"
   }
};

void intel_decode(char* Buffer, uint32_t BufferSize, std::ofstream& out)
{
   for (uint32_t i = 0; i < BufferSize; i++)
   {
      T_OpcodeByte opcode;
      
      *(uint8_t*)&opcode = Buffer[i++];

      switch (opcode.Opcode)
      {
         case INTEL_MOV_OPCODE:
         {
            T_RegisterByte reg;

            *(uint8_t*)&reg = Buffer[i];

            if (reg.Mode != 3)
            {
               std::cout << "Error: Only know how to decode register-to-register MOV" << std::endl;
               continue;
            }

            if (opcode.Destination)
               out << "mov " << RegisterTable[opcode.Word][reg.Reg] << ", " << RegisterTable[opcode.Word][reg.Rm] << std::endl;
            else
               out << "mov " << RegisterTable[opcode.Word][reg.Rm] << ", " << RegisterTable[opcode.Word][reg.Reg] << std::endl;

            break;
         }
         default:
            std::cout << "Error: Uknown opcode " << std::hex << (uint16_t)opcode.Opcode << " at " << i << std::endl;
      }
   }
}

int main(int argc, char* argv[])
{
   char*         buffer = nullptr;
   uint32_t      buffer_size;
   std::ifstream input_file;
   std::ofstream output_file("output.asm");

   if (argc != 2)
   {
      std::cout << "Usage: main <assembly-file>" << std::endl;
      return 0;
   }

   input_file.open(argv[1], std::ios::binary);

   if (!input_file.is_open())
   {
      std::cout << "Error opening file " << argv[1] << std::endl;
      return 0;
   }

   // get length of file
   input_file.seekg (0, input_file.end);
   buffer_size = input_file.tellg();
   input_file.seekg (0, input_file.beg);

    // allocate memory
   buffer = new char[buffer_size];

   input_file.read(buffer, buffer_size);

   output_file << "; output.asm" << std::endl << std::endl;
   output_file << "bits 16" << std::endl << std::endl;

   std::cout << "Disassembler running over " << buffer_size << " bytes" << std::endl;
   intel_decode(buffer, buffer_size, output_file);

   input_file.close();
   output_file.close();

   return 1;
}