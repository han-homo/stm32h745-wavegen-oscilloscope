#include "menu.h"
#include "OLED.h"
#include "viz.h"
#include "app_config.h"
#include <stdio.h>
#include <string.h>

#define HAVE_OLED_INVERT   0    // 若你的驱动提供 OLED_InvertArea，可改为 1 用真正反色

/* 当前发生器参数（UI 显示用） */
static float  s_fout = APP_GEN_FOUT_HZ;
static float  s_amp  = APP_GEN_AMP_V;
static wave_t s_wave = APP_GEN_WAVE;

/* 频谱缓存（用于 UI_PEAKS 找前 4 峰） */
static const float* s_mag_db = NULL;
static int   s_bins = 0;
static float s_fs   = APP_ADC_FS_HZ;

void MENU_SetGenParams(float f, float a, wave_t w){ s_fout=f; s_amp=a; s_wave=w; }
void MENU_GetGenParams(float* f, float* a, wave_t* w){ if(f)*f=s_fout; if(a)*a=s_amp; if(w)*w=s_wave; }
void MENU_UpdateSpectrum(const float* mag_db, int nfft2, float fs_hz){
    s_mag_db = mag_db; s_bins = nfft2; s_fs=fs_hz;
}

/* 小工具：画一行文本（可高亮） */
static void draw_item(int y, const char* text, uint8_t selected){
    if (selected){
#if HAVE_OLED_INVERT
        OLED_InvertArea(0, y, 128, 12);
        OLED_ShowString(8, y, text, OLED_6X8);
#else
        /* 用高亮条 + 箭头模拟反色 */
        OLED_ClearArea(0, y, 128, 12);         // 清一次防重影
        OLED_DrawLine(0, y+11, 127, y+11);     // 下划线
        OLED_ShowString(0, y, ">", OLED_6X8);  // 箭头
        OLED_ShowString(8, y, text, OLED_6X8);
#endif
    }else{
        OLED_ClearArea(0, y, 128, 12);
        OLED_ShowString(8, y, text, OLED_6X8);
    }
}

/* 一级：首页 */
static void draw_home(uint8_t sel){
    OLED_Clear();
    OLED_ShowString(0, 0, "== Main Menu ==", OLED_6X8);
    draw_item(16, "1) Wave Generation",  sel==0);
    draw_item(28, "2) Spectrum Analysis",sel==1);

    /* 右上角显示当前发生器简况 */
    char buf[20];
    const char* wn = (s_wave==W_SINE)?"SINE":(s_wave==W_SQUARE)?"SQR":"TRI";
    snprintf(buf,sizeof(buf),"%s %.1fV", wn, s_amp);
    OLED_ShowString(64, 0, buf, OLED_6X8);
    snprintf(buf,sizeof(buf),"%5.1fHz", s_fout);
    OLED_ShowString(64, 8, buf, OLED_6X8);

    OLED_Update();
}

/* 四级：波形菜单（确认后立即生成） */
static void draw_wave_menu(uint8_t sel){
    OLED_Clear();
    OLED_ShowString(0, 0, "== Wave Select ==", OLED_6X8);
    draw_item(16, "Triangle", sel==0);
    draw_item(28, "Square",   sel==1);
    draw_item(40, "Sine",     sel==2);

    OLED_Update();
}

/* 三级：频谱界面（由主循环的 VIZ_Draw_Frame 渲染，此处仅给标题） */
static void draw_spectrum_ui(void){
    OLED_ClearArea(0, 0, 128, 12);
    OLED_ShowString(0, 0, "== Spectrum ==", OLED_6X8);
    OLED_UpdateArea(0, 0, 128, 12);
}

/* 五级：Top4 峰值列表 */
static void draw_peaks_ui(void){
    OLED_Clear();
    OLED_ShowString(0, 0, "== Peaks (1..4) ==", OLED_6X8);

    if (!s_mag_db || s_bins<=2){
        OLED_ShowString(0, 16, "No data", OLED_6X8);
        OLED_Update();
        return;
    }
    /* 简单找 4 个最大峰（忽略 DC bin=0） */
    float fval[4]={-1e9,-1e9,-1e9,-1e9};
    int   find[4]={1,1,1,1}, idx[4]={0};
    for (int k=1; k<s_bins; ++k){
        float v = s_mag_db[k];
        for (int j=0;j<4;j++){
            if (v > fval[j]){
                for (int m=3;m>j;m--){ fval[m]=fval[m-1]; idx[m]=idx[m-1]; }
                fval[j]=v; idx[j]=k; break;
            }
        }
    }
    char ln[24];
    for (int j=0;j<4;j++){
        float hz = (float)idx[j] * s_fs / (float)(APP_FFT_SIZE);
        snprintf(ln, sizeof(ln), "%d) %7.1f Hz", j+1, hz);
        OLED_ShowString(0, 16+12*j, ln, OLED_6X8);
    }
    OLED_Update();
}

void MENU_Init(ui_ctx_t* ctx){
    ctx->state = UI_HOME;
    ctx->sel   = 0;
    draw_home(ctx->sel);
}

void MENU_Draw(const ui_ctx_t* ctx){
    switch (ctx->state){
        case UI_HOME:      draw_home(ctx->sel);      break;
        case UI_WAVE_MENU: draw_wave_menu(ctx->sel); break;
        case UI_SPECTRUM:  draw_spectrum_ui();       break;
        case UI_PEAKS:     draw_peaks_ui();          break;
        default: break;
    }
}
