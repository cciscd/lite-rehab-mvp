#ifndef LITE_REHAB_SSD1306_H
#define LITE_REHAB_SSD1306_H

#include <stdbool.h>
#include <stdint.h>
#include "driver/i2c_master.h"
#include "esp_err.h"

typedef struct {
    i2c_master_dev_handle_t device;
    uint8_t framebuffer[1024];
    bool available;
} ssd1306_t;

esp_err_t ssd1306_init(ssd1306_t *display, i2c_master_bus_handle_t bus);
void ssd1306_clear(ssd1306_t *display);
void ssd1306_text(ssd1306_t *display, unsigned row, const char *text);
esp_err_t ssd1306_flush(ssd1306_t *display);

#endif
