/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dac.h"
#include "dma.h"
#include "i2c.h"
#include "tim.h"

#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "app_config.h"
#include "dsp_fft.h"
#include "capture_adc.h"
#include "gen_dac.h"
//#include "ssd1306_i2c.h"
//#include "ui_shell.h"
#include "OLED.h"
#include "viz.h"
//#include "buttons.h"
#include "keys.h"
#include "menu.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */



/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* DUAL_CORE_BOOT_SYNC_SEQUENCE: Define for dual core boot synchronization    */
/*                             demonstration code based on hardware semaphore */
/* This define is present in both CM7/CM4 projects                            */
/* To comment when developping/debugging on a single core                     */
#define DUAL_CORE_BOOT_SYNC_SEQUENCE

#if defined(DUAL_CORE_BOOT_SYNC_SEQUENCE)
#ifndef HSEM_ID_0
#define HSEM_ID_0 (0U) /* HW semaphore 0*/
#endif
#endif /* DUAL_CORE_BOOT_SYNC_SEQUENCE */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
static float g_mag_db[APP_FFT_SIZE/2];
static float s_fout = APP_GEN_FOUT_HZ;
static float s_amp  = APP_GEN_AMP_V;
static wave_t s_wave = APP_GEN_WAVE;
static float clampf(float v, float lo, float hi){ if(v<lo) return lo; if(v>hi) return hi; return v; }
/* —— 计算 TIM6 触发频率（= ADC 采样率）——
   H7 规则：当 APB1 预分频 != 1 时，定时器内核时钟 = 2 * PCLK1 */
static float get_fs_real_from_tim6(void)
{
    uint32_t pclk1  = HAL_RCC_GetPCLK1Freq();
    uint32_t timclk = pclk1;
    if ((RCC->D2CFGR & RCC_D2CFGR_D2PPRE1) != RCC_D2CFGR_D2PPRE1_DIV1) {
        timclk *= 2;
    }
    uint32_t psc = htim6.Instance->PSC;
    uint32_t arr = htim6.Instance->ARR;
    return (float)timclk / ((psc + 1U) * (arr + 1U));
}
//static float s_spec_db[APP_FFT_SIZE/2];
//static float s_peak_hz = 0.f;
//static const uint16_t *s_half0, *s_half1;
//static uint32_t s_halflen = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
static void MPU_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */


  /* USER CODE END 1 */
/* USER CODE BEGIN Boot_Mode_Sequence_0 */
#if defined(DUAL_CORE_BOOT_SYNC_SEQUENCE)
  int32_t timeout;
#endif /* DUAL_CORE_BOOT_SYNC_SEQUENCE */
/* USER CODE END Boot_Mode_Sequence_0 */

  /* MPU Configuration--------------------------------------------------------*/
  MPU_Config();

/* USER CODE BEGIN Boot_Mode_Sequence_1 */

/* USER CODE END Boot_Mode_Sequence_1 */
  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* Configure the peripherals common clocks */
  PeriphCommonClock_Config();
