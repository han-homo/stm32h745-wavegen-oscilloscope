// dsp_fft.c
#include "dsp_fft.h"
#include "app_config.h"
#include <math.h>
#include <string.h>

static arm_rfft_fast_instance_f32 s_rfft;
static float32_t s_win[APP_FFT_SIZE];
static float32_t s_in[APP_FFT_SIZE];
static float32_t s_fft[APP_FFT_SIZE];
static float32_t s_mag[APP_FFT_SIZE/2];

static inline void build_hann(void){
    for (uint32_t n=0;n<APP_FFT_SIZE;n++){
        s_win[n] = 0.5f - 0.5f * arm_cos_f32(2.0f*PI*n/(APP_FFT_SIZE-1));
    }
}

void DSP_Init(void){
    arm_rfft_fast_init_f32(&s_rfft, APP_FFT_SIZE);
    build_hann();
}

void DSP_Process_Block_u16_to_db(const uint16_t *src, uint32_t n, float fs,
                                 float *mag_db_out, float *peak_freq)
{
    // u16 -> f32 & 去均值 & 加窗
    float32_t mean;
    for (uint32_t i=0;i<n;i++) s_in[i]=(float32_t)src[i];
    arm_mean_f32(s_in, n, &mean);
    arm_offset_f32(s_in, -mean, s_in, n);
    arm_mult_f32(s_in, s_win, s_in, n);

    // RFFT
    arm_rfft_fast_f32(&s_rfft, s_in, s_fft, 0);
    arm_cmplx_mag_f32(s_fft, s_mag, n/2);

    // dB & 峰值频率
    float32_t maxv; uint32_t kmax;
    for (uint32_t k=0;k<n/2;k++){
        float v = s_mag[k] + 1e-12f;
        mag_db_out[k] = 20.0f*log10f(v);
    }
    arm_max_f32(mag_db_out, n/2, &maxv, &kmax);
    if (peak_freq) *peak_freq = (float)kmax * fs / (float)APP_FFT_SIZE;
}
