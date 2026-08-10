#include "rom.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bus.h"
#include "useful_utils.h"

struct Cartridge_context_t Cartridge = {0};

_u8 *read_rom(_u16 address)
{
    return &(Cartridge.rom_data[address]);
}

void write_rom(_u16 address, _u8 value)
{
    Cartridge.rom_data[address] = value;
}

_u8 *read_game_bank(_u16 address){
    return &(Cartridge.game_bank[address]);
}

void write_game_bank(_u16 address, _u8 value)
{
    Cartridge.game_bank[address] = value;
}

_u8 *read_ram_banks(_u16 address){
    return &(Cartridge.ram_banks[address]);
}

void write_ram_banks(_u16 address, _u8 value){
    Cartridge.ram_banks[address] = value;
}

void load_rom(const _s8 filename[1024])
{
    FILE *f;
    fopen_s(&f, filename, "rb");

    if (!f) {
        debug_print("[ERR] Failed to open: %s\n", filename);
        exit(-1);
    }
    fseek(f, 0L, SEEK_END);
    Cartridge.rom_size = ftell(f);
    fseek(f, 0L, SEEK_SET);
    fread(&Cartridge.game_bank,1,0x200000, f);
    fclose(f);

    memcpy(Cartridge.filename, filename, sizeof(Cartridge.filename));
    memcpy(&Cartridge.rom_data, &Cartridge.game_bank[0], 0x8000);
    memset(&Cartridge.ram_banks,0, sizeof(Cartridge.ram_banks));
    Cartridge.current_rom_bank = 1;
    Cartridge.current_ram_bank = 0;

    /* Getting some info about the rom (Ref:
     * https://gbdev.io/pandocs/The_Cartridge_Header.html) */
    Cartridge.header = (Cartridge_header_t *) (Cartridge.game_bank + CATRIDGE_HEADER_START);
    Cartridge.header->title[15] = '\0';
    debug_print("[+] Cartridge loaded : %s\n", Cartridge.filename);
    debug_print("\tNintendo logo:");
    for(_u8 i = 0; i < 0x30; ++i){
        debug_print(" %02X",Cartridge.header->logo[i]);
    }
    debug_print("\n\tROM Name title : (%s)\n", Cartridge.header->title);
    debug_print("\tROM Type     : %02X (%s)\n", Cartridge.header->type, ROM_TYPES[Cartridge.header->type]);
    debug_print("\tROM Size     : %dKBytes (%d banks)\n", (32 << Cartridge.header->rom_size), (32 << Cartridge.header->rom_size) / 16); // Note the conversing from kb to banks just work if the rom size is less that 512Kbytes.
    debug_print("\tRAM Size     : %02X (%s)\n", Cartridge.header->ram_size, RAM_SIZE[Cartridge.header->ram_size]);
    debug_print("\tSGB Flag     : %02X (%s)\n", Cartridge.header->sgb_flag, ROM_SGB_TYPE[Cartridge.header->sgb_flag]);
    debug_print("\tOld Licensee : %02X (%s)\n", Cartridge.header->lic_code, LIC_CODE[Cartridge.header->lic_code]);
    debug_print("\tCountry code : %02X (%s)\n", Cartridge.header->dest_code, (!Cartridge.header->dest_code) ? "Japan" : "Non-Japan");
    debug_print("\tMask ROM Version : %02X\n", Cartridge.header->version);
    /*Calculating the header checksum*/
    _u16 checksum = 0, i = 0;
    for (i = 0x0134; i <= 0x014C; i++)
        checksum = checksum - Cartridge.game_bank[i] - 1;
    debug_print("\tHeader Checksum  : %02X (%s)\n",Cartridge.header->checksum, (checksum & 0xFF) ? "OK" : "FAIL");
    debug_print("\tGlobal Checksum  : %04X (The Gameboy doesn't verify this checksum)\n", Cartridge.header->global_checksum);
}

void unload_rom(void)
{
    Cartridge.header = NULL;
    Cartridge.current_ram_bank = 0;
    Cartridge.current_rom_bank = 0;
    Cartridge.rom_size = 0;
    memset(Cartridge.filename, 0, sizeof(Cartridge.filename));
    memset(Cartridge.ram_banks,0, MAX_RAM_BANKS);
    memset(Cartridge.game_bank,0, sizeof(Cartridge.game_bank));
    memset(Cartridge.rom_data,0, sizeof(Cartridge.rom_data));
    debug_print("[-] ROM Unloaded.\n");
}
