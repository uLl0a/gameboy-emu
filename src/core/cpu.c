#include "cpu.h"
#include <stdlib.h>
#include "LR35902.h"
#include "bus.h"

struct cpu_context_t cpu = { 0 };

void cpu_reset(void)
{
    cpu.regs.af.hi = 0x01;
    cpu.regs.af.lo = 0xB0;
    cpu.regs.bc.hi = 0x00;
    cpu.regs.bc.lo = 0x13;
    cpu.regs.de.hi = 0x00;
    cpu.regs.de.lo = 0xD8;
    cpu.regs.hl.hi = 0x01;
    cpu.regs.hl.lo = 0x4D;
    cpu.regs.sp.reg = 0xFFFE;
    cpu.regs.pc.reg = 0x100;
    cpu.regs.bc.reg = 0x0013;
    cpu.regs.de.reg = 0x00D8;
    cpu.regs.hl.reg = 0x014D;
    cpu.halted = 0;

    Cartridge.rom_data[0xFF00] = 0xFF;
    Cartridge.rom_data[0xFF05] = 0x00;
    Cartridge.rom_data[0xFF06] = 0x00;
    Cartridge.rom_data[0xFF07] = 0x00;
    Cartridge.rom_data[0xFF10] = 0x80;
    Cartridge.rom_data[0xFF11] = 0xBF;
    Cartridge.rom_data[0xFF12] = 0xF3;
    Cartridge.rom_data[0xFF14] = 0xBF;
    Cartridge.rom_data[0xFF16] = 0x3F;
    Cartridge.rom_data[0xFF17] = 0x00;
    Cartridge.rom_data[0xFF19] = 0xBF;
    Cartridge.rom_data[0xFF1A] = 0x7F;
    Cartridge.rom_data[0xFF1B] = 0xFF;
    Cartridge.rom_data[0xFF1C] = 0x9F;
    Cartridge.rom_data[0xFF1E] = 0xBF;
    Cartridge.rom_data[0xFF20] = 0xFF;
    Cartridge.rom_data[0xFF21] = 0x00;
    Cartridge.rom_data[0xFF22] = 0x00;
    Cartridge.rom_data[0xFF23] = 0xBF;
    Cartridge.rom_data[0xFF24] = 0x77;
    Cartridge.rom_data[0xFF25] = 0xF3;
    Cartridge.rom_data[0xFF26] = 0xF1;
    Cartridge.rom_data[0xFF40] = 0x91;
    Cartridge.rom_data[0xFF42] = 0x00;
    Cartridge.rom_data[0xFF43] = 0x00;
    Cartridge.rom_data[0xFF45] = 0x00;
    Cartridge.rom_data[0xFF47] = 0xFC;
    Cartridge.rom_data[0xFF48] = 0xFF;
    Cartridge.rom_data[0xFF49] = 0xFF;
    Cartridge.rom_data[0xFF4A] = 0x00;
    Cartridge.rom_data[0xFF4B] = 0x00;
    Cartridge.rom_data[0xFFFF] = 0x00;
}

void cpu_fetch_opcode(void)
{
    cpu.cur_opcode = *bus_read(cpu.regs.pc.reg);
#ifdef GBEMU_VERBOSE
    debug_print("%04X: %02X (%02X %02X %02X) cycles: %d A: %02X F: %02X B: %02X C: %02X D: %02X E: %02X H: %02X L: %02X AF: %02X BC: %02X DE: %02X HL: %02X\n", cpu.regs.pc.reg, cpu.cur_opcode, *bus_read(cpu.regs.pc.reg + 1), *bus_read(cpu.regs.pc.reg + 2), *bus_read(cpu.regs.pc.reg + 3), cpu.cycles, cpu.regs.af.hi,cpu.regs.af.lo, cpu.regs.bc.hi, cpu.regs.bc.lo, cpu.regs.de.hi, cpu.regs.de.lo, cpu.regs.hl.hi, cpu.regs.hl.lo,cpu.regs.af.reg,cpu.regs.bc.reg,cpu.regs.de.reg, cpu.regs.hl.reg);
#endif
    cpu.regs.pc.reg += 1;
}

