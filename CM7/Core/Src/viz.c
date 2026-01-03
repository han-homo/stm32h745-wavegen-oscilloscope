#include "viz.h"
#include "OLED.h"
#include "app_config.h"
#include <string.h>
#include <math.h>
#include <stdio.h>   // snprintf

static int clampi(int v, int lo, int hi){ if(v<lo) return lo; if(v>hi) return hi; return v; }

/* dB 数组寻找最大值 */
static float find_max(const float *a, int n){
    float m = a[0];
    for (int i=1;i<n;i++) if (a[i]>m) m=a[i];
    return m;
}

void VIZ_Init(void){
    OLED_Init();
    OLED_Clear();
    OLED_ShowString(0, 0,  "STM32H745 Signal+FFT", OLED_6X8);
    OLED_ShowString(0, 8,  "CH1:Wave  CH2:ADC",   OLED_6X8);
    OLED_Update();
}

/* 顶部 0..31：时域；底部 32..63：频谱 */
void VIZ_Draw_Frame(const uint16_t *time_u16, uint32_t half_len,
                    const float *mag_db, float peak_hz, float fs_hz)
{
    /* === 时域 === */
    OLED_ClearArea(0, 0, 128, 32);

    uint16_t minv=0xFFFF, maxv=0;
    for (uint32_t i=0;i<half_len;i++){ if(time_u16[i]<minv) minv=time_u16[i]; if(time_u16[i]>maxv) maxv=time_u16[i]; }
    float scale = (maxv>minv) ? (31.0f/(float)(maxv-minv)) : 1.0f;

    for (int x=0; x<128; x++){
        uint32_t idx = (uint32_t)((float)x * (float)half_len / 128.0f);
        int y = 31 - (int)((time_u16[idx]-minv)*scale + 0.5f);
        y = clampi(y, 0, 31);
        OLED_DrawPoint(x, y);                 // 注意：无颜色参数（与你的 OLED.h 一致）:contentReference[oaicite:2]{index=2}
        if (x>0){
            uint32_t idxp = (uint32_t)((float)(x-1) * (float)half_len / 128.0f);
            int yp = 31 - (int)((time_u16[idxp]-minv)*scale + 0.5f);
            yp = clampi(yp, 0, 31);
            OLED_DrawLine(x-1, yp, x, y);     // 同上，无颜色参数:contentReference[oaicite:3]{index=3}
        }
    }

    /* === 频谱 === */
    OLED_ClearArea(0, 32, 128, 32);
    const int nfft2 = APP_FFT_SIZE/2;
    float maxdb = find_max(mag_db, nfft2);
    float floor = maxdb - 60.0f;
    if (floor > maxdb-10.f) floor = maxdb - 10.f;

    for (int x=0; x<128; x++){
        int k = (int)((float)x * (float)nfft2 / 128.0f);
        float db = mag_db[k];
        if (db < floor) db = floor;
        float norm = (db - floor) / (maxdb - floor + 1e-6f);
        int h = (int)(norm * 30.0f + 0.5f);
        if (h<0) h=0;
        if (h>30) h=30;
        for (int yy=0; yy<h; yy++){
            OLED_DrawPoint(x, 63-yy);         // 无颜色参数
        }
    }

    char buf[24];
    snprintf(buf, sizeof(buf), "Pk:%5.1fHz", peak_hz);
    OLED_ShowString(0, 32, buf, OLED_6X8);

    OLED_UpdateArea(0, 0,   128, 32);
    OLED_UpdateArea(0, 32,  128, 32);
}
/* 把 LUT（12-bit/16-bit 数字量）映射到 0..63 像素高，横向压缩/拉伸到 128 列 */
void VIZ_Draw_GenWave_LUT(const uint16_t *lut, uint32_t lut_len)
{
    if (!lut || lut_len == 0) return;

    // 估一个 min/max 做归一化（更直观；若你知道满量程=0..4095也可直接用）
    uint16_t minv=0xFFFF, maxv=0;
    for (uint32_t i=0;i<lut_len;i++){ if(lut[i]<minv) minv=lut[i]; if(lut[i]>maxv) maxv=lut[i]; }
    float scale = (maxv>minv) ? (63.0f/(float)(maxv-minv)) : 1.0f;

    OLED_Clear();  // 全屏清空
    // 画 0 轴（中线）可选：OLED_DrawLine(0, 32, 127, 32);

    int lastx = 0;
    int lasty = 63 - (int)((lut[0]-minv)*scale + 0.5f);
    lasty = clampi(lasty, 0, 63);

    for (int x=0; x<128; x++){
        uint32_t idx = (uint32_t)((float)x * (float)lut_len / 128.0f); // 重采样索引
        int y = 63 - (int)((lut[idx]-minv)*scale + 0.5f);
        y = clampi(y, 0, 63);
        if (x==0){
            OLED_DrawPoint(x, y);
        }else{
            OLED_DrawLine(lastx, lasty, x, y);
        }
        lastx = x; lasty = y;
    }

    // 小标签：波形类型在右上角（可选）
    // OLED_ShowString(92, 0, "GEN", OLED_6X8);

    OLED_Update(); // 整屏刷新（这一屏不大，用整屏更简单）
}
