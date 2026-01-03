// capture_adc.h
#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void CAP_InitAndStart(float fs_hz);
void CAP_GetLatestFrame(const uint16_t **half0, const uint16_t **half1, uint32_t *half_len);
// 回调里会触发一次“处理请求”标志
int  CAP_FetchAndClearProcessFlag(uint8_t *which_half);

#ifdef __cplusplus
}
#endif
