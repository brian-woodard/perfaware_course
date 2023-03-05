#include <iostream>
#include <fstream>
#include <string>

#define INTEL_REG_MOV_OPCODE    0x88
#define INTEL_IMM_TO_REG_OPCODE 0xB0

struct T_RegisterMovOpcodeByte
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

struct T_ImmediateToRegisterOpcodeByte
{
   uint8_t Reg    : 3;
   uint8_t Word   : 1; // if set, use 16-bit data
   uint8_t Opcode : 4;
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

const char* EffectiveAddressTable[] =
{
   "bx + si",
   "bx + di",
   "bp + si",
   "bp + di",
   "si",
   "di",
   "bp",
   "bx"
};

void intel_decode(char* Buffer, uint32_t BufferSize, std::ofstream& out)
{
   for (uint32_t i = 0; i < BufferSize;)
   {
      if ((Buffer[i] & 0xfc) == INTEL_REG_MOV_OPCODE)
      {
         T_RegisterByte          reg;
         T_RegisterMovOpcodeByte opcode;
         uint16_t                displacement = 0;

         *(uint8_t*)&opcode = Buffer[i++];
         *(uint8_t*)&reg = Buffer[i++];

         if (reg.Mode == 0)
         {
            if (opcode.Destination)
               out << "mov " << RegisterTable[opcode.Word][reg.Reg] << ", " << "[" << EffectiveAddressTable[reg.Rm] << "]" << std::endl;
            else
               out << "mov " << "[" << EffectiveAddressTable[reg.Rm] << "], " << RegisterTable[opcode.Word][reg.Reg] << std::endl;
         }
         else if (reg.Mode == 1)
         {
            std::string effective_address;

            // read one byte for displacement
            displacement = Buffer[i++];

            effective_address = "[";
            effective_address += EffectiveAddressTable[reg.Rm];

            if (displacement)
               effective_address += " + " + std::to_string(displacement) + "]";
            else
               effective_address += "]";

            if (opcode.Destination)
               out << "mov " << RegisterTable[opcode.Word][reg.Reg] << ", " << effective_address << std::endl;
            else
               out << "mov " << effective_address << ", " << RegisterTable[opcode.Word][reg.Reg] << std::endl;
         }
         else if (reg.Mode == 2)
         {
            std::string effective_address;

            // read two bytes for displacement
            displacement = *(uint16_t*)&Buffer[i];
            i += 2;

            effective_address = "[";
            effective_address += EffectiveAddressTable[reg.Rm];

            if (displacement)
               effective_address += " + " + std::to_string(displacement) + "]";
            else
               effective_address += "]";

            if (opcode.Destination)
               out << "mov " << RegisterTable[opcode.Word][reg.Reg] << ", " << effective_address << std::endl;
            else
               out << "mov " << effective_address << ", " << RegisterTable[opcode.Word][reg.Reg] << std::endl;
         }
         else if (reg.Mode == 3)
         {
            if (opcode.Destination)
               out << "mov " << RegisterTable[opcode.Word][reg.Reg] << ", " << RegisterTable[opcode.Word][reg.Rm] << std::endl;
            else
               out << "mov " << RegisterTable[opcode.Word][reg.Rm] << ", " << RegisterTable[opcode.Word][reg.Reg] << std::endl;
         }
      }
      else if ((Buffer[i] & 0xf0) == INTEL_IMM_TO_REG_OPCODE)
      {
         T_ImmediateToRegisterOpcodeByte opcode;
         uint16_t                        immediate_value = 0;
         
         *(uint8_t*)&opcode = Buffer[i++];

         if (opcode.Word)
         {
            immediate_value = *(uint16_t*)&Buffer[i];
            i += 2;
         }
         else
            immediate_value = Buffer[i++];

         out << "mov " << RegisterTable[opcode.Word][opcode.Reg] << ", " << immediate_value << std::endl;
      }
      else
      {
         std::cout << "Error: Uknown opcode " << std::hex << (uint16_t)Buffer[i] << " at " << i << std::endl;
         i++;
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