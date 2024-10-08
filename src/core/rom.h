#ifndef GBEMU_ROM_ROM_H_
#define GBEMU_ROM_ROM_H_
#include <stdio.h>
#include "cpu.h"

#define RAM_BANK_SIZE 0x2000
#define MAX_RAM_BANKS 0x8000

/* Ref: https://gbdev.io/pandocs/The_Cartridge_Header.html */
typedef struct {
    _u8 entry[4];
    _u8 logo[0x30];

    _s8 title[16];
    _u16 new_lic_code;
    _u8 sgb_flag;
    _u8 type;
    _u8 rom_size;
    _u8 ram_size;
    _u8 dest_code;
    _u8 lic_code;
    _u8 version;
    _u8 checksum;
    _u16 global_checksum;
} Cartridge_header_t;

struct Cartridge_context_t{
    _s8 filename[1024];
    size_t rom_size;
    _u8 game_bank[0x200000];
    _u8 rom_data[0x100000];
    _u8 ram_banks[MAX_RAM_BANKS];
    _u8 current_rom_bank;
    _u8 current_ram_bank;
    Cartridge_header_t *header;
};

extern struct Cartridge_context_t Cartridge;

_u8 *read_rom(_u16 address);
void write_rom(_u16 address, _u8 value);

_u8 *read_ram_banks(_u16 address);
void write_ram_banks(_u16 address, _u8 value);

_u8 *read_game_bank(_u16 address);
void write_game_bank(_u16 address, _u8 value);

void load_rom(const _s8 filename[256]);
void unload_rom(void);

/* https://raw.githubusercontent.com/gb-archive/salvage/master/txt-files/gbrom.txt
 */

static const char* ROM_SGB_TYPE[] = {
    [0x00] = "No SGB functions (Normal Game Boy or CGB only game)",
    [0x03] = "Game support SGB functions",
};

static const char* RAM_SIZE[] = {
    [0x00] = "0KBytes No RAM",
    [0x01] = "2KBytes Unused",
    [0x02] = "8KBytes 1 bank",
    [0x03] = "32KBytes 4 banks of 8 KB each",
    [0x04] = "128KBytes 16 banks of 8 KB each",
    [0x05] = "64KBytes 8 banks of 8 KB each",
};

static const char *ROM_TYPES[] = {
    [0x00] = "ROM ONLY",
    [0x01] = "MBC1",
    [0x02] = "MBC1+RAM",
    [0x03] = "MBC1+RAM+BATTERY",
    [0x05] = "MBC2",
    [0x06] = "MBC2+BATTERY",
    [0x08] = "ROM+RAM",
    [0x09] = "ROM+RAM+BATTERY",
    [0x0B] = "MMMO1",
    [0x0C] = "MMMO1+RAM",
    [0x0D] = "MMMO1+RAM+BATTERY",
    [0x0F] = "MBC3+TIMER+BATTERY",
    [0x10] = "MBC3+TIMER+RAM+BATTERY",
    [0x11] = "MBC3",
    [0x12] = "MBC3+RAM",
    [0x13] = "MBC3+RAM+BATTERY",
    [0x19] = "MBC5",
    [0x1A] = "MBC5+RAM",
    [0x1B] = "MBC5+RAM+BATTERY",
    [0x1C] = "MBC5+RUMBLE",
    [0x1D] = "MBC5+RUMBLE+RAM",
    [0x1E] = "MBC5+RUMBLE+RAM+BATTERY",
    [0x20] = "MBC6",
    [0x22] = "MBC7+SENSOR+RUMBLE+RAM+BATTERY",
    [0xFC] = "POCKET CAMERA",
    [0xFD] = "BANDAI TAMA5",
    [0xFE] = "HuC3",
    [0xFF] = "HuC1+RAM+BATTERY",
};

static const char *LIC_CODE[0xA5] = {
    [0x00] = "None",
    [0x01] = "Nintendo R&D1",
    [0x08] = "Capcom",
    [0x13] = "Electronic Arts",
    [0x18] = "Hudson Soft",
    [0x19] = "b-ai",
    [0x20] = "kss",
    [0x22] = "pow",
    [0x24] = "PCM Complete",
    [0x25] = "san-x",
    [0x28] = "Kemco Japan",
    [0x29] = "seta",
    [0x30] = "Viacom",
    [0x31] = "Nintendo",
    [0x32] = "Bandai",
    [0x33] = "Ocean/Acclaim",
    [0x34] = "Konami",
    [0x35] = "Hector",
    [0x37] = "Taito",
    [0x38] = "Hudson",
    [0x39] = "Banpresto",
    [0x41] = "Ubi Soft",
    [0x42] = "Atlus",
    [0x44] = "Malibu",
    [0x46] = "angel",
    [0x47] = "Bullet-Proof",
    [0x49] = "irem",
    [0x50] = "Absolute",
    [0x51] = "Acclaim",
    [0x52] = "Activision",
    [0x53] = "American sammy",
    [0x54] = "Konami",
    [0x55] = "Hi tech entertainment",
    [0x56] = "LJN",
    [0x57] = "Matchbox",
    [0x58] = "Mattel",
    [0x59] = "Milton Bradley",
    [0x60] = "Titus",
    [0x61] = "Virgin",
    [0x64] = "LucasArts",
    [0x67] = "Ocean",
    [0x69] = "Electronic Arts",
    [0x70] = "Infogrames",
    [0x71] = "Interplay",
    [0x72] = "Broderbund",
    [0x73] = "sculptured",
    [0x75] = "sci",
    [0x78] = "THQ",
    [0x79] = "Accolade",
    [0x80] = "misawa",
    [0x83] = "lozc",
    [0x86] = "Tokuma Shoten Intermedia",
    [0x87] = "Tsukuda Original",
    [0x91] = "Chunsoft",
    [0x92] = "Video system",
    [0x93] = "Ocean/Acclaim",
    [0x95] = "Varie",
    [0x96] = "Yonezawa/s’pal",
    [0x97] = "Kaneko",
    [0x99] = "Pack in soft",
    [0xA4] = "Konami (Yu-Gi-Oh!)"
};

#endif  // GBEMU_ROM_ROM_H_
