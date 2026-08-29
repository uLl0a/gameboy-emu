#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#define SDL_MAIN_HANDLED
#include "SDL.h"
#include "rom.h"
#include "cpu.h"
#include "ppu.h"
#include "apu.h"
#include "useful_utils.h"

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Texture *texture = NULL;
static SDL_AudioDeviceID audio_device = 0;

static void audio_callback(void *userdata, Uint8 *stream, int len)
{
    (void)userdata;
    int16_t *samples = (int16_t *)stream;
    int sample_count = len / (int)sizeof(int16_t);
    apu_generate_samples(samples, (size_t)sample_count);
}

static bool init_gui(void)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        printf("SDL_Init failed: %s\n", SDL_GetError());
        return false;
    }

    window = SDL_CreateWindow(
        "Game Boy Emulator",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        GB_PPU_WIDTH * 3,
        GB_PPU_HEIGHT * 3,
        SDL_WINDOW_SHOWN
    );
    if (!window) {
        printf("SDL_CreateWindow failed: %s\n", SDL_GetError());
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        printf("SDL_CreateRenderer failed: %s\n", SDL_GetError());
        return false;
    }

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, GB_PPU_WIDTH, GB_PPU_HEIGHT);
    if (!texture) {
        printf("SDL_CreateTexture failed: %s\n", SDL_GetError());
        return false;
    }

    SDL_AudioSpec desired = {0};
    desired.freq = 44100;
    desired.format = AUDIO_S16SYS;
    desired.channels = 1;
    desired.samples = 2048;
    desired.callback = audio_callback;

    audio_device = SDL_OpenAudioDevice(NULL, 0, &desired, NULL, 0);
    if (audio_device == 0) {
        printf("SDL_OpenAudioDevice failed: %s\n", SDL_GetError());
    } else {
        SDL_PauseAudioDevice(audio_device, 0);
    }

    return true;
}

static void destroy_gui(void)
{
    if (audio_device != 0) {
        SDL_CloseAudioDevice(audio_device);
    }
    if (texture) {
        SDL_DestroyTexture(texture);
        texture = NULL;
    }
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = NULL;
    }
    if (window) {
        SDL_DestroyWindow(window);
        window = NULL;
    }
    SDL_Quit();
}

static void render_frame(void)
{
    SDL_UpdateTexture(texture, NULL, ppu_get_framebuffer(), GB_PPU_WIDTH * 4);
    SDL_RenderClear(renderer);

    SDL_Rect dst = {
        0,
        0,
        GB_PPU_WIDTH * 3,
        GB_PPU_HEIGHT * 3
    };

    SDL_RenderCopy(renderer, texture, NULL, &dst);
    SDL_RenderPresent(renderer);
}

int main(int argc, char **argv)
{
    /* checks if have any arguments */
    if (argc <= 1)
        goto usage;

    /* step foward in the array */
    --argc;
    ++argv;

    /* parssing the emulator arguments */
    while (argc > 0) {
        if (!strcmp(argv[0], "-rom")) {
            --argc;
            ++argv;
            if (!argc || argv[0][0] == '-') {
                goto usage;
            }
            load_rom(argv[0]);
            --argc;
            ++argv;
        } else if (!strcmp(argv[0], "-bios")) {
            --argc;
            ++argv;
            if (!argc || argv[0][0] == '-') {
                goto usage;
            }
            load_bios_from_path(argv[0]);
            --argc;
            ++argv;
        } else {
            goto usage;
        }
    }

    if (!init_gui()) {
        return 1;
    }

    printf("[MAIN] SDL initialized OK\n");
    cpu_reset();
    ppu_reset();
    apu_reset();

    bool running = true;
    unsigned long long frame_count = 0ULL;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
                running = false;
            }
        }

        for (int i = 0; i < 456; ++i) {
            cpu_step();
            ppu_step(1);
            apu_step(1);
        }

        ppu_render_frame();
        render_frame();
        ++frame_count;
        if ((frame_count % 60ULL) == 0ULL) {
            printf("[MAIN] frame=%llu scanline=%u mode=%d cycle=%u\n", frame_count, ppu.scanline, ppu.mode, ppu.cycle);
        }
        SDL_Delay(16);
    }

    destroy_gui();
    unload_rom();
    goto end;

usage:
    printf("Usage: gbemu [option] ...\n");
    printf("-bios <DMG_ROM.bin>\n");
    printf("-rom <rom.gb>\n");
end:
    exit(0);
}
