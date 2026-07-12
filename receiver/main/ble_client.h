#ifndef LITE_REHAB_BLE_CLIENT_H
#define LITE_REHAB_BLE_CLIENT_H

#include <stdbool.h>
#include "esp_err.h"
#include "motion_packet.h"

typedef void (*ble_packet_callback_t)(const motion_packet_t *packet);
typedef void (*ble_connection_callback_t)(bool connected);

esp_err_t ble_client_init(ble_packet_callback_t packet_callback,
                          ble_connection_callback_t connection_callback);

#endif
