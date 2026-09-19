// Host-side VoxLink client tests: CRC/codec/parser, golden vectors, handshake,
// transactional snapshot, SET/PARAM_CHANGED, NACK rollback, QUEUE_FULL,
// reconnect and capability gating. No Arduino/LVGL.
#include <unity.h>

#include <cstdio>
#include <cstring>
#include <vector>

#include "voxlink/VoxLinkClient.h"
#include "voxlink/VoxLinkCodec.h"
#include "voxlink/VoxLinkCrc.h"
#include "voxlink/VoxLinkParser.h"
#include "voxlink/VoxLinkProtocol.h"
#include "voxlink/generated/voxlink_v1_vectors.h"

using namespace voxlink;

namespace {

struct Harness {
    VoxLinkClient client;
    uint32_t now = 0;

    void feed_frame(MsgType type, uint8_t seq, const uint8_t *payload, size_t len) {
        uint8_t buf[kMaxFrame];
        const size_t n = encode_frame(type, 0, seq, payload, len, buf, sizeof(buf));
        client.feed(buf, n, now);
    }
    void feed_raw(const uint8_t *data, size_t len) { client.feed(data, len, now); }

    std::vector<Frame> drain_tx() {
        uint8_t buf[VoxLinkClient::kTxCapacity];
        std::vector<Frame> frames;
        size_t total = client.take_tx(buf, sizeof(buf));
        size_t off = 0;
        while (off < total) {
            Frame f;
            size_t consumed = 0;
            if (decode_frame(buf + off, total - off, &f, &consumed) !=
                DecodeStatus::Ok)
                break;
            frames.push_back(f);
            off += consumed;
        }
        return frames;
    }

    std::vector<Event> drain_events() {
        std::vector<Event> events;
        Event e;
        while (client.take_event(&e)) events.push_back(e);
        return events;
    }

    // --- server-side payload builders -------------------------------------
    void hello_ack(uint32_t caps = 0, uint32_t rev = 5) {
        uint8_t p[64];
        size_t n = 0;
        p[n++] = kVersion;
        put_u32(p + n, caps); n += 4;
        put_u32(p + n, 44100); n += 4;
        put_u16(p + n, 64); n += 2;
        p[n++] = 1; // harmony voices
        put_u32(p + n, rev); n += 4;
        p[n++] = 1; // product
        p[n++] = 1; // hw
        p[n++] = 0; p[n++] = 1; p[n++] = 0; // fw 0.1.0
        p[n++] = 0; // sha len
        feed_frame(MsgType::HelloAck, 0, p, n);
    }

    void caps_param(uint16_t id, ValueTag tag, float lo, float hi, float def,
                    float step) {
        uint8_t p[28] = {0};
        put_u16(p, id);
        p[2] = static_cast<uint8_t>(tag);
        put_u32(p + 3, 0);
        put_f32(p + 7, lo);
        put_f32(p + 11, hi);
        put_f32(p + 15, def);
        put_f32(p + 19, step);
        put_f32(p + 23, def);
        p[27] = 0;
        feed_frame(MsgType::CapsParam, 0, p, sizeof(p));
    }

