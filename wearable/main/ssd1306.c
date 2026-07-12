#include "ssd1306.h"

#include <ctype.h>
#include <string.h>

#define OLED_ADDRESS 0x3C

typedef struct { char ch; uint8_t bits[5]; } glyph_t;

static const glyph_t font[] = {
    {' ', {0,0,0,0,0}}, {'-', {8,8,8,8,8}}, {':', {0,0x36,0x36,0,0}},
    {'0', {0x3e,0x51,0x49,0x45,0x3e}}, {'1', {0,0x42,0x7f,0x40,0}},
    {'2', {0x42,0x61,0x51,0x49,0x46}}, {'3', {0x21,0x41,0x45,0x4b,0x31}},
    {'4', {0x18,0x14,0x12,0x7f,0x10}}, {'5', {0x27,0x45,0x45,0x45,0x39}},
    {'6', {0x3c,0x4a,0x49,0x49,0x30}}, {'7', {1,0x71,9,5,3}},
    {'8', {0x36,0x49,0x49,0x49,0x36}}, {'9', {6,0x49,0x49,0x29,0x1e}},
    {'A', {0x7e,0x11,0x11,0x11,0x7e}}, {'B', {0x7f,0x49,0x49,0x49,0x36}},
    {'C', {0x3e,0x41,0x41,0x41,0x22}}, {'D', {0x7f,0x41,0x41,0x22,0x1c}},
    {'E', {0x7f,0x49,0x49,0x49,0x41}}, {'F', {0x7f,9,9,9,1}},
    {'G', {0x3e,0x41,0x49,0x49,0x7a}}, {'I', {0,0x41,0x7f,0x41,0}},
    {'H', {0x7f,8,8,8,0x7f}},
    {'K', {0x7f,8,0x14,0x22,0x41}}, {'L', {0x7f,0x40,0x40,0x40,0x40}},
    {'M', {0x7f,2,0x0c,2,0x7f}}, {'N', {0x7f,2,4,8,0x7f}},
    {'O', {0x3e,0x41,0x41,0x41,0x3e}},
    {'P', {0x7f,9,9,9,6}}, {'R', {0x7f,9,0x19,0x29,0x46}},
    {'S', {0x46,0x49,0x49,0x49,0x31}}, {'T', {1,1,0x7f,1,1}},
    {'U', {0x3f,0x40,0x40,0x40,0x3f}},
    {'W', {0x7f,0x20,0x18,0x20,0x7f}}, {'Y', {3,4,0x78,4,3}},
};

static const uint8_t *glyph_for(char ch)
{
    ch = (char)toupper((unsigned char)ch);
    for (size_t i = 0; i < sizeof(font) / sizeof(font[0]); ++i) {
        if (font[i].ch == ch) return font[i].bits;
    }
    return font[0].bits;
}

static esp_err_t command(ssd1306_t *display, const uint8_t *commands, size_t count)
{
    uint8_t data[32];
    if (count + 1 > sizeof(data)) return ESP_ERR_INVALID_SIZE;
    data[0] = 0x00;
    memcpy(&data[1], commands, count);
    return i2c_master_transmit(display->device, data, count + 1, 100);
}

esp_err_t ssd1306_init(ssd1306_t *display, i2c_master_bus_handle_t bus)
{
    if (display == NULL || bus == NULL) return ESP_ERR_INVALID_ARG;
    memset(display, 0, sizeof(*display));
    const i2c_device_config_t config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = OLED_ADDRESS,
        .scl_speed_hz = 100000,
    };
    esp_err_t error = i2c_master_bus_add_device(bus, &config, &display->device);
    if (error != ESP_OK) return error;
    const uint8_t init[] = {
        0xae, 0x20, 0x00, 0x40, 0xa1, 0xc8, 0x81, 0x7f,
        0xa6, 0xa8, 0x3f, 0xd3, 0x00, 0xd5, 0x80, 0xd9,
        0xf1, 0xda, 0x12, 0xdb, 0x40, 0x8d, 0x14, 0xaf,
    };
    error = command(display, init, sizeof(init));
    if (error != ESP_OK) return error;
    display->available = true;
    ssd1306_clear(display);
    return ssd1306_flush(display);
}

void ssd1306_clear(ssd1306_t *display)
{
    if (display != NULL) memset(display->framebuffer, 0, sizeof(display->framebuffer));
}

void ssd1306_text(ssd1306_t *display, unsigned row, const char *text)
{
    if (display == NULL || text == NULL || row >= 8) return;
    unsigned x = 0;
    while (*text != '\0' && x + 6 <= 128) {
        const uint8_t *glyph = glyph_for(*text++);
        for (int i = 0; i < 5; ++i) display->framebuffer[row * 128 + x++] = glyph[i];
        display->framebuffer[row * 128 + x++] = 0;
    }
}

esp_err_t ssd1306_flush(ssd1306_t *display)
{
    if (display == NULL || !display->available) return ESP_ERR_INVALID_STATE;
    for (uint8_t page = 0; page < 8; ++page) {
        const uint8_t cmds[] = { (uint8_t)(0xb0 | page), 0x00, 0x10 };
        esp_err_t error = command(display, cmds, sizeof(cmds));
        if (error != ESP_OK) return error;
        uint8_t data[129];
        data[0] = 0x40;
        memcpy(&data[1], &display->framebuffer[page * 128], 128);
        error = i2c_master_transmit(display->device, data, sizeof(data), 100);
        if (error != ESP_OK) return error;
    }
    return ESP_OK;
}
