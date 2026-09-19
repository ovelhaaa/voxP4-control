#ifndef VOXLINK_CLIENT_H
#define VOXLINK_CLIENT_H

// VoxLink v1 client for the CYD. Pure C++ (no Arduino/LVGL) so the whole
// protocol + state machine runs under `pio test -e native`. The transport glue
// owns a single instance from one task and exchanges events/intents with the UI.
#include "voxlink/VoxLinkParser.h"
#include "voxlink/VoxLinkProtocol.h"
#include <cstddef>
#include <cstdint>

namespace voxlink {

enum class State : uint8_t {
    Disconnected = 0,
    HelloSent,
    CapsReceiving,
    StateReceiving,
    Active,
};

enum class EventType : uint8_t {
    LinkActive = 0,
    LinkDown,
    ParamAuthoritative, // id/value accepted by the P4
    ParamRevert,        // request failed (NACK/timeout): roll back to last auth
};

struct Event {
    EventType type = EventType::LinkDown;
    uint16_t id = 0;
    float value = 0.0f;
};

struct CapsParam {
    bool used = false;
    uint16_t id = 0;
    ValueTag tag = ValueTag::Float32;
    uint32_t flags = 0;
    float min = 0.0f;
    float max = 0.0f;
    float step = 0.0f;
    float default_value = 0.0f;
};

struct CapsSnapshot {
    bool valid = false;
    uint8_t version = 0;
    uint32_t caps = 0;
    uint32_t sample_rate = 0;
    uint16_t block_size = 0;
    uint8_t harmony_voices = 0;
    uint16_t param_count = 0;
    CapsParam params[kMaxCapsParams];
};

struct Counters {
    uint32_t frames_rx = 0;
    uint32_t frames_tx = 0;
    uint32_t bytes_rx = 0;
    uint32_t bytes_tx = 0;
    uint32_t crc_errors = 0;
    uint32_t length_errors = 0;
    uint32_t version_errors = 0;
    uint32_t unknown_messages = 0;
    uint32_t unknown_params = 0;
    uint32_t parse_errors = 0;
    uint32_t timeouts = 0;
    uint32_t nacks = 0;
    uint32_t queue_full = 0;
    uint32_t reconnects = 0;
    uint32_t tx_drops = 0;
};

class VoxLinkClient {
public:
    static constexpr size_t kTxCapacity = 1024;
    static constexpr size_t kEventCapacity = 32;
    static constexpr size_t kPendingCapacity = 8;
    static constexpr size_t kCoalesceCapacity = 8;

    VoxLinkClient();

    // Fully resets the client and (re)starts the handshake at `now_ms`.
    void begin(uint32_t now_ms);
    void stop();

    void feed(const uint8_t *data, size_t len, uint32_t now_ms);
    void tick(uint32_t now_ms);

    size_t take_tx(uint8_t *out, size_t capacity);
    bool take_event(Event *out);

    // UI intent. Only sent while Active and the parameter is advertised.
    bool set_parameter(uint16_t id, float value, uint32_t now_ms);
    bool get_parameter(uint16_t id, uint32_t now_ms);

    bool parameter_supported(uint16_t id) const;
    bool presets_supported() const;

    State state() const { return state_; }
    bool active() const { return state_ == State::Active; }
    uint32_t revision() const { return revision_; }
    bool have_revision() const { return have_revision_; }
    const CapsSnapshot &caps() const { return caps_; }
    const Counters &counters() const { return counters_; }
    size_t pending_count() const;

private:
    struct Pending {
        bool used = false;
        uint8_t seq = 0;
        MsgType type = MsgType::SetParam;
        uint16_t id = 0;
        uint32_t sent_ms = 0;
    };
    struct Coalesce {
        bool used = false;
        bool dirty = false;
        uint16_t id = 0;
        float value = 0.0f;
    };

    void start_handshake(uint32_t now_ms);
    void disconnect(uint32_t now_ms, bool timeout);
    void handle_frame(const Frame &frame, uint32_t now_ms);
    void handle_hello_ack(const Frame &frame, uint32_t now_ms);
    void handle_caps_begin(const Frame &frame);
    void handle_caps_param(const Frame &frame);
    void handle_caps_end(const Frame &frame, uint32_t now_ms);
    void handle_state_begin(const Frame &frame);
    void handle_state_param(const Frame &frame);
    void handle_state_end(const Frame &frame, uint32_t now_ms);
    void handle_param_changed(const Frame &frame);
    void handle_param_value(const Frame &frame);
    void handle_ack_nack(const Frame &frame, bool nack, uint32_t now_ms);

    bool send_set_now(uint16_t id, ValueTag tag, float value, uint32_t now_ms);
    bool send_simple(MsgType type, uint8_t seq, const uint8_t *payload,
                     size_t len);
    void send_heartbeat(uint32_t now_ms);
    void flush_coalesced(uint32_t now_ms);
    void expire_pending(uint32_t now_ms);
    void coalesce_put(uint16_t id, float value);
    Coalesce *coalesce_find(uint16_t id);
    Pending *pending_find_seq(uint8_t seq);
    void pending_remove(Pending *p);
    CapsParam *caps_find(uint16_t id);

    void push_event(EventType type, uint16_t id, float value);
    bool tx_push(const uint8_t *data, size_t len);

    Parser parser_;
    State state_ = State::Disconnected;
    Counters counters_{};
    CapsSnapshot caps_{};

    uint8_t tx_[kTxCapacity] = {0};
    size_t tx_head_ = 0;
    size_t tx_tail_ = 0;

    Event events_[kEventCapacity];
    uint8_t ev_head_ = 0;
    uint8_t ev_tail_ = 0;

    Pending pending_[kPendingCapacity];
    Coalesce coalesce_[kCoalesceCapacity];

    uint8_t next_seq_ = 0;
    uint32_t last_rx_ms_ = 0;
    uint32_t last_heartbeat_ms_ = 0;
    uint32_t last_flush_ms_ = 0;
    uint32_t deadline_ms_ = 0;
    uint32_t next_attempt_ms_ = 0;
    uint16_t backoff_ms_ = 0;
    uint8_t backoff_index_ = 0;
    bool caps_receiving_ = false;
    bool caps_received_ = false;
    uint16_t caps_received_count_ = 0;

    bool in_snapshot_ = false;
    uint16_t snap_expected_ = 0;
    uint16_t snap_count_ = 0;
    uint32_t snap_revision_ = 0;
    struct SnapEntry {
        uint16_t id;
        ValueTag tag;
        float value;
    };
    SnapEntry snap_[kMaxSnapshotParams];

    uint32_t revision_ = 0;
    bool have_revision_ = false;
    uint64_t parser_version_baseline_ = 0;
};

} // namespace voxlink

#endif // VOXLINK_CLIENT_H
