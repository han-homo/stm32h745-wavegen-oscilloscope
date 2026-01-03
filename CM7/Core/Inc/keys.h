#pragma once
#include "stm32h7xx_hal.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* === 按键脚位映射（默认：上拉输入，按下=0 有效）=== */
#define KEY_ACTIVE_LEVEL            0   // 0: 按下为低；1: 按下为高

#define KEY_OK_PORT                 GPIOE
#define KEY_OK_PIN                  GPIO_PIN_2

#define KEY_BACK_PORT               GPIOE
#define KEY_BACK_PIN                GPIO_PIN_4

#define KEY_NEXT_PORT               GPIOE
#define KEY_NEXT_PIN                GPIO_PIN_5

#define KEY_FREQ_PORT               GPIOC
#define KEY_FREQ_PIN                GPIO_PIN_13

/* 事件类型（边沿/短按/长按/双击） */
typedef enum {
    K_EVT_NONE = 0,
    K_EVT_RISING,        // 松开边沿（可不用）
    K_EVT_FALLING,       // 按下边沿（可用作“瞬时确认”）
    K_EVT_SHORT,         // 短按
    K_EVT_LONG,          // 长按
    K_EVT_DOUBLE         // 双击
} key_evt_t;

/* 快速读电平 → 归一化为 0/1（按下=1） */
static inline uint8_t KEY_Read(GPIO_TypeDef* port, uint16_t pin){
    uint8_t raw = (HAL_GPIO_ReadPin(port, pin) == GPIO_PIN_SET) ? 1 : 0;
    return (KEY_ACTIVE_LEVEL ? raw : (uint8_t)!raw);
}

/* 初始化内部状态 */
void KEYS_Init(void);

/* 轮询各键事件（每次调用返回一次“单次事件”；无事件返回 K_EVT_NONE） */
key_evt_t KEY_Get_OK(void);
key_evt_t KEY_Get_BACK(void);
key_evt_t KEY_Get_NEXT(void);
key_evt_t KEY_Get_FREQ(void);

#ifdef __cplusplus
}
#endif
