/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "bmp180_for_stm32_hal.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "usart.h"
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

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
extern float x_ekseni, y_ekseni, z_ekseni;
extern float ucus_sicaklik;
extern float ucus_basinc;
extern float ucus_sicaklik;
extern float ucus_basinc;
extern float ucus_irtifa;
extern float yer_basinci;
extern float filtrelenmis_irtifa;
extern float max_irtifa;
extern UcusDurumu roket_durumu;
extern float Hareketli_Ortalama_Filtresi(float yeni_deger);
uint8_t motor_sondu = 1;
extern volatile bool SUT_Modu_Aktif;
extern volatile bool paket_hazir_mi;
extern float aci_x, aci_y, aci_z;
extern uint8_t sut_buffer[34];
/* USER CODE END Variables */
osThreadId defaultTaskHandle;
osThreadId SensorTaskHandle;
osThreadId ApogeeTaskHandle;
osThreadId AnaParasutTaskHandle;
osThreadId TelemetriTaskHandle;
osThreadId SinyalTaskHandle;
osSemaphoreId btnSemHandle;
osSemaphoreId ApogeeSemaphoreHandle;
osSemaphoreId AnaParasutSemaphoreHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
bool verify_checksum(uint8_t *packet, uint8_t length);
float get_float_from_packet(uint8_t *packet, int start_index);
void insert_float_to_packet(float value, uint8_t *packet, int start_index) {

    float rounded_value = roundf(value * 100.0f) / 100.0f;

    FLOAT32_UINT8_DONUSTURUCU converter;
    converter.sayi_f32 = rounded_value;

    packet[start_index] = converter.array[3];     // M
    packet[start_index + 1] = converter.array[2];
    packet[start_index + 2] = converter.array[1];
    packet[start_index + 3] = converter.array[0]; // L
}
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);
void StartSensorTask(void const * argument);
void StartApogeeTask(void const * argument);
void StartAnaParasutTask(void const * argument);
void StartTelemetriTask(void const * argument);
void StartSinyalTask(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* definition and creation of btnSem */
  osSemaphoreDef(btnSem);
  btnSemHandle = osSemaphoreCreate(osSemaphore(btnSem), 1);

  /* definition and creation of ApogeeSemaphore */
  osSemaphoreDef(ApogeeSemaphore);
  ApogeeSemaphoreHandle = osSemaphoreCreate(osSemaphore(ApogeeSemaphore), 1);

  /* definition and creation of AnaParasutSemaphore */
  osSemaphoreDef(AnaParasutSemaphore);
  AnaParasutSemaphoreHandle = osSemaphoreCreate(osSemaphore(AnaParasutSemaphore), 1);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* definition and creation of SensorTask */
  osThreadDef(SensorTask, StartSensorTask, osPriorityHigh, 0, 256);
  SensorTaskHandle = osThreadCreate(osThread(SensorTask), NULL);

  /* definition and creation of ApogeeTask */
  osThreadDef(ApogeeTask, StartApogeeTask, osPriorityBelowNormal, 0, 128);
  ApogeeTaskHandle = osThreadCreate(osThread(ApogeeTask), NULL);

  /* definition and creation of AnaParasutTask */
  osThreadDef(AnaParasutTask, StartAnaParasutTask, osPriorityBelowNormal, 0, 128);
  AnaParasutTaskHandle = osThreadCreate(osThread(AnaParasutTask), NULL);

  /* definition and creation of TelemetriTask */
  osThreadDef(TelemetriTask, StartTelemetriTask, osPriorityBelowNormal, 0, 256);
  TelemetriTaskHandle = osThreadCreate(osThread(TelemetriTask), NULL);

  /* definition and creation of SinyalTask */
  osThreadDef(SinyalTask, StartSinyalTask, osPriorityLow, 0, 128);
  SinyalTaskHandle = osThreadCreate(osThread(SinyalTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartSensorTask */
/**
* @brief Function implementing the SensorTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartSensorTask */
void StartSensorTask(void const * argument)
{
  /* USER CODE BEGIN StartSensorTask */
  /* Infinite loop */
  for(;;)
  {
      // 1. VERİ SEÇİMİ
	  if (SUT_Modu_Aktif == false) {
	  	            //NORMAL UÇUŞ MODU
	  	            ADXL_ReadAccel(&x_ekseni, &y_ekseni, &z_ekseni);
	  	            aci_x = atan2f(y_ekseni, sqrtf(x_ekseni * x_ekseni + z_ekseni * z_ekseni)) * 57.29578f;
	  	            aci_y = atan2f(-x_ekseni, sqrtf(y_ekseni * y_ekseni + z_ekseni * z_ekseni)) * 57.29578f;
	  	            aci_z = 0.0f;
	  	            ucus_sicaklik = BMP180_GetTemperature();

	                  // DEĞİŞİKLİK BURADA: Pa'dan mbar'a çeviriyoruz (100'e bölerek)
	  	            ucus_basinc = (float)BMP180_GetPressure() / 100.0f;

	  	            // İrtifa hesabı
	  	            ucus_irtifa = 44330.0 * (1.0 - pow((ucus_basinc / yer_basinci), (1.0 / 5.255)));

	  	            filtrelenmis_irtifa = Hareketli_Ortalama_Filtresi(ucus_irtifa);
	  	        }
	        else {
	            // SUT TEST MODU
	            if (paket_hazir_mi == true) {

	                ucus_irtifa = get_float_from_packet((uint8_t*)sut_buffer, 1);

	                if (yer_basinci > 0) {
	                    ucus_basinc = 101325.0f * pow((1.0 - (ucus_irtifa / 44330.0)), 5.255);
	                }

	                ucus_sicaklik = 25.0f - (0.0065f * ucus_irtifa);
	                z_ekseni = get_float_from_packet((uint8_t*)sut_buffer, 17);

	                // FİLTREYİ BURAYA ALDIK
	                filtrelenmis_irtifa = Hareketli_Ortalama_Filtresi(ucus_irtifa);

	                paket_hazir_mi = false;
	            }
	        }
      //2. ORTAK İŞLEMLER
      filtrelenmis_irtifa = Hareketli_Ortalama_Filtresi(ucus_irtifa);

      // 3. KURTARMA ALGORİTMASI-
      switch(roket_durumu) {
          case BEKLEME:
              if(filtrelenmis_irtifa > 2.0) {
                  roket_durumu = YUKSELIS;
              }
              break;

          case YUKSELIS:
              if(motor_sondu == 0) {
                  if(z_ekseni < 0.0) {
                      motor_sondu = 1;
                  }
              }
              else {
                  if(filtrelenmis_irtifa > max_irtifa) {
                      max_irtifa = filtrelenmis_irtifa;
                  }
                  else if((max_irtifa - filtrelenmis_irtifa) > APOGEE_ESIK_DEGERI) {
                      xSemaphoreGive(ApogeeSemaphoreHandle);
                      roket_durumu = APOGEE_TESPIT;
                  }
              }
              break;

          case APOGEE_TESPIT:
        	  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
        	  roket_durumu = DUSUS;
              break;

          case DUSUS:
              if(filtrelenmis_irtifa <= ANA_PARASUT_HEDEF) {
                  // ANA PARAŞÜT
                  xSemaphoreGive(AnaParasutSemaphoreHandle);
                  roket_durumu = ANA_PARASUT_ACIK;
              }
              break;

          case ANA_PARASUT_ACIK:
              if(filtrelenmis_irtifa < 10.0) {
                  roket_durumu = INIS_TAMAM;
              }
              break;

          case INIS_TAMAM:
                        if(filtrelenmis_irtifa < 2.0) {
                            roket_durumu = BEKLEME;
                            max_irtifa = 0.0;
                            motor_sondu = 1;

                            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
                            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET);
                            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
                        }
                        break;
      }

      osDelay(20);
  }
}
  /* USER CODE END StartSensorTask */


/* USER CODE BEGIN Header_StartApogeeTask */
/**
* @brief Function implementing the ApogeeTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartApogeeTask */
void StartApogeeTask(void const * argument)
{
  /* USER CODE BEGIN StartApogeeTask */
	  /* Infinite loop */
	  for(;;)
	  {
		  if (xSemaphoreTake(ApogeeSemaphoreHandle, portMAX_DELAY) == pdTRUE) {
			  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET);
		        }
	  }
  /* USER CODE END StartApogeeTask */
}

/* USER CODE BEGIN Header_StartAnaParasutTask */
/**
* @brief Function implementing the AnaParasutTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartAnaParasutTask */
void StartAnaParasutTask(void const * argument)
{
  /* USER CODE BEGIN StartAnaParasutTask */
	  /* Infinite loop */
	  for(;;)
	  {
	      if (xSemaphoreTake(AnaParasutSemaphoreHandle, portMAX_DELAY) == pdTRUE) {

	    	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);

	      }
	  }
  /* USER CODE END StartAnaParasutTask */
}

