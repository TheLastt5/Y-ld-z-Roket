/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "cmsis_os.h"
#include "adc.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "bmp180_for_stm32_hal.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "ADXL.h"
#include <stdbool.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef union {
    float sayi_f32;
    uint8_t array[4];
} FLOAT32_UINT8_DONUSTURUCU;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define HEDEF_IRTIFA       8000.0  // Mert dedi
#define ANA_PARASUT_HEDEF  2000.0  // Mert dedi
#define APOGEE_ESIK_DEGERI 5.0 // Doğruluktan emin olmak için 5 metre düşüş istiyorum
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
float x_ekseni, y_ekseni, z_ekseni;
uint32_t pot_degeri = 0;
float ucus_sicaklik = 0;
float ucus_basinc = 0;
float ucus_irtifa = 0;
float yer_basinci = 0.0;
volatile bool SUT_Modu_Aktif = false;
volatile bool paket_hazir_mi = false;
volatile uint8_t sut_buffer[34];
uint8_t rx_byte;
uint8_t rx_index = 0;
float aci_x = 0.0, aci_y = 0.0, aci_z = 0.0;
// Filtre Değişkenleri
#define FILTRE_BOYUTU 10
float irtifa_dizisi[FILTRE_BOYUTU];
uint8_t filtre_indeks = 0;
float filtrelenmis_irtifa = 0.0;

// Roket Durumu
UcusDurumu roket_durumu = BEKLEME;
float max_irtifa = 0.0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */
bool verify_checksum(uint8_t *packet, uint8_t length);
float get_float_from_packet(uint8_t *packet, int start_index);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
// Hareketli Ortalama Filtresi tabi doğruysa
float Hareketli_Ortalama_Filtresi(float yeni_deger) {
    irtifa_dizisi[filtre_indeks] = yeni_deger;
    filtre_indeks++;

    if (filtre_indeks >= FILTRE_BOYUTU) {
        filtre_indeks = 0; // Dizinin başına dön işler karışmasın
    }

    float toplam = 0.0;
    for (int i = 0; i < FILTRE_BOYUTU; i++) {
        toplam += irtifa_dizisi[i];
    }
    return (toplam / FILTRE_BOYUTU);
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_ADC1_Init();
  MX_TIM6_Init();
  MX_I2C1_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  ADXL_Config ivme_ayari;
    ivme_ayari.Range = ADXL_RANGE_2G;
    ivme_ayari.Resolution = ADXL_FULL_RES;

    ADXL_Init(&ivme_ayari);
    HAL_TIM_Base_Start_IT(&htim6);
      BMP180_Init(&hi2c1);
      BMP180_SetOversampling(BMP180_ULTRA);
      BMP180_UpdateCalibrationData();

      // Rampada Sıfırlama 1 saniye olsun diye 50 x 20 yaptım
      float toplam_basinc = 0;
      for(int i = 0; i < 50; i++) {
          toplam_basinc += ((float)BMP180_GetPressure() / 100.0f);
          HAL_Delay(20);
      }
      yer_basinci = toplam_basinc / 50.0;

      HAL_UART_Receive_IT(&huart2, &rx_byte, 1);
  /* USER CODE END 2 */

  /* Call init function for freertos objects (in cmsis_os2.c) */
  MX_FREERTOS_Init();

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
    {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

      osDelay(100);
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if(GPIO_Pin == GPIO_PIN_6){
    ADXL_IntProto();
  }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if(huart->Instance == USART2) {

        sut_buffer[rx_index] = rx_byte; // Gelen 1 baytı diziye koy

        if (rx_index == 0) {
            if (sut_buffer[0] == 0xAB) {
                rx_index++;
            }
            else if (sut_buffer[0] == 0xAA) {
                rx_index++;
            }

        }
        // Eğer 2 baytlık bir SUT komutu geldiyse
        else if (rx_index == 1 && sut_buffer[0] == 0xAA) {
            if (sut_buffer[1] == 0x22) SUT_Modu_Aktif = true;
            if (sut_buffer[1] == 0x24) SUT_Modu_Aktif = false;

            rx_index = 0;
        }
        else {
            rx_index++;
            if (rx_index >= 34) {
                if (SUT_Modu_Aktif == true) {
                    paket_hazir_mi = true;
                }
                rx_index = 0; // Bir sonraki paket için indeksi başa sar
            }
        }

        // Bir sonraki 1 baytı yakalamak için kesmeyi tekrar kur
        HAL_UART_Receive_IT(&huart2, &rx_byte, 1);
    }
}

bool verify_checksum(uint8_t *packet, uint8_t length) {
    uint16_t sum = 0;

    for(int i = 0; i < (length - 3); i++) {
        sum += packet[i];
    }

    uint8_t calculated_checksum = sum % 256;

    return (calculated_checksum == packet[length - 3]);
}
float get_float_from_packet(uint8_t *packet, int start_index) {
    FLOAT32_UINT8_DONUSTURUCU converter;

    converter.array[3] = packet[start_index];
    converter.array[2] = packet[start_index + 1];
    converter.array[1] = packet[start_index + 2];
    converter.array[0] = packet[start_index + 3];

    return converter.sayi_f32;
}
/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM7 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM7)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
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
