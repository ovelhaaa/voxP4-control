#ifndef VOXLINK_CRC_H
#define VOXLINK_CRC_H

// CRC-16/CCITT-FALSE: poly 0x1021, init 0xFFFF, no reflection, xorout 0.
// Check value for "123456789" is 0x29B1.
#include <cstddef>
#include <cstdint>

namespace voxlink {
uint16_t crc16_ccitt_false(const uint8_t *data, size_t len);
}

#endif // VOXLINK_CRC_H
