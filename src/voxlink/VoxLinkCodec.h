#ifndef VOXLINK_CODEC_H
#define VOXLINK_CODEC_H

// Explicit little-endian VoxLink frame/value serialization. No compiler struct
// is written to the wire.
#include "voxlink/VoxLinkProtocol.h"
#include <cstddef>
#include <cstdint>

namespace voxlink {

struct Frame {
    uint8_t version = kVersion;
    MsgType type = MsgType::Error;
    uint8_t flags = 0;
    uint8_t seq = 0;
    uint16_t payload_len = 0;
    uint8_t payload[kMaxPayload] = {0};
};

enum class DecodeStatus {
    Ok,
    ShortBuffer,
    BadSof,
    UnsupportedVersion,
    LengthTooLarge,
    BadCrc,
};

size_t encode_frame(MsgType type, uint8_t flags, uint8_t seq,
                    const uint8_t *payload, size_t payload_len, uint8_t *out,
                    size_t out_capacity);

DecodeStatus decode_frame(const uint8_t *data, size_t len, Frame *out,
                          size_t *consumed);

void put_u16(uint8_t *out, uint16_t value);
void put_u32(uint8_t *out, uint32_t value);
void put_i32(uint8_t *out, int32_t value);
void put_f32(uint8_t *out, float value);
uint16_t get_u16(const uint8_t *in);
uint32_t get_u32(const uint8_t *in);
int32_t get_i32(const uint8_t *in);
float get_f32(const uint8_t *in);

// Value body size for a tag (0 if unknown).
size_t value_tag_size(ValueTag tag);
// Decodes a typed value body into a float. Returns false on bad tag/short input.
bool decode_value(ValueTag tag, const uint8_t *in, size_t len, float *out,
                  size_t *consumed);
// Encodes a typed value body. Returns bytes written or 0 on error.
size_t encode_value(ValueTag tag, float value, uint8_t *out, size_t out_capacity);

} // namespace voxlink

#endif // VOXLINK_CODEC_H
