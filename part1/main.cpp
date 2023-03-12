#include <iostream>
#include <fstream>
#include <string>

#define INTEL_REG_MOV_OPCODE           0x88
#define INTEL_MOV_IMM_TO_REG_OPCODE    0xB0
#define INTEL_MOV_IMM_TO_REGMEM_OPCODE 0xC4
#define INTEL_MOV_MEM_TO_ACCUM_OPCODE  0xA0
#define INTEL_MOV_ACCUM_TO_MEM_OPCODE  0xA2
#define INTEL_REG_ADD_OPCODE           0x00
#define INTEL_IMM_TO_REGMEM_OPCODE     0x80
#define INTEL_ADD_IMM_TO_ACCUM_OPCODE  0x04
#define INTEL_REG_SUB_OPCODE           0x28
#define INTEL_SUB_IMM_TO_ACCUM_OPCODE  0x2C
#define INTEL_REG_CMP_OPCODE           0x38
#define INTEL_CMP_IMM_TO_ACCUM_OPCODE  0x3C

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

uint32_t decode_register_to_register(const char* Command, char* Buffer, uint32_t Index, std::ofstream& out)
{
   T_RegisterByte          reg;
   T_RegisterMovOpcodeByte opcode;
   uint16_t                displacement = 0;
   uint32_t                i = Index;

   *(uint8_t*)&opcode = Buffer[i++];
   *(uint8_t*)&reg = Buffer[i++];

   if (reg.Mode == 0)
   {
      uint16_t direct_address = 0;

      if (reg.Rm == 6)
      {
         // read two bytes for direct address
         direct_address = *(uint16_t*)&Buffer[i];
         i += 2;
      }

      if (direct_address)
         out << Command << " " << RegisterTable[opcode.Word][reg.Reg] << ", " << "[" << direct_address << "]" << std::endl;
      else if (opcode.Destination)
         out << Command << " " << RegisterTable[opcode.Word][reg.Reg] << ", " << "[" << EffectiveAddressTable[reg.Rm] << "]" << std::endl;
      else
         out << Command << " " << "[" << EffectiveAddressTable[reg.Rm] << "], " << RegisterTable[opcode.Word][reg.Reg] << std::endl;
   }
   else if (reg.Mode == 1)
   {
      std::string effective_address;
      int16_t     signed_displacement;

      // read one byte for displacement
      displacement = Buffer[i++];
      signed_displacement = displacement;

      effective_address = "[";
      effective_address += EffectiveAddressTable[reg.Rm];

      if (signed_displacement < 0)
         effective_address += " - " + std::to_string(abs(signed_displacement)) + "]";
      else if (displacement)
         effective_address += " + " + std::to_string(displacement) + "]";
      else
         effective_address += "]";

      if (opcode.Destination)
         out << Command << " " << RegisterTable[opcode.Word][reg.Reg] << ", " << effective_address << std::endl;
      else
         out << Command << " " << effective_address << ", " << RegisterTable[opcode.Word][reg.Reg] << std::endl;
   }
   else if (reg.Mode == 2)
   {
      std::string effective_address;
      int16_t     signed_displacement;

      // read two bytes for displacement
      displacement = *(uint16_t*)&Buffer[i];
      signed_displacement = displacement;
      i += 2;

      effective_address = "[";
      effective_address += EffectiveAddressTable[reg.Rm];

      if (signed_displacement < 0)
         effective_address += " - " + std::to_string(abs(signed_displacement)) + "]";
      else if (displacement)
         effective_address += " + " + std::to_string(displacement) + "]";
      else
         effective_address += "]";

      if (opcode.Destination)
         out << Command << " " << RegisterTable[opcode.Word][reg.Reg] << ", " << effective_address << std::endl;
      else
         out << Command << " " << effective_address << ", " << RegisterTable[opcode.Word][reg.Reg] << std::endl;
   }
   else if (reg.Mode == 3)
   {
      if (opcode.Destination)
         out << Command << " " << RegisterTable[opcode.Word][reg.Reg] << ", " << RegisterTable[opcode.Word][reg.Rm] << std::endl;
      else
         out << Command << " " << RegisterTable[opcode.Word][reg.Rm] << ", " << RegisterTable[opcode.Word][reg.Reg] << std::endl;
   }

   return i;
}

