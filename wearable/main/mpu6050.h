#ifndef LITE_REHAB_MPU6050_H
#define LITE_REHAB_MPU6050_H

#include <stdint.h>
#include "driver/i2c_master.h"
#include "esp_err.h"

typedef struct {
    int16_t accel[3];
    int16_t gyro[3];
} mpu6050_sample_t;

typedef struct {
    i2c_master_dev_handle_t device;
    int32_t gyro_bias[3];
} mpu6050_t;

esp_err_t mpu6050_init(mpu6050_t *sensor, i2c_master_bus_handle_t bus);
esp_err_t mpu6050_calibrate(mpu6050_t *sensor, unsigned sample_count);
esp_err_t mpu6050_read(mpu6050_t *sensor, mpu6050_sample_t *sample);

#endif
