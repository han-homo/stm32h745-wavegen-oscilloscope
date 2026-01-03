#include "keys.h"

typedef struct {
    uint8_t  stable;        // 稳定电平（按下=1）
    uint8_t  last_raw;      // 上一次瞬时采样
    uint32_t t_change;      // 最近电平变化时刻
    uint8_t  pressed;       // 是否处于按下状态
    uint8_t  clicks;        // 单/双击计数
    uint32_t t_release;     // 最近一次释放时刻
} key_sm_t;

#define DEBOUNCE_MS     15u
#define LONG_MS         600u
#define DOUBLE_MS       280u

static key_sm_t s_ok, s_back, s_next, s_freq;

/* 通用状态机轮询 */
static key_evt_t poll_key(key_sm_t* k, uint8_t raw_now){
    uint32_t now = HAL_GetTick();
    key_evt_t evt = K_EVT_NONE;

    if (raw_now != k->last_raw){
        k->last_raw = raw_now;
        k->t_change = now;
    }

    /* 消抖判定：稳定到 DEBOUNCE_MS 才确认电平变化 */
    if ((now - k->t_change) >= DEBOUNCE_MS && raw_now != k->stable){
        k->stable = raw_now;
        if (k->stable){                // -> 按下稳定
            k->pressed = 1;
            evt = K_EVT_FALLING;       // 可作为“确认”瞬时触发
        }else{                         // -> 释放稳定
            if (k->pressed){
                k->pressed = 0;
                if ((now - k->t_change) < LONG_MS){
                    /* 计入点击次数 */
                    k->clicks++;
                    k->t_release = now;
                }else{
                    /* 长按释放 */
                    evt = K_EVT_LONG;  // 仅报一次
                    k->clicks = 0;
                }
            }
            if (evt == K_EVT_NONE) evt = K_EVT_RISING;
        }
    }

    /* 长按滞后判定（按住超过 LONG_MS） */
    if (k->pressed && (now - k->t_change) >= LONG_MS){
        k->pressed = 0;                // 只报一次
        k->clicks = 0;
        return K_EVT_LONG;
    }

    /* 单/双击窗口 */
    if (k->clicks > 0 && (now - k->t_release) >= DOUBLE_MS){
        evt = (k->clicks >= 2) ? K_EVT_DOUBLE : K_EVT_SHORT;
        k->clicks = 0;
    }

    return evt;
}

void KEYS_Init(void){
    uint8_t s;
    s = KEY_Read(KEY_OK_PORT,   KEY_OK_PIN);   s_ok.stable=s; s_ok.last_raw=s; s_ok.t_change=HAL_GetTick();
    s = KEY_Read(KEY_BACK_PORT, KEY_BACK_PIN); s_back.stable=s; s_back.last_raw=s; s_back.t_change=HAL_GetTick();
    s = KEY_Read(KEY_NEXT_PORT, KEY_NEXT_PIN); s_next.stable=s; s_next.last_raw=s; s_next.t_change=HAL_GetTick();
    s = KEY_Read(KEY_FREQ_PORT, KEY_FREQ_PIN); s_freq.stable=s; s_freq.last_raw=s; s_freq.t_change=HAL_GetTick();
}

key_evt_t KEY_Get_OK(void)   { return poll_key(&s_ok,   KEY_Read(KEY_OK_PORT,   KEY_OK_PIN)); }
key_evt_t KEY_Get_BACK(void) { return poll_key(&s_back, KEY_Read(KEY_BACK_PORT, KEY_BACK_PIN)); }
key_evt_t KEY_Get_NEXT(void) { return poll_key(&s_next, KEY_Read(KEY_NEXT_PORT, KEY_NEXT_PIN)); }
key_evt_t KEY_Get_FREQ(void) { return poll_key(&s_freq, KEY_Read(KEY_FREQ_PORT, KEY_FREQ_PIN)); }
