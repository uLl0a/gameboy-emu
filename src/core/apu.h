#ifndef GBEMU_APU_H_
#define GBEMU_APU_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct {
    bool enabled;
    uint32_t sample_rate;
    uint32_t phase;
    uint32_t phase_ch1;
    uint32_t phase_ch2;
    uint32_t phase_ch4;
    float square1_volume;
    float square2_volume;
    float noise_volume;
    float master_left;
    float master_right;
    uint32_t frame_counter;
    uint8_t duty1;
    uint8_t duty2;
    uint16_t lfsr;
} apu_context_t;

extern apu_context_t apu;

void apu_reset(void);
void apu_step(uint32_t cycles);
float apu_sample(void);
void apu_generate_samples(int16_t *buffer, size_t sample_count);

#endif // GBEMU_APU_H_
