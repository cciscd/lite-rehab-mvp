#include <stdio.h>
#include <string.h>

#include "ble_server.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "motion_logic.h"
#include "motion_packet.h"
#include "mpu6050.h"
#include "ssd1306.h"
#include "wearable_status.h"

#define I2C_SDA_GPIO 21
#define I2C_SCL_GPIO 22
#define SAMPLE_PERIOD_MS 20

static const char *TAG = "wearable";

static const char *state_name(motion_state_t state)
{
    switch (state) {
    case MOTION_STATE_FOREARM_ROTATION: return "ROTATE";
    case MOTION_STATE_ELBOW_FLEXION: return "ELBOW";
    default: return "IDLE";
    }
}

static const char *quality_name(motion_quality_t quality)
{
    switch (quality) {
    case MOTION_QUALITY_OK: return "OK";
    case MOTION_QUALITY_TOO_FAST: return "FAST";
    case MOTION_QUALITY_INSUFFICIENT_RANGE: return "RANGE";
    default: return "WAIT";
    }
}

static void update_display(ssd1306_t *display, const motion_result_t *result)
{
    if (!display->available) return;
    char count[20];
    snprintf(count, sizeof(count), "REPS %u", result->rep_count);
    ssd1306_clear(display);
    ssd1306_text(display, 0, "LITEREHAB");
    ssd1306_text(display, 2, state_name(result->state));
    ssd1306_text(display, 4, count);
    ssd1306_text(display, 6, quality_name(result->quality));
    ssd1306_text(display, 7, ble_server_connected() ? "BLE OK" : "BLE WAIT");
    if (ssd1306_flush(display) != ESP_OK) display->available = false;
}

void app_main(void)
{
    ESP_ERROR_CHECK(wearable_status_init());
    i2c_master_bus_handle_t bus = NULL;
    const i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = I2C_SDA_GPIO,
        .scl_io_num = I2C_SCL_GPIO,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &bus));

    ESP_LOGI(TAG, "I2C scan starting...");
    for (uint8_t addr = 1; addr < 127; ++addr) {
        if (i2c_master_probe(bus, addr, 10) == ESP_OK) {
            ESP_LOGI(TAG, "I2C device at 0x%02X", addr);
        }
    }
    ESP_LOGI(TAG, "I2C scan done");

    ssd1306_t display;
    if (ssd1306_init(&display, bus) != ESP_OK) {
        memset(&display, 0, sizeof(display));
        ESP_LOGW(TAG, "OLED not found; continuing without display");
    }

    mpu6050_t sensor;
    if (mpu6050_init(&sensor, bus) != ESP_OK) {
        wearable_status_set(false, true);
        if (display.available) {
            ssd1306_clear(&display);
            ssd1306_text(&display, 2, "MPU ERROR");
            ssd1306_flush(&display);
        }
        ESP_LOGE(TAG, "MPU6050 not found at address 0x68");
        return;
    }

    if (display.available) {
        ssd1306_clear(&display);
        ssd1306_text(&display, 2, "KEEP STILL");
        ssd1306_text(&display, 4, "CALIBRATE");
        ssd1306_flush(&display);
    }
    ESP_LOGI(TAG, "Keep the wearable still for two seconds");
    ESP_ERROR_CHECK(mpu6050_calibrate(&sensor, 100));
    ESP_ERROR_CHECK(ble_server_init());

    motion_logic_t logic;
    motion_config_t config = motion_default_config();
    motion_logic_init(&logic, &config);
    uint16_t sequence = 0;
    unsigned display_divider = 0;
    TickType_t last_wake = xTaskGetTickCount();

    while (true) {
        mpu6050_sample_t sample;
        if (mpu6050_read(&sensor, &sample) == ESP_OK) {
            const uint32_t now_ms = (uint32_t)(esp_timer_get_time() / 1000);
            motion_result_t result = motion_logic_update(
                &logic,
                sample.gyro[0] / 131.0f,
                sample.gyro[1] / 131.0f,
                sample.gyro[2] / 131.0f,
                sample.accel[0] / 16384.0f,
                sample.accel[1] / 16384.0f,
                sample.accel[2] / 16384.0f,
                now_ms);

            motion_packet_t packet = {
                .magic = MOTION_PACKET_MAGIC,
                .version = MOTION_PACKET_VERSION,
                .sequence = sequence++,
                .timestamp_ms = now_ms,
                .rep_count = result.rep_count,
                .state = (uint8_t)result.state,
                .quality = (uint8_t)result.quality,
            };
            memcpy(packet.accel, sample.accel, sizeof(packet.accel));
            memcpy(packet.gyro, sample.gyro, sizeof(packet.gyro));
            motion_packet_finalize(&packet);
            if (ble_server_connected()) (void)ble_server_notify(&packet);
            wearable_status_set(ble_server_connected(), false);

            if (++display_divider >= 10) {
                display_divider = 0;
                update_display(&display, &result);
            }
        } else {
            ESP_LOGW(TAG, "MPU6050 read failed");
        }
        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(SAMPLE_PERIOD_MS));
    }
}
