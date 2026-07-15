#ifndef LITE_REHAB_MOTION_LOGIC_H
#define LITE_REHAB_MOTION_LOGIC_H

#include <stdbool.h>
#include <stdint.h>

#include "motion_packet.h"

typedef struct {//这是动作逻辑的配置参数
    float filter_alpha;//低通滤波系数
    float comp_alpha;//修正滤波系数
    float enter_threshold_dps;//进入和退出阈值，用了滞环控制，避免抖动
    float exit_threshold_dps;
    float dominance_ratio;//用于判断六轴陀螺仪哪个占主导地位，前臂旋转主要是x轴陀螺仪，肘关节屈曲主要是y轴陀螺仪
    float min_range_deg;
    float too_fast_peak_dps;
    uint32_t min_rep_duration_ms;
    uint32_t max_rep_duration_ms;
    uint32_t refractory_ms;
    float adapt_k;
    float adapt_floor_dps;
    float adapt_ceil_dps;
} motion_config_t;

typedef struct {//对外输出的动作逻辑结果
    motion_state_t state;
    motion_state_t last_completed_state;
    motion_quality_t quality;
    uint16_t rep_count;//累计动作次数
    bool rep_completed;
    float filtered_gyro[3];//滤波后的三轴陀螺仪数据
    float roll_deg;//互补滤波后的姿态角
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
