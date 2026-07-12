#ifndef LITE_REHAB_WEARABLE_STATUS_H
#define LITE_REHAB_WEARABLE_STATUS_H

#include <stdbool.h>
#include "esp_err.h"

esp_err_t wearable_status_init(void);
void wearable_status_set(bool connected, bool error);

#endif
