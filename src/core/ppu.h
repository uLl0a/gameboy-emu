#ifndef GBEMU_PPU_H_
#define GBEMU_PPU_H_

#include <stdint.h>
#include <stdbool.h>

#define GB_PPU_WIDTH 160
#define GB_PPU_HEIGHT 144
#define GB_PPU_FRAMEBUFFER_SIZE (GB_PPU_WIDTH * GB_PPU_HEIGHT)

#define PPU_MODE_HBLANK 0
#define PPU_MODE_VBLANK 1
#define PPU_MODE_OAM 2
#define PPU_MODE_TRANSFER 3

typedef struct {
    uint8_t vram[0x2000];
    uint8_t oam[0xA0];
    uint8_t framebuffer[GB_PPU_FRAMEBUFFER_SIZE * 4];
    uint8_t palette[4];
    uint8_t bg_palette;
    uint8_t obj_palette_0;
    uint8_t obj_palette_1;
    uint16_t scanline;
    uint16_t cycle;
    uint8_t mode;
    bool enabled;
    uint8_t lcd_control;
    uint8_t lcd_status;
    uint8_t scroll_x;
    uint8_t scroll_y;
    uint8_t window_x;
    uint8_t window_y;
    bool bg_enabled;
    bool window_enabled;
    bool obj_enabled;
} ppu_context_t;

extern ppu_context_t ppu;

void ppu_reset(void);
void ppu_step(uint32_t cycles);
void ppu_render_scanline(void);
void ppu_render_frame(void);
void ppu_set_pixel(uint16_t x, uint16_t y, uint8_t color_index);
void ppu_draw_debug_frame(void);
const uint8_t *ppu_get_framebuffer(void);

#endif // GBEMU_PPU_H_
