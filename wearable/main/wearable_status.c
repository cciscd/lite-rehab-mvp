#include "wearable_status.h"

#include "driver/gpio.h"

#define STATUS_LED_GPIO GPIO_NUM_2

esp_err_t wearable_status_init(void)
{
    const gpio_config_t config = {
        .pin_bit_mask = 1ULL << STATUS_LED_GPIO,
        .mode = GPIO_MODE_OUTPUT,
    };
    esp_err_t error = gpio_config(&config);
    if (error == ESP_OK) {
        gpio_set_level(STATUS_LED_GPIO, 0);
    }
    return error;
}

void wearable_status_set(bool connected, bool error)
{
    gpio_set_level(STATUS_LED_GPIO, error ? 1 : (connected ? 1 : 0));
}
