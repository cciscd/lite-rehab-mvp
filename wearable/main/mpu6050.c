#include "mpu6050.h"

#include <string.h>
#include "esp_check.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define MPU6050_ADDRESS_PRIMARY 0x68
#define MPU6050_ADDRESS_ALT 0x69
#define REG_SMPLRT_DIV 0x19
#define REG_CONFIG 0x1A
#define REG_GYRO_CONFIG 0x1B
#define REG_ACCEL_CONFIG 0x1C
#define REG_ACCEL_XOUT_H 0x3B
#define REG_PWR_MGMT_1 0x6B
#define REG_WHO_AM_I 0x75

static esp_err_t write_register(mpu6050_t *sensor, uint8_t reg, uint8_t value)
{
    const uint8_t data[2] = {reg, value};
    return i2c_master_transmit(sensor->device, data, sizeof(data), 100);
}

static esp_err_t read_registers(mpu6050_t *sensor, uint8_t reg,
                                uint8_t *data, size_t length)
{
    return i2c_master_transmit_receive(sensor->device, &reg, 1, data, length, 100);
}

static int16_t be16(const uint8_t *bytes)
{
    return (int16_t)(((uint16_t)bytes[0] << 8) | bytes[1]);
}

esp_err_t mpu6050_init(mpu6050_t *sensor, i2c_master_bus_handle_t bus)
{
    if (sensor == NULL || bus == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    memset(sensor, 0, sizeof(*sensor));
    static const char *TAG = "mpu6050";
    const uint8_t addresses[] = {MPU6050_ADDRESS_PRIMARY, MPU6050_ADDRESS_ALT};
    for (int attempt = 0; attempt < 2; ++attempt) {
        const uint8_t addr = addresses[attempt];
        ESP_LOGI(TAG, "trying address 0x%02X", addr);
        i2c_device_config_t config = {
            .dev_addr_length = I2C_ADDR_BIT_LEN_7,
            .device_address = addr,
            .scl_speed_hz = 100000,
        };
        if (i2c_master_bus_add_device(bus, &config, &sensor->device) != ESP_OK) {
            ESP_LOGW(TAG, "add_device failed for 0x%02X", addr);
            continue;
        }
        uint8_t identity = 0;
        if (read_registers(sensor, REG_WHO_AM_I, &identity, 1) != ESP_OK) {
            ESP_LOGW(TAG, "WHO_AM_I read failed for 0x%02X", addr);
            i2c_master_bus_rm_device(sensor->device);
            sensor->device = NULL;
            continue;
        }
        ESP_LOGI(TAG, "WHO_AM_I = 0x%02X at address 0x%02X", identity, addr);
        ESP_RETURN_ON_ERROR(write_register(sensor, REG_PWR_MGMT_1, 0x01),
                            "mpu6050", "wake");
        ESP_RETURN_ON_ERROR(write_register(sensor, REG_SMPLRT_DIV, 19),
                            "mpu6050", "sample divider");
        ESP_RETURN_ON_ERROR(write_register(sensor, REG_CONFIG, 0x03),
                            "mpu6050", "filter");
        ESP_RETURN_ON_ERROR(write_register(sensor, REG_GYRO_CONFIG, 0x00),
                            "mpu6050", "gyro range");
        return write_register(sensor, REG_ACCEL_CONFIG, 0x00);
    }
    return ESP_ERR_NOT_FOUND;
}

esp_err_t mpu6050_read(mpu6050_t *sensor, mpu6050_sample_t *sample)
{
    if (sensor == NULL || sample == NULL || sensor->device == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    uint8_t bytes[14];
    ESP_RETURN_ON_ERROR(read_registers(sensor, REG_ACCEL_XOUT_H,
                                       bytes, sizeof(bytes)),
                        "mpu6050", "sample");
    for (int i = 0; i < 3; ++i) {
        sample->accel[i] = be16(&bytes[i * 2]);
        sample->gyro[i] = (int16_t)(be16(&bytes[8 + i * 2]) - sensor->gyro_bias[i]);
    }
    return ESP_OK;
}

esp_err_t mpu6050_calibrate(mpu6050_t *sensor, unsigned sample_count)
{
    if (sensor == NULL || sample_count == 0) {
        return ESP_ERR_INVALID_ARG;
    }
    int64_t sums[3] = {0};
    sensor->gyro_bias[0] = sensor->gyro_bias[1] = sensor->gyro_bias[2] = 0;
    for (unsigned i = 0; i < sample_count; ++i) {
        mpu6050_sample_t sample;
        ESP_RETURN_ON_ERROR(mpu6050_read(sensor, &sample),
                            "mpu6050", "calibration read");
        for (int axis = 0; axis < 3; ++axis) {
            sums[axis] += sample.gyro[axis];
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }
    for (int axis = 0; axis < 3; ++axis) {
        sensor->gyro_bias[axis] = (int32_t)(sums[axis] / sample_count);
    }
    return ESP_OK;
}
