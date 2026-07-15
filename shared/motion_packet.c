#include "motion_packet.h"

#include <stddef.h>
// Ensure the motion_packet_t structure is packed and has the expected size
//主要是为了BLE传输数据的大小限制，确保结构体的大小为26字节
_Static_assert(sizeof(motion_packet_t) == MOTION_PACKET_SIZE,
               "motion_packet_t layout changed");

uint16_t motion_packet_crc16(const void *data, size_t length)
{
    const uint8_t *bytes = (const uint8_t *)data;
    uint16_t crc = 0xFFFFu;
    for (size_t i = 0; i < length; ++i) {
        crc ^= (uint16_t)bytes[i] << 8;
        for (int bit = 0; bit < 8; ++bit) {
            crc = (crc & 0x8000u) ? (uint16_t)((crc << 1) ^ 0x1021u)
                                  : (uint16_t)(crc << 1);
        }
    }
    return crc;
}

void motion_packet_finalize(motion_packet_t *packet)
{
    if (packet == NULL) {
        return;
    }
    packet->crc16 = motion_packet_crc16(packet, offsetof(motion_packet_t, crc16));
}

bool motion_packet_is_valid(const motion_packet_t *packet)
{
    if (packet == NULL || packet->magic != MOTION_PACKET_MAGIC ||
        packet->version != MOTION_PACKET_VERSION) {
        return false;
    }
    return packet->crc16 ==
           motion_packet_crc16(packet, offsetof(motion_packet_t, crc16));
}
