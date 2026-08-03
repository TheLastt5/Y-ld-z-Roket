#include "ms5611_yrt.h"

YRT_STATUS_t MS5611_Init(I2C_HandleTypeDef *hi2c, MS5611_t *sensor_data, uint16_t i2c_addr) {
    uint8_t cmd;
    uint8_t buffer[2];

    /* Varsayılan atamalar Init fonksiyonunda yapılıyor */
    sensor_data->i2c_addr = i2c_addr;
    sensor_data->osr_value = MS5611_OSR_4096; 
    sensor_data->reference_pressure = 1013.25f; 
    sensor_data->filter_value = 1.0f; 
    sensor_data->filtered_altitude = 0.0f;
    
    sensor_data->state = MS5611_STATE_IDLE;
    sensor_data->wait_time_ms = 10; 

    /* Sensöre Reset Atma */
    cmd = MS5611_COMMAND_RESET;
    if (HAL_I2C_Master_Transmit(hi2c, sensor_data->i2c_addr, &cmd, 1, 100) != HAL_OK) {
        return YRT_ERROR;
    }
    HAL_Delay(3); /* Reset sonrası zorunlu bekleme */

    /* Kalibrasyon (PROM) Verilerini Okuma */
    cmd = MS5611_COMMAND_C1;
    HAL_I2C_Master_Transmit(hi2c, sensor_data->i2c_addr, &cmd, 1, 100);
    HAL_I2C_Master_Receive(hi2c, sensor_data->i2c_addr, buffer, 2, 100);
    sensor_data->C1 = (buffer[0] << 8) | buffer[1];

    cmd = MS5611_COMMAND_C2;
    HAL_I2C_Master_Transmit(hi2c, sensor_data->i2c_addr, &cmd, 1, 100);
    HAL_I2C_Master_Receive(hi2c, sensor_data->i2c_addr, buffer, 2, 100);
    sensor_data->C2 = (buffer[0] << 8) | buffer[1];
    
    cmd = MS5611_COMMAND_C3;
    HAL_I2C_Master_Transmit(hi2c, sensor_data->i2c_addr, &cmd, 1, 100);
    HAL_I2C_Master_Receive(hi2c, sensor_data->i2c_addr, buffer, 2, 100);
    sensor_data->C3 = (buffer[0] << 8) | buffer[1];

    cmd = MS5611_COMMAND_C4;
    HAL_I2C_Master_Transmit(hi2c, sensor_data->i2c_addr, &cmd, 1, 100);
    HAL_I2C_Master_Receive(hi2c, sensor_data->i2c_addr, buffer, 2, 100);
    sensor_data->C4 = (buffer[0] << 8) | buffer[1];

    cmd = MS5611_COMMAND_C5;
    HAL_I2C_Master_Transmit(hi2c, sensor_data->i2c_addr, &cmd, 1, 100);
    HAL_I2C_Master_Receive(hi2c, sensor_data->i2c_addr, buffer, 2, 100);
    sensor_data->C5 = (buffer[0] << 8) | buffer[1];

    cmd = MS5611_COMMAND_C6;
    HAL_I2C_Master_Transmit(hi2c, sensor_data->i2c_addr, &cmd, 1, 100);
    HAL_I2C_Master_Receive(hi2c, sensor_data->i2c_addr, buffer, 2, 100);
    sensor_data->C6 = (buffer[0] << 8) | buffer[1];

    /* OSR Değerine Göre ADC Dönüşüm Bekleme Süresi */
    switch(sensor_data->osr_value) {
        case MS5611_OSR_4096: sensor_data->wait_time_ms = 10; break;
        case MS5611_OSR_2048: sensor_data->wait_time_ms = 5;  break;
        case MS5611_OSR_1024: sensor_data->wait_time_ms = 3;  break;
        case MS5611_OSR_512:  sensor_data->wait_time_ms = 2;  break;
        case MS5611_OSR_256:  sensor_data->wait_time_ms = 1;  break;
        default: sensor_data->wait_time_ms = 10; break;
    }

    return YRT_OK;
}