    void caps_begin(uint16_t count, uint32_t caps) {
        uint8_t p[7];
        p[0] = kVersion;
        put_u32(p + 1, caps);
        put_u16(p + 5, count);
        feed_frame(MsgType::CapsBegin, 0, p, sizeof(p));
    }
    void caps_end(uint16_t count) {
        uint8_t p[2];
        put_u16(p, count);
        feed_frame(MsgType::CapsEnd, 0, p, sizeof(p));
    }
    void state_begin(uint32_t rev, uint16_t count) {
        uint8_t p[6];
        put_u32(p, rev);
        put_u16(p + 4, count);
        feed_frame(MsgType::StateBegin, 0, p, sizeof(p));
    }
    void state_param(uint16_t id, ValueTag tag, float value) {
        uint8_t p[8];
        put_u16(p, id);
        p[2] = static_cast<uint8_t>(tag);
        size_t n = 3 + encode_value(tag, value, p + 3, sizeof(p) - 3);
        feed_frame(MsgType::StateParam, 0, p, n);
    }
    void state_end(uint32_t rev) {
        uint8_t p[4];
        put_u32(p, rev);
        feed_frame(MsgType::StateEnd, 0, p, sizeof(p));
    }
    void param_changed(uint16_t id, uint32_t rev, uint8_t seq, ValueTag tag,
                       float value) {
        uint8_t p[12];
        put_u16(p, id);
        put_u32(p + 2, rev);
        p[6] = 0; // origin controller
        p[7] = static_cast<uint8_t>(tag);
        size_t n = 8 + encode_value(tag, value, p + 8, sizeof(p) - 8);
        feed_frame(MsgType::ParamChanged, seq, p, n);
    }
    void ack_nack(MsgType type, uint8_t ref_type, uint8_t ref_seq, Code code) {
        uint8_t p[4] = {ref_type, ref_seq, 0, 0};
        put_u16(p + 2, static_cast<uint16_t>(code));
        feed_frame(type, ref_seq, p, sizeof(p));
    }
};

void advertise_common(Harness &h, bool include_formant = true,
                      bool include_presets = false) {
    const ValueTag B = ValueTag::Bool, I = ValueTag::Int32, F = ValueTag::Float32,
                  E = ValueTag::Enum16;
    struct P { uint16_t id; ValueTag tag; };
    std::vector<P> params = {
        {VOXP4_PARAM_HARMONY_ENABLE, B},      {VOXP4_PARAM_HARMONY_INTERVAL, I},
        {VOXP4_PARAM_HARMONY_LEVEL, F},       {VOXP4_PARAM_HARMONY_MODE, E},
        {VOXP4_PARAM_HARMONY_KEY, E},         {VOXP4_PARAM_HARMONY_SCALE, E},
        {VOXP4_PARAM_HARMONY_VOICE1_PAN, F},  {VOXP4_PARAM_HARMONY_VOICE1_DEGREE, I},
        {VOXP4_PARAM_HARMONY_VOICE1_SMOOTHING_MS, F},
        {VOXP4_PARAM_HARMONY_FORMANT_ENABLE, B},
        {VOXP4_PARAM_HARMONY_ATTACK_MS, F},   {VOXP4_PARAM_HARMONY_RELEASE_MS, F},
        {VOXP4_PARAM_HARMONY_LIMITER_ENABLE, B},
        {VOXP4_PARAM_HARMONY_LIMITER_THRESHOLD_DB, F},
        {VOXP4_PARAM_HARMONY_VOICE1_NON_SCALE_POLICY, E},
        {VOXP4_PARAM_HARMONY_VOICE1_VOICE_LEADING, B},
        {VOXP4_PARAM_REVERB_ENABLE, B},       {VOXP4_PARAM_REVERB_WET, F},
        {VOXP4_PARAM_REVERB_DECAY_S, F},      {VOXP4_PARAM_REVERB_DAMPING, F},
        {VOXP4_PARAM_DELAY_ENABLE, B},        {VOXP4_PARAM_DELAY_LEFT_MS, F},
        {VOXP4_PARAM_DELAY_RIGHT_MS, F},      {VOXP4_PARAM_DELAY_FEEDBACK, F},
        {VOXP4_PARAM_DELAY_WET, F},           {VOXP4_PARAM_DELAY_DRY, F},
        {VOXP4_PARAM_DELAY_FEEDBACK_LOWPASS_HZ, F},
    };
    if (include_formant) params.push_back({VOXP4_PARAM_HARMONY_FORMANT_AMOUNT, F});
    uint32_t caps = kCapHarmony | kCapReverb | kCapDelay | kCapLimiter |
                    kCapGate | kCapCompressor | kCapFormantPreservation;
    if (include_presets) caps |= kCapPresets;
    h.caps_begin(static_cast<uint16_t>(params.size()), caps);
    for (const auto &p : params) h.caps_param(p.id, p.tag, 0.0f, 1.0f, 0.0f, 0.01f);
    h.caps_end(static_cast<uint16_t>(params.size()));
}

// Drives the full handshake and leaves the client ACTIVE.
void complete_handshake(Harness &h) {
    h.client.begin(0);
    h.now = 0;
    h.drain_tx(); // HELLO
    h.hello_ack(kCapHarmony | kCapReverb | kCapDelay | kCapLimiter);
    h.drain_tx(); // CAPS_REQUEST
    advertise_common(h);
    h.drain_tx(); // GET_STATE
    h.state_begin(7, 2);
    h.state_param(VOXP4_PARAM_REVERB_WET, ValueTag::Float32, 0.18f);
    h.state_param(VOXP4_PARAM_DELAY_LEFT_MS, ValueTag::Float32, 250.0f);
    h.state_end(7);
    h.drain_tx();
    h.drain_events();
}

// Full 49-parameter schema straight from the generated ABI constants.
struct IdTag {
    uint16_t id;
    ValueTag tag;
};
const IdTag kFullSchema[] = {
    {VOXP4_PARAM_HARMONY_ENABLE, ValueTag::Bool},
    {VOXP4_PARAM_HARMONY_INTERVAL, ValueTag::Int32},
    {VOXP4_PARAM_HARMONY_LEVEL, ValueTag::Float32},
    {VOXP4_PARAM_HARMONY_MODE, ValueTag::Enum16},
    {VOXP4_PARAM_HARMONY_KEY, ValueTag::Enum16},
    {VOXP4_PARAM_HARMONY_SCALE, ValueTag::Enum16},
    {VOXP4_PARAM_HARMONY_VOICE1_PAN, ValueTag::Float32},
    {VOXP4_PARAM_HARMONY_VOICE1_DEGREE, ValueTag::Int32},
    {VOXP4_PARAM_HARMONY_VOICE1_SMOOTHING_MS, ValueTag::Float32},
    {VOXP4_PARAM_HARMONY_FORMANT_ENABLE, ValueTag::Bool},
    {VOXP4_PARAM_HARMONY_FORMANT_AMOUNT, ValueTag::Float32},
    {VOXP4_PARAM_HARMONY_ATTACK_MS, ValueTag::Float32},
    {VOXP4_PARAM_HARMONY_RELEASE_MS, ValueTag::Float32},
    {VOXP4_PARAM_HARMONY_LIMITER_ENABLE, ValueTag::Bool},
    {VOXP4_PARAM_HARMONY_LIMITER_THRESHOLD_DB, ValueTag::Float32},
    {VOXP4_PARAM_HARMONY_DRY_ALIGNMENT_ENABLE, ValueTag::Bool},
    {VOXP4_PARAM_HARMONY_DRY_ALIGNMENT_MS, ValueTag::Float32},
    {VOXP4_PARAM_HARMONY_VOICE1_NON_SCALE_POLICY, ValueTag::Enum16},
    {VOXP4_PARAM_HARMONY_VOICE1_VOICE_LEADING, ValueTag::Bool},
    {VOXP4_PARAM_HARMONY_VOICE1_MIN_MIDI, ValueTag::Float32},
    {VOXP4_PARAM_HARMONY_VOICE1_MAX_MIDI, ValueTag::Float32},
    {VOXP4_PARAM_COMPRESSOR_ENABLE, ValueTag::Bool},
    {VOXP4_PARAM_COMPRESSOR_THRESHOLD_DB, ValueTag::Float32},
    {VOXP4_PARAM_COMPRESSOR_RATIO, ValueTag::Float32},
    {VOXP4_PARAM_COMPRESSOR_ATTACK_MS, ValueTag::Float32},
    {VOXP4_PARAM_COMPRESSOR_RELEASE_MS, ValueTag::Float32},
    {VOXP4_PARAM_COMPRESSOR_MAKEUP_DB, ValueTag::Float32},
    {VOXP4_PARAM_COMPRESSOR_KNEE_DB, ValueTag::Float32},
    {VOXP4_PARAM_GATE_ENABLE, ValueTag::Bool},
    {VOXP4_PARAM_GATE_THRESHOLD_DB, ValueTag::Float32},
    {VOXP4_PARAM_GATE_ATTACK_MS, ValueTag::Float32},
    {VOXP4_PARAM_GATE_HOLD_MS, ValueTag::Float32},
    {VOXP4_PARAM_GATE_RELEASE_MS, ValueTag::Float32},
    {VOXP4_PARAM_GATE_RANGE_DB, ValueTag::Float32},
    {VOXP4_PARAM_DELAY_ENABLE, ValueTag::Bool},
    {VOXP4_PARAM_DELAY_LEFT_MS, ValueTag::Float32},
    {VOXP4_PARAM_DELAY_RIGHT_MS, ValueTag::Float32},
    {VOXP4_PARAM_DELAY_FEEDBACK, ValueTag::Float32},
    {VOXP4_PARAM_DELAY_WET, ValueTag::Float32},
    {VOXP4_PARAM_DELAY_DRY, ValueTag::Float32},
    {VOXP4_PARAM_DELAY_FEEDBACK_LOWPASS_HZ, ValueTag::Float32},
    {VOXP4_PARAM_REVERB_ENABLE, ValueTag::Bool},
    {VOXP4_PARAM_REVERB_WET, ValueTag::Float32},
    {VOXP4_PARAM_REVERB_DECAY_S, ValueTag::Float32},
    {VOXP4_PARAM_REVERB_DAMPING, ValueTag::Float32},
    {VOXP4_PARAM_LIMITER_CEILING, ValueTag::Float32},
    {VOXP4_PARAM_OUTPUT_MUTE_DRY, ValueTag::Bool},
    {VOXP4_PARAM_OUTPUT_SPATIAL_ROUTING, ValueTag::Enum16},
    {VOXP4_PARAM_OUTPUT_SPATIAL_SOURCE, ValueTag::Enum16},
};
constexpr size_t kFullSchemaCount = sizeof(kFullSchema) / sizeof(kFullSchema[0]);

void advertise_full(Harness &h, uint32_t caps) {
    h.caps_begin(static_cast<uint16_t>(kFullSchemaCount), caps);
    for (const auto &e : kFullSchema)
        h.caps_param(e.id, e.tag, 0.0f, 1.0f, 0.0f, 0.01f);
    h.caps_end(static_cast<uint16_t>(kFullSchemaCount));
}

const Frame *find(const std::vector<Frame> &v, MsgType t) {
    for (const auto &f : v)
        if (f.type == t) return &f;
    return nullptr;
}

} // namespace

void setUp() {}
void tearDown() {}

// --- CRC / codec / golden --------------------------------------------------
void test_crc_check_value(void) {
    const uint8_t text[] = "123456789";
    TEST_ASSERT_EQUAL_HEX16(0x29B1,
                            crc16_ccitt_false(text, sizeof(text) - 1));
}

void test_golden_decode(void) {
    Frame f;
    size_t consumed = 0;
    TEST_ASSERT_EQUAL((int)DecodeStatus::Ok,
                      (int)decode_frame(voxlink_vectors::kHello,
                                        sizeof(voxlink_vectors::kHello), &f,
                                        &consumed));
    TEST_ASSERT_EQUAL((int)MsgType::Hello, (int)f.type);
    TEST_ASSERT_EQUAL_UINT8(0x2A, f.seq);
    TEST_ASSERT_EQUAL((int)DecodeStatus::Ok,
                      (int)decode_frame(voxlink_vectors::kSetParam,
                                        sizeof(voxlink_vectors::kSetParam), &f,
                                        &consumed));
    TEST_ASSERT_EQUAL((int)MsgType::SetParam, (int)f.type);
    TEST_ASSERT_EQUAL((int)DecodeStatus::BadCrc,
                      (int)decode_frame(voxlink_vectors::kHelloBadCrc,
                                        sizeof(voxlink_vectors::kHelloBadCrc),
                                        &f, &consumed));
}