/* USER CODE BEGIN Header_StartTelemetriTask */
/**
* @brief Function implementing the TelemetriTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTelemetriTask */
void StartTelemetriTask(void const * argument)
{
  /* USER CODE BEGIN StartTelemetriTask */
  uint8_t telemetri_paketi[36];

  /* Infinite loop */
  for(;;)
  {
      telemetri_paketi[0] = 0xAB;

      insert_float_to_packet(filtrelenmis_irtifa, telemetri_paketi, 1);
      insert_float_to_packet(ucus_basinc, telemetri_paketi, 5);

      // DEĞİŞİKLİK BURADA: İvme verilerini g'den m/s²'ye çevirip pakete ekliyoruz
      insert_float_to_packet((x_ekseni * 9.81f), telemetri_paketi, 9);
      insert_float_to_packet((y_ekseni * 9.81f), telemetri_paketi, 13);
      insert_float_to_packet((z_ekseni * 9.81f), telemetri_paketi, 17);

      insert_float_to_packet(aci_x, telemetri_paketi, 21);
      insert_float_to_packet(aci_y, telemetri_paketi, 25);
      insert_float_to_packet(aci_z, telemetri_paketi, 29);

      uint32_t sum = 0;
      for(int i = 1; i <= 32; i++) {
          sum += telemetri_paketi[i];
      }
      telemetri_paketi[33] = (uint8_t)(sum % 256);

      telemetri_paketi[34] = 0x0D;
      telemetri_paketi[35] = 0x0A;

      HAL_UART_Transmit(&huart2, telemetri_paketi, 36, 100);
      osDelay(100);
  }
  /* USER CODE END StartTelemetriTask */
}
  /* USER CODE END StartTelemetriTask */

/* USER CODE BEGIN Header_StartSinyalTask */
/**
* @brief Function implementing the SinyalTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartSinyalTask */
void StartSinyalTask(void const * argument)
{
  /* USER CODE BEGIN StartSinyalTask */
  /* Infinite loop */
	for(;;)
	  {
		osDelay(50);
	  }
  /* USER CODE END StartSinyalTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */
