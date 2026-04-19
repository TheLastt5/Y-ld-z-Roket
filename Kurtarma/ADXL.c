#include "ADXL.h"

static void ADXL_Write(uint8_t reg, uint8_t value) {
    HAL_I2C_Mem_Write(&hi2c1, ADXL_ADDR, reg, 1, &value, 1, 100);
}

static void ADXL_Read(uint8_t reg, uint8_t *data, uint8_t len) {
    HAL_I2C_Mem_Read(&hi2c1, ADXL_ADDR, reg, 1, data, len, 100);
}

uint8_t ADXL_Init(ADXL_Config *config) {
    uint8_t device_id = 0;
    ADXL_Read(ADXL_REG_DEVID, &device_id, 1);
    if (device_id != 0xE5) return 0;

    ADXL_Write(ADXL_REG_POWER_CTL, 0x08);
    ADXL_Write(ADXL_REG_DATA_F, config->Range | config->Resolution);
    return 1;
}

void ADXL_ReadAccel(float *x, float *y, float *z) {
    uint8_t buffer[6];
    ADXL_Read(ADXL_REG_DATAX0, buffer, 6);
    int16_t raw_x = (int16_t)(buffer[1] << 8) | buffer[0];
    int16_t raw_y = (int16_t)(buffer[3] << 8) | buffer[2];
    int16_t raw_z = (int16_t)(buffer[5] << 8) | buffer[4];
    *x = raw_x * 0.0039f;
    *y = raw_y * 0.0039f;
    *z = raw_z * 0.0039f;
}

void ADXL_IntProto(void) {
    uint8_t dummy;
    ADXL_Read(ADXL_REG_INT_SOURCE, &dummy, 1); // Bayrakları temizle
}
