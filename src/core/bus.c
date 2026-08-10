#include "bus.h"

_u8 *bus_read(_u16 address) 
{
    if(_IS_BETWEEN(address, ROM_BANK_0_START, ROM_BANK_1_END)){
        _u16 new_address = address - ROM_BANK_1_START;
        return read_game_bank(new_address + (Cartridge.current_rom_bank * ROM_BANK_1_START));
    }

    else if(_IS_BETWEEN(address, CATRIDGE_EXTERN_RAM_START, CATRIDGE_EXTERN_RAM_END)){
        _u16 new_address = address - CATRIDGE_EXTERN_RAM_START;
        return read_ram_banks(new_address + (Cartridge.current_ram_bank * RAM_BANK_SIZE));
    }

    return read_rom(address);
}

void bus_write(_u16 address, _u8 value)
{
    if (address <= ROM_BANK_1_END) {
        /* Its read only memory */
    } 

	// we're right to internal RAM, remember that it needs to echo it
    else if(_IS_BETWEEN(address, ROM_BANK_0_WRAM_START, ROM_BANK_1_WRAM_END)){
        write_rom(address, value);
    }

    else if (_IS_BETWEEN(address, ECHO_RAM_START, ECHO_RAM_END)) {
        /* writing to echo ram also write in ram */
        write_rom(address, value);
        write_rom(address - RAM_BANK_SIZE, value);
    }

    else if (_IS_BETWEEN(address, NOT_USABLE_MEMORY_START, NOT_USABLE_MEMORY_END)) {
        /* its a restricted area */
    } 


    else {
        /* No control here so write it */
        write_rom(address, value);
    }
}

const _u16 _16bit_bus_read(_u16 address){
    const _u8 *lo = bus_read(address);
    const _u8 *hi = bus_read(address + 1);
    return *lo | ( *hi << 8 );
}

void _16bit_bus_write(_u16 address, _u16 value){
    bus_write(address, (value & 8));
    bus_write(address + 1, (value >> 8));
}