/* USER CODE BEGIN Boot_Mode_Sequence_2 */
#if defined(DUAL_CORE_BOOT_SYNC_SEQUENCE)
/* When system initialization is finished, Cortex-M7 will release Cortex-M4 by means of
HSEM notification */
/*HW semaphore Clock enable*/
__HAL_RCC_HSEM_CLK_ENABLE();
/*Take HSEM */
HAL_HSEM_FastTake(HSEM_ID_0);
/*Release HSEM in order to notify the CPU2(CM4)*/
HAL_HSEM_Release(HSEM_ID_0,0);
/* wait until CPU2 wakes up from stop mode */
timeout = 0xFFFF;
while((__HAL_RCC_GET_FLAG(RCC_FLAG_D2CKRDY) == RESET) && (timeout-- > 0));
if ( timeout < 0 )
{
Error_Handler();
}
#endif /* DUAL_CORE_BOOT_SYNC_SEQUENCE */
/* USER CODE END Boot_Mode_Sequence_2 */

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC3_Init();
  MX_DAC1_Init();
  MX_I2C1_Init();
  MX_TIM6_Init();
  MX_TIM7_Init();
  MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */
  /* 业务初始化 */
    VIZ_Init();                       // OLED 欢迎页
    DSP_Init();                       // RFFT + Hann 窗
    //BTN_Init();
    KEYS_Init();
    /* 1) 设定目标采样率并启动采集
       - 如果你就想 100 kHz，就传 100000.0f 或 APP_ADC_FS_HZ
       - 如果你想跟着菜单/按键改采样率，也把这里的值做成变量 */
    const float fs_target = APP_ADC_FS_HZ;   // 或者直接写 100000.0f
    CAP_InitAndStart(fs_target);             // TIM6→TRGO 配置+ADC3+DMA 启动
    GEN_InitAndStart(s_fout, s_amp, s_wave);
    /* 2) 读取“真实采样率”，留给 FFT/绘图使用 */
   // float fs_real = get_fs_real_from_tim6();
    //CAP_InitAndStart(APP_ADC_FS_HZ);  // 启动 ADC3 采样（TIM6→TRGO）:contentReference[oaicite:14]{index=14}
   // GEN_InitAndStart(APP_GEN_FOUT_HZ, APP_GEN_AMP_V, APP_GEN_WAVE); // 启动 DAC1 输出（TIM7→TRGO）:contentReference[oaicite:15]{index=15}
    /* 启动后，先把“当前发生器 LUT”画到整屏，直观可见输出波形 */
    ui_ctx_t ui;  MENU_SetGenParams(s_fout, s_amp, s_wave);  MENU_Init(&ui);

    const uint16_t *half0, *half1;
      uint32_t half_len;
      CAP_GetLatestFrame(&half0, &half1, &half_len); // half_len = APP_FFT_SIZE/2

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
     /* ====== 按键事件 ====== */
    key_evt_t e_ok   = KEY_Get_OK();
    key_evt_t e_back = KEY_Get_BACK();
    key_evt_t e_next = KEY_Get_NEXT();
    key_evt_t e_freq = KEY_Get_FREQ();     // PC13：短按/双击 -> 调频

    /* PC13：只在“波形生成路径”（UI_HOME 选择了波形生成 → UI_WAVE_MENU）与波形已设置后有效 */
    if (e_freq == K_EVT_SHORT){
        s_fout = clampf(s_fout + STEP_FOUT_SMALL_HZ, FOUT_MIN_HZ, FOUT_MAX_HZ);
        GEN_Update(s_fout, s_amp, s_wave);
        MENU_SetGenParams(s_fout, s_amp, s_wave);
        if (ui.state == UI_HOME || ui.state == UI_WAVE_MENU) MENU_Draw(&ui);
    }else if (e_freq == K_EVT_DOUBLE){
        s_fout = clampf(s_fout - STEP_FOUT_SMALL_HZ, FOUT_MIN_HZ, FOUT_MAX_HZ);
        GEN_Update(s_fout, s_amp, s_wave);
        MENU_SetGenParams(s_fout, s_amp, s_wave);
        if (ui.state == UI_HOME || ui.state == UI_WAVE_MENU) MENU_Draw(&ui);
    }

    /* 三个功能键的 UI 状态机 */
    if (e_next == K_EVT_SHORT || e_next == K_EVT_FALLING){   // NEXT：切下一项
        switch (ui.state){
            case UI_HOME:
                ui.sel = (ui.sel+1) % 2;
                MENU_Draw(&ui);
                break;
            case UI_WAVE_MENU:
                ui.sel = (ui.sel+1) % 3;
                MENU_Draw(&ui);
                break;
            case UI_SPECTRUM:
                /* 在频谱界面按 NEXT → 查看 Top4 峰值（五级） */
                ui.state = UI_PEAKS;
                MENU_Draw(&ui);
                break;
            case UI_PEAKS:
                /* 从 Top4 返回频谱（也可改成循环切换） */
                ui.state = UI_SPECTRUM;
                MENU_Draw(&ui);
                break;
            default: break;
        }
    }

    if (e_back == K_EVT_SHORT || e_back == K_EVT_FALLING){   // BACK：返回上级
        switch (ui.state){
            case UI_HOME:      /* 顶层无上级 */ break;
            case UI_WAVE_MENU: ui.state = UI_HOME; ui.sel = 0; MENU_Draw(&ui); break;
            case UI_SPECTRUM:  ui.state = UI_HOME; ui.sel = 1; MENU_Draw(&ui); break;
            case UI_PEAKS:     ui.state = UI_SPECTRUM; MENU_Draw(&ui); break;
            default: break;
        }
    }

    if (e_ok == K_EVT_SHORT || e_ok == K_EVT_FALLING){       // OK：确认
        switch (ui.state){
            case UI_HOME:
                if (ui.sel==0){           // 进入波形菜单（四级）
                    ui.state = UI_WAVE_MENU; ui.sel = 0; MENU_Draw(&ui);
                }else{                    // 进入频谱界面（三级）
                    ui.state = UI_SPECTRUM; MENU_Draw(&ui);
                }
                break;

            case UI_WAVE_MENU: {          // 选择波形并立即生效
                wave_t w = (ui.sel==0)? W_TRI : (ui.sel==1)? W_SQUARE : W_SINE;
                s_wave = w;
                GEN_Update(s_fout, s_amp, s_wave);
                MENU_SetGenParams(s_fout, s_amp, s_wave);
                /* 生成波形后仍停留在波形菜单，或返回首页均可。这里选择停留，便于继续调整。 */
                MENU_Draw(&ui);
            } break;

            case UI_SPECTRUM:
                /* 在频谱界面按 OK：切换到 Top4 峰值 */
                ui.state = UI_PEAKS; MENU_Draw(&ui);
                break;

            case UI_PEAKS:
                /* 在 Top4 界面按 OK：返回频谱 */
                ui.state = UI_SPECTRUM; MENU_Draw(&ui);
                break;

            default: break;
        }
    }

    /* ====== 采样 → FFT → 频谱图（仅在 UI_SPECTRUM 时绘“图像”） ====== */
    uint8_t which;
    if (CAP_FetchAndClearProcessFlag(&which)) {
    	const uint16_t *cur = (which == 0) ? half0 : half1;

    	    /* 用 TIM6 的实际分频计算出真实采样率 */
    	    float fs_real = get_fs_real_from_tim6();

    	    float peak_hz = 0.f;
    	    DSP_Process_Block_u16_to_db(cur, APP_FFT_SIZE, fs_real, g_mag_db, &peak_hz);  // ← 传 fs_real

    	    /* 频谱绘制/菜单峰值页也用同一 fs */
    	    MENU_UpdateSpectrum(g_mag_db, APP_FFT_SIZE/2, fs_real);
    	    if (ui.state == UI_SPECTRUM) {
    	        VIZ_Draw_Frame(cur, half_len, g_mag_db, peak_hz, fs_real);
    	    }
    }
    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_DIRECT_SMPS_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 5;
  RCC_OscInitStruct.PLL.PLLN = 96;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV1;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief Peripherals Common Clock Configuration
  * @retval None
  */
void PeriphCommonClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Initializes the peripherals clock
  */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_ADC|RCC_PERIPHCLK_USART3;
  PeriphClkInitStruct.PLL2.PLL2M = 2;
  PeriphClkInitStruct.PLL2.PLL2N = 12;
  PeriphClkInitStruct.PLL2.PLL2P = 2;
  PeriphClkInitStruct.PLL2.PLL2Q = 2;
  PeriphClkInitStruct.PLL2.PLL2R = 2;
  PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_3;
  PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOMEDIUM;
  PeriphClkInitStruct.PLL2.PLL2FRACN = 0;
  PeriphClkInitStruct.Usart234578ClockSelection = RCC_USART234578CLKSOURCE_PLL2;
  PeriphClkInitStruct.AdcClockSelection = RCC_ADCCLKSOURCE_PLL2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

 /* MPU Configuration */

void MPU_Config(void)
{
	MPU_Region_InitTypeDef r;

	  HAL_MPU_Disable();

	  /* Region 1: D1 AXI SRAM (0x24000000, 512KB) - Cacheable, Write-Back, Non-shareable */
	  r.Enable           = MPU_REGION_ENABLE;
	  r.Number           = MPU_REGION_NUMBER1;
	  r.BaseAddress      = 0x24000000;
	  r.Size             = MPU_REGION_SIZE_512KB;
	  r.SubRegionDisable = 0x00;
	  r.TypeExtField     = MPU_TEX_LEVEL1;
	  r.AccessPermission = MPU_REGION_FULL_ACCESS;
	  r.DisableExec      = MPU_INSTRUCTION_ACCESS_ENABLE;
	  r.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
	  r.IsCacheable      = MPU_ACCESS_CACHEABLE;
	  r.IsBufferable     = MPU_ACCESS_BUFFERABLE;
	  HAL_MPU_ConfigRegion(&r);

	  /* Region 2: D2 SRAM1/2 (0x30000000, 256KB) - Non-Cacheable, Shareable  —— 给 DMA 缓冲 */
	  r.Enable           = MPU_REGION_ENABLE;
	  r.Number           = MPU_REGION_NUMBER2;
	  r.BaseAddress      = 0x30000000;
	  r.Size             = MPU_REGION_SIZE_256KB;   // 覆盖 SRAM1+SRAM2
	  r.SubRegionDisable = 0x00;
	  r.TypeExtField     = MPU_TEX_LEVEL0;
	  r.AccessPermission = MPU_REGION_FULL_ACCESS;
	  r.DisableExec      = MPU_INSTRUCTION_ACCESS_ENABLE;
	  r.IsShareable      = MPU_ACCESS_SHAREABLE;
	  r.IsCacheable      = MPU_ACCESS_NOT_CACHEABLE;
	  r.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
	  HAL_MPU_ConfigRegion(&r);

	  /* (可选) Region 3: D3 SRAM4 (0x38000000, 64KB) - Cacheable */
	  r.Enable           = MPU_REGION_ENABLE;
	  r.Number           = MPU_REGION_NUMBER3;
	  r.BaseAddress      = 0x38000000;
	  r.Size             = MPU_REGION_SIZE_64KB;
	  r.SubRegionDisable = 0x00;
	  r.TypeExtField     = MPU_TEX_LEVEL1;
	  r.AccessPermission = MPU_REGION_FULL_ACCESS;
	  r.DisableExec      = MPU_INSTRUCTION_ACCESS_ENABLE;
	  r.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
	  r.IsCacheable      = MPU_ACCESS_CACHEABLE;
	  r.IsBufferable     = MPU_ACCESS_BUFFERABLE;
	  HAL_MPU_ConfigRegion(&r);

	  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

	  /* 建议：紧接着开启 Cache（若你工程中未开） */
	  SCB_EnableICache();
	  SCB_EnableDCache();
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
