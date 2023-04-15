#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <string.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t   s8;
typedef int16_t  s16;
typedef int32_t  s32;
typedef int64_t  s64;
typedef float    f32;
typedef double   f64;

///////////////////////////////////////////////////////////////////////////////
// Types and constants for decoding
///////////////////////////////////////////////////////////////////////////////

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
#define INTEL_JO_OPCODE                0x70
#define INTEL_JNO_OPCODE               0x71
#define INTEL_JB_OPCODE                0x72
#define INTEL_JNB_OPCODE               0x73
#define INTEL_JE_OPCODE                0x74
#define INTEL_JNZ_OPCODE               0x75
#define INTEL_JBE_OPCODE               0x76
#define INTEL_JA_OPCODE                0x77
#define INTEL_JS_OPCODE                0x78
#define INTEL_JNS_OPCODE               0x79
#define INTEL_JP_OPCODE                0x7A
#define INTEL_JNP_OPCODE               0x7B
#define INTEL_JL_OPCODE                0x7C
#define INTEL_JNL_OPCODE               0x7D
#define INTEL_JLE_OPCODE               0x7E
#define INTEL_JG_OPCODE                0x7F
#define INTEL_LOOPNZ_OPCODE            0xE0
#define INTEL_LOOPZ_OPCODE             0xE1
#define INTEL_LOOP_OPCODE              0xE2
#define INTEL_JCXZ_OPCODE              0xE3

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

