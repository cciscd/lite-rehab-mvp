#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "motion_packet.h"

int main(void)
{
    motion_packet_t packet = {
        .magic = MOTION_PACKET_MAGIC,
        .version = MOTION_PACKET_VERSION,
        .sequence = 42,
        .timestamp_ms = 123456,
        .accel = {100, -200, 16384},
        .gyro = {10, -20, 30},
        .rep_count = 7,
        .state = MOTION_STATE_FOREARM_ROTATION,
        .quality = MOTION_QUALITY_OK,
    };

    assert(sizeof(packet) == MOTION_PACKET_SIZE);
    motion_packet_finalize(&packet);
    assert(motion_packet_is_valid(&packet));

    motion_packet_t changed = packet;
    changed.gyro[1]++;
    assert(!motion_packet_is_valid(&changed));

    changed = packet;
    changed.magic = 0;
    motion_packet_finalize(&changed);
    assert(!motion_packet_is_valid(&changed));

    puts("test_motion_packet: PASS");
    return 0;
}
