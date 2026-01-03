// gen_dac.c
#include "gen_dac.h"
#include "app_config.h"
#include "dac.h"
#include "tim.h"
#include <math.h>

extern DAC_HandleTypeDef hdac1;    // CubeMX
extern TIM_HandleTypeDef htim7;

static uint16_t s_lut[DAC_LUT_LEN] __attribute__((section(".DTCMRAM")));

static void build_lut(float amp_v, wave_t w){
    float scale = 4095.0f / APP_VREF; // 12-bit DAC
    for (uint32_t i=0;i<DAC_LUT_LEN;i++){
        float x = (float)i / (float)DAC_LUT_LEN;
        float v = 0.f;
        switch (w){
            case W_SINE:   v = DAC_OFFSET_V + amp_v * sinf(2.f*M_PI*x); break;
            case W_SQUARE: v = DAC_OFFSET_V + amp_v * (x<0.5f ? 1.f : -1.f); break;
            case W_TRI:    v = DAC_OFFSET_V + amp_v * (4.f*fabsf(x-0.5f)-1.f); break;
        }
        if (v < 0.f)       v = 0.f;
        if (v > APP_VREF)  v = APP_VREF;

        s_lut[i] = (uint16_t)(v * scale);
    }
}
static uint32_t get_tim7_kernel_clk(void){
    uint32_t pclk1  = HAL_RCC_GetPCLK1Freq();
    uint32_t timclk = pclk1;
    /* APB1 预分频 != 1 时，定时器时钟 = 2 * PCLK1 （H7 的老规矩） */
    if ((RCC->D2CFGR & RCC_D2CFGR_D2PPRE1) != RCC_D2CFGR_D2PPRE1_DIV1) timclk *= 2;
    return timclk;
}
static void TIM7_SetFs_Safe(float fs_hz){
    uint32_t timclk = get_tim7_kernel_clk();        // ← 不要再写死 240MHz
    /* 目标：TIM7 更新率 = fs_hz */
    /* 选一组 16-bit 内的 PSC/ARR，尽量让 PSC 小一些、ARR 落在 16 位 */
    uint32_t target = (uint32_t)( (double)timclk / (double)fs_hz + 0.5 );
    uint32_t psc, arr;
    for (psc = 0; psc <= 0xFFFF; ++psc) {
        arr = target / (psc + 1);
        if (arr >= 1 && arr <= 0x10000) {  // ARR 寄存器写入值需 -1
            break;
        }
    }
    if (arr == 0) arr = 1;

    __HAL_TIM_DISABLE(&htim7);
    __HAL_TIM_SET_PRESCALER(&htim7, psc);
    __HAL_TIM_SET_AUTORELOAD(&htim7, arr - 1);
    __HAL_TIM_SET_COUNTER(&htim7, 0);
    __HAL_TIM_ENABLE(&htim7);
}
void GEN_InitAndStart(float fout_hz, float amp_v, wave_t w){
    build_lut(amp_v, w);
    float fs_dac = fout_hz * (float)DAC_LUT_LEN;
    TIM7_SetFs_Safe(fs_dac);                         // TIM7 → TRGO=Update（你已配置好）
    HAL_TIM_Base_Start(&htim7);

    // DAC1 CH1 触发源：TIM7 TRGO（你已配置） + DMA（循环）
    HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_1, (uint32_t*)s_lut, DAC_LUT_LEN, DAC_ALIGN_12B_R);
}

void GEN_Update(float fout_hz, float amp_v, wave_t w){
    build_lut(amp_v, w);
    float fs_dac = fout_hz * (float)DAC_LUT_LEN;
    TIM7_SetFs_Safe(fs_dac);
    // DMA 循环输出 LUT，无需停再启（需要时可调用 HAL_DAC_Stop_DMA 再 Start）
}
void GEN_GetLUT(const uint16_t **buf, uint32_t *len){
    if (buf) *buf = s_lut;          // s_lut：你已有的 DAC 查表数组（DTCM）:contentReference[oaicite:1]{index=1}
    if (len) *len = DAC_LUT_LEN;    // 与 DMA 循环长度一致
}