void test_parser_fragmentation_and_resync(void) {
    Parser parser;
    int frames = 0;
    auto emit = [&frames](const Frame &) { ++frames; };
    for (size_t i = 0; i < sizeof(voxlink_vectors::kHello); ++i)
        parser.feed(&voxlink_vectors::kHello[i], 1, emit);
    TEST_ASSERT_EQUAL_INT(1, frames);

    Parser parser2;
    int frames2 = 0;
    auto emit2 = [&frames2](const Frame &) { ++frames2; };
    const uint8_t garbage[4] = {0x00, 0x11, 0xA5, 0x22};
    parser2.feed(garbage, sizeof(garbage), emit2);
    parser2.feed(voxlink_vectors::kHelloBadCrc,
                 sizeof(voxlink_vectors::kHelloBadCrc), emit2);
    parser2.feed(voxlink_vectors::kGetParam, sizeof(voxlink_vectors::kGetParam),
                 emit2);
    TEST_ASSERT_EQUAL_INT(1, frames2);
    TEST_ASSERT_TRUE(parser2.counters().crc_errors >= 1);
}

// --- Handshake -------------------------------------------------------------
void test_full_handshake(void) {
    Harness h;
    h.client.begin(0);
    auto tx = h.drain_tx();
    TEST_ASSERT_NOT_NULL(find(tx, MsgType::Hello));

    h.hello_ack();
    tx = h.drain_tx();
    TEST_ASSERT_NOT_NULL(find(tx, MsgType::CapsRequest));

    advertise_common(h);
    tx = h.drain_tx();
    TEST_ASSERT_NOT_NULL(find(tx, MsgType::GetState));

    h.state_begin(7, 1);
    h.state_param(VOXP4_PARAM_REVERB_WET, ValueTag::Float32, 0.18f);
    h.state_end(7);
    h.drain_tx();

    TEST_ASSERT_TRUE(h.client.active());
    auto events = h.drain_events();
    bool link = false;
    for (const auto &e : events)
        if (e.type == EventType::LinkActive) link = true;
    TEST_ASSERT_TRUE(link);
}

// --- Snapshot --------------------------------------------------------------
void test_snapshot_transactional(void) {
    Harness h;
    complete_handshake(h);
    TEST_ASSERT_TRUE(h.client.active());
    // A second GET_STATE-like snapshot with more values.
    h.state_begin(9, 4);
    h.state_param(VOXP4_PARAM_REVERB_WET, ValueTag::Float32, 0.35f);
    h.state_param(VOXP4_PARAM_DELAY_LEFT_MS, ValueTag::Float32, 420.0f);
    h.state_param(VOXP4_PARAM_HARMONY_MODE, ValueTag::Enum16, 1.0f);
    h.state_param(VOXP4_PARAM_HARMONY_KEY, ValueTag::Enum16, 2.0f);
    // No events applied before STATE_END.
    TEST_ASSERT_EQUAL_INT(0, (int)h.drain_events().size());
    h.state_end(9);
    auto events = h.drain_events();
    float reverb = -1.0f, delay = -1.0f, mode = -1.0f;
    for (const auto &e : events) {
        if (e.id == VOXP4_PARAM_REVERB_WET) reverb = e.value;
        if (e.id == VOXP4_PARAM_DELAY_LEFT_MS) delay = e.value;
        if (e.id == VOXP4_PARAM_HARMONY_MODE) mode = e.value;
    }
    TEST_ASSERT_EQUAL_FLOAT(0.35f, reverb);
    TEST_ASSERT_EQUAL_FLOAT(420.0f, delay);
    TEST_ASSERT_EQUAL_FLOAT(1.0f, mode);
    TEST_ASSERT_EQUAL_UINT32(9, h.client.revision());
}

// --- SET / PARAM_CHANGED ---------------------------------------------------
void test_set_and_param_changed(void) {
    Harness h;
    complete_handshake(h);
    TEST_ASSERT_TRUE(h.client.set_parameter(VOXP4_PARAM_REVERB_WET, 0.353f, h.now));
    h.now += 40;
    h.client.tick(h.now);
    auto tx = h.drain_tx();
    const Frame *set = find(tx, MsgType::SetParam);
    TEST_ASSERT_NOT_NULL(set);
    TEST_ASSERT_EQUAL_HEX16(VOXP4_PARAM_REVERB_WET, get_u16(set->payload));
    TEST_ASSERT_EQUAL_UINT8((uint8_t)ValueTag::Float32, set->payload[2]);

    h.param_changed(VOXP4_PARAM_REVERB_WET, 10, set->seq, ValueTag::Float32, 0.35f);
    auto events = h.drain_events();
    float value = -1.0f;
    for (const auto &e : events)
        if (e.type == EventType::ParamAuthoritative &&
            e.id == VOXP4_PARAM_REVERB_WET)
            value = e.value;
    TEST_ASSERT_EQUAL_FLOAT(0.35f, value); // accepted/quantized value
}

