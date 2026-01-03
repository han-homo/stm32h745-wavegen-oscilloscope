#pragma once

/* ==== 全局配置 ==== */
#define APP_FFT_SIZE        1024        // 与 dsp_fft.c 一致
#define APP_VREF            3.3f

/* DAC 输出 LUT 长度 & 中点偏置（12-bit DAC → 0..4095） */
#define DAC_LUT_LEN         256
#define DAC_OFFSET_V        1.65f

/* 采样与输出默认参数（可运行时通过 GEN_Update 调整） */
#define APP_ADC_FS_HZ       10000.0f   // ADC 采样率
#define APP_GEN_FOUT_HZ     1000.0f     // 输出频率
#define APP_GEN_AMP_V       1.5f        // 峰值幅度（叠加中点）
#define APP_GEN_WAVE        W_SINE      // 默认正弦波
/* === 按键调节步进 === */
#define STEP_FOUT_SMALL_HZ   100.0f
#define STEP_FOUT_BIG_HZ     1000.0f
#define STEP_AMP_SMALL_V     0.1f
#define STEP_AMP_BIG_V       0.5f
#define AMP_MIN_V            0.1f
#define AMP_MAX_V            1.65f       // 避免超出偏置后顶到 0/3.3V
#define FOUT_MIN_HZ          10.0f
#define FOUT_MAX_HZ          20000.0f    // 受 LUT 长度与 DAC 速率限制
