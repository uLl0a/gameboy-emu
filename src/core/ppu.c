#include "ppu.h"
#include "rom.h"
#include <string.h>
#include <stdio.h>

ppu_context_t ppu = {0};
static uint8_t background_pixels[GB_PPU_WIDTH * GB_PPU_HEIGHT];
static uint32_t ppu_log_counter = 0U;

static const uint8_t palette_colors[4][4] = {
    { 0x9B, 0xBC, 0x0F, 0xFF },
    { 0x8B, 0xAC, 0x0F, 0xFF },
    { 0x30, 0x62, 0x30, 0xFF },
    { 0x0F, 0x38, 0x0F, 0xFF }
};

static void ppu_clear_framebuffer(void)
{
    memset(ppu.framebuffer, 0, sizeof(ppu.framebuffer));
    memset(background_pixels, 0, sizeof(background_pixels));
}

static bool ppu_vram_is_empty(void)
{
    for (size_t i = 0; i < sizeof(Cartridge.vram); ++i) {
        if (Cartridge.vram[i] != 0U) {
            return false;
        }
    }
    return true;
}

static uint8_t ppu_get_palette_color(uint8_t palette, uint8_t color_index)
{
    return (uint8_t)((palette >> (color_index * 2U)) & 0x03U);
}

static uint16_t ppu_pattern_offset_for_tile_id(int16_t tile_id, uint16_t row_offset)
{
    uint16_t offset = 0U;

    if ((ppu.lcd_control & 0x10U) != 0U) {
        offset = (uint16_t)((uint16_t)tile_id * 16U);
    } else {
        offset = (uint16_t)(((uint16_t)((int16_t)tile_id + 128) * 16U));
    }

    return (uint16_t)(offset + row_offset);
}

static uint8_t ppu_get_tile_color_from_pattern(int16_t tile_id, uint16_t tile_x, uint16_t tile_y)
{
    uint16_t pattern_offset = ppu_pattern_offset_for_tile_id(tile_id, tile_y * 2U);
    uint8_t low = Cartridge.vram[pattern_offset];
    uint8_t high = Cartridge.vram[pattern_offset + 1U];
    uint8_t bit = (uint8_t)(7U - tile_x);
    return (uint8_t)(((low >> bit) & 0x01U) | (((high >> bit) & 0x01U) << 1U));
}

static uint8_t ppu_get_background_tile_id(uint16_t map_base, uint16_t tile_col, uint16_t tile_row)
{
    uint16_t map_index = (uint16_t)((tile_row * 32U) + tile_col);
    return Cartridge.vram[(map_base - 0x8000U) + map_index];
}

static uint8_t ppu_get_window_pixel_color(uint16_t x, uint16_t y)
{
    if ((ppu.lcd_control & 0x20U) == 0U || x < (ppu.window_x - 7U) || y < ppu.window_y) {
        return 0U;
    }

    uint16_t window_x = x - (ppu.window_x - 7U);
    uint16_t window_y = y - ppu.window_y;
    uint16_t tile_col = (window_x >> 3U) & 0x1FU;
    uint16_t tile_row = (window_y >> 3U) & 0x1FU;
    uint16_t tile_x = window_x & 0x07U;
    uint16_t tile_y = window_y & 0x07U;
    uint16_t map_base = (ppu.lcd_control & 0x40U) ? 0x9C00U : 0x9800U;
    uint8_t tile_id = ppu_get_background_tile_id(map_base, tile_col, tile_row);
    int16_t signed_tile_id = (int16_t)tile_id;
    if ((ppu.lcd_control & 0x10U) == 0U) {
        signed_tile_id = (int16_t)(int8_t)tile_id;
    }

    return ppu_get_tile_color_from_pattern(signed_tile_id, tile_x, tile_y);
}

void ppu_draw_debug_frame(void)
{
    ppu_clear_framebuffer();
    for (uint16_t y = 0; y < GB_PPU_HEIGHT; ++y) {
        for (uint16_t x = 0; x < GB_PPU_WIDTH; ++x) {
            uint8_t color_index = 0U;
            if (x < 8U || y < 8U || x >= GB_PPU_WIDTH - 8U || y >= GB_PPU_HEIGHT - 8U) {
                color_index = 3U;
            } else if ((x + y) % 24U < 12U) {
                color_index = 2U;
            } else {
                color_index = 1U;
            }
            ppu_set_pixel(x, y, color_index);
        }
    }
}

static void ppu_init_background_pattern(void)
{
    /* Keep VRAM clear unless the ROM/boot sequence initializes it.
       The real hardware VRAM lives in the bus memory map, not in a detached
       local mirror. The BIOS writes directly to Cartridge.vram, so we must keep
       that canonical memory visible to the PPU. */
    memset(Cartridge.vram, 0, sizeof(Cartridge.vram));
}