uint32_t decode_immediate_to_register(const char* Command, char* Buffer, uint32_t Index, std::ofstream& out)
{
   uint32_t       i = Index;
   T_RegisterByte reg;
   std::string    explicit_size;
   std::string    effective_address;
   uint16_t       displacement = 0;
   int16_t        signed_displacement = 0;
   uint16_t       data = 0;
   int16_t        signed_data = 0;
   uint8_t        sign = ((Buffer[i] >> 1) & 0x1);
   uint8_t        word = (Buffer[i] & 0x1);

   i++;
   *(uint8_t*)&reg = Buffer[i++];

   // read one or two bytes for displacement
   if (reg.Mode == 1)
   {
      displacement = Buffer[i++];
      signed_displacement = displacement;
   }
   else if (reg.Mode == 2 ||
            (reg.Mode == 0 && reg.Rm == 6))
   {
      displacement = *(uint16_t*)&Buffer[i];
      signed_displacement = displacement;
      i += 2;
   }

   // read one or two bytes for data
   if (word && sign)
   {
      // read one byte and sign-extend to two bytes
      data = Buffer[i++];
      signed_data = data;
      explicit_size = "word";
   }
   else if (word)
   {
      data = *(uint16_t*)&Buffer[i];
      signed_data = data;
      i += 2;
      explicit_size = "word";
   }
   else
   {
      data = Buffer[i++];
      signed_data = data;
      explicit_size = "byte";
   }

   if (reg.Mode == 3)
   {
      out << Command << " " << RegisterTable[word][reg.Rm] << ", " << signed_data << std::endl;
   }
   else
   {
      if (reg.Mode == 0 && reg.Rm == 6) // direct address
      {
         effective_address = "[" + std::to_string(displacement) + "]";
      }
      else
      {
         effective_address = "[";
         effective_address += EffectiveAddressTable[reg.Rm];

         if (signed_displacement < 0)
            effective_address += " - " + std::to_string(abs(signed_displacement)) + "]";
         else if (displacement)
            effective_address += " + " + std::to_string(displacement) + "]";
         else
            effective_address += "]";
      }

      out << Command << " " << effective_address << ", " << explicit_size << " " << data << std::endl;
   }

   return i;
}

void intel_decode(char* Buffer, uint32_t BufferSize, std::ofstream& out)
{
   for (uint32_t i = 0; i < BufferSize;)
   {
      if ((Buffer[i] & 0xfc) == INTEL_REG_MOV_OPCODE)
      {
         i = decode_register_to_register("mov", Buffer, i, out);
      }
      else if ((Buffer[i] & 0xfc) == INTEL_MOV_IMM_TO_REGMEM_OPCODE)
      {
         i = decode_immediate_to_register("mov", Buffer, i, out);
      }
      else if ((Buffer[i] & 0xfc) == INTEL_REG_ADD_OPCODE)
      {
         i = decode_register_to_register("add", Buffer, i, out);
      }
      else if ((Buffer[i] & 0xfc) == INTEL_IMM_TO_REGMEM_OPCODE)
      {
         uint8_t reg = ((Buffer[i+1] >> 3) & 0x7);

         if (reg == 0) // add
            i = decode_immediate_to_register("add", Buffer, i, out);
         else if (reg == 5) // sub
            i = decode_immediate_to_register("sub", Buffer, i, out);
         else if (reg == 7) // cmp
            i = decode_immediate_to_register("cmp", Buffer, i, out);
         else
            return;
      }
      else if ((Buffer[i] & 0xfc) == INTEL_REG_SUB_OPCODE)
      {
         i = decode_register_to_register("sub", Buffer, i, out);
      }
      else if ((Buffer[i] & 0xfc) == INTEL_REG_CMP_OPCODE)
      {
         i = decode_register_to_register("cmp", Buffer, i, out);
      }
      else if ((Buffer[i] & 0xf0) == INTEL_MOV_IMM_TO_REG_OPCODE)
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
      else if ((Buffer[i] & 0xfe) == INTEL_MOV_MEM_TO_ACCUM_OPCODE ||
               (Buffer[i] & 0xfe) == INTEL_MOV_ACCUM_TO_MEM_OPCODE ||
               (Buffer[i] & 0xfe) == INTEL_ADD_IMM_TO_ACCUM_OPCODE ||
               (Buffer[i] & 0xfe) == INTEL_SUB_IMM_TO_ACCUM_OPCODE ||
               (Buffer[i] & 0xfe) == INTEL_CMP_IMM_TO_ACCUM_OPCODE)
      {
         uint16_t direct_address = 0;
         uint8_t  word = (Buffer[i] & 0x1);
         uint8_t  mem_to_accum = (Buffer[i] & 0xfe) == INTEL_MOV_MEM_TO_ACCUM_OPCODE;
         uint8_t  opcode = (Buffer[i] & 0xfe);

         i++;

         // read one or two bytes for data
         if (word)
         {
            direct_address = *(uint16_t*)&Buffer[i];
            i += 2;
         }
         else
            direct_address = Buffer[i++];

         if (opcode == INTEL_ADD_IMM_TO_ACCUM_OPCODE)
         {
            if (word)
               out << "add ax, " << (int16_t)direct_address << std::endl;
            else
               out << "add al, " << (int16_t)direct_address << std::endl;
         }
         else if (opcode == INTEL_SUB_IMM_TO_ACCUM_OPCODE)
         {
            if (word)
               out << "sub ax, " << (int16_t)direct_address << std::endl;
            else
               out << "sub al, " << (int16_t)direct_address << std::endl;
         }
         else if (opcode == INTEL_CMP_IMM_TO_ACCUM_OPCODE)
         {
            if (word)
               out << "cmp ax, " << (int16_t)direct_address << std::endl;
            else
               out << "cmp al, " << (int16_t)direct_address << std::endl;
         }
         else
         {
            if (mem_to_accum)
               out << "mov ax, [" << direct_address << "]" << std::endl;
            else
               out << "mov [" << direct_address << "], ax" << std::endl;
         }
      }
      else
      {
         std::cout << "Error: Uknown opcode " << std::hex << (uint16_t)Buffer[i] << " at " << i << std::endl;
         return;
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