// --- NACK rollback / QUEUE_FULL --------------------------------------------
void test_nack_rollback(void) {
    Harness h;
    complete_handshake(h);
    h.client.set_parameter(VOXP4_PARAM_REVERB_WET, 0.9f, h.now);
    h.now += 40;
    h.client.tick(h.now);
    auto tx = h.drain_tx();
    const Frame *set = find(tx, MsgType::SetParam);
    TEST_ASSERT_NOT_NULL(set);
    h.ack_nack(MsgType::Nack, (uint8_t)MsgType::SetParam, set->seq,
               Code::OutOfRange);
    auto events = h.drain_events();
    bool revert = false;
    for (const auto &e : events)
        if (e.type == EventType::ParamRevert &&
            e.id == VOXP4_PARAM_REVERB_WET)
            revert = true;
    TEST_ASSERT_TRUE(revert);
    TEST_ASSERT_EQUAL_UINT32(1, h.client.counters().nacks);
}

void test_queue_full_keeps_latest(void) {
    Harness h;
    complete_handshake(h);
    h.client.set_parameter(VOXP4_PARAM_REVERB_WET, 0.3f, h.now);
    h.now += 40;
    h.client.tick(h.now);
    auto tx = h.drain_tx();
    const Frame *first = find(tx, MsgType::SetParam);
    TEST_ASSERT_NOT_NULL(first);
    h.ack_nack(MsgType::Nack, (uint8_t)MsgType::SetParam, first->seq,
               Code::QueueFull);
    h.drain_events();
    TEST_ASSERT_EQUAL_UINT32(1, h.client.counters().queue_full);
    // A newer value arrives; the next window must send only the latest.
    h.client.set_parameter(VOXP4_PARAM_REVERB_WET, 0.5f, h.now);
    h.now += 40;
    h.client.tick(h.now);
    auto tx2 = h.drain_tx();
    const Frame *second = find(tx2, MsgType::SetParam);
    TEST_ASSERT_NOT_NULL(second);
    TEST_ASSERT_EQUAL_FLOAT(0.5f, get_f32(second->payload + 3));
}

// --- Reconnect -------------------------------------------------------------
void test_reconnect_p4_wins(void) {
    Harness h;
    complete_handshake(h);
    // Offline-ish local edit while still "active" but not confirmed.
    h.client.set_parameter(VOXP4_PARAM_DELAY_WET, 0.8f, h.now);
    // Link drops (heartbeat timeout).
    h.now += 4000;
    h.client.tick(h.now);
    auto down = h.drain_events();
    bool link_down = false;
    for (const auto &e : down)
        if (e.type == EventType::LinkDown) link_down = true;
    TEST_ASSERT_TRUE(link_down);
    TEST_ASSERT_FALSE(h.client.active());

    // Reconnect after the backoff window; P4 reports DelayWet = 0.20 which wins.
    h.drain_tx();
    h.now += 300;
    h.client.tick(h.now); // start handshake
    h.drain_tx();         // HELLO
    h.hello_ack();
    h.drain_tx();
    advertise_common(h);
    h.drain_tx();
    h.state_begin(11, 1);
    h.state_param(VOXP4_PARAM_DELAY_WET, ValueTag::Float32, 0.20f);
    h.state_end(11);
    auto events = h.drain_events();
    float delay = -1.0f;
    for (const auto &e : events)
        if (e.type == EventType::ParamAuthoritative &&
            e.id == VOXP4_PARAM_DELAY_WET)
            delay = e.value;
    TEST_ASSERT_EQUAL_FLOAT(0.20f, delay);
    TEST_ASSERT_TRUE(h.client.active());
}

// --- Capability gating -----------------------------------------------------
void test_caps_gating_and_unknown_param(void) {
    Harness h;
    h.client.begin(0);
    h.drain_tx();
    h.hello_ack();
    h.drain_tx();
    advertise_common(h, /*include_formant=*/false);
    h.drain_tx();
    h.state_begin(3, 1);
    // Unknown param in the snapshot must not disturb the session.
    h.state_param(0x7FFF, ValueTag::Float32, 0.5f);
    h.state_end(3);
    h.drain_events();
    TEST_ASSERT_TRUE(h.client.active());
    TEST_ASSERT_FALSE(h.client.parameter_supported(VOXP4_PARAM_HARMONY_FORMANT_AMOUNT));
    TEST_ASSERT_TRUE(h.client.parameter_supported(VOXP4_PARAM_REVERB_WET));
    // Unsupported parameter must not emit a SET intent.
    TEST_ASSERT_FALSE(h.client.set_parameter(
        VOXP4_PARAM_HARMONY_FORMANT_AMOUNT, 0.5f, h.now));
}

void test_version_mismatch_rejected(void) {
    Harness h;
    h.client.begin(0);
    h.drain_tx();
    uint8_t p[22] = {0};
    p[0] = 0x20; // major 2
    uint8_t buf[kMaxFrame];
    const size_t n = encode_frame(MsgType::HelloAck, 0, 0, p, sizeof(p), buf,
                                  sizeof(buf));
    h.feed_raw(buf, n);
    TEST_ASSERT_FALSE(h.client.active());
    TEST_ASSERT_TRUE(h.client.counters().version_errors >= 1);
}

