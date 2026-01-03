// gen_dac.h
#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum { W_SINE=0, W_SQUARE=1, W_TRI=2 } wave_t;

void GEN_InitAndStart(float fout_hz, float amp_v, wave_t w);
void GEN_Update(float fout_hz, float amp_v, wave_t w);
/* 读取当前 DAC 查找表（与 DMA 正在循环输出的同一份） */
void GEN_GetLUT(const uint16_t **buf, uint32_t *len);


#ifdef __cplusplus
}
#endif
