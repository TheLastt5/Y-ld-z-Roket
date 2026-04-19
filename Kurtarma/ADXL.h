#ifndef __ADXL_H
#define __ADXL_H

#include "main.h"

// I2C Donanımı
extern I2C_HandleTypeDef hi2c1;
#define ADXL_ADDR          (0x53 << 1)

// Register Adresleri
#define ADXL_REG_DEVID     0x00
#define ADXL_REG_POWER_CTL 0x2D
#define ADXL_REG_DATA_F    0x31
#define ADXL_REG_DATAX0    0x32
#define ADXL_REG_INT_SOURCE 0x30

// Yapılandırma Sabitleri
#define ADXL_RANGE_2G      0x00
#define ADXL_FULL_RES      0x08

typedef struct {
    uint8_t Range;
    uint8_t Resolution;
} ADXL_Config;

// Fonksiyon Prototipleri
uint8_t ADXL_Init(ADXL_Config *config);
void ADXL_ReadAccel(float *x, float *y, float *z);
void ADXL_IntProto(void);

#endif