// --- M6.1 hardening --------------------------------------------------------
void test_full_49_param_snapshot_no_drop(void) {
    Harness h;
    h.client.begin(0);
    h.drain_tx();
    h.hello_ack();
    h.drain_tx();
    uint32_t caps = kCapHarmony | kCapReverb | kCapDelay | kCapLimiter |
                    kCapGate | kCapCompressor | kCapFormantPreservation;
    advertise_full(h, caps);
    h.drain_tx();

    h.state_begin(7, (uint16_t)kFullSchemaCount);
    for (const auto &e : kFullSchema)
        h.state_param(e.id, e.tag, e.tag == ValueTag::Float32 ? 0.5f : 1.0f);
    h.state_end(7);

    auto events = h.drain_events();
    int authoritative = 0, link = 0;
    for (const auto &e : events) {
        if (e.type == EventType::ParamAuthoritative) ++authoritative;
        if (e.type == EventType::LinkActive) ++link;
    }
    TEST_ASSERT_EQUAL_INT((int)kFullSchemaCount, authoritative);
    TEST_ASSERT_EQUAL_INT(1, link);
    TEST_ASSERT_EQUAL_UINT32(0, h.client.counters().event_drops);
    TEST_ASSERT_TRUE(h.client.active());
    TEST_ASSERT_TRUE(!events.empty());
    // LinkActive must be last so the UI sees a complete snapshot first.
    TEST_ASSERT_EQUAL_INT((int)EventType::LinkActive, (int)events.back().type);
}

void test_set_ack_then_same_revision_param_changed(void) {
    Harness h;
    complete_handshake(h); // revision 7
    h.client.set_parameter(VOXP4_PARAM_REVERB_WET, 0.18f, h.now);
    h.now += 40;
    h.client.tick(h.now);
    auto tx = h.drain_tx();
    const Frame *set = find(tx, MsgType::SetParam);
    TEST_ASSERT_NOT_NULL(set);

    h.ack_nack(MsgType::Ack, (uint8_t)MsgType::SetParam, set->seq, Code::Ok);
    // ACK must NOT finalize a SET.
    TEST_ASSERT_EQUAL_INT(1, (int)h.client.pending_count());

    // Same revision, direct no-op confirmation must still be accepted.
    h.param_changed(VOXP4_PARAM_REVERB_WET, 7, set->seq, ValueTag::Float32,
                    0.18f);
    TEST_ASSERT_EQUAL_INT(0, (int)h.client.pending_count());
    auto events = h.drain_events();
    bool auth = false;
    for (const auto &e : events)
        if (e.type == EventType::ParamAuthoritative &&
            e.id == VOXP4_PARAM_REVERB_WET)
            auth = true;
    TEST_ASSERT_TRUE(auth);
}

void test_heartbeat_ack_seq_collision(void) {
    Harness h;
    complete_handshake(h);
    // First client request uses seq 0.
    h.client.set_parameter(VOXP4_PARAM_REVERB_WET, 0.3f, h.now);
    h.now += 40;
    h.client.tick(h.now);
    auto tx = h.drain_tx();
    const Frame *set = find(tx, MsgType::SetParam);
    TEST_ASSERT_NOT_NULL(set);
    TEST_ASSERT_EQUAL_UINT8(0, set->seq);

    // A heartbeat ACK (ref_type Heartbeat, seq 0) must not touch the SET.
    h.ack_nack(MsgType::Ack, (uint8_t)MsgType::Heartbeat, 0, Code::Ok);
    TEST_ASSERT_EQUAL_INT(1, (int)h.client.pending_count());

    // The matching PARAM_CHANGED still finalizes it.
    h.param_changed(VOXP4_PARAM_REVERB_WET, 7, set->seq, ValueTag::Float32, 0.3f);
    TEST_ASSERT_EQUAL_INT(0, (int)h.client.pending_count());
}

void test_bool_queue_full_retry_success(void) {
    Harness h;
    complete_handshake(h);
    TEST_ASSERT_TRUE(
        h.client.set_parameter(VOXP4_PARAM_REVERB_ENABLE, 0.0f, h.now));
    auto tx = h.drain_tx();
    const Frame *set = find(tx, MsgType::SetParam);
    TEST_ASSERT_NOT_NULL(set);
    h.ack_nack(MsgType::Nack, (uint8_t)MsgType::SetParam, set->seq,
               Code::QueueFull);
    TEST_ASSERT_EQUAL_UINT32(1, h.client.counters().queue_full);

    h.now += 40;
    h.client.tick(h.now);
    auto tx2 = h.drain_tx();
    const Frame *retry = find(tx2, MsgType::SetParam);
    TEST_ASSERT_NOT_NULL(retry);
    h.param_changed(VOXP4_PARAM_REVERB_ENABLE, 8, retry->seq, ValueTag::Bool,
                    0.0f);
    auto events = h.drain_events();
    bool auth = false;
    for (const auto &e : events)
        if (e.type == EventType::ParamAuthoritative &&
            e.id == VOXP4_PARAM_REVERB_ENABLE && e.value == 0.0f)
            auth = true;
    TEST_ASSERT_TRUE(auth);
}

