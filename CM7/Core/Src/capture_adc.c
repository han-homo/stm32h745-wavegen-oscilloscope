// capture_adc.c
#include "capture_adc.h"
#include "app_config.h"
#include "adc.h"
#include "tim.h"
#include "stm32h7xx.h"   // for SCB_InvalidateDCache_by_Addr

#include <string.h>

extern ADC_HandleTypeDef hadc3;   // 来自 CubeMX
extern TIM_HandleTypeDef htim6;

__ALIGNED(32) static uint16_t s_adc_buf[APP_FFT_SIZE] __attribute__((section(".RAM_D2")));
static volatile uint8_t s_flag = 0; // bit0: half, bit1: full
// —— 计算 APB1 域的“定时器内核时钟” ——
// H7 规则：当 APB1 预分频 != 1 时，timclk = 2 * PCLK1
static uint32_t get_tim6_kernel_clk(void){
    uint32_t pclk1 = HAL_RCC_GetPCLK1Freq();
    if ((RCC->D2CFGR & RCC_D2CFGR_D2PPRE1) != RCC_D2CFGR_D2PPRE1_DIV1) {
        pclk1 *= 2;
    }
    return pclk1;
}
// —— 仅配置 PSC/ARR，不在这里启动 TIM6 ——
// 目标：TIM6 更新率 = fs_hz（TRGO=Update 已在 MX_TIM6_Init 里设好）
static void TIM6_SetFs_Safe(float fs_hz){
    uint32_t timclk = get_tim6_kernel_clk();

    // 选取一组 16-bit 可用的 PSC/ARR，使 timclk / ((PSC+1)*(ARR+1)) ≈ fs_hz
    uint32_t target = (uint32_t)((double)timclk / (double)fs_hz + 0.5);
    uint32_t best_psc = 0, best_arr = 0;
    for (uint32_t psc = 0; psc <= 0xFFFF; ++psc) {
        uint32_t arr = target / (psc + 1);
        if (arr >= 1 && arr <= 0x10000) {     // ARR 实际写入寄存器要减一
            best_psc = psc;
            best_arr = arr - 1;
            break;
        }
    }
    if (best_arr == 0) best_arr = 1;

    __HAL_TIM_DISABLE(&htim6);
    __HAL_TIM_SET_PRESCALER(&htim6, best_psc);
    __HAL_TIM_SET_AUTORELOAD(&htim6, best_arr);
    __HAL_TIM_SET_COUNTER(&htim6, 0);

    // 屏蔽一次 UG 产生 TRGO，避免瞬间伪触发
    htim6.Instance->CR1 |= TIM_CR1_UDIS;
    __HAL_TIM_CLEAR_FLAG(&htim6, TIM_FLAG_UPDATE);
    htim6.Instance->EGR = TIM_EGR_UG;     // 装载新 PSC/ARR
    htim6.Instance->CR1 &= ~TIM_CR1_UDIS;
}


static inline void dcache_invalidate_aligned(void *addr, uint32_t size){
    uintptr_t a = (uintptr_t)addr;
    uintptr_t a_aln = a & ~((uintptr_t)31);          // 向下 32B 对齐
    uint32_t  add   = (uint32_t)(a - a_aln);
    uint32_t  len   = ((size + add + 31) & ~31);     // 向上 32B 对齐
    SCB_InvalidateDCache_by_Addr((uint32_t*)a_aln, len);
}

void CAP_InitAndStart(float fs_hz){
    // 1) 配好 TIM6 分频
    TIM6_SetFs_Safe(fs_hz);

    // 2) 启动 TIM6（TRGO=Update）
    HAL_TIM_Base_Start(&htim6);

    // 3) 启动 ADC3+DMA 环形
    HAL_ADC_Start_DMA(&hadc3, (uint32_t*)s_adc_buf, APP_FFT_SIZE);
}
void CAP_GetLatestFrame(const uint16_t **half0, const uint16_t **half1, uint32_t *half_len){
    if (half0) *half0 = &s_adc_buf[0];
    if (half1) *half1 = &s_adc_buf[APP_FFT_SIZE/2];
    if (half_len) *half_len = APP_FFT_SIZE/2;
}

// HAL 回调（来自 BDMA 中断）
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc){
    if (hadc->Instance == ADC3){
        dcache_invalidate_aligned(&s_adc_buf[0], (APP_FFT_SIZE/2)*sizeof(uint16_t));
        s_flag |= 0x01;
    }
}
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc){
    if (hadc->Instance == ADC3){
        dcache_invalidate_aligned(&s_adc_buf[APP_FFT_SIZE/2], (APP_FFT_SIZE/2)*sizeof(uint16_t));
        s_flag |= 0x02;
    }
}


int CAP_FetchAndClearProcessFlag(uint8_t *which_half){
    uint8_t f = s_flag;
    if (!f) return 0;
    __disable_irq();
    s_flag = 0;
    __enable_irq();
    if (which_half){
        *which_half = (f & 0x01) ? 0 : 1; // half=0，full=1
    }
    return 1;
}
