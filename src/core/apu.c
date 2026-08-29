#include "apu.h"

apu_context_t apu = {0};

static float square_wave(float phase, uint8_t duty)
{
    const float wave[4][8] = {
        { 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f },
        { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f },
        { 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f },
        { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f }
    };

    const uint8_t index = (uint8_t)((phase * 8.0f) + 0.5f) % 8U;
    return wave[duty & 0x03U][index];
}

static float noise_wave(uint32_t phase)
{
    uint32_t steps = phase / 32U;
    if (steps == 0U) {
        return 0.0f;
    }

    uint32_t bit = (apu.lfsr ^ (apu.lfsr >> 1U)) & 1U;
    apu.lfsr = (apu.lfsr >> 1U) | (bit << 14U);
    return (apu.lfsr & 0x01U) ? 1.0f : 0.0f;
}

void apu_reset(void)
{
    apu.enabled = true;
    apu.sample_rate = 44100U;
    apu.phase = 0U;
    apu.phase_ch1 = 0U;
    apu.phase_ch2 = 0U;
    apu.phase_ch4 = 0U;
    apu.square1_volume = 0.35f;
    apu.square2_volume = 0.25f;
    apu.noise_volume = 0.15f;
    apu.master_left = 0.8f;
    apu.master_right = 0.8f;
    apu.frame_counter = 0U;
    apu.duty1 = 0U;
    apu.duty2 = 2U;
    apu.lfsr = 0x7FFFU;
}

void apu_step(uint32_t cycles)
{
    apu.frame_counter += cycles;
    if (apu.enabled) {
        apu.phase = (apu.phase + cycles) % 0x10000U;
        apu.phase_ch1 = (apu.phase_ch1 + cycles) % 0x10000U;
        apu.phase_ch2 = (apu.phase_ch2 + cycles * 2U) % 0x10000U;
        apu.phase_ch4 = (apu.phase_ch4 + cycles * 3U) % 0x10000U;
    }
}

float apu_sample(void)
{
    if (!apu.enabled) {
        return 0.0f;
    }

    const float phase1 = (float)(apu.phase_ch1 & 0xFFFFU) / 65535.0f;
    const float phase2 = (float)(apu.phase_ch2 & 0xFFFFU) / 65535.0f;
    const float phase4 = (float)(apu.phase_ch4 & 0xFFFFU) / 65535.0f;

    const float tone1 = square_wave(phase1 * 4.0f + 0.05f, apu.duty1) * apu.square1_volume;
    const float tone2 = square_wave(phase2 * 6.0f + 0.25f, apu.duty2) * apu.square2_volume;
    const float noise = noise_wave((uint32_t)(phase4 * 17.0f)) * apu.noise_volume;

    const float mixed = (tone1 + tone2 + noise) * 0.55f;
    return mixed * apu.master_left;
}

void apu_generate_samples(int16_t *buffer, size_t sample_count)
{
    if (buffer == NULL || sample_count == 0U) {
        return;
    }

    for (size_t i = 0; i < sample_count; ++i) {
        const float sample = apu_sample();
        buffer[i] = (int16_t)(sample * 32767.0f);
        apu.phase = (apu.phase + 1U) % 0x10000U;
        apu.phase_ch1 = (apu.phase_ch1 + 1U) % 0x10000U;
        apu.phase_ch2 = (apu.phase_ch2 + 1U) % 0x10000U;
        apu.phase_ch4 = (apu.phase_ch4 + 1U) % 0x10000U;
    }
}