void test_bool_queue_full_retry_exhaustion_rollback(void) {
    Harness h;
    complete_handshake(h);
    h.client.set_parameter(VOXP4_PARAM_REVERB_ENABLE, 0.0f, h.now);
    auto tx = h.drain_tx();
    const Frame *set = find(tx, MsgType::SetParam);
    TEST_ASSERT_NOT_NULL(set);
    uint8_t seq = set->seq;
    for (int i = 0; i < 4; ++i) {
        h.ack_nack(MsgType::Nack, (uint8_t)MsgType::SetParam, seq, Code::QueueFull);
        h.now += 40;
        h.client.tick(h.now);
        auto t = h.drain_tx();
        const Frame *next = find(t, MsgType::SetParam);
        if (next != nullptr) seq = next->seq;
    }
    auto events = h.drain_events();
    bool revert = false;
    for (const auto &e : events)
        if (e.type == EventType::ParamRevert &&
            e.id == VOXP4_PARAM_REVERB_ENABLE)
            revert = true;
    TEST_ASSERT_TRUE(revert);
    TEST_ASSERT_TRUE(h.client.counters().queue_full_exhausted >= 1);
}

void test_enum_queue_full_latest_wins(void) {
    Harness h;
    complete_handshake(h);
    h.client.set_parameter(VOXP4_PARAM_HARMONY_MODE, 1.0f, h.now);
    auto tx = h.drain_tx();
    const Frame *set = find(tx, MsgType::SetParam);
    TEST_ASSERT_NOT_NULL(set);
    h.ack_nack(MsgType::Nack, (uint8_t)MsgType::SetParam, set->seq,
               Code::QueueFull);
    // User changes again before the retry window.
    h.client.set_parameter(VOXP4_PARAM_HARMONY_MODE, 2.0f, h.now);
    h.now += 40;
    h.client.tick(h.now);
    auto tx2 = h.drain_tx();
    const Frame *retry = find(tx2, MsgType::SetParam);
    TEST_ASSERT_NOT_NULL(retry);
    TEST_ASSERT_EQUAL_UINT16(2, get_u16(retry->payload + 3));
}

void test_pending_pool_saturation(void) {
    Harness h;
    complete_handshake(h);
    const uint16_t ids[8] = {
        VOXP4_PARAM_HARMONY_INTERVAL,
        VOXP4_PARAM_HARMONY_VOICE1_DEGREE,
        VOXP4_PARAM_HARMONY_KEY,
        VOXP4_PARAM_HARMONY_SCALE,
        VOXP4_PARAM_HARMONY_MODE,
        VOXP4_PARAM_HARMONY_VOICE1_NON_SCALE_POLICY,
        VOXP4_PARAM_HARMONY_VOICE1_VOICE_LEADING,
        VOXP4_PARAM_HARMONY_FORMANT_ENABLE,
    };
    for (uint16_t id : ids)
        TEST_ASSERT_TRUE(h.client.set_parameter(id, 1.0f, h.now));
    TEST_ASSERT_EQUAL_INT((int)VoxLinkClient::kPendingCapacity,
                          (int)h.client.pending_count());
    const uint32_t before = h.client.counters().pending_full;
    TEST_ASSERT_FALSE(
        h.client.set_parameter(VOXP4_PARAM_HARMONY_LIMITER_ENABLE, 1.0f, h.now));
    TEST_ASSERT_TRUE(h.client.counters().pending_full > before);
    TEST_ASSERT_TRUE(h.client.pending_count() <=
                     (int)VoxLinkClient::kPendingCapacity);

    auto tx = h.drain_tx();
    std::vector<uint8_t> seqs;
    for (const auto &f : tx)
        if (f.type == MsgType::SetParam) seqs.push_back(f.seq);
    for (size_t i = 0; i < seqs.size(); ++i)
        for (size_t j = i + 1; j < seqs.size(); ++j)
            TEST_ASSERT_TRUE(seqs[i] != seqs[j]);
}

void test_tx_ring_saturation(void) {
    Harness h;
    complete_handshake(h);
    h.drain_tx();
    for (int i = 0; i < 300; ++i) {
        h.feed_frame(MsgType::Heartbeat, 0, nullptr, 0); // keep link alive
        h.now += 1000;
        h.client.tick(h.now);
    }
    TEST_ASSERT_TRUE(h.client.counters().tx_drops > 0);
    TEST_ASSERT_TRUE(h.client.tx_used() <= VoxLinkClient::kTxCapacity - 1);

    uint8_t buf[VoxLinkClient::kTxCapacity];
    const size_t n = h.client.take_tx(buf, sizeof(buf));
    TEST_ASSERT_TRUE(n <= VoxLinkClient::kTxCapacity - 1);
    size_t off = 0;
    while (off < n) {
        Frame f;
        size_t consumed = 0;
        if (decode_frame(buf + off, n - off, &f, &consumed) != DecodeStatus::Ok)
            break;
        off += consumed;
    }
    TEST_ASSERT_EQUAL_UINT((unsigned)off, (unsigned)n); // ring stayed intact
    TEST_ASSERT_EQUAL_UINT(0, (unsigned)h.client.tx_used());
}