static void ppu_render_background_full(void)
{
    if ((ppu.lcd_control & 0x80U) == 0U) {
        memset(background_pixels, 0, sizeof(background_pixels));
        return;
    }

    for (uint16_t y = 0; y < GB_PPU_HEIGHT; ++y) {
        for (uint16_t x = 0; x < GB_PPU_WIDTH; ++x) {
            uint16_t screen_index = (uint16_t)(y * GB_PPU_WIDTH + x);
            uint8_t color_index = 0U;

            if ((ppu.lcd_control & 0x20U) != 0U && ppu.window_enabled && x >= (ppu.window_x - 7U) && y >= ppu.window_y) {
                color_index = ppu_get_window_pixel_color(x, y);
            } else if ((ppu.lcd_control & 0x01U) != 0U && ppu.bg_enabled) {
                uint16_t screen_y = (uint16_t)(y + ppu.scroll_y);
                uint16_t screen_x = (uint16_t)(x + ppu.scroll_x);
                uint16_t tile_row = (screen_y >> 3U) & 0x1FU;
                uint16_t tile_col = (screen_x >> 3U) & 0x1FU;
                uint16_t tile_y = screen_y & 0x07U;
                uint16_t tile_x = screen_x & 0x07U;
                uint16_t map_base = (ppu.lcd_control & 0x08U) ? 0x9C00U : 0x9800U;
                uint8_t tile_id = ppu_get_background_tile_id(map_base, tile_col, tile_row);
                int16_t signed_tile_id = (int16_t)tile_id;
                if ((ppu.lcd_control & 0x10U) == 0U) {
                    signed_tile_id = (int16_t)(int8_t)tile_id;
                }
                color_index = ppu_get_tile_color_from_pattern(signed_tile_id, tile_x, tile_y);
            }

            background_pixels[screen_index] = color_index;
            ppu_set_pixel(x, y, ppu_get_palette_color(ppu.bg_palette, color_index));
        }
    }
}

static void ppu_render_sprites(void)
{
    if (!ppu.obj_enabled) {
        return;
    }

    const uint8_t obj_size = (ppu.lcd_control & 0x04U) ? 16U : 8U;
    uint8_t sprite_count = 0U;

    for (int sprite_index = 0; sprite_index < 40 && sprite_count < 10; ++sprite_index) {
        int offset = sprite_index * 4;
        uint8_t y_pos = ppu.oam[offset + 0U];
        uint8_t x_pos = ppu.oam[offset + 1U];
        uint8_t tile_id = ppu.oam[offset + 2U];
        uint8_t flags = ppu.oam[offset + 3U];

        int sprite_y = (int)y_pos - 16;
        int sprite_x = (int)x_pos - 8;
        if (sprite_y < -16 || sprite_x < -8 || sprite_x >= GB_PPU_WIDTH || y_pos == 0U || y_pos >= 160U) {
            continue;
        }

        bool flip_x = (flags & 0x20U) != 0U;
        bool flip_y = (flags & 0x40U) != 0U;
        bool use_palette_1 = (flags & 0x10U) != 0U;
        bool priority = (flags & 0x80U) != 0U;
        uint8_t palette = use_palette_1 ? ppu.obj_palette_1 : ppu.obj_palette_0;

        sprite_count += 1U;

        for (uint8_t row = 0; row < obj_size; ++row) {
            int screen_y = sprite_y + (int)(flip_y ? (obj_size - 1U - row) : row);
            if (screen_y < 0 || screen_y >= GB_PPU_HEIGHT) {
                continue;
            }

            uint8_t tile_row = (uint8_t)(flip_y ? (obj_size - 1U - row) : row);
            uint8_t tile_pattern = tile_id;
            if (obj_size == 16U) {
                tile_pattern = (uint8_t)(tile_id & 0xFEU);
                if (row >= 8U) {
                    tile_pattern = (uint8_t)(tile_pattern | 0x01U);
                }
            }

            for (uint8_t col = 0; col < 8U; ++col) {
                int screen_x = sprite_x + (int)(flip_x ? (7U - col) : col);
                if (screen_x < 0 || screen_x >= GB_PPU_WIDTH) {
                    continue;
                }

                uint8_t color_index = ppu_get_tile_color_from_pattern((int16_t)tile_pattern, (flip_x ? (7U - col) : col), tile_row & 0x07U);
                if (color_index == 0U) {
                    continue;
                }

                uint8_t bg_color = background_pixels[(uint32_t)screen_y * GB_PPU_WIDTH + (uint32_t)screen_x];
                if (priority && bg_color != 0U) {
                    continue;
                }

                uint8_t final_color = ppu_get_palette_color(palette, color_index);
                ppu_set_pixel((uint16_t)screen_x, (uint16_t)screen_y, final_color);
            }
        }
    }
}

