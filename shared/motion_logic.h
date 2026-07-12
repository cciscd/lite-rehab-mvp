#ifndef LITE_REHAB_MOTION_LOGIC_H
#define LITE_REHAB_MOTION_LOGIC_H

#include <stdbool.h>
#include <stdint.h>

#include "motion_packet.h"

typedef struct {
    float filter_alpha;
    float comp_alpha;
    float enter_threshold_dps;
    float exit_threshold_dps;
    float dominance_ratio;
    float min_range_deg;
    float too_fast_peak_dps;
    uint32_t min_rep_duration_ms;
    uint32_t max_rep_duration_ms;
    uint32_t refractory_ms;
    float adapt_k;
    float adapt_floor_dps;
    float adapt_ceil_dps;
} motion_config_t;

typedef struct {
    motion_state_t state;
    motion_state_t last_completed_state;
    motion_quality_t quality;
    uint16_t rep_count;
    bool rep_completed;
    float filtered_gyro[3];
    float roll_deg;
    float pitch_deg;
} motion_result_t;

typedef struct {
    motion_config_t config;
    motion_result_t result;
    uint32_t previous_ms;
    uint32_t start_ms;
    uint32_t last_completed_ms;
    float integrated_abs_deg;
    float peak_dps;
    float roll_deg;
    float pitch_deg;
    float signal_rms_sq;
    float adaptive_enter;
    float adaptive_exit;
    int8_t start_sign;
    uint8_t phase;
    motion_state_t active_state;
    bool initialized;
    bool cf_initialized;
} motion_logic_t;

motion_config_t motion_default_config(void);
void motion_logic_init(motion_logic_t *logic, const motion_config_t *config);
motion_result_t motion_logic_update(motion_logic_t *logic,
                                    float gx_dps, float gy_dps, float gz_dps,
                                    float ax_g, float ay_g, float az_g,
                                    uint32_t timestamp_ms);

#endif