enum EffectiveAddressEnum
{
   EA_Regs_bx_si,
   EA_Regs_bx_di,
   EA_Regs_bp_si,
   EA_Regs_bp_di,
   EA_Reg_si,
   EA_Reg_di,
   EA_Reg_bp,
   EA_Reg_bx
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

///////////////////////////////////////////////////////////////////////////////
// Types, constants and functions for simulating
///////////////////////////////////////////////////////////////////////////////

enum RegisterEnum
{
   Reg_ax,
   Reg_cx,
   Reg_dx,
   Reg_bx,
   Reg_sp,
   Reg_bp,
   Reg_si,
   Reg_di,
   Num_Registers
};

enum FlagsEnum
{
   ZeroFlag = 0x0001,
   SignFlag = 0x0002,
};

const u32 MEMORY_SIZE = 1024 * 1024;

u8 Memory[MEMORY_SIZE] = {};
u16 Registers[Num_Registers] = {};
u16 IpRegister = 0;
u16 IpRegisterPv = 0;
u16 Flags = 0;

void sim86_check_flags(u16 NewFlags, std::ofstream& Out)
{
   if (NewFlags != Flags)
   {
      Out << " flags:";

      if (Flags & ZeroFlag)
         Out << "Z";

      if (Flags & SignFlag)
         Out << "S";

      Out << "->";

      if (NewFlags & ZeroFlag)
         Out << "Z";

      if (NewFlags & SignFlag)
         Out << "S";

      Flags = NewFlags;
   }
}

void sim86_check_ip(std::ofstream& Out)
{
   if (IpRegister != IpRegisterPv)
   {
      Out << " ip"
         << std::hex << ":0x" << IpRegisterPv
         << "->0x" << IpRegister
         << std::dec;

      IpRegisterPv = IpRegister;
   }
}

void sim86_jnz(s16 Jump, std::ofstream& Out)
{
   if (!(Flags & ZeroFlag))
   {
      IpRegister += Jump;
   }

   Out << " ;";

   sim86_check_ip(Out);
}

void sim86_move_word(u16 Dst, u16 Value, std::ofstream& Out)
{
   Out << "; ";

   if (Registers[Dst] != Value)
   {
      Out << RegisterTable[1][Dst]
          << std::hex << ":0x" << Registers[Dst]
          << "->0x" << Value << std::dec;

      Registers[Dst] = Value;
   }

   sim86_check_ip(Out);
}

void sim86_move_reg_word(u16 Dst, u16 Src, std::ofstream& Out)
{
   Out << "; ";

   if (Registers[Dst] != Registers[Src])
   {
      Out << RegisterTable[1][Dst]
          << std::hex << ":0x" << Registers[Dst]
          << "->0x" << Registers[Src] << std::dec;

      Registers[Dst] = Registers[Src];
   }

   sim86_check_ip(Out);
}

void sim86_add_word(u16 Dst, u16 Value, std::ofstream& Out)
{
   u16 result = Registers[Dst] + Value;
   u16 flags = 0;

   flags |= ((result & 0x8000) >> (16 - SignFlag)) & SignFlag;
   flags |= (result == 0) ? ZeroFlag : 0;

   Out << "; " << RegisterTable[1][Dst]
       << std::hex << ":0x" << Registers[Dst]
       << "->0x" << result << std::dec;

   sim86_check_ip(Out);

   sim86_check_flags(flags, Out);

   Registers[Dst] = result;
}

void sim86_add_reg_word(u16 Dst, u16 Src, std::ofstream& Out)
{
   u16 result = Registers[Dst] + Registers[Src];
   u16 flags = 0;

   flags |= ((result & 0x8000) >> (16 - SignFlag)) & SignFlag;
   flags |= (result == 0) ? ZeroFlag : 0;

   Out << "; " << RegisterTable[1][Dst]
       << std::hex << ":0x" << Registers[Dst]
       << "->0x" << result << std::dec;

   sim86_check_ip(Out);

   sim86_check_flags(flags, Out);

   Registers[Dst] = result;
}

void sim86_sub_word(u16 Dst, u16 Value, std::ofstream& Out)
{
   u16 result = Registers[Dst] - Value;
   u16 flags = 0;

   flags |= ((result & 0x8000) >> (16 - SignFlag)) & SignFlag;
   flags |= (result == 0) ? ZeroFlag : 0;

   Out << "; " << RegisterTable[1][Dst]
       << std::hex << ":0x" << Registers[Dst]
       << "->0x" << result << std::dec;

   sim86_check_ip(Out);

   sim86_check_flags(flags, Out);

   Registers[Dst] = result;
}

void sim86_sub_reg_word(u16 Dst, u16 Src, std::ofstream& Out)
{
   u16 result = Registers[Dst] - Registers[Src];
   u16 flags = 0;

   flags |= ((result & 0x8000) >> (16 - SignFlag)) & SignFlag;
   flags |= (result == 0) ? ZeroFlag : 0;

   Out << "; " << RegisterTable[1][Dst]
       << std::hex << ":0x" << Registers[Dst]
       << "->0x" << result << std::dec;

   sim86_check_ip(Out);

   sim86_check_flags(flags, Out);

   Registers[Dst] = result;
}

void sim86_cmp_word(u16 Dst, u16 Value, std::ofstream& Out)
{
   u16 result = Registers[Dst] - Value;
   u16 flags = 0;

   flags |= ((result & 0x8000) >> (16 - SignFlag)) & SignFlag;
   flags |= (result == 0) ? ZeroFlag : 0;

   Out << "; ";

   sim86_check_ip(Out);

   sim86_check_flags(flags, Out);
}

void sim86_cmp_reg_word(u16 Dst, u16 Src, std::ofstream& Out)
{
   u16 result = Registers[Dst] - Registers[Src];
   u16 flags = 0;

   flags |= ((result & 0x8000) >> (16 - SignFlag)) & SignFlag;
   flags |= (result == 0) ? ZeroFlag : 0;

   Out << "; ";

   sim86_check_flags(flags, Out);
}

void sim86_store_word(u16 Address, u16 Data, std::ofstream& Out)
{
   Memory[Address] = Data;

   Out << " ;";

   sim86_check_ip(Out);
}

void sim86_store_word(u16 Dst, u16 Address, u16 Data, std::ofstream& Out)
{
   u16 address = Registers[Dst] + Address;
   Memory[address] = Data;

   Out << " ;";

   sim86_check_ip(Out);
}

void sim86_store_word_from_reg(u16 Dst, u16 Address, u16 Src, std::ofstream& Out)
{
   u16 address = Registers[Dst] + Address;
   Memory[address] = Registers[Src];

   Out << " ;";

   sim86_check_ip(Out);
}

void sim86_store_word_ea(u16 Dst, u16 Src, std::ofstream& Out)
{
   u16 address = 0;

   if (Dst == EA_Regs_bp_si)
      address = Registers[Reg_bp] + Registers[Reg_si];

   Memory[address] = Registers[Src];

   Out << " ;";

   sim86_check_ip(Out);
}

void sim86_load_word(u16 Dst, u16 Address, std::ofstream& Out)
{
   u16 result = Memory[Address];

   Out << "; " << RegisterTable[1][Dst]
       << std::hex << ":0x" << Registers[Dst]
       << "->0x" << result << std::dec;

   sim86_check_ip(Out);

   Registers[Dst] = result;
}

void sim86_load_word_ea(u16 Dst, u16 Src, std::ofstream& Out)
{
   u16 address = 0;

   if (Src == EA_Regs_bp_si)
      address = Registers[Reg_bp] + Registers[Reg_si];

   Out << "; " << RegisterTable[1][Dst]
       << std::hex << ":0x" << Registers[Dst];

   Registers[Dst] = Memory[address];

   Out << "->0x" << Registers[Dst] << std::dec;

   sim86_check_ip(Out);
}

///////////////////////////////////////////////////////////////////////////////

uint32_t decode_register_to_register(u8 Opcode, const char* Command, char* Buffer, u16& Index, std::ofstream& out, bool execute = false)
{
   T_RegisterByte          reg;
   T_RegisterMovOpcodeByte opcode;
   uint16_t                displacement = 0;
   auto&                   i = Index;

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
      {
         out << Command << " " << RegisterTable[opcode.Word][reg.Reg] << ", " << "[" << direct_address << "]";

         if (Opcode == INTEL_REG_MOV_OPCODE && execute && opcode.Word)
         {
            sim86_load_word(reg.Reg, direct_address, out);
         }

         out << std::endl;
      }
      else if (opcode.Destination)
      {
         out << Command << " " << RegisterTable[opcode.Word][reg.Reg] << ", " << "[" << EffectiveAddressTable[reg.Rm] << "]";

         if (Opcode == INTEL_REG_MOV_OPCODE && execute && opcode.Word)
         {
            sim86_load_word_ea(reg.Reg, reg.Rm, out);
         }

         out << std::endl;
      }
      else
      {
         out << Command << " " << "[" << EffectiveAddressTable[reg.Rm] << "], " << RegisterTable[opcode.Word][reg.Reg];

         if (Opcode == INTEL_REG_MOV_OPCODE && execute && opcode.Word)
         {
            sim86_store_word_ea(reg.Rm, reg.Reg, out);
         }

         out << std::endl;
      }
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
      {
         out << Command << " " << RegisterTable[opcode.Word][reg.Reg] << ", " << effective_address;

         if (Opcode == INTEL_REG_MOV_OPCODE && execute && opcode.Word)
         {
            u16 temp_reg = 0;

            if (reg.Rm == EA_Reg_bp)
               temp_reg = Reg_bp;

            //sim86_store_word(temp_reg, displacement, 
         }

         out << std::endl;
      }
      else
      {
         out << Command << " " << effective_address << ", " << RegisterTable[opcode.Word][reg.Reg];

         if (Opcode == INTEL_REG_MOV_OPCODE && execute && opcode.Word)
         {
            u16 temp_reg = 0;

            if (reg.Rm == EA_Reg_bp)
               temp_reg = Reg_bp;

            sim86_store_word_from_reg(temp_reg, displacement, reg.Reg, out);
         }

         out << std::endl;
      }
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
         out << Command << " " << RegisterTable[opcode.Word][reg.Reg] << ", " << RegisterTable[opcode.Word][reg.Rm];
      else
         out << Command << " " << RegisterTable[opcode.Word][reg.Rm] << ", " << RegisterTable[opcode.Word][reg.Reg];

      if (execute)
      {
         if (Opcode == INTEL_REG_MOV_OPCODE && opcode.Word)
         {
            if (opcode.Destination)
               sim86_move_reg_word(reg.Reg, reg.Rm, out);
            else
               sim86_move_reg_word(reg.Rm, reg.Reg, out);
         }
         else if (Opcode == INTEL_REG_ADD_OPCODE && opcode.Word)
         {
            if (opcode.Destination)
               sim86_add_reg_word(reg.Reg, reg.Rm, out);
            else
               sim86_add_reg_word(reg.Rm, reg.Reg, out);
         }
         else if (Opcode == INTEL_REG_SUB_OPCODE && opcode.Word)
         {
            if (opcode.Destination)
               sim86_sub_reg_word(reg.Reg, reg.Rm, out);
            else
               sim86_sub_reg_word(reg.Rm, reg.Reg, out);
         }
         else if (Opcode == INTEL_REG_CMP_OPCODE && opcode.Word)
         {
            if (opcode.Destination)
               sim86_cmp_reg_word(reg.Reg, reg.Rm, out);
            else
               sim86_cmp_reg_word(reg.Rm, reg.Reg, out);
         }
      }

      out << std::endl;
   }

