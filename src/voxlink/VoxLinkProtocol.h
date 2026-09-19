#ifndef VOXLINK_PROTOCOL_H
#define VOXLINK_PROTOCOL_H

// VoxLink v1 wire constants for the CYD client. Mirrors docs/voxlink_v1.md.
// Pure C++ (no Arduino/LVGL) so it can run under `pio test -e native`.

#include <cstddef>
#include <cstdint>

#include "voxlink/VoxLinkContract.h" // VOXP4_VOXLINK_VERSION

namespace voxlink {

constexpr uint8_t kSof0 = 0xA5;
constexpr uint8_t kSof1 = 0x5A;
constexpr uint8_t kVersionMajor = 1;
constexpr uint8_t kVersionMinor = 0;
constexpr uint8_t kVersion = 0x10;

constexpr size_t kHeaderSize = 8;
constexpr size_t kCrcSize = 2;
constexpr size_t kMaxPayload = 512;
constexpr size_t kMaxFrame = kHeaderSize + kMaxPayload + kCrcSize;

enum class MsgType : uint8_t {
    Hello = 0x01,
    HelloAck = 0x02,
    CapsRequest = 0x03,
    CapsBegin = 0x04,
    CapsParam = 0x05,
    CapsEnd = 0x06,
    GetState = 0x07,
    StateBegin = 0x08,
    StateParam = 0x09,
    StateEnd = 0x0A,
    GetParam = 0x0B,
    ParamValue = 0x0C,
    SetParam = 0x0D,
    ParamChanged = 0x0E,
    Action = 0x0F,
    Ack = 0x10,
    Nack = 0x11,
    Error = 0x12,
    Heartbeat = 0x60,
    MeterFrame = 0x61,
    PitchFrame = 0x62,
    DspStatus = 0x63,
};

enum class Code : uint16_t {
    Ok = 0,
    UnknownMessage = 1,
    UnsupportedVersion = 2,
    BadLength = 3,
    BadType = 4,
    UnknownParam = 5,
    ReadOnly = 6,
    OutOfRange = 7,
    InvalidEnum = 8,
    QueueFull = 9,
    Busy = 10,
    NotSupported = 11,
    InternalError = 12,
    BadCrc = 13,
    Malformed = 14,
};

enum class ValueTag : uint8_t {
    Bool = 1,
    Int32 = 2,
    Float32 = 3,
    Enum16 = 4,
};

size_t value_tag_size(ValueTag tag);

// Capability bits (HELLO_ACK / CAPS_BEGIN).
enum : uint32_t {
    kCapHarmony = 1u << 0,
    kCapCompressor = 1u << 1,
    kCapDelay = 1u << 2,
    kCapReverb = 1u << 3,
    kCapLimiter = 1u << 4,
    kCapGate = 1u << 5,
    kCapMeters = 1u << 6,
    kCapPitchTelemetry = 1u << 7,
    kCapDspStatus = 1u << 8,
    kCapPresets = 1u << 9,
    kCapMidi = 1u << 10,
    kCapScenes = 1u << 11,
    kCapFormantPreservation = 1u << 12,
};

constexpr size_t kMaxCapsParams = 64;
constexpr size_t kMaxSnapshotParams = 64;

} // namespace voxlink

#endif // VOXLINK_PROTOCOL_H
