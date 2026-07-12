#ifndef LITE_REHAB_RECEIVER_OUTPUTS_H
#define LITE_REHAB_RECEIVER_OUTPUTS_H

#include <stdbool.h>
#include "esp_err.h"
#include "motion_packet.h"

esp_err_t receiver_outputs_init(void);
void receiver_outputs_set_connected(bool connected);
void receiver_outputs_feedback(motion_quality_t quality, bool new_rep);

#endif