   return i;
}

uint32_t decode_immediate_to_register(const char* Command, char* Buffer, u16& Index, std::ofstream& out, bool execute = false)
{
   auto&          i = Index;
   T_RegisterByte reg;
   std::string    explicit_size;
   std::string    effective_address;
   uint16_t       displacement = 0;
   int16_t        signed_displacement = 0;
   uint16_t       data = 0;
   int16_t        signed_data = 0;
   uint8_t        sign = ((Buffer[i] >> 1) & 0x1);
   uint8_t        word = (Buffer[i] & 0x1);
   bool           mov_cmd = false;
   bool           add_cmd = false;
   bool           sub_cmd = false;
   bool           cmp_cmd = false;

   if (strncmp("mov", Command, 3) == 0)
      mov_cmd = true;
   else if (strncmp("add", Command, 3) == 0)
      add_cmd = true;
   else if (strncmp("sub", Command, 3) == 0)
      sub_cmd = true;
   else
      cmp_cmd = true;

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
   if (word && sign && !mov_cmd)
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
      out << Command << " " << RegisterTable[word][reg.Rm] << ", " << signed_data;

      if (execute)
      {
         if (add_cmd && word)
         {
            sim86_add_word(reg.Rm, signed_data, out);
         }
         else if (sub_cmd && word)
         {
            sim86_sub_word(reg.Rm, signed_data, out);
         }
         else if (cmp_cmd && word)
         {
            sim86_cmp_word(reg.Rm, signed_data, out);
         }
      }

      out << std::endl;
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

      out << Command << " " << effective_address << ", " << explicit_size << " " << data;

      if (mov_cmd && execute)
      {
         if (reg.Mode == 0 && reg.Rm == 6) // direct address
            sim86_store_word(displacement, data, out);
         else
         { 
            u16 temp_reg = 0;

            if (reg.Rm == 7)
               temp_reg = 3;
            else if (reg.Rm == EA_Reg_bp)
               temp_reg = Reg_bp;
            else if (reg.Rm == 5)
               temp_reg = 7;
            else if (reg.Rm == 4)
               temp_reg = 6;
            sim86_store_word(temp_reg, displacement, data, out);
         }
      }

      out << std::endl;
   }

   return i;
}

