#include "bus.h"
#include <stdint.h>

_u8 *bus_read(_u16 address) 
{
    if (Cartridge.bios_active && address < 0x0100U) {
        return &Cartridge.bios[address];
    }

    if (_IS_BETWEEN(address, ROM_BANK_0_START, ROM_BANK_0_END)) {
        return read_rom(address);
    }

    if (_IS_BETWEEN(address, ROM_BANK_1_START, ROM_BANK_1_END)) {
        _u16 bank_offset = (address - ROM_BANK_1_START) & 0x3FFFU;
        _u32 bank_index = ((_u32)Cartridge.current_rom_bank * 0x4000U) + bank_offset;
        return read_game_bank((_u16)bank_index);
    }

    if (_IS_BETWEEN(address, CATRIDGE_EXTERN_RAM_START, CATRIDGE_EXTERN_RAM_END)) {
        _u16 new_address = address - CATRIDGE_EXTERN_RAM_START;
        return read_ram_banks(new_address + (Cartridge.current_ram_bank * RAM_BANK_SIZE));
    }

    if (_IS_BETWEEN(address, VRAM_START, VRAM_END)) {
        return &Cartridge.vram[address - VRAM_START];
    }

    if (_IS_BETWEEN(address, ROM_BANK_0_WRAM_START, ROM_BANK_1_WRAM_END)) {
        return &Cartridge.wram[address - ROM_BANK_0_WRAM_START];
    }

    if (_IS_BETWEEN(address, ECHO_RAM_START, ECHO_RAM_END)) {
        return &Cartridge.wram[address - ECHO_RAM_START];
    }

    if (_IS_BETWEEN(address, SPRITE_ATTRIBUTE_TABLE_OAM_START, SPRITE_ATTRIBUTE_TABLE_OAM_END)) {
        return &Cartridge.oam[address - SPRITE_ATTRIBUTE_TABLE_OAM_START];
    }

    if (_IS_BETWEEN(address, IO_PORTS_START, IO_PORTS_END)) {
        return &Cartridge.io[address - IO_PORTS_START];
    }

    if (_IS_BETWEEN(address, HIGH_RAM_START, HIGH_RAM_END)) {
        return &Cartridge.hram[address - HIGH_RAM_START];
    }

    if (address == 0xFF00U) {
        return &Cartridge.io[0];
    }

    if (address == INTERRUPT_ENABLE_REGISTER_IE) {
        return &Cartridge.ie;
    }

    static _u8 invalid_byte = 0xFF;
    return &invalid_byte;
}

void bus_write(_u16 address, _u8 value)
{
    if (address == 0xFF50U) {
        if (value != 0U && Cartridge.bios_active) {
            debug_print("[BOOT] write to $FF50 = $%02X -> BIOS unmapped, cartridge ROM begins at $0100\n", value);
        }
        Cartridge.bios_active = (value == 0U);
        return;
    }

    if (_IS_BETWEEN(address, ROM_BANK_0_START, ROM_BANK_1_END)) {
        return;
    }

    if (_IS_BETWEEN(address, CATRIDGE_EXTERN_RAM_START, CATRIDGE_EXTERN_RAM_END)) {
        _u16 new_address = address - CATRIDGE_EXTERN_RAM_START;
        write_ram_banks(new_address + (Cartridge.current_ram_bank * RAM_BANK_SIZE), value);
        return;
    }

    if (_IS_BETWEEN(address, VRAM_START, VRAM_END)) {
        Cartridge.vram[address - VRAM_START] = value;
        return;
    }

    if (_IS_BETWEEN(address, ROM_BANK_0_WRAM_START, ROM_BANK_1_WRAM_END)) {
        Cartridge.wram[address - ROM_BANK_0_WRAM_START] = value;
        return;
    }

    if (_IS_BETWEEN(address, ECHO_RAM_START, ECHO_RAM_END)) {
        Cartridge.wram[address - ECHO_RAM_START] = value;
        return;
    }

    if (_IS_BETWEEN(address, SPRITE_ATTRIBUTE_TABLE_OAM_START, SPRITE_ATTRIBUTE_TABLE_OAM_END)) {
        Cartridge.oam[address - SPRITE_ATTRIBUTE_TABLE_OAM_START] = value;
        return;
    }

    if (_IS_BETWEEN(address, NOT_USABLE_MEMORY_START, NOT_USABLE_MEMORY_END)) {
        return;
    }

    if (_IS_BETWEEN(address, IO_PORTS_START, IO_PORTS_END)) {
        Cartridge.io[address - IO_PORTS_START] = value;
        return;
    }

    if (address == 0xFF00U) {
        Cartridge.io[0] = value;
        return;
    }

    if (_IS_BETWEEN(address, HIGH_RAM_START, HIGH_RAM_END)) {
        Cartridge.hram[address - HIGH_RAM_START] = value;
        return;
    }

    if (address == INTERRUPT_ENABLE_REGISTER_IE) {
        Cartridge.ie = value;
        return;
    }
}

const _u16 _16bit_bus_read(_u16 address){
    const _u8 *lo = bus_read(address);
    const _u8 *hi = bus_read(address + 1U);
    return (const _u16)((_u16)(*lo) | ((_u16)(*hi) << 8U));
}

void _16bit_bus_write(_u16 address, _u16 value){
    bus_write(address, (value & 0xFFU));
    bus_write(address + 1U, (value >> 8U));
}

