#include "rom.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bus.h"
#include "useful_utils.h"

struct Cartridge_context_t Cartridge = {0};

static bool file_exists(const char *path)
{
    FILE *f = NULL;
    if (path == NULL || path[0] == '\0') {
        return false;
    }

    if (fopen_s(&f, path, "rb") != 0 || f == NULL) {
        return false;
    }

    fclose(f);
    return true;
}

static bool resolve_candidate_path(char *out_path, size_t out_size, const char *input_path)
{
    if (out_path == NULL || out_size == 0 || input_path == NULL || input_path[0] == '\0') {
        return false;
    }

    if (file_exists(input_path)) {
        snprintf(out_path, out_size, "%s", input_path);
        return true;
    }

    const char *base = strrchr(input_path, '\\');
    if (base == NULL) {
        base = strrchr(input_path, '/');
    }
    const char *filename = (base != NULL) ? (base + 1) : input_path;

    const char *fallbacks[] = {
        "DMG_ROM.bin",
        "./DMG_ROM.bin",
        ".\\DMG_ROM.bin",
        "bios/DMG_ROM.bin",
        "bios\\DMG_ROM.bin",
        "../bios/DMG_ROM.bin",
        "..\\bios\\DMG_ROM.bin",
        "../../bios/DMG_ROM.bin",
        "..\\..\\bios\\DMG_ROM.bin",
        "../../build/Debug/DMG_ROM.bin",
        "..\\..\\build\\Debug\\DMG_ROM.bin"
    };

    for (size_t i = 0; i < sizeof(fallbacks) / sizeof(fallbacks[0]); ++i) {
        if (file_exists(fallbacks[i])) {
            snprintf(out_path, out_size, "%s", fallbacks[i]);
            return true;
        }
    }

    if (filename != NULL && filename != input_path) {
        const char *base_only = filename;
        const char *extra_fallbacks[] = {
            base_only,
            "./",
            ".\\",
            "../",
            "..\\",
            "../../",
            "..\\..\\"
        };

        for (size_t i = 0; i < sizeof(extra_fallbacks) / sizeof(extra_fallbacks[0]); ++i) {
            char candidate[2048];
            snprintf(candidate, sizeof(candidate), "%s%s", extra_fallbacks[i], base_only);
            if (file_exists(candidate)) {
                snprintf(out_path, out_size, "%s", candidate);
                return true;
            }
        }
    }

    return false;
}

void load_bios_from_path(const _s8 filename[1024])
{
    char resolved_path[2048] = {0};
    if (!resolve_candidate_path(resolved_path, sizeof(resolved_path), filename)) {
        debug_print("[ERR] BIOS could not be opened: %s\n", filename);
        exit(-1);
    }

    FILE *f = NULL;
    if (fopen_s(&f, resolved_path, "rb") != 0 || f == NULL) {
        debug_print("[ERR] BIOS could not be opened: %s\n", filename);
        exit(-1);
    }

    memset(Cartridge.bios, 0x00, sizeof(Cartridge.bios));
    size_t read_bytes = fread(Cartridge.bios, 1, sizeof(Cartridge.bios), f);
    fclose(f);

    if (read_bytes == 0) {
        debug_print("[ERR] BIOS file is empty: %s\n", resolved_path);
        exit(-1);
    }

    Cartridge.bios_active = true;
    Cartridge.boot_handoff_logged = false;
    debug_print("[+] BIOS loaded from %s\n", resolved_path);
}

void load_bios(void)
{
    if (Cartridge.bios_active) {
        return;
    }

    const char *bios_paths[] = {
        "bios\\DMG_ROM.bin",
        "bios/DMG_ROM.bin",
        "..\\bios\\DMG_ROM.bin",
        "..//bios//DMG_ROM.bin",
        "..\\..\\bios\\DMG_ROM.bin",
        "..\\..\\build\\Debug\\DMG_ROM.bin",
        "DMG_ROM.bin",
        ".\\DMG_ROM.bin"
    };

    for (size_t i = 0; i < sizeof(bios_paths) / sizeof(bios_paths[0]); ++i) {
        FILE *f = NULL;
        if (fopen_s(&f, bios_paths[i], "rb") == 0) {
            fread(Cartridge.bios, 1, sizeof(Cartridge.bios), f);
            fclose(f);
            Cartridge.bios_active = true;
            debug_print("[+] BIOS loaded from %s\n", bios_paths[i]);
            return;
        }
    }

    debug_print("[WARN] BIOS not found; boot sequence will start directly at cartridge entry 0x0100.\n");
}

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
    if (!Cartridge.bios_active) {
        load_bios();
    }

    char resolved_path[2048] = {0};
    if (!resolve_candidate_path(resolved_path, sizeof(resolved_path), filename)) {
        debug_print("[ERR] Failed to open: %s\n", filename);
        exit(-1);
    }

    FILE *f = NULL;
    if (fopen_s(&f, resolved_path, "rb") != 0 || f == NULL) {
        debug_print("[ERR] Failed to open: %s\n", resolved_path);
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

void memory_reset(void)
{
    memset(Cartridge.ram_banks, 0, sizeof(Cartridge.ram_banks));
    memset(Cartridge.vram, 0, sizeof(Cartridge.vram));
    memset(Cartridge.wram, 0, sizeof(Cartridge.wram));
    memset(Cartridge.oam, 0, sizeof(Cartridge.oam));
    memset(Cartridge.io, 0, sizeof(Cartridge.io));
    memset(Cartridge.hram, 0, sizeof(Cartridge.hram));
    Cartridge.ie = 0x00;
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
    memory_reset();
    debug_print("[-] ROM Unloaded.\n");
}
