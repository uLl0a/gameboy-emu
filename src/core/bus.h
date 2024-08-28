#ifndef GBEMU_CPU_BUS_H_
#define GBEMU_CPU_BUS_H_
#include "rom.h"
#include "cpu.h"
/*
                    Gameboy general memory map
   0000-3FFF   16KB ROM Bank 00     (in cartridge, fixed at bank 00)
   4000-7FFF   16KB ROM Bank 01..NN (in cartridge, switchable bank number)
   8000-9FFF   8KB Video RAM (VRAM) (switchable bank 0-1 in CGB Mode)
   A000-BFFF   8KB External RAM     (in cartridge, switchable bank, if any)
   C000-CFFF   4KB Work RAM Bank 0 (WRAM)
   D000-DFFF   4KB Work RAM Bank 1 (WRAM)  (switchable bank 1-7 in CGB Mode)
   E000-FDFF   Same as C000-DDFF (ECHO)    (typically not used)
   FE00-FE9F   Sprite Attribute Table (OAM)
   FEA0-FEFF   Not Usable
   FF00-FF7F   I/O Ports
   FF80-FFFE   High RAM (HRAM)
   FFFF        Interrupt Enable Register
*/
#define CATRIDGE_START 0x0000
#define CATRIDGE_END 0xFFFF

#define CATRIDGE_HEADER_START 0x100
#define CATRIDGE_HEADER_END 0x14F

/* 16KB ROM Bank 00     (in cartridge, fixed at bank 00) */
#define ROM_BANK_0_START 0x0000
#define ROM_BANK_0_END 0x3FFF

/* 16KB ROM Bank 01..NN (in cartridge, switchable bank number) */
#define ROM_BANK_1_START 0x4000
#define ROM_BANK_1_END 0x7FFF

/* 8KB Video RAM (VRAM) (switchable bank 0-1 in CGB Mode) */
#define VRAM_START 0x8000
#define VRAM_END 0x9FFF

/* 8KB External RAM     (in cartridge, switchable bank, if any) */
#define CATRIDGE_EXTERN_RAM_START 0xA000
#define CATRIDGE_EXTERN_RAM_END 0xBFFF

 /* 4KB Work RAM Bank 0 (WRAM) */
#define ROM_BANK_0_WRAM_START 0xC000
#define ROM_BANK_0_WRAM_END 0xCFFF

/* 4KB Work RAM Bank 1 (WRAM)  (switchable bank 1-7 in CGB Mode) */
#define ROM_BANK_1_WRAM_START 0xD000
#define ROM_BANK_1_WRAM_END 0xDFFF

/* Same as C000-DDFF (ECHO)   (typically not used, nintendo says use of this area is prohibited )*/
#define ECHO_RAM_START 0xE000
#define ECHO_RAM_END 0xFDFF

/* Sprite Attribute Table (OAM) */
#define SPRITE_ATTRIBUTE_TABLE_OAM_START 0xFE00
#define SPRITE_ATTRIBUTE_TABLE_OAM_END 0xFE9F

/* Not Usable  (Nintendo says use of this area is prohibited) */
#define NOT_USABLE_MEMORY_START 0xFEA0
#define NOT_USABLE_MEMORY_END 0xFEFF

/* I/O Ports */
#define IO_PORTS_START 0xFF00
#define IO_PORTS_END 0xFF7F

/* High RAM (HRAM) */
#define HIGH_RAM_START 0xFF80
#define HIGH_RAM_END 0xFFFE

/* Interrupt Enable register (IE) */
#define INTERRUPT_ENABLE_REGISTER_IE 0xFFFF

_u8 *bus_read(_u16 address);
void bus_write(_u16 address, _u8 value);

const _u16 _16bit_bus_read(_u16 address);
void _16bit_bus_write(_u16 address, _u16 value);

#endif  // GBEMU_CPU_BUS_H_
