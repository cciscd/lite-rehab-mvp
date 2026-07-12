#include <stdio.h>

#include "ble_client.h"
#include "esp_log.h"
#include "receiver_outputs.h"
#include "serial_telemetry.h"

static const char *TAG = "receiver";
static uint16_t previous_sequence;
static uint16_t previous_reps;
static bool have_sequence;

static void connection_changed(bool connected)
{
    receiver_outputs_set_connected(connected);
    printf("# BLE %s\n", connected ? "connected" : "disconnected");
    fflush(stdout);
    if (!connected) have_sequence = false;
}

static void packet_received(const motion_packet_t *packet)
{
    if (have_sequence && (uint16_t)(previous_sequence + 1) != packet->sequence) {
        printf("# packet_gap expected=%u received=%u\n",
               (uint16_t)(previous_sequence + 1), packet->sequence);
    }
    previous_sequence = packet->sequence;
    have_sequence = true;
    const bool new_rep = packet->rep_count != previous_reps;
    previous_reps = packet->rep_count;
    receiver_outputs_feedback((motion_quality_t)packet->quality, new_rep);
    serial_telemetry_packet(packet);
}

void app_main(void)
{
    ESP_ERROR_CHECK(receiver_outputs_init());
    serial_telemetry_header();
    ESP_LOGI(TAG, "starting BLE receiver");
    ESP_ERROR_CHECK(ble_client_init(packet_received, connection_changed));
}