void intel_decode(char* Buffer, uint32_t BufferSize, std::ofstream& out, bool execute)
{
   for (IpRegister = 0; IpRegister < BufferSize;)
   {
      auto& i = IpRegister;

      if ((Buffer[i] & 0xfc) == INTEL_REG_MOV_OPCODE)
      {
         i = decode_register_to_register(INTEL_REG_MOV_OPCODE, "mov", Buffer, i, out, execute);
      }
      else if ((Buffer[i] & 0xfc) == INTEL_MOV_IMM_TO_REGMEM_OPCODE)
      {
         i = decode_immediate_to_register("mov", Buffer, i, out, execute);
      }
      else if ((Buffer[i] & 0xfc) == INTEL_REG_ADD_OPCODE)
      {
         i = decode_register_to_register(INTEL_REG_ADD_OPCODE, "add", Buffer, i, out, execute);
      }
      else if ((Buffer[i] & 0xfc) == INTEL_IMM_TO_REGMEM_OPCODE)
      {
         uint8_t reg = ((Buffer[i+1] >> 3) & 0x7);

         if (reg == 0) // add
            i = decode_immediate_to_register("add", Buffer, i, out, execute);
         else if (reg == 5) // sub
            i = decode_immediate_to_register("sub", Buffer, i, out, execute);
         else if (reg == 7) // cmp
            i = decode_immediate_to_register("cmp", Buffer, i, out, execute);
         else
            return;
      }
      else if ((Buffer[i] & 0xfc) == INTEL_REG_SUB_OPCODE)
      {
         i = decode_register_to_register(INTEL_REG_SUB_OPCODE, "sub", Buffer, i, out, execute);
      }
      else if ((Buffer[i] & 0xfc) == INTEL_REG_CMP_OPCODE)
      {
         i = decode_register_to_register(INTEL_REG_CMP_OPCODE, "cmp", Buffer, i, out, execute);
      }
      else if ((Buffer[i] & 0xf0) == INTEL_MOV_IMM_TO_REG_OPCODE)
      {
         T_ImmediateToRegisterOpcodeByte opcode;
         u16                             immediate_value = 0;
         
         *(uint8_t*)&opcode = Buffer[i++];

         if (opcode.Word)
         {
            immediate_value = *(uint16_t*)&Buffer[i];
            i += 2;
         }
         else
            immediate_value = Buffer[i++];

         out << "mov " << RegisterTable[opcode.Word][opcode.Reg] << ", " << immediate_value;

         if (execute && opcode.Word)
            sim86_move_word(opcode.Reg, immediate_value, out);

         out << std::endl;
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
      else if (Buffer[i] == INTEL_JO_OPCODE)
      {
         int8_t instruction_pointer = 0;
         i++;

         // read one byte for instruction pointer
         instruction_pointer = *(int8_t*)&Buffer[i++];

         out << "jo " << (int16_t)instruction_pointer << std::endl;
      }
      else if (Buffer[i] == INTEL_JNO_OPCODE)
      {
         int8_t instruction_pointer = 0;
         i++;

         // read one byte for instruction pointer
         instruction_pointer = *(int8_t*)&Buffer[i++];

         out << "jno " << (int16_t)instruction_pointer << std::endl;
      }
      else if (Buffer[i] == INTEL_JB_OPCODE)
      {
         int8_t instruction_pointer = 0;
         i++;

         // read one byte for instruction pointer
         instruction_pointer = *(int8_t*)&Buffer[i++];

         out << "jb " << (int16_t)instruction_pointer << std::endl;
      }
      else if (Buffer[i] == INTEL_JNB_OPCODE)
      {
         int8_t instruction_pointer = 0;
         i++;

         // read one byte for instruction pointer
         instruction_pointer = *(int8_t*)&Buffer[i++];

         out << "jnb " << (int16_t)instruction_pointer << std::endl;
      }
      else if (Buffer[i] == INTEL_JE_OPCODE)
      {
         int8_t instruction_pointer = 0;
         i++;

         // read one byte for instruction pointer
         instruction_pointer = *(int8_t*)&Buffer[i++];

         out << "je " << (int16_t)instruction_pointer << std::endl;
      }
      else if (Buffer[i] == INTEL_JNZ_OPCODE)
      {
         int8_t instruction_pointer = 0;
         i++;

         // read one byte for instruction pointer
         instruction_pointer = *(int8_t*)&Buffer[i++];

         out << "jnz " << (int16_t)instruction_pointer;

         if (execute)
            sim86_jnz(instruction_pointer, out);

         out << std::endl;
      }
      else if (Buffer[i] == INTEL_JBE_OPCODE)
      {
         int8_t instruction_pointer = 0;
         i++;

         // read one byte for instruction pointer
         instruction_pointer = *(int8_t*)&Buffer[i++];

         out << "jbe " << (int16_t)instruction_pointer << std::endl;
      }
      else if (Buffer[i] == INTEL_JA_OPCODE)
      {
         int8_t instruction_pointer = 0;
         i++;

         // read one byte for instruction pointer
         instruction_pointer = *(int8_t*)&Buffer[i++];

         out << "ja " << (int16_t)instruction_pointer << std::endl;
      }
      else if (Buffer[i] == INTEL_JS_OPCODE)
      {
         int8_t instruction_pointer = 0;
         i++;

         // read one byte for instruction pointer
         instruction_pointer = *(int8_t*)&Buffer[i++];

         out << "js " << (int16_t)instruction_pointer << std::endl;
      }
      else if (Buffer[i] == INTEL_JNS_OPCODE)
      {
         int8_t instruction_pointer = 0;
         i++;

         // read one byte for instruction pointer
         instruction_pointer = *(int8_t*)&Buffer[i++];

         out << "jns " << (int16_t)instruction_pointer << std::endl;
      }
      else if (Buffer[i] == INTEL_JP_OPCODE)
      {
         int8_t instruction_pointer = 0;
         i++;

         // read one byte for instruction pointer
         instruction_pointer = *(int8_t*)&Buffer[i++];

         out << "jp " << (int16_t)instruction_pointer << std::endl;
      }
      else if (Buffer[i] == INTEL_JNP_OPCODE)
      {
         int8_t instruction_pointer = 0;
         i++;

         // read one byte for instruction pointer
         instruction_pointer = *(int8_t*)&Buffer[i++];

         out << "jnp " << (int16_t)instruction_pointer << std::endl;
      }
      else if (Buffer[i] == INTEL_JL_OPCODE)
      {
         int8_t instruction_pointer = 0;
         i++;

         // read one byte for instruction pointer
         instruction_pointer = *(int8_t*)&Buffer[i++];

         out << "jl " << (int16_t)instruction_pointer << std::endl;
      }
      else if (Buffer[i] == INTEL_JNL_OPCODE)
      {
         int8_t instruction_pointer = 0;
         i++;

         // read one byte for instruction pointer
         instruction_pointer = *(int8_t*)&Buffer[i++];

         out << "jnl " << (int16_t)instruction_pointer << std::endl;
      }
      else if (Buffer[i] == INTEL_JLE_OPCODE)
      {
         int8_t instruction_pointer = 0;
         i++;

         // read one byte for instruction pointer
         instruction_pointer = *(int8_t*)&Buffer[i++];

         out << "jle " << (int16_t)instruction_pointer << std::endl;
      }
      else if (Buffer[i] == INTEL_JG_OPCODE)
      {
         int8_t instruction_pointer = 0;
         i++;

         // read one byte for instruction pointer
         instruction_pointer = *(int8_t*)&Buffer[i++];

         out << "jg " << (int16_t)instruction_pointer << std::endl;
      }
      else if ((Buffer[i] & 0xff) == INTEL_LOOPNZ_OPCODE)
      {
         int8_t instruction_pointer = 0;
         i++;

         // read one byte for instruction pointer
         instruction_pointer = *(int8_t*)&Buffer[i++];

         out << "loopnz " << (int16_t)instruction_pointer << std::endl;
      }
      else if ((Buffer[i] & 0xff) == INTEL_LOOPZ_OPCODE)
      {
         int8_t instruction_pointer = 0;
         i++;

         // read one byte for instruction pointer
         instruction_pointer = *(int8_t*)&Buffer[i++];

         out << "loopz " << (int16_t)instruction_pointer << std::endl;
      }
      else if ((Buffer[i] & 0xff) == INTEL_LOOP_OPCODE)
      {
         int8_t instruction_pointer = 0;
         i++;

         // read one byte for instruction pointer
         instruction_pointer = *(int8_t*)&Buffer[i++];

         out << "loop " << (int16_t)instruction_pointer << std::endl;
      }
      else if ((Buffer[i] & 0xff) == INTEL_JCXZ_OPCODE)
      {
         int8_t instruction_pointer = 0;
         i++;

         // read one byte for instruction pointer
         instruction_pointer = *(int8_t*)&Buffer[i++];

         out << "jcxz " << (int16_t)instruction_pointer << std::endl;
      }
      else
      {
         std::cout << "Error: Uknown opcode " << std::hex << (uint16_t)Buffer[i] << " at " << std::dec << i << std::endl;
         return;
      }
   }
}

int main(int argc, char* argv[])
{
   char*         buffer = nullptr;
   uint32_t      buffer_size;
   bool          execute = false;
   std::ifstream input_file;
   std::ofstream output_file("output.asm");
   std::ofstream dump_file;

   if (argc < 2 || argc > 4)
   {
      std::cout << "Usage: main [-dump] [-exec] <assembly-file>" << std::endl;
      return 0;
   }

   if (argc == 2)
      input_file.open(argv[1], std::ios::binary);
   else if (argc == 3)
   {
      input_file.open(argv[2], std::ios::binary);
      execute = true;
   }
   else if (argc == 4)
   {
      input_file.open(argv[3], std::ios::binary);
      dump_file.open("dump.data", std::ios::binary);
      execute = true;
   }

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
   intel_decode(buffer, buffer_size, output_file, execute);

   output_file << std::endl << "Final registers:" << std::endl;

   for (int i = 0; i < Num_Registers; i++)
   {
      if (Registers[i])
      {
         output_file << "      " << RegisterTable[1][i] << ": 0x" 
            << std::hex << std::setfill('0') << std::setw(4) << Registers[i] 
            << std::dec << " (" << Registers[i] << ")" << std::endl;
      }
   }

   output_file << "      " << "ip: 0x" 
      << std::hex << std::setfill('0') << std::setw(4) << IpRegister 
      << std::dec << " (" << IpRegister << ")" << std::endl;

   if (Flags)
   {
      output_file << "   flags: ";

      if (Flags & ZeroFlag)
         output_file << "Z";
      if (Flags & SignFlag)
         output_file << "S";

      output_file << std::endl;
   }

   input_file.close();
   output_file.close();

   if (dump_file.is_open())
   {
      dump_file.write((char*)Memory, MEMORY_SIZE);
      dump_file.close();
   }

   return 1;
}