void test_garbage_does_not_keep_link_alive(void) {
    Harness h;
    complete_handshake(h);
    h.drain_tx();
    const uint8_t zeros[16] = {0};
    bool down = false;
    for (int i = 0; i < 40 && !down; ++i) {
        h.now += 100;
        h.feed_raw(zeros, sizeof(zeros));
        h.feed_raw(voxlink_vectors::kHelloBadCrc,
                   sizeof(voxlink_vectors::kHelloBadCrc));
        h.client.tick(h.now);
        for (const auto &e : h.drain_events())
            if (e.type == EventType::LinkDown) down = true;
    }
    TEST_ASSERT_TRUE(down);
    TEST_ASSERT_FALSE(h.client.active());
}

void test_stale_param_value_ignored(void) {
    Harness h;
    complete_handshake(h); // revision 7
    h.param_changed(VOXP4_PARAM_REVERB_WET, 20, 0xFE, ValueTag::Float32, 0.50f);
    h.drain_events();
    TEST_ASSERT_EQUAL_UINT32(20, h.client.revision());

    uint8_t p[11];
    put_u16(p, VOXP4_PARAM_REVERB_WET);
    put_u32(p + 2, 19);
    p[6] = (uint8_t)ValueTag::Float32;
    put_f32(p + 7, 0.20f);
    h.feed_frame(MsgType::ParamValue, 0xFD, p, sizeof(p));
    auto events = h.drain_events();
    TEST_ASSERT_EQUAL_INT(0, (int)events.size());
    TEST_ASSERT_EQUAL_UINT32(20, h.client.revision());
}

void test_reconnect_replaces_caps(void) {
    Harness h;
    complete_handshake(h); // advertises formant
    TEST_ASSERT_TRUE(
        h.client.parameter_supported(VOXP4_PARAM_HARMONY_FORMANT_AMOUNT));

    h.now += 4000;
    h.client.tick(h.now);
    h.drain_events();
    TEST_ASSERT_FALSE(h.client.active());

    h.now += 300;
    h.client.tick(h.now);
    h.drain_tx();
    h.hello_ack();
    h.drain_tx();
    advertise_common(h, /*include_formant=*/false);
    h.drain_tx();
    h.state_begin(12, 0);
    h.state_end(12);

    TEST_ASSERT_TRUE(h.client.active());
    TEST_ASSERT_FALSE(
        h.client.parameter_supported(VOXP4_PARAM_HARMONY_FORMANT_AMOUNT));
}

void test_caps_overflow_rejected(void) {
    Harness h;
    h.client.begin(0);
    h.drain_tx();
    h.hello_ack();
    h.drain_tx();
    h.caps_begin(65, kCapHarmony); // > kMaxCapsParams
    TEST_ASSERT_FALSE(h.client.caps_valid());
    TEST_ASSERT_TRUE(h.client.counters().caps_overflow >= 1);
    auto tx = h.drain_tx();
    TEST_ASSERT_NULL(find(tx, MsgType::GetState));
}

void test_snapshot_overflow_rejected(void) {
    Harness h;
    complete_handshake(h);
    const uint32_t rev = h.client.revision();
    h.state_begin(rev + 1, 65); // > kMaxSnapshotParams
    h.state_param(VOXP4_PARAM_REVERB_WET, ValueTag::Float32, 0.9f);
    h.state_end(rev + 1);
    auto events = h.drain_events();
    TEST_ASSERT_TRUE(h.client.counters().snapshot_overflow >= 1);
    TEST_ASSERT_EQUAL_UINT32(rev, h.client.revision());
    TEST_ASSERT_EQUAL_INT(0, (int)events.size());
}

int main(int, char **) {
    UNITY_BEGIN();
    RUN_TEST(test_crc_check_value);
    RUN_TEST(test_golden_decode);
    RUN_TEST(test_parser_fragmentation_and_resync);
    RUN_TEST(test_full_handshake);
    RUN_TEST(test_snapshot_transactional);
    RUN_TEST(test_set_and_param_changed);
    RUN_TEST(test_nack_rollback);
    RUN_TEST(test_queue_full_keeps_latest);
    RUN_TEST(test_reconnect_p4_wins);
    RUN_TEST(test_caps_gating_and_unknown_param);
    RUN_TEST(test_version_mismatch_rejected);
    RUN_TEST(test_full_49_param_snapshot_no_drop);
    RUN_TEST(test_set_ack_then_same_revision_param_changed);
    RUN_TEST(test_heartbeat_ack_seq_collision);
    RUN_TEST(test_bool_queue_full_retry_success);
    RUN_TEST(test_bool_queue_full_retry_exhaustion_rollback);
    RUN_TEST(test_enum_queue_full_latest_wins);
    RUN_TEST(test_pending_pool_saturation);
    RUN_TEST(test_tx_ring_saturation);
    RUN_TEST(test_garbage_does_not_keep_link_alive);
    RUN_TEST(test_stale_param_value_ignored);
    RUN_TEST(test_reconnect_replaces_caps);
    RUN_TEST(test_caps_overflow_rejected);
    RUN_TEST(test_snapshot_overflow_rejected);
    return UNITY_END();
}
