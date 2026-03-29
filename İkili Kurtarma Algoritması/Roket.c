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
#include "bmp180_for_stm32_hal.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

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
ADC_HandleTypeDef hadc1;

I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim6;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
uint32_t pot_degeri = 0;
float ucus_sicaklik = 0;
float ucus_basinc = 0;
float ucus_irtifa = 0;
float yer_basinci = 0.0;
// FİLTRE DEĞİŞKENLERİ
#define FILTRE_BOYUTU 10
float irtifa_dizisi[FILTRE_BOYUTU];
uint8_t filtre_indeks = 0;
float filtrelenmis_irtifa = 0.0;

// YENİ EKLENENLER: Roket Uçuş Durumları
typedef enum {
    BEKLEME,            // fırlatma bekleniyor
    YUKSELIS,           // Motor ateşlendi, irtifa artıyor
    APOGEE_TESPIT,      // Tepe noktası
    DUSUS,              // Sürüklenme paraşütü
    ANA_PARASUT_ACIK,
    INIS_TAMAM
} UcusDurumu;

UcusDurumu roket_durumu = BEKLEME;

float max_irtifa = 0.0;

/* USER CODE END PV */

