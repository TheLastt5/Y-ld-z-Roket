#ifndef MS5611_YRT_H
#define MS5611_YRT_H

#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <math.h>

/* I2C Adresleri (CSB pininin durumuna göre değişir) */
#define MS5611_I2C_ADDR_CSB_LOW  (0x77 << 1) 
#define MS5611_I2C_ADDR_CSB_HIGH (0x76 << 1)

/* Komut Setleri */
#define MS5611_COMMAND_RESET   0x1E
#define MS5611_COMMAND_C1      0xA2
#define MS5611_COMMAND_C2      0xA4
#define MS5611_COMMAND_C3      0xA6
#define MS5611_COMMAND_C4      0xA8
#define MS5611_COMMAND_C5      0xAA
#define MS5611_COMMAND_C6      0xAC
#define MS5611_COMMAND_ADC     0x00

typedef enum {
    YRT_OK,
    YRT_ERROR
} YRT_STATUS_t;

/* OSR (Oversampling Ratio) Değerleri */
typedef enum {
    MS5611_OSR_256  = 0x00,
    MS5611_OSR_512  = 0x02,
    MS5611_OSR_1024 = 0x04,
    MS5611_OSR_2048 = 0x06,
    MS5611_OSR_4096 = 0x08
} MS5611_OSR_t;

/* Sonlu Durum Makinesi (State Machine) Durumları */
typedef enum {
    MS5611_STATE_IDLE,
    MS5611_STATE_REQ_D1,
    MS5611_STATE_WAIT_D1,
    MS5611_STATE_READ_D1,
    MS5611_STATE_REQ_D2,
    MS5611_STATE_WAIT_D2,
    MS5611_STATE_READ_D2,
    MS5611_STATE_CALCULATE
} MS5611_State_t;

/* Sensör Veri Yapısı */
typedef struct {
    uint16_t i2c_addr; 
    
    uint16_t C1, C2, C3, C4, C5, C6;
    uint32_t D1, D2;

    float temperature;
    float pressure;
    float reference_pressure;
    float altitude;

    float filter_value;
    float filtered_altitude;

    MS5611_OSR_t osr_value;
    
    /* State Machine Takip Değişkenleri */
    MS5611_State_t state;
    uint32_t last_tick;
    uint32_t wait_time_ms;
} MS5611_t;

/* Fonksiyon Prototipleri */
YRT_STATUS_t MS5611_Init(I2C_HandleTypeDef *hi2c, MS5611_t *sensor_data, uint16_t i2c_addr);
void MS5611_Update(I2C_HandleTypeDef *hi2c, MS5611_t *sensor_data);
void MS5611_Calculate(MS5611_t *sensor_data);
void MS5611_SetReferencePressure(MS5611_t *sensor_data);

#endif
