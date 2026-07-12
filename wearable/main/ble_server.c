#include "ble_server.h"

#include <assert.h>
#include <string.h>
#include "esp_check.h"
#include "esp_log.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "nvs_flash.h"
#include "os/os_mbuf.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

static const char *TAG = "ble_wearable";
static uint8_t own_addr_type;
static uint16_t connection_handle = BLE_HS_CONN_HANDLE_NONE;
static uint16_t value_handle;
static bool notify_enabled;

static const ble_uuid128_t service_uuid = BLE_UUID128_INIT(
    0x00,0x10,0x2d,0x4c,0x7e,0x6b,0x1a,0x9c,0x2a,0x4b,0x57,0x7e,0x01,0x00,0x57,0x7e);
static const ble_uuid128_t characteristic_uuid = BLE_UUID128_INIT(
    0x00,0x10,0x2d,0x4c,0x7e,0x6b,0x1a,0x9c,0x2a,0x4b,0x57,0x7e,0x02,0x00,0x57,0x7e);

static int access_callback(uint16_t conn_handle, uint16_t attr_handle,
                           struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    (void)conn_handle; (void)attr_handle; (void)ctxt; (void)arg;
    return 0;
}

static const struct ble_gatt_svc_def services[] = {{
    .type = BLE_GATT_SVC_TYPE_PRIMARY,
    .uuid = &service_uuid.u,
    .characteristics = (struct ble_gatt_chr_def[]){{
        .uuid = &characteristic_uuid.u,
        .access_cb = access_callback,
        .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
        .val_handle = &value_handle,
    }, {0}},
}, {0}};

static void advertise(void);

static int gap_event(struct ble_gap_event *event, void *arg)
{
    (void)arg;
    switch (event->type) {
    case BLE_GAP_EVENT_CONNECT:
        if (event->connect.status == 0) {
            connection_handle = event->connect.conn_handle;
            ESP_LOGI(TAG, "receiver connected");
        } else {
            advertise();
        }
        return 0;
    case BLE_GAP_EVENT_DISCONNECT:
        connection_handle = BLE_HS_CONN_HANDLE_NONE;
        notify_enabled = false;
        advertise();
        return 0;
    case BLE_GAP_EVENT_SUBSCRIBE:
        notify_enabled = event->subscribe.cur_notify != 0;
        return 0;
    case BLE_GAP_EVENT_MTU:
        ESP_LOGI(TAG, "MTU %d", event->mtu.value);
        return 0;
    default:
        return 0;
    }
}

static void advertise(void)
{
    struct ble_hs_adv_fields fields = {0};
    fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;
    const char *name = ble_svc_gap_device_name();
    fields.name = (uint8_t *)name;
    fields.name_len = strlen(name);
    fields.name_is_complete = 1;
    ble_gap_adv_set_fields(&fields);
    struct ble_gap_adv_params params = {0};
    params.conn_mode = BLE_GAP_CONN_MODE_UND;
    params.disc_mode = BLE_GAP_DISC_MODE_GEN;
    int rc = ble_gap_adv_start(own_addr_type, NULL, BLE_HS_FOREVER,
                               &params, gap_event, NULL);
    if (rc != 0) ESP_LOGE(TAG, "advertising failed: %d", rc);
}

static void on_sync(void)
{
    int rc = ble_hs_util_ensure_addr(0);
    assert(rc == 0);
    rc = ble_hs_id_infer_auto(0, &own_addr_type);
    assert(rc == 0);
    advertise();
}

static void host_task(void *param)
{
    (void)param;
    nimble_port_run();
    nimble_port_freertos_deinit();
}

esp_err_t ble_server_init(void)
{
    esp_err_t error = nvs_flash_init();
    if (error == ESP_ERR_NVS_NO_FREE_PAGES || error == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        error = nvs_flash_init();
    }
    ESP_RETURN_ON_ERROR(error, TAG, "NVS");
    ESP_RETURN_ON_ERROR(nimble_port_init(), TAG, "NimBLE");
    ble_svc_gap_init();
    ble_svc_gatt_init();
    int rc = ble_gatts_count_cfg(services);
    if (rc == 0) rc = ble_gatts_add_svcs(services);
    if (rc != 0) return ESP_FAIL;
    ble_svc_gap_device_name_set("LiteRehab-Wear");
    ble_hs_cfg.sync_cb = on_sync;
    nimble_port_freertos_init(host_task);
    return ESP_OK;
}

bool ble_server_connected(void)
{
    return connection_handle != BLE_HS_CONN_HANDLE_NONE && notify_enabled;
}

esp_err_t ble_server_notify(const motion_packet_t *packet)
{
    if (packet == NULL) return ESP_ERR_INVALID_ARG;
    if (!ble_server_connected()) return ESP_ERR_INVALID_STATE;
    struct os_mbuf *buffer = ble_hs_mbuf_from_flat(packet, sizeof(*packet));
    if (buffer == NULL) return ESP_ERR_NO_MEM;
    int rc = ble_gatts_notify_custom(connection_handle, value_handle, buffer);
    return rc == 0 ? ESP_OK : ESP_FAIL;
}