void ppu_render_frame(void)
{
    ppu_clear_framebuffer();

    if (ppu_vram_is_empty()) {
        return;
    }

    ppu_render_background_full();
    ppu_render_sprites();
}

void ppu_reset(void)
{
    memset(&ppu, 0, sizeof(ppu));
    ppu.enabled = true;
    ppu.mode = PPU_MODE_OAM;
    ppu.lcd_control = 0x91;
    ppu.bg_palette = 0xE4;
    ppu.obj_palette_0 = 0xE4;
    ppu.obj_palette_1 = 0xE4;
    ppu.palette[0] = 0x3;
    ppu.palette[1] = 0x2;
    ppu.palette[2] = 0x1;
    ppu.palette[3] = 0x0;
    ppu.bg_enabled = true;
    ppu.window_enabled = true;
    ppu.obj_enabled = true;
    ppu.window_x = 7U;
    ppu.window_y = 0U;
    ppu_log_counter = 0U;
    ppu_clear_framebuffer();
    ppu_init_background_pattern();
    printf("[PPU] reset -> black framebuffer until VRAM is initialized by BIOS/cart ROM\n");
}

void ppu_set_pixel(uint16_t x, uint16_t y, uint8_t color_index)
{
    if (x >= GB_PPU_WIDTH || y >= GB_PPU_HEIGHT) {
        return;
    }

    const uint8_t *color = palette_colors[color_index & 0x3U];
    uint32_t offset = (((uint32_t)y * GB_PPU_WIDTH) + x) * 4U;
    ppu.framebuffer[offset + 0U] = color[0];
    ppu.framebuffer[offset + 1U] = color[1];
    ppu.framebuffer[offset + 2U] = color[2];
    ppu.framebuffer[offset + 3U] = color[3];
}

void ppu_render_scanline(void)
{
    if (!ppu.enabled || ppu.scanline >= GB_PPU_HEIGHT) {
        return;
    }

    for (uint16_t x = 0; x < GB_PPU_WIDTH; ++x) {
        uint8_t color_index = 0U;
        uint16_t y = ppu.scanline;

        if ((ppu.lcd_control & 0x20U) != 0U && x >= (ppu.window_x - 7U) && y >= ppu.window_y) {
            color_index = ppu_get_window_pixel_color(x, y);
        } else if ((ppu.lcd_control & 0x01U) != 0U) {
            uint16_t screen_y = (uint16_t)(y + ppu.scroll_y);
            uint16_t screen_x = (uint16_t)(x + ppu.scroll_x);
            uint16_t tile_row = (screen_y >> 3U) & 0x1FU;
            uint16_t tile_col = (screen_x >> 3U) & 0x1FU;
            uint16_t tile_y = screen_y & 0x07U;
            uint16_t tile_x = screen_x & 0x07U;
            uint16_t map_base = (ppu.lcd_control & 0x08U) ? 0x9C00U : 0x9800U;
            uint8_t tile_id = ppu_get_background_tile_id(map_base, tile_col, tile_row);
            int16_t signed_tile_id = (int16_t)tile_id;
            if ((ppu.lcd_control & 0x10U) == 0U) {
                signed_tile_id = (int16_t)(int8_t)tile_id;
            }
            color_index = ppu_get_tile_color_from_pattern(signed_tile_id, tile_x, tile_y);
        }

        ppu_set_pixel(x, y, ppu_get_palette_color(ppu.bg_palette, color_index));
    }
}

void ppu_step(uint32_t cycles)
{
    if (!ppu.enabled) {
        return;
    }

    uint8_t previous_mode = ppu.mode;
    uint16_t previous_scanline = ppu.scanline;

    ppu.cycle += (uint16_t)cycles;
    if (ppu.cycle >= 456U) {
        ppu.cycle -= 456U;
        ppu.scanline += 1U;
        if (ppu.scanline >= 154U) {
            ppu.scanline = 0U;
        }
    }

    if (ppu.scanline < GB_PPU_HEIGHT) {
        if (ppu.cycle < 80U) {
            ppu.mode = PPU_MODE_OAM;
        } else if (ppu.cycle < 252U) {
            ppu.mode = PPU_MODE_TRANSFER;
            ppu_render_scanline();
        } else {
            ppu.mode = PPU_MODE_HBLANK;
        }
    } else {
        ppu.mode = PPU_MODE_VBLANK;
    }

    if (ppu.mode != previous_mode || ppu.scanline != previous_scanline) {
#ifdef GBEMU_VERBOSE
        ppu_log_counter += 1U;
        if (ppu_log_counter % 120U == 0U) {
            printf("[PPU] scanline=%u mode=%d cycle=%u\n", ppu.scanline, ppu.mode, ppu.cycle);
        }
#endif
    }
}

const uint8_t *ppu_get_framebuffer(void)
{
    return ppu.framebuffer;
}
