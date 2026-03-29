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
  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start_IT(&htim6);
    BMP180_Init(&hi2c1);
    BMP180_SetOversampling(BMP180_ULTRA);
    BMP180_UpdateCalibrationData();

    // Rampada Sıfırlama 1 saniye olsun diye 50 x 20 yaptım 
    float toplam_basinc = 0;
    for(int i = 0; i < 50; i++) {
        toplam_basinc += (float)BMP180_GetPressure();
        HAL_Delay(20);
    }
    yer_basinci = toplam_basinc / 50.0; // Yerin referans basıncı hesaplandı
    while (1)
    {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    	ucus_sicaklik = BMP180_GetTemperature();
    	    ucus_basinc = (float)BMP180_GetPressure();

    	    // İrtifa hesaplama (Hareket filtresine sokmadığım veri)
    	    ucus_irtifa = 44330.0 * (1.0 - pow((ucus_basinc / yer_basinci), (1.0 / 5.255)));

    	    // Filtreden geçirme işlemi (G kuvveti ve dalgalanmalara karşı)
    	    filtrelenmis_irtifa = Hareketli_Ortalama_Filtresi(ucus_irtifa);

    	    // ROKET KURTARMA ALGORİTMASI
    	    switch(roket_durumu) {

    	        case BEKLEME:
    	            // Roket 15 metre kalkarsa yükselmiştir 
    	            if(filtrelenmis_irtifa > 15.0) {
    	                roket_durumu = YUKSELIS;
    	            }
    	            break;

    	        case YUKSELIS:
    	            	            // Zirveyi  sürekli yenile
    	            	            if(filtrelenmis_irtifa > max_irtifa) {
    	            	                max_irtifa = filtrelenmis_irtifa;
    	            	            }
    	            	            // Zirveden 5 metre aşağı düştüysek, Apogee geçilmiştir!
    	            	            else if((max_irtifa - filtrelenmis_irtifa) > APOGEE_ESIK_DEGERI) {

    	            	                // 1. AYRILMA: Sürüklenme Paraşütünü Ateşle
    	            	                HAL_GPIO_WritePin(BARUT_APOGEE_GPIO_Port, BARUT_APOGEE_Pin, GPIO_PIN_SET);

    	            	                roket_durumu = APOGEE_TESPIT;
    	            	            }
    	            	            break;

    	        case APOGEE_TESPIT:
    	            roket_durumu = DUSUS;
    	            break;

    	        case DUSUS:
    	            // Merin verdiği ana paraşüt irtifasının altına inildi mi?
    	        	if(filtrelenmis_irtifa <= ANA_PARASUT_HEDEF) {

    	            	                // 2. AYRILMA: Ana Paraşütü Ateşle
    	            	                HAL_GPIO_WritePin(BARUT_ANA_GPIO_Port, BARUT_ANA_Pin, GPIO_PIN_SET);

    	            	                roket_durumu = ANA_PARASUT_ACIK;
    	            	            }
    	            	            break;

    	        case ANA_PARASUT_ACIK:
    	            // Yere indiğini anla
    	            if(filtrelenmis_irtifa < 10.0) {
    	                roket_durumu = INIS_TAMAM;
    	            }
    	            break;

    	        case INIS_TAMAM:

    	            break;
    	    }

    	    // Telemetri ham ve filtrelenmiş irtifa burda
    	    char telemetri_mesaji[120];
    	    sprintf(telemetri_mesaji, "Ham: %.1f m | Filtre: %.1f m | Max: %.1f m | Durum: %d\r\n", ucus_irtifa, filtrelenmis_irtifa, max_irtifa, roket_durumu);

    	    HAL_UART_Transmit(&huart2, (uint8_t*)telemetri_mesaji, strlen(telemetri_mesaji), 100);
    	    HAL_Delay(100);
    }
  /* USER CODE END 3 */

