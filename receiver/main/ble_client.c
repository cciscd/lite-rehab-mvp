#include "ble_client.h"

#include <assert.h>
#include <string.h>
#include "esp_check.h"
#include "esp_log.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "nvs_flash.h"

static const char *TAG = "ble_receiver";
static uint8_t own_addr_type;
static uint16_t conn_handle = BLE_HS_CONN_HANDLE_NONE;
static uint16_t service_start;
static uint16_t service_end;
static uint16_t characteristic_value;
static ble_packet_callback_t on_packet;
static ble_connection_callback_t on_connection;

static const ble_uuid128_t service_uuid = BLE_UUID128_INIT(
    0x00,0x10,0x2d,0x4c,0x7e,0x6b,0x1a,0x9c,0x2a,0x4b,0x57,0x7e,0x01,0x00,0x57,0x7e);
static const ble_uuid128_t characteristic_uuid = BLE_UUID128_INIT(
    0x00,0x10,0x2d,0x4c,0x7e,0x6b,0x1a,0x9c,0x2a,0x4b,0x57,0x7e,0x02,0x00,0x57,0x7e);

static int gap_event(struct ble_gap_event *event, void *arg);
static void start_scan(void);

static int subscribe_done(uint16_t handle, const struct ble_gatt_error *error,
                          struct ble_gatt_attr *attr, void *arg)
{
    (void)handle; (void)attr; (void)arg;
    if (error->status == 0) ESP_LOGI(TAG, "motion notifications enabled");
    else ESP_LOGE(TAG, "CCCD write failed: %d", error->status);
    return 0;
}

static int descriptor_discovered(uint16_t handle,
                                 const struct ble_gatt_error *error,
                                 uint16_t chr_val_handle,
                                 const struct ble_gatt_dsc *descriptor,
                                 void *arg)
{
    (void)chr_val_handle; (void)arg;
    if (error->status == 0 && descriptor != NULL &&
        ble_uuid_u16(&descriptor->uuid.u) == BLE_GATT_DSC_CLT_CFG_UUID16) {
        const uint8_t enable_notify[2] = {1, 0};
        int rc = ble_gattc_write_flat(handle, descriptor->handle,
                                      enable_notify, sizeof(enable_notify),
                                      subscribe_done, NULL);
        if (rc != 0) ESP_LOGE(TAG, "subscribe start failed: %d", rc);
    } else if (error->status != 0 && error->status != BLE_HS_EDONE) {
        ESP_LOGE(TAG, "descriptor discovery failed: %d", error->status);
    }
    return 0;
}

static int characteristic_discovered(uint16_t handle,
                                     const struct ble_gatt_error *error,
                                     const struct ble_gatt_chr *chr,
                                     void *arg)
{
    (void)arg;
    if (error->status == 0 && chr != NULL) {
        characteristic_value = chr->val_handle;
        return 0;
    }
    if (error->status == BLE_HS_EDONE && characteristic_value != 0) {
        const uint8_t enable_notify[2] = {1, 0};
        int rc = ble_gattc_write_flat(handle, characteristic_value + 1,
                                      enable_notify, sizeof(enable_notify),
                                      subscribe_done, NULL);
        if (rc != 0) ESP_LOGE(TAG, "subscribe start failed: %d", rc);
    } else if (error->status != BLE_HS_EDONE) {
        ESP_LOGE(TAG, "characteristic discovery failed: %d", error->status);
    }
    return 0;
}

static int service_discovered(uint16_t handle,
                              const struct ble_gatt_error *error,
                              const struct ble_gatt_svc *service,
                              void *arg)
{
    (void)arg;
    if (error->status == 0 && service != NULL) {
        service_start = service->start_handle;
        service_end = service->end_handle;
        return 0;
    }
    if (error->status == BLE_HS_EDONE && service_start != 0) {
        int rc = ble_gattc_disc_chrs_by_uuid(handle, service_start, service_end,
                                             &characteristic_uuid.u,
                                             characteristic_discovered, NULL);
        if (rc != 0) ESP_LOGE(TAG, "characteristic discovery start failed: %d", rc);
    } else if (error->status != BLE_HS_EDONE) {
        ESP_LOGE(TAG, "service discovery failed: %d", error->status);
    }
    return 0;
}

