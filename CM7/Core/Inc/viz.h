#pragma once
#include <stdint.h>
#include <stdbool.h>

/* 初始化 OLED 并清屏（封装一层便于 main 使用） */
void VIZ_Init(void);

/* 画一帧“时域 + 频谱”，输入：
   - time_u16:   时域样本（uint16）长度 half_len
   - half_len:   半缓冲长度（= FFT_SIZE/2）
   - mag_db:     频谱 dB（FFT_SIZE/2）
   - peak_hz:    主峰频率（Hz）
   - fs_hz:      采样率（Hz）
*/
void VIZ_Draw_Frame(const uint16_t *time_u16, uint32_t half_len,
                    const float *mag_db, float peak_hz, float fs_hz);
/* 直接把发生器 LUT 画到 OLED（全屏 128x64） */
void VIZ_Draw_GenWave_LUT(const uint16_t *lut, uint32_t lut_len);
