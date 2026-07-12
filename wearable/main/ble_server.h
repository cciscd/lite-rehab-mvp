#ifndef LITE_REHAB_BLE_SERVER_H
#define LITE_REHAB_BLE_SERVER_H

#include <stdbool.h>
#include "esp_err.h"
#include "motion_packet.h"

esp_err_t ble_server_init(void);
bool ble_server_connected(void);
esp_err_t ble_server_notify(const motion_packet_t *packet);

#endif