void cpu_decode_and_execute(void)
{
    switch (cpu.cur_opcode) {
    case 0x00: cpu.cycles += 4; break;
    /* LD n,nn */
    case 0x01: _16bit_ld(&cpu.regs.bc.reg, _16bit_bus_read(cpu.regs.pc.reg),3,12);break;
    case 0x11: _16bit_ld(&cpu.regs.de.reg, _16bit_bus_read(cpu.regs.pc.reg),3,12);break;
    case 0x21: _16bit_ld(&cpu.regs.hl.reg, _16bit_bus_read(cpu.regs.pc.reg),3,12);break;
    case 0x31: _16bit_ld(&cpu.regs.sp.reg, _16bit_bus_read(cpu.regs.pc.reg),3,12);break;
    case 0x06: _8bit_ld(&cpu.regs.bc.hi, *bus_read(cpu.regs.pc.reg),2, 8); break;
    case 0x0E: _8bit_ld(&cpu.regs.bc.lo, *bus_read(cpu.regs.pc.reg),2, 8); break;
    case 0x16: _8bit_ld(&cpu.regs.de.hi, *bus_read(cpu.regs.pc.reg),2, 8); break;
    case 0x1E: _8bit_ld(&cpu.regs.de.lo, *bus_read(cpu.regs.pc.reg),2, 8); break;
    case 0x26: _8bit_ld(&cpu.regs.hl.hi, *bus_read(cpu.regs.pc.reg),2, 8); break;
    case 0x2E: _8bit_ld(&cpu.regs.hl.lo, *bus_read(cpu.regs.pc.reg),2, 8); break;
    /* LD SP,H */
    case 0xF9: _16bit_ld(&cpu.regs.sp.reg,cpu.regs.hl.reg,1,8);break;
    /* LDHL SP,n */
    case 0xF8: 
               {
                   _s8 n = *(_s8*)bus_read(cpu.regs.pc.reg);
                   cpu.regs.hl.reg = (cpu.regs.sp.reg + n) & 0xFFFF;
                   _CPU_RESET_BIT(CPU_FLAG_Z);
                   _CPU_RESET_BIT(CPU_FLAG_N);
                   if(_16BIT_HALF_CARRY(cpu.regs.sp.reg, n)){
                       _CPU_SET_BIT(CPU_FLAG_N);
                   }else{
                       _CPU_RESET_BIT(CPU_FLAG_N);
                   }

                   if(_16BIT_CARRY(cpu.regs.sp.reg, n)){
                       _CPU_SET_BIT(CPU_FLAG_C);
                   }else{
                       _CPU_RESET_BIT(CPU_FLAG_C);
                   }
                   cpu.cycles += 12;
                   cpu.regs.pc.reg += 2;
               }
               break;
    /* LD (nn),SP */
    case 0x08: _16bit_bus_write(cpu.regs.pc.reg, cpu.regs.sp.reg); cpu.cycles += 20; cpu.regs.pc.reg += 3;break;
    /* LD r1,r2 */
    case 0x78: _8bit_ld(&cpu.regs.af.hi,cpu.regs.bc.hi,1,4); break;
    case 0x79: _8bit_ld(&cpu.regs.af.hi,cpu.regs.bc.lo,1,4); break;
    case 0x7A: _8bit_ld(&cpu.regs.af.hi,cpu.regs.de.hi,1,4); break;
    case 0x7B: _8bit_ld(&cpu.regs.af.hi,cpu.regs.de.lo,1,4); break;
    case 0x7C: _8bit_ld(&cpu.regs.af.hi,cpu.regs.hl.hi,1,4); break;
    case 0x7D: _8bit_ld(&cpu.regs.af.hi,cpu.regs.hl.lo,1,4); break;
    case 0x0A: _8bit_ld(&cpu.regs.af.hi,*bus_read(cpu.regs.bc.reg),1,8); break;
    case 0x1A: _8bit_ld(&cpu.regs.af.hi,*bus_read(cpu.regs.de.reg),1,8); break;
    case 0x7E: _8bit_ld(&cpu.regs.af.hi,*bus_read(cpu.regs.hl.reg),1,8); break;
    case 0xFA: _8bit_ld(&cpu.regs.af.hi,*bus_read(_16bit_bus_read(cpu.regs.pc.reg)),3,16); break;
    case 0x3E: _8bit_ld(&cpu.regs.af.hi,*bus_read(cpu.regs.pc.reg),2,8); break;
    case 0x40: _8bit_ld(&cpu.regs.bc.hi,cpu.regs.bc.hi,1,4); break;
    case 0x41: _8bit_ld(&cpu.regs.bc.hi,cpu.regs.bc.lo,1,4); break;
    case 0x42: _8bit_ld(&cpu.regs.bc.hi,cpu.regs.de.hi,1,4); break;
    case 0x43: _8bit_ld(&cpu.regs.bc.hi,cpu.regs.de.lo,1,4); break;
    case 0x44: _8bit_ld(&cpu.regs.bc.hi,cpu.regs.hl.hi,1,4); break;
    case 0x45: _8bit_ld(&cpu.regs.bc.hi,cpu.regs.hl.lo,1,4); break;
    case 0x46: _8bit_ld(&cpu.regs.bc.hi,*bus_read(cpu.regs.hl.reg),1,8); break;
    case 0x48: _8bit_ld(&cpu.regs.de.lo,cpu.regs.bc.hi,1,4); break;
    case 0x49: _8bit_ld(&cpu.regs.bc.lo,cpu.regs.bc.lo,1,4); break;
    case 0x4A: _8bit_ld(&cpu.regs.bc.lo,cpu.regs.de.hi,1,4); break;
    case 0x4B: _8bit_ld(&cpu.regs.bc.lo,cpu.regs.de.lo,1,4); break;
    case 0x4C: _8bit_ld(&cpu.regs.bc.lo,cpu.regs.hl.hi,1,4); break;
    case 0x4D: _8bit_ld(&cpu.regs.bc.lo,cpu.regs.hl.lo,1,4); break;
    case 0x4E: _8bit_ld(&cpu.regs.bc.lo,*bus_read(cpu.regs.hl.reg),1,8); break;
    case 0x50: _8bit_ld(&cpu.regs.de.hi,cpu.regs.bc.hi,1,4); break;
    case 0x51: _8bit_ld(&cpu.regs.de.hi,cpu.regs.bc.lo,1,4); break;
    case 0x52: _8bit_ld(&cpu.regs.de.hi,cpu.regs.de.hi,1,4); break;
    case 0x53: _8bit_ld(&cpu.regs.de.hi,cpu.regs.de.lo,1,4); break;
    case 0x54: _8bit_ld(&cpu.regs.de.hi,cpu.regs.hl.hi,1,4); break;
    case 0x55: _8bit_ld(&cpu.regs.de.hi,cpu.regs.hl.lo,1,4); break;
    case 0x56: _8bit_ld(&cpu.regs.de.hi,*bus_read(cpu.regs.hl.reg),1,8); break;
    case 0x58: _8bit_ld(&cpu.regs.de.lo,cpu.regs.bc.hi,1,4); break;
    case 0x59: _8bit_ld(&cpu.regs.de.lo,cpu.regs.bc.lo,1,4); break;
    case 0x5A: _8bit_ld(&cpu.regs.de.lo,cpu.regs.de.hi,1,4); break;
    case 0x5B: _8bit_ld(&cpu.regs.de.lo,cpu.regs.de.lo,1,4); break;
    case 0x5C: _8bit_ld(&cpu.regs.de.lo,cpu.regs.hl.hi,1,4); break;
    case 0x5D: _8bit_ld(&cpu.regs.de.lo,cpu.regs.hl.lo,1,4); break;
    case 0x5E: _8bit_ld(&cpu.regs.de.lo,*bus_read(cpu.regs.hl.reg),1,8); break;
    case 0x60: _8bit_ld(&cpu.regs.hl.hi,cpu.regs.bc.hi,1,4); break;
    case 0x61: _8bit_ld(&cpu.regs.hl.hi,cpu.regs.bc.lo,1,4); break;
    case 0x62: _8bit_ld(&cpu.regs.hl.hi,cpu.regs.de.hi,1,4); break;
    case 0x63: _8bit_ld(&cpu.regs.hl.hi,cpu.regs.de.lo,1,4); break;
    case 0x64: _8bit_ld(&cpu.regs.hl.hi,cpu.regs.hl.hi,1,4); break;
    case 0x65: _8bit_ld(&cpu.regs.hl.hi,cpu.regs.hl.lo,1,4); break;
    case 0x66: _8bit_ld(&cpu.regs.hl.hi,*bus_read(cpu.regs.hl.reg),1,8); break;
    case 0x68: _8bit_ld(&cpu.regs.hl.lo,cpu.regs.bc.hi,1,4); break;
    case 0x69: _8bit_ld(&cpu.regs.hl.lo,cpu.regs.bc.lo,1,4); break;
    case 0x6A: _8bit_ld(&cpu.regs.hl.lo,cpu.regs.de.hi,1,4); break;
    case 0x6B: _8bit_ld(&cpu.regs.hl.lo,cpu.regs.de.lo,1,4); break;
    case 0x6C: _8bit_ld(&cpu.regs.hl.lo,cpu.regs.hl.hi,1,4); break;
    case 0x6D: _8bit_ld(&cpu.regs.hl.lo,cpu.regs.hl.lo,1,4); break;
    case 0x6E: _8bit_ld(&cpu.regs.hl.lo,*bus_read(cpu.regs.hl.reg),1,8); break;
    case 0x70: bus_write(cpu.regs.hl.reg, cpu.regs.bc.hi); cpu.cycles += 8;  cpu.regs.pc.reg += 1;break;
    case 0x71: bus_write(cpu.regs.hl.reg, cpu.regs.bc.lo); cpu.cycles += 8; cpu.regs.pc.reg += 1;break;
    case 0x72: bus_write(cpu.regs.hl.reg, cpu.regs.de.hi); cpu.cycles += 8; cpu.regs.pc.reg += 1;break;
    case 0x73: bus_write(cpu.regs.hl.reg, cpu.regs.de.lo); cpu.cycles += 8; cpu.regs.pc.reg += 1;break;
    case 0x74: bus_write(cpu.regs.hl.reg, cpu.regs.hl.hi); cpu.cycles += 8; cpu.regs.pc.reg += 1;break;
    case 0x75: bus_write(cpu.regs.hl.reg, cpu.regs.hl.lo); cpu.cycles += 8; cpu.regs.pc.reg += 1;break;
    case 0x36: bus_write(cpu.regs.hl.reg, *bus_read(cpu.regs.pc.reg)); cpu.cycles += 12; cpu.regs.pc.reg += 2;break;
    /* LD n,A */
    case 0x7F: _8bit_ld(&cpu.regs.af.hi, cpu.regs.af.hi,1, 4); break;
    case 0x47: _8bit_ld(&cpu.regs.bc.hi, cpu.regs.af.hi,1, 4); break;
    case 0x4F: _8bit_ld(&cpu.regs.bc.lo, cpu.regs.af.hi,1, 4); break;
    case 0x57: _8bit_ld(&cpu.regs.de.hi, cpu.regs.af.hi,1, 4); break;
    case 0x5F: _8bit_ld(&cpu.regs.de.lo, cpu.regs.af.hi,1, 4); break;
    case 0x67: _8bit_ld(&cpu.regs.hl.hi, cpu.regs.af.hi,1, 4); break;
    case 0x6F: _8bit_ld(&cpu.regs.hl.lo, cpu.regs.af.hi,1, 4); break;
    case 0x02: bus_write(cpu.regs.bc.reg, cpu.regs.af.hi); cpu.cycles += 8; cpu.regs.pc.reg += 1; break;
    case 0x12: bus_write(cpu.regs.de.reg, cpu.regs.af.hi); cpu.cycles += 8; cpu.regs.pc.reg += 1; break;
    case 0x77: bus_write(cpu.regs.hl.reg, cpu.regs.af.hi); cpu.cycles += 8; cpu.regs.pc.reg += 1; break;
    case 0xEA: bus_write(_16bit_bus_read(cpu.regs.pc.reg), cpu.regs.af.hi); cpu.cycles += 16; cpu.regs.pc.reg += 3;break;
    /* LD A,(C) */
    case 0xF2: _8bit_ld(&cpu.regs.af.hi, *bus_read(0xFF00 + cpu.regs.bc.lo),1, 8); break;
    /* LD (C),A */
    case 0xE2: bus_write(0xFF00 + cpu.regs.bc.lo, cpu.regs.af.hi); cpu.cycles += 8; cpu.regs.pc.reg += 1;break;
    /* LD A,(HL+ */
    case 0x2A: _8bit_ld(&cpu.regs.af.hi, *bus_read(cpu.regs.hl.reg),1,4); _16bit_inc(&cpu.regs.hl.reg,0, 4); break; 
    /* LD (HL+),A */
    case 0x22: bus_write(cpu.regs.hl.reg, cpu.regs.af.hi); _16bit_inc(&cpu.regs.hl.reg,1, 8);break;
    /* LD A,(HL-) */
    case 0x3A: _8bit_ld(&cpu.regs.af.hi, *bus_read(cpu.regs.hl.reg),1,4); _16bit_dec(&cpu.regs.hl.reg,0, 4); break; 
    /* LD (HL-),A */
    case 0x32: bus_write(cpu.regs.hl.reg, cpu.regs.af.hi); _16bit_dec(&cpu.regs.hl.reg,1, 8);break;
    /* LDH (n),A */
    case 0xE0: bus_write(0xFF00 + cpu.regs.pc.reg, cpu.regs.af.hi); cpu.cycles += 12; cpu.regs.pc.reg += 2; break;
    /* LDH A,(n) */
    case 0xF0: _8bit_ld(&cpu.regs.af.hi, *bus_read(0xFF00 + cpu.regs.pc.reg),2, 12); break;
    /* INC nn */
    case 0x03: _16bit_inc(&cpu.regs.bc.reg,1, 8); break;
    case 0x13: _16bit_inc(&cpu.regs.de.reg,1, 8); break;
    case 0x23: _16bit_inc(&cpu.regs.hl.reg,1, 8); break;
    case 0x33: _16bit_inc(&cpu.regs.sp.reg,1, 8);break;
    /* DEC nn */
    case 0x0B: _16bit_dec(&cpu.regs.bc.reg,1, 8); break;
    case 0x1B: _16bit_dec(&cpu.regs.de.reg,1, 8); break;
    case 0x2B: _16bit_dec(&cpu.regs.hl.reg,1, 8); break;
    case 0x3B: _16bit_dec(&cpu.regs.sp.reg,1, 8); break;
    /* INC n */
    case 0x3C: _8bit_inc(&cpu.regs.af.hi,1, 4); break;
    case 0x04: _8bit_inc(&cpu.regs.bc.hi,1, 4); break;
    case 0x0C: _8bit_inc(&cpu.regs.bc.lo,1, 4); break;
    case 0x14: _8bit_inc(&cpu.regs.de.hi,1, 4); break;
    case 0x1C: _8bit_inc(&cpu.regs.de.lo,1, 4); break;
    case 0x24: _8bit_inc(&cpu.regs.hl.hi,1, 4); break;
    case 0x2C: _8bit_inc(&cpu.regs.hl.lo,1, 4); break;
    case 0x34: _8bit_memory_inc(cpu.regs.hl.reg,1, 12); break;
    /* DEC n */
    case 0x3D: _8bit_dec(&cpu.regs.af.hi,1, 4); break;
    case 0x05: _8bit_dec(&cpu.regs.bc.hi,1, 4); break;
    case 0x0D: _8bit_dec(&cpu.regs.bc.lo,1, 4); break;
    case 0x15: _8bit_dec(&cpu.regs.de.hi,1, 4); break;
    case 0x1D: _8bit_dec(&cpu.regs.de.lo,1, 4); break;
    case 0x25: _8bit_dec(&cpu.regs.hl.hi,1, 4); break;
    case 0x2D: _8bit_dec(&cpu.regs.hl.lo,1, 4); break;
    case 0x35: _8bit_memory_dec(cpu.regs.af.hi,1, 12); break;
    case 0xC3: _jp(_16bit_bus_read((cpu.regs.pc.reg)),0,3, 12); break;
    case 0xC2: _jp(_16bit_bus_read(cpu.regs.pc.reg),CPU_FLAG_NZ,3,12); break;
    case 0xCA: _jp(_16bit_bus_read(cpu.regs.pc.reg),CPU_FLAG_Z,3,12); break;
    case 0xD2: _jp(_16bit_bus_read(cpu.regs.pc.reg),CPU_FLAG_NC,3,12); break;
    case 0xDA: _jp(_16bit_bus_read(cpu.regs.pc.reg),CPU_FLAG_C,3,12); break;
    case 0xE9: _jp(_16bit_bus_read(cpu.regs.hl.reg),0,1, 4); break;
    case 0x18: _jr(*(_s8*)bus_read(cpu.regs.pc.reg),0,2,8); break;
    case 0x20: _jr(*(_s8*)bus_read(cpu.regs.pc.reg),CPU_FLAG_NZ,2,8); break;
    case 0x28: _jr(*(_s8*)bus_read(cpu.regs.pc.reg),CPU_FLAG_Z,2,8); break;
    case 0x30: _jr(*(_s8*)bus_read(cpu.regs.pc.reg),CPU_FLAG_NC,2,8); break;
    case 0x38: _jr(*(_s8*)bus_read(cpu.regs.pc.reg),CPU_FLAG_C,2,8); break;
    case 0xCD: _call(_16bit_bus_read(cpu.regs.pc.reg),0,3,12); break;
    case 0xC4: _call(_16bit_bus_read(cpu.regs.pc.reg),CPU_FLAG_NZ,3, 12); break;
    case 0xCC: _call(_16bit_bus_read(cpu.regs.pc.reg),CPU_FLAG_Z,3, 12); break;
    case 0xD4: _call(_16bit_bus_read(cpu.regs.pc.reg),CPU_FLAG_NC,3, 12); break;
    case 0xDC: _call(_16bit_bus_read(cpu.regs.pc.reg),CPU_FLAG_C,3, 12); break;
    case 0xC9: _ret(0,1,8); break;
    case 0xC0: _ret(CPU_FLAG_NZ,1, 8); break;
    case 0xC8: _ret(CPU_FLAG_Z,1, 8); break;
    case 0xD0: _ret(CPU_FLAG_NC,1, 8); break;
    case 0xD8: _ret(CPU_FLAG_C,1, 8); break;
    case 0xCB: cpu_decode_and_execute_extended_opcode(); break;
    /* CP n */
    case 0xBF: _cp(cpu.regs.af.hi,1,4); break;
    case 0xB8: _cp(cpu.regs.bc.hi,1,4); break;
    case 0xB9: _cp(cpu.regs.bc.lo,1,4); break;
    case 0xBA: _cp(cpu.regs.de.hi,1,4); break;
    case 0xBB: _cp(cpu.regs.de.lo,1,4); break;
    case 0xBC: _cp(cpu.regs.hl.hi,1,4); break;
    case 0xBD: _cp(cpu.regs.hl.lo,1,4); break;
    case 0xBE: _cp(*bus_read(cpu.regs.hl.reg),1,8); break;
    case 0xFE: _cp(*bus_read(cpu.regs.pc.reg),2,8); break;
    /* XOR n */
    case 0xAF: _xor(&cpu.regs.af.hi,1, 4); break;
    case 0xA8: _xor(&cpu.regs.bc.hi,1, 4); break;
    case 0xA9: _xor(&cpu.regs.bc.lo,1, 4); break;
    case 0xAA: _xor(&cpu.regs.de.hi,1, 4); break;
    case 0xAB: _xor(&cpu.regs.de.lo,1, 4); break;
    case 0xAC: _xor(&cpu.regs.hl.hi,1, 4); break;
    case 0xAD: _xor(&cpu.regs.hl.lo,1, 4); break;
    case 0xAE: _xor(bus_read(cpu.regs.hl.reg),1, 8); break;
    case 0xEE: _xor(bus_read(cpu.regs.pc.reg),2, 8); break;
    /* OR n */
    case 0xB7: _or(&cpu.regs.af.hi,1, 4); break;
    case 0xB0: _or(&cpu.regs.bc.hi,1, 4); break;
    case 0xB1: _or(&cpu.regs.bc.lo,1, 4); break;
    case 0xB2: _or(&cpu.regs.de.hi,1, 4); break;
    case 0xB3: _or(&cpu.regs.de.lo,1, 4); break;
    case 0xB4: _or(&cpu.regs.hl.hi,1, 4); break;
    case 0xB5: _or(&cpu.regs.hl.lo,1, 4); break;
    case 0xB6: _or(bus_read(cpu.regs.hl.reg),1, 8); break;
    case 0xF6: _or(bus_read(cpu.regs.pc.reg),2, 8); break;
    /* AND n */
    case 0xA7: _and(&cpu.regs.af.hi,1, 4); break;
    case 0xA0: _and(&cpu.regs.bc.hi,1, 4); break;
    case 0xA1: _and(&cpu.regs.bc.lo,1, 4); break;
    case 0xA2: _and(&cpu.regs.de.hi,1, 4); break;
    case 0xA3: _and(&cpu.regs.de.lo,1, 4); break;
    case 0xA4: _and(&cpu.regs.hl.hi,1, 4); break;
    case 0xA5: _and(&cpu.regs.hl.lo,1, 4); break;
    case 0xA6: _and(bus_read(cpu.regs.hl.reg),1, 8); break;
    case 0xE6: _and(bus_read(cpu.regs.pc.reg),2, 8); break;
    /* PUSH nn */
    case 0xF5: _push(cpu.regs.af.reg,1,16); break;
    case 0xC5: _push(cpu.regs.bc.reg,1,16); break;
    case 0xD5: _push(cpu.regs.de.reg,1,16); break;
    case 0xE5: _push(cpu.regs.hl.reg,1,16); break;
    /* POP nn */
    case 0xF1: cpu.regs.af.reg = _pop(1,12); break;
    case 0xC1: cpu.regs.bc.reg = _pop(1,12); break;
    case 0xD1: cpu.regs.de.reg = _pop(1,12); break;
    case 0xE1: cpu.regs.hl.reg = _pop(1,12); break;
    /* RST n */
    case 0xC7: _rst(0x00); break;
    case 0xCF: _rst(0x08); break;
    case 0xD7: _rst(0x10); break;
    case 0xDF: _rst(0x18); break;
    case 0xE7: _rst(0x20); break;
    case 0xEF: _rst(0x28); break;
    case 0xF7: _rst(0x30); break;
    case 0xFF: _rst(0x38); break;
    /* ADD A,n */
    case 0x87: _8bit_add(&cpu.regs.af.hi, cpu.regs.af.hi,1, 4); break;
    case 0x80: _8bit_add(&cpu.regs.af.hi, cpu.regs.bc.hi,1, 4); break;
    case 0x81: _8bit_add(&cpu.regs.af.hi, cpu.regs.bc.lo,1, 4); break;
    case 0x82: _8bit_add(&cpu.regs.af.hi, cpu.regs.de.hi,1, 4); break;
    case 0x83: _8bit_add(&cpu.regs.af.hi, cpu.regs.de.lo,1, 4); break;
    case 0x84: _8bit_add(&cpu.regs.af.hi, cpu.regs.hl.hi,1, 4); break;
    case 0x85: _8bit_add(&cpu.regs.af.hi, cpu.regs.hl.lo,1, 4); break;
    case 0x86: _8bit_add(&cpu.regs.af.hi, *bus_read(cpu.regs.hl.reg),1, 8); break;
    case 0xC6: _8bit_add(&cpu.regs.af.hi, *bus_read(cpu.regs.pc.reg),2, 8); break;
    /* ADC A,n */
    case 0x8F: {_u8 c = _CPU_TEST_BIT(CPU_FLAG_C) ? 1 : 0; _8bit_add(&cpu.regs.af.hi, cpu.regs.af.hi + c,1, 4); }break;
    case 0x88: {_u8 c = _CPU_TEST_BIT(CPU_FLAG_C) ? 1 : 0; _8bit_add(&cpu.regs.af.hi, cpu.regs.bc.hi + c,1, 4); }break;
    case 0x89: {_u8 c = _CPU_TEST_BIT(CPU_FLAG_C) ? 1 : 0; _8bit_add(&cpu.regs.af.hi, cpu.regs.bc.lo + c,1, 4); }break;
    case 0x8A: {_u8 c = _CPU_TEST_BIT(CPU_FLAG_C) ? 1 : 0; _8bit_add(&cpu.regs.af.hi, cpu.regs.de.hi + c,1, 4); }break;
    case 0x8B: {_u8 c = _CPU_TEST_BIT(CPU_FLAG_C) ? 1 : 0; _8bit_add(&cpu.regs.af.hi, cpu.regs.de.lo + c,1, 4); }break;
    case 0x8C: {_u8 c = _CPU_TEST_BIT(CPU_FLAG_C) ? 1 : 0; _8bit_add(&cpu.regs.af.hi, cpu.regs.hl.hi + c,1, 4); }break;
    case 0x8D: {_u8 c = _CPU_TEST_BIT(CPU_FLAG_C) ? 1 : 0; _8bit_add(&cpu.regs.af.hi, cpu.regs.hl.lo + c,1, 4); }break;
    case 0x8E: {_u8 c = _CPU_TEST_BIT(CPU_FLAG_C) ? 1 : 0; _8bit_add(&cpu.regs.af.hi, *bus_read(cpu.regs.hl.reg) + c,1, 4); }break;
    case 0xCE: {_u8 c = _CPU_TEST_BIT(CPU_FLAG_C) ? 1 : 0; _8bit_add(&cpu.regs.af.hi, *bus_read(cpu.regs.pc.reg) + c,2, 4); }break;
    /* SUB n */
    case 0x97: _8bit_sub(&cpu.regs.af.hi, cpu.regs.af.hi,1, 4); break;
    case 0x90: _8bit_sub(&cpu.regs.af.hi, cpu.regs.bc.hi,1, 4); break;
    case 0x91: _8bit_sub(&cpu.regs.af.hi, cpu.regs.bc.lo,1, 4); break;
    case 0x92: _8bit_sub(&cpu.regs.af.hi, cpu.regs.de.hi,1, 4); break;
    case 0x93: _8bit_sub(&cpu.regs.af.hi, cpu.regs.de.lo,1, 4); break;
    case 0x94: _8bit_sub(&cpu.regs.af.hi, cpu.regs.hl.hi,1, 4); break;
    case 0x95: _8bit_sub(&cpu.regs.af.hi, cpu.regs.hl.lo,1, 4); break;
    case 0x96: _8bit_sub(&cpu.regs.af.hi, *bus_read(cpu.regs.hl.reg),1, 8); break;
    case 0xD6: _8bit_sub(&cpu.regs.af.hi, *bus_read(cpu.regs.pc.reg),2, 8); break;
    /* SBC A,n */
    case 0x9F: {_u8 c = _CPU_TEST_BIT(CPU_FLAG_C) ? 1 : 0; _8bit_sub(&cpu.regs.af.hi, cpu.regs.af.hi + c,1, 4); }break;
    case 0x98: {_u8 c = _CPU_TEST_BIT(CPU_FLAG_C) ? 1 : 0; _8bit_sub(&cpu.regs.af.hi, cpu.regs.bc.hi + c,1, 4); }break;
    case 0x99: {_u8 c = _CPU_TEST_BIT(CPU_FLAG_C) ? 1 : 0; _8bit_sub(&cpu.regs.af.hi, cpu.regs.bc.lo + c,1, 4); }break;
    case 0x9A: {_u8 c = _CPU_TEST_BIT(CPU_FLAG_C) ? 1 : 0; _8bit_sub(&cpu.regs.af.hi, cpu.regs.de.hi + c,1, 4); }break;
    case 0x9B: {_u8 c = _CPU_TEST_BIT(CPU_FLAG_C) ? 1 : 0; _8bit_sub(&cpu.regs.af.hi, cpu.regs.de.lo + c,1, 4); }break;
    case 0x9C: {_u8 c = _CPU_TEST_BIT(CPU_FLAG_C) ? 1 : 0; _8bit_sub(&cpu.regs.af.hi, cpu.regs.hl.hi + c,1, 4); }break;
    case 0x9D: {_u8 c = _CPU_TEST_BIT(CPU_FLAG_C) ? 1 : 0; _8bit_sub(&cpu.regs.af.hi, cpu.regs.hl.lo + c,1, 4); }break;
    case 0x9E: {_u8 c = _CPU_TEST_BIT(CPU_FLAG_C) ? 1 : 0; _8bit_sub(&cpu.regs.af.hi, *bus_read(cpu.regs.hl.reg) + c,2, 4); }break;
    /* case 0x??: {_u8 c = _CPU_TEST_BIT(CPU_FLAG_C) ? 1 : 0; _8bit_sub(&cpu.regs.af.hi, *bus_read(cpu.regs.pc.reg++) + c, ?); }break; */
    /* ADD HL,n */
    case 0x09: _16bit_add(&cpu.regs.hl.reg, cpu.regs.bc.reg,1, 8); break;
    case 0x19: _16bit_add(&cpu.regs.hl.reg, cpu.regs.de.reg,1, 8); break;
    case 0x29: _16bit_add(&cpu.regs.hl.reg, cpu.regs.hl.reg,1, 8); break;
    case 0x39: _16bit_add(&cpu.regs.hl.reg, cpu.regs.sp.reg,1, 8); break;
    /* ADD SP,n */
    case 0xE8: {_16bit_add(&cpu.regs.sp.reg,*bus_read(cpu.regs.pc.reg),2, 16); _CPU_RESET_BIT(CPU_FLAG_Z);} break;
    /* CCF */
    case 0x3F: _ccf(); break;
    /* SCF */
    case 0x37: _scf(); break;
    /* HALT */
    case 0x76: _halted(); break;
    /* STOP */
    case 0x10: _stop(); break;
    /* DI */
    case 0xF3: _di(); break;
    /* EI */
    case 0xFB: _ei(); break;
    default:
        debug_print("Unhandled Opcode! 0x%02X\n",cpu.cur_opcode);
        exit(-1);
        break;
    }

    if(cpu.pending_for_interupt_disable){
        if(cpu.cur_opcode != 0xF3){
            cpu.pending_for_interupt_disable = false;
        }
    } 
    if(cpu.pending_for_interupt_enable){
        if(cpu.cur_opcode != 0xF8){
            cpu.pending_for_interupt_enable = false;
        }
    }
}

void cpu_decode_and_execute_extended_opcode(void){
    cpu_fetch_opcode();
    switch (cpu.cur_opcode) { case 0x00: break;
    default:
        debug_print("Unhandled Extended Opcode! 0x%02X\n",cpu.cur_opcode);
        exit(-1);
        break;
    }

}

void interupt_service(){
}

void cpu_step(void)
{
    if (cpu.halted != 1) {
        cpu_fetch_opcode();
        cpu_decode_and_execute();
        interupt_service();
    }
}

