#ifndef LITE_REHAB_MOTION_PACKET_H
#define LITE_REHAB_MOTION_PACKET_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define MOTION_PACKET_MAGIC 0xA5u
#define MOTION_PACKET_VERSION 1u
#define MOTION_PACKET_SIZE 26u

typedef enum {
    MOTION_STATE_IDLE = 0,
    MOTION_STATE_FOREARM_ROTATION = 1,
    MOTION_STATE_ELBOW_FLEXION = 2,
} motion_state_t;

typedef enum {
    MOTION_QUALITY_NONE = 0,
    MOTION_QUALITY_OK = 1,
    MOTION_QUALITY_TOO_FAST = 2,
    MOTION_QUALITY_INSUFFICIENT_RANGE = 3,
} motion_quality_t;

typedef struct __attribute__((packed)) {
    uint8_t magic;
    uint8_t version;
    uint16_t sequence;
    uint32_t timestamp_ms;
    int16_t accel[3];
    int16_t gyro[3];
    uint16_t rep_count;
    uint8_t state;
    uint8_t quality;
    uint16_t crc16;
} motion_packet_t;

uint16_t motion_packet_crc16(const void *data, size_t length);
void motion_packet_finalize(motion_packet_t *packet);
bool motion_packet_is_valid(const motion_packet_t *packet);

#endif
