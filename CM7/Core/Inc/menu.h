#pragma once
#include <stdint.h>
#include "gen_dac.h"

/* UI 状态 */
typedef enum {
    UI_HOME = 0,       // 一级：首页（波形生成 / 频谱分析）
    UI_WAVE_MENU,      // 四级：波形选择（三角 / 方波 / 正弦）
    UI_SPECTRUM,       // 三级：频谱图
    UI_PEAKS           // 五级：峰值 1~4
} ui_state_t;

/* 选项索引（根据不同界面含义不同） */
typedef struct {
    ui_state_t state;
    uint8_t    sel;          // 当前选中项（行号）
} ui_ctx_t;

/* 初始化 UI（清屏+首页） */
void MENU_Init(ui_ctx_t* ctx);

/* 绘制当前状态 */
void MENU_Draw(const ui_ctx_t* ctx);

/* 设置/获取当前发生器参数（用于 UI 内显示） */
void MENU_SetGenParams(float fout_hz, float amp_v, wave_t w);
void MENU_GetGenParams(float* fout_hz, float* amp_v, wave_t* w);

/* 更新频谱数据（用于 UI_PEAKS 显示 Top4） */
void MENU_UpdateSpectrum(const float* mag_db, int nfft2, float fs_hz);
