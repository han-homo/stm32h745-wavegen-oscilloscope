// dsp_fft.h
#pragma once
#include "arm_math.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void DSP_Init(void);
void DSP_Process_Block_u16_to_db(const uint16_t *src, uint32_t n, float fs,
                                 float *mag_db_out /* n/2 */, float *peak_freq);

#ifdef __cplusplus
}
#endif