void MS5611_Update(I2C_HandleTypeDef *hi2c, MS5611_t *sensor_data) {
    uint8_t cmd;
    uint8_t buffer[3];
    uint32_t current_tick = HAL_GetTick();

    switch (sensor_data->state) {
        
        case MS5611_STATE_IDLE:
            sensor_data->state = MS5611_STATE_REQ_D1;
            break;

        case MS5611_STATE_REQ_D1:
            cmd = 0x40 + sensor_data->osr_value;
            if (HAL_I2C_Master_Transmit(hi2c, sensor_data->i2c_addr, &cmd, 1, 100) == HAL_OK) {
                sensor_data->last_tick = current_tick;
                sensor_data->state = MS5611_STATE_WAIT_D1;
            }
            break;

        case MS5611_STATE_WAIT_D1:
            if ((current_tick - sensor_data->last_tick) >= sensor_data->wait_time_ms) {
                sensor_data->state = MS5611_STATE_READ_D1;
            }
            break;

        case MS5611_STATE_READ_D1:
            cmd = MS5611_COMMAND_ADC;
            if (HAL_I2C_Master_Transmit(hi2c, sensor_data->i2c_addr, &cmd, 1, 100) == HAL_OK) {
                if (HAL_I2C_Master_Receive(hi2c, sensor_data->i2c_addr, buffer, 3, 100) == HAL_OK) {
                    sensor_data->D1 = (buffer[0] << 16) | (buffer[1] << 8) | buffer[2];
                    sensor_data->state = MS5611_STATE_REQ_D2;
                }
            }
            break;

        case MS5611_STATE_REQ_D2:
            cmd = 0x50 + sensor_data->osr_value;
            if (HAL_I2C_Master_Transmit(hi2c, sensor_data->i2c_addr, &cmd, 1, 100) == HAL_OK) {
                sensor_data->last_tick = current_tick;
                sensor_data->state = MS5611_STATE_WAIT_D2;
            }
            break;

        case MS5611_STATE_WAIT_D2:
            if ((current_tick - sensor_data->last_tick) >= sensor_data->wait_time_ms) {
                sensor_data->state = MS5611_STATE_READ_D2;
            }
            break;

        case MS5611_STATE_READ_D2:
            cmd = MS5611_COMMAND_ADC;
            if (HAL_I2C_Master_Transmit(hi2c, sensor_data->i2c_addr, &cmd, 1, 100) == HAL_OK) {
                if (HAL_I2C_Master_Receive(hi2c, sensor_data->i2c_addr, buffer, 3, 100) == HAL_OK) {
                    sensor_data->D2 = (buffer[0] << 16) | (buffer[1] << 8) | buffer[2];
                    sensor_data->state = MS5611_STATE_CALCULATE;
                }
            }
            break;

        case MS5611_STATE_CALCULATE:
            MS5611_Calculate(sensor_data);
            sensor_data->state = MS5611_STATE_IDLE; 
            break;
    }
}

void MS5611_Calculate(MS5611_t *sensor_data) {
    int32_t TEMP;
    int32_t dT;
    int64_t OFF;
    int64_t SENS;
    int32_t P;
    int64_t T2 = 0, OFF2 = 0, SENS2 = 0;

    dT = sensor_data->D2 - ((uint32_t)sensor_data->C5 * 256);
    TEMP = 2000 + (((int64_t)dT * sensor_data->C6) >> 23);
    OFF = ((int64_t)sensor_data->C2 << 16) + (((int64_t)sensor_data->C4 * dT) >> 7);
    SENS = ((int64_t)sensor_data->C1 << 15) + (((int64_t)sensor_data->C3 * dT) >> 8);

    /* 2. Derece Sıcaklık Kompanzasyonu */
    if (TEMP < 2000) {
        T2 = ((int64_t)dT * dT) >> 31;
        OFF2 = 5 * (((int64_t)TEMP - 2000) * (TEMP - 2000)) / 2;
        SENS2 = 5 * (((int64_t)TEMP - 2000) * (TEMP - 2000)) / 4;

        if (TEMP < -1500) {
            OFF2 = OFF2 + 7 * (((int64_t)TEMP + 1500) * (TEMP + 1500));
            SENS2 = SENS2 + 11 * (((int64_t)TEMP + 1500) * (TEMP + 1500)) / 2;
        }

        TEMP = TEMP - T2;
        OFF = OFF - OFF2;
        SENS = SENS - SENS2;
    }

    P = ((((int64_t)sensor_data->D1 * SENS) >> 21) - OFF) >> 15;

    sensor_data->temperature = (float)TEMP / 100.0f; 
    sensor_data->pressure = (float)P / 100.0f; 
    sensor_data->altitude = 44330.0f * (1.0f - pow((sensor_data->pressure / sensor_data->reference_pressure), 0.190284f));

    /* Basit Alçak Geçiren Filtre (Low-Pass Filter) */
    sensor_data->filtered_altitude = (sensor_data->filtered_altitude * (1.0f - sensor_data->filter_value)) + (sensor_data->altitude * sensor_data->filter_value);
}

void MS5611_SetReferencePressure(MS5611_t *sensor_data) {
    sensor_data->reference_pressure = sensor_data->pressure;
}