static int mtu_exchanged(uint16_t handle, const struct ble_gatt_error *error,
                         uint16_t mtu, void *arg)
{
    (void)arg;
    ESP_LOGI(TAG, "MTU %u status %d", mtu, error->status);
    service_start = service_end = characteristic_value = 0;
    int rc = ble_gattc_disc_svc_by_uuid(handle, &service_uuid.u,
                                        service_discovered, NULL);
    if (rc != 0) ESP_LOGE(TAG, "service discovery start failed: %d", rc);
    return 0;
}

static bool is_wearable(const struct ble_gap_disc_desc *disc)
{
    struct ble_hs_adv_fields fields;
    if (ble_hs_adv_parse_fields(&fields, disc->data, disc->length_data) != 0 ||
        fields.name == NULL) return false;
    static const char expected[] = "LiteRehab-Wear";
    return fields.name_len == sizeof(expected) - 1 &&
           memcmp(fields.name, expected, sizeof(expected) - 1) == 0;
}

static void connect_to(const struct ble_gap_disc_desc *disc)
{
    if (!is_wearable(disc)) return;
    ble_gap_disc_cancel();
    int rc = ble_gap_connect(own_addr_type, &disc->addr, 10000, NULL,
                             gap_event, NULL);
    if (rc != 0) {
        ESP_LOGE(TAG, "connect failed to start: %d", rc);
        start_scan();
    }
}

static int gap_event(struct ble_gap_event *event, void *arg)
{
    (void)arg;
    switch (event->type) {
    case BLE_GAP_EVENT_DISC:
        connect_to(&event->disc);
        return 0;
    case BLE_GAP_EVENT_CONNECT:
        if (event->connect.status == 0) {
            conn_handle = event->connect.conn_handle;
            if (on_connection) on_connection(true);
            int rc = ble_gattc_exchange_mtu(conn_handle, mtu_exchanged, NULL);
            if (rc != 0) mtu_exchanged(conn_handle,
                &(struct ble_gatt_error){.status = rc}, 23, NULL);
        } else {
            start_scan();
        }
        return 0;
    case BLE_GAP_EVENT_DISCONNECT:
        conn_handle = BLE_HS_CONN_HANDLE_NONE;
        if (on_connection) on_connection(false);
        start_scan();
        return 0;
    case BLE_GAP_EVENT_NOTIFY_RX: {
        motion_packet_t packet;
        uint16_t length = 0;
        int rc = ble_hs_mbuf_to_flat(event->notify_rx.om, &packet,
                                     sizeof(packet), &length);
        if (rc == 0 && length == sizeof(packet) &&
            motion_packet_is_valid(&packet) && on_packet != NULL) {
            on_packet(&packet);
        } else {
            ESP_LOGW(TAG, "invalid motion notification length=%u rc=%d", length, rc);
        }
        return 0;
    }
    case BLE_GAP_EVENT_DISC_COMPLETE:
        start_scan();
        return 0;
    default:
        return 0;
    }
}

static void start_scan(void)
{
    if (ble_gap_disc_active()) return;
    const struct ble_gap_disc_params params = {
        .passive = 0,
        .filter_duplicates = 1,
    };
    int rc = ble_gap_disc(own_addr_type, BLE_HS_FOREVER, &params,
                          gap_event, NULL);
    if (rc != 0) ESP_LOGE(TAG, "scan failed: %d", rc);
}

static void on_sync(void)
{
    int rc = ble_hs_util_ensure_addr(0);
    assert(rc == 0);
    rc = ble_hs_id_infer_auto(0, &own_addr_type);
    assert(rc == 0);
    start_scan();
}

static void host_task(void *arg)
{
    (void)arg;
    nimble_port_run();
    nimble_port_freertos_deinit();
}

esp_err_t ble_client_init(ble_packet_callback_t packet_callback,
                          ble_connection_callback_t connection_callback)
{
    on_packet = packet_callback;
    on_connection = connection_callback;
    esp_err_t error = nvs_flash_init();
    if (error == ESP_ERR_NVS_NO_FREE_PAGES || error == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        error = nvs_flash_init();
    }
    ESP_RETURN_ON_ERROR(error, TAG, "NVS");
    ESP_RETURN_ON_ERROR(nimble_port_init(), TAG, "NimBLE");
    ble_hs_cfg.sync_cb = on_sync;
    nimble_port_freertos_init(host_task);
    return ESP_OK;
}
