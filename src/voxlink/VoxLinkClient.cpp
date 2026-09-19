#include "voxlink/VoxLinkClient.h"

#include <cstring>

namespace voxlink {
namespace {
constexpr uint32_t kHelloTimeoutMs = 1500;
constexpr uint32_t kCapsTimeoutMs = 2000;
constexpr uint32_t kStateTimeoutMs = 2000;
constexpr uint32_t kRequestTimeoutMs = 500;
constexpr uint32_t kHeartbeatIntervalMs = 1000;
constexpr uint32_t kHeartbeatTimeoutMs = 3000;
constexpr uint32_t kCoalesceIntervalMs = 33;
constexpr uint8_t kClientTypeCyd = 1;
} // namespace

VoxLinkClient::VoxLinkClient() { begin(0); }

void VoxLinkClient::begin(uint32_t now_ms) {
    parser_.reset();
    parser_.reset_counters();
    parser_version_baseline_ = 0;
    state_ = State::Disconnected;
    caps_ = CapsSnapshot{};
    tx_head_ = tx_tail_ = 0;
    ev_head_ = ev_tail_ = 0;
    for (auto &p : pending_) p = Pending{};
    for (auto &c : coalesce_) c = Coalesce{};
    next_seq_ = 0;
    last_rx_ms_ = now_ms;
    last_heartbeat_ms_ = now_ms;
    last_flush_ms_ = now_ms;
    backoff_index_ = 0;
    backoff_ms_ = 0;
    next_attempt_ms_ = now_ms;
    in_snapshot_ = false;
    snap_count_ = 0;
    start_handshake(now_ms);
}

void VoxLinkClient::stop() {
    state_ = State::Disconnected;
    for (auto &p : pending_) p = Pending{};
}

void VoxLinkClient::start_handshake(uint32_t now_ms) {
    parser_.reset();
    in_snapshot_ = false;
    snap_count_ = 0;
    caps_receiving_ = false;
    caps_received_ = false;
    caps_received_count_ = 0;
    for (auto &p : pending_) p = Pending{};
    for (auto &c : coalesce_) c.dirty = false;

    uint8_t payload[3] = {kVersion, kClientTypeCyd, 0};
    send_simple(MsgType::Hello, 0, payload, sizeof(payload));
    state_ = State::HelloSent;
    deadline_ms_ = now_ms + kHelloTimeoutMs;
    last_rx_ms_ = now_ms;
    last_heartbeat_ms_ = now_ms;
}

void VoxLinkClient::disconnect(uint32_t now_ms, bool timeout) {
    state_ = State::Disconnected;
    if (timeout) ++counters_.timeouts;
    for (auto &p : pending_) p = Pending{};
    for (auto &c : coalesce_) c.dirty = false;

    static const uint16_t kBackoff[] = {250, 500, 1000, 2000, 2000};
    backoff_ms_ = kBackoff[backoff_index_];
    if (backoff_index_ < 4) ++backoff_index_;
    next_attempt_ms_ = now_ms + backoff_ms_;
    push_event(EventType::LinkDown, 0, 0.0f);
}

void VoxLinkClient::feed(const uint8_t *data, size_t len, uint32_t now_ms) {
    if (data == nullptr || len == 0) return;
    counters_.bytes_rx += static_cast<uint32_t>(len);
    last_rx_ms_ = now_ms;
    parser_.feed(data, len, [this, now_ms](const Frame &f) {
        handle_frame(f, now_ms);
    });
    const ParserCounters &pc = parser_.counters();
    counters_.frames_rx = static_cast<uint32_t>(pc.frames);
    counters_.crc_errors = static_cast<uint32_t>(pc.crc_errors);
    counters_.length_errors = static_cast<uint32_t>(pc.length_errors);
    // Parser version errors are cumulative; fold in the delta so HelloAck
    // version mismatches (counted in handle_hello_ack) are preserved.
    counters_.version_errors +=
        static_cast<uint32_t>(pc.version_errors - parser_version_baseline_);
    parser_version_baseline_ = pc.version_errors;
}

void VoxLinkClient::tick(uint32_t now_ms) {
    if (state_ == State::Active) {
        if ((int32_t)(now_ms - last_rx_ms_) > (int32_t)kHeartbeatTimeoutMs) {
            disconnect(now_ms, true);
            return;
        }
        if ((int32_t)(now_ms - last_heartbeat_ms_) >= (int32_t)kHeartbeatIntervalMs) {
            send_heartbeat(now_ms);
        }
        if ((int32_t)(now_ms - last_flush_ms_) >= (int32_t)kCoalesceIntervalMs) {
            flush_coalesced(now_ms);
        }
        expire_pending(now_ms);
        return;
    }
    if (state_ == State::Disconnected) {
        if ((int32_t)(now_ms - next_attempt_ms_) >= 0) start_handshake(now_ms);
        return;
    }
    // Handshake in progress.
    if ((int32_t)(now_ms - deadline_ms_) >= 0) disconnect(now_ms, true);
}

size_t VoxLinkClient::take_tx(uint8_t *out, size_t capacity) {
    if (out == nullptr) return 0;
    size_t n = 0;
    while (n < capacity && tx_tail_ != tx_head_) {
        out[n++] = tx_[tx_tail_];
        tx_tail_ = (tx_tail_ + 1) % kTxCapacity;
    }
    return n;
}

bool VoxLinkClient::take_event(Event *out) {
    if (out == nullptr || ev_head_ == ev_tail_) return false;
    *out = events_[ev_tail_];
    ev_tail_ = static_cast<uint8_t>((ev_tail_ + 1) % kEventCapacity);
    return true;
}

// ---------------------------------------------------------------------------
// TX helpers
// ---------------------------------------------------------------------------
bool VoxLinkClient::tx_push(const uint8_t *data, size_t len) {
    const size_t used = (tx_head_ + kTxCapacity - tx_tail_) % kTxCapacity;
    if (used + len > kTxCapacity) {
        ++counters_.tx_drops;
        return false;
    }
    for (size_t i = 0; i < len; ++i) {
        tx_[tx_head_] = data[i];
        tx_head_ = (tx_head_ + 1) % kTxCapacity;
    }
    return true;
}

bool VoxLinkClient::send_simple(MsgType type, uint8_t seq, const uint8_t *payload,
                                size_t len) {
    uint8_t scratch[kMaxFrame];
    const size_t n = encode_frame(type, 0, seq, payload, len, scratch, sizeof(scratch));
    if (n == 0) return false;
    if (!tx_push(scratch, n)) return false;
    ++counters_.frames_tx;
    counters_.bytes_tx += static_cast<uint32_t>(n);
    return true;
}

bool VoxLinkClient::send_set_now(uint16_t id, ValueTag tag, float value,
                                 uint32_t now_ms) {
    uint8_t body[8];
    put_u16(body, id);
    body[2] = static_cast<uint8_t>(tag);
    const size_t vn = encode_value(tag, value, body + 3, sizeof(body) - 3);
    if (vn == 0) return false;
    const size_t len = 3 + vn;

    uint8_t seq = 0;
    for (int attempt = 0; attempt < 256; ++attempt) {
        seq = next_seq_++;
        if (pending_find_seq(seq) == nullptr) break;
    }
    for (auto &p : pending_) {
        if (!p.used) {
            p.used = true;
            p.seq = seq;
            p.type = MsgType::SetParam;
            p.id = id;
            p.sent_ms = now_ms;
            break;
        }
    }
    return send_simple(MsgType::SetParam, seq, body, len);
}

void VoxLinkClient::send_heartbeat(uint32_t now_ms) {
    last_heartbeat_ms_ = now_ms;
    send_simple(MsgType::Heartbeat, 0, nullptr, 0);
}

void VoxLinkClient::coalesce_put(uint16_t id, float value) {
    for (auto &c : coalesce_) {
        if (c.used && c.id == id) {
            c.value = value;
            c.dirty = true;
            return;
        }
    }
    for (auto &c : coalesce_) {
        if (!c.used) {
            c.used = true;
            c.id = id;
            c.value = value;
            c.dirty = true;
            return;
        }
    }
    // Table full: fall back to an immediate send on the next tick is not
    // possible without a slot, so drop. Capacity 8 covers the editor.
}

VoxLinkClient::Coalesce *VoxLinkClient::coalesce_find(uint16_t id) {
    for (auto &c : coalesce_)
        if (c.used && c.id == id) return &c;
    return nullptr;
}

void VoxLinkClient::flush_coalesced(uint32_t now_ms) {
    last_flush_ms_ = now_ms;
    for (auto &c : coalesce_) {
        if (!c.used || !c.dirty) continue;
        CapsParam *cp = caps_find(c.id);
        if (cp == nullptr) {
            c.dirty = false;
            continue;
        }
        if (send_set_now(c.id, cp->tag, c.value, now_ms)) c.dirty = false;
    }
}

// ---------------------------------------------------------------------------
// Pending requests
// ---------------------------------------------------------------------------
VoxLinkClient::Pending *VoxLinkClient::pending_find_seq(uint8_t seq) {
    for (auto &p : pending_)
        if (p.used && p.seq == seq) return &p;
    return nullptr;
}

void VoxLinkClient::pending_remove(Pending *p) {
    if (p != nullptr) *p = Pending{};
}

size_t VoxLinkClient::pending_count() const {
    size_t n = 0;
    for (const auto &p : pending_)
        if (p.used) ++n;
    return n;
}

void VoxLinkClient::expire_pending(uint32_t now_ms) {
    for (auto &p : pending_) {
        if (!p.used) continue;
        if ((int32_t)(now_ms - p.sent_ms) > (int32_t)kRequestTimeoutMs) {
            ++counters_.timeouts;
            if (p.type == MsgType::SetParam) push_event(EventType::ParamRevert, p.id, 0.0f);
            p = Pending{};
        }
    }
}

// ---------------------------------------------------------------------------
// UI intents
// ---------------------------------------------------------------------------
bool VoxLinkClient::set_parameter(uint16_t id, float value, uint32_t now_ms) {
    if (state_ != State::Active) return false;
    CapsParam *cp = caps_find(id);
    if (cp == nullptr) {
        ++counters_.unknown_params;
        return false;
    }
    if (cp->tag == ValueTag::Float32) {
        coalesce_put(id, value);
        return true;
    }
    return send_set_now(id, cp->tag, value, now_ms);
}

bool VoxLinkClient::get_parameter(uint16_t id, uint32_t now_ms) {
    if (state_ != State::Active) return false;
    if (caps_find(id) == nullptr) return false;
    uint8_t body[2];
    put_u16(body, id);
    uint8_t seq = 0;
    for (int attempt = 0; attempt < 256; ++attempt) {
        seq = next_seq_++;
        if (pending_find_seq(seq) == nullptr) break;
    }
    for (auto &p : pending_) {
        if (!p.used) {
            p.used = true;
            p.seq = seq;
            p.type = MsgType::GetParam;
            p.id = id;
            p.sent_ms = now_ms;
            break;
        }
    }
    return send_simple(MsgType::GetParam, seq, body, sizeof(body));
}

// ---------------------------------------------------------------------------
// Caps
// ---------------------------------------------------------------------------
CapsParam *VoxLinkClient::caps_find(uint16_t id) {
    for (auto &p : caps_.params)
        if (p.used && p.id == id) return &p;
    return nullptr;
}

bool VoxLinkClient::parameter_supported(uint16_t id) const {
    if (!caps_.valid) return true; // offline local development
    for (const auto &p : caps_.params)
        if (p.used && p.id == id) return true;
    return false;
}

bool VoxLinkClient::presets_supported() const {
    return caps_.valid && (caps_.caps & kCapPresets) != 0;
}

// ---------------------------------------------------------------------------
// Frame handling
// ---------------------------------------------------------------------------
void VoxLinkClient::handle_frame(const Frame &frame, uint32_t now_ms) {
    switch (frame.type) {
        case MsgType::HelloAck:
            handle_hello_ack(frame, now_ms);
            break;
        case MsgType::CapsBegin:
            handle_caps_begin(frame);
            break;
        case MsgType::CapsParam:
            handle_caps_param(frame);
            break;
        case MsgType::CapsEnd:
            handle_caps_end(frame, now_ms);
            break;
        case MsgType::StateBegin:
            handle_state_begin(frame);
            break;
        case MsgType::StateParam:
            handle_state_param(frame);
            break;
        case MsgType::StateEnd:
            handle_state_end(frame, now_ms);
            break;
        case MsgType::ParamChanged:
            handle_param_changed(frame);
            break;
        case MsgType::ParamValue:
            handle_param_value(frame);
            break;
        case MsgType::Ack:
            handle_ack_nack(frame, false, now_ms);
            break;
        case MsgType::Nack:
            handle_ack_nack(frame, true, now_ms);
            break;
        case MsgType::Error:
            ++counters_.parse_errors;
            break;
        case MsgType::Heartbeat:
            // Any valid frame already refreshed last_rx_ms_; nothing else.
            break;
        default:
            ++counters_.unknown_messages;
            break;
    }
}

void VoxLinkClient::handle_hello_ack(const Frame &frame, uint32_t now_ms) {
    if (state_ != State::HelloSent) return;
    if (frame.payload_len < 22) {
        ++counters_.parse_errors;
        return;
    }
    const uint8_t *p = frame.payload;
    if ((p[0] >> 4) != kVersionMajor) {
        ++counters_.version_errors;
        disconnect(now_ms, false);
        return;
    }
    caps_.valid = true;
    caps_.version = p[0];
    caps_.caps = get_u32(p + 1);
    caps_.sample_rate = get_u32(p + 5);
    caps_.block_size = get_u16(p + 9);
    caps_.harmony_voices = p[11];
    // state_revision at 12, product 16, hw 17, fw 18..20, sha 21...
    uint8_t req = 0;
    send_simple(MsgType::CapsRequest, 0, &req, 0);
    state_ = State::CapsReceiving;
    deadline_ms_ = now_ms + kCapsTimeoutMs;
}

void VoxLinkClient::handle_caps_begin(const Frame &frame) {
    if (state_ != State::CapsReceiving) return;
    if (frame.payload_len < 7) {
        ++counters_.parse_errors;
        return;
    }
    const uint8_t *p = frame.payload;
    caps_.version = p[0];
    caps_.caps = get_u32(p + 1);
    caps_.param_count = get_u16(p + 5);
    for (auto &param : caps_.params) param = CapsParam{};
    caps_received_count_ = 0;
    caps_receiving_ = true;
}

void VoxLinkClient::handle_caps_param(const Frame &frame) {
    if (state_ != State::CapsReceiving || !caps_receiving_) return;
    if (frame.payload_len < 28) {
        ++counters_.parse_errors;
        return;
    }
    const uint8_t *p = frame.payload;
    if (caps_received_count_ >= kMaxCapsParams) {
        ++counters_.parse_errors;
        return;
    }
    CapsParam cp;
    cp.used = true;
    cp.id = get_u16(p);
    cp.tag = static_cast<ValueTag>(p[2]);
    cp.flags = get_u32(p + 3);
    cp.min = get_f32(p + 7);
    cp.max = get_f32(p + 11);
    cp.default_value = get_f32(p + 15);
    cp.step = get_f32(p + 19);
    // current value at 23, group at 27 (ignored here).
    if (value_tag_size(cp.tag) == 0) {
        ++counters_.parse_errors;
        return;
    }
    caps_.params[caps_received_count_++] = cp;
}

void VoxLinkClient::handle_caps_end(const Frame &frame, uint32_t now_ms) {
    if (state_ != State::CapsReceiving) return;
    if (frame.payload_len < 2) {
        ++counters_.parse_errors;
        return;
    }
    const uint16_t count = get_u16(frame.payload);
    if (count != caps_received_count_) {
        ++counters_.parse_errors;
        return;
    }
    caps_receiving_ = false;
    caps_received_ = true;
    uint8_t req = 0;
    send_simple(MsgType::GetState, 0, &req, 0);
    state_ = State::StateReceiving;
    deadline_ms_ = now_ms + kStateTimeoutMs;
}

void VoxLinkClient::handle_state_begin(const Frame &frame) {
    // Accept a snapshot during boot and for later resync while Active.
    if (state_ != State::StateReceiving && state_ != State::Active) return;
    if (frame.payload_len < 6) {
        ++counters_.parse_errors;
        return;
    }
    snap_revision_ = get_u32(frame.payload);
    snap_expected_ = get_u16(frame.payload + 4);
    snap_count_ = 0;
    in_snapshot_ = true;
}

void VoxLinkClient::handle_state_param(const Frame &frame) {
    if (!in_snapshot_) return;
    if (frame.payload_len < 3) {
        ++counters_.parse_errors;
        return;
    }
    const uint16_t id = get_u16(frame.payload);
    const ValueTag tag = static_cast<ValueTag>(frame.payload[2]);
    float value = 0.0f;
    size_t consumed = 0;
    if (!decode_value(tag, frame.payload + 3, frame.payload_len - 3, &value,
                      &consumed)) {
        ++counters_.parse_errors;
        return;
    }
    if (snap_count_ >= kMaxSnapshotParams) {
        ++counters_.parse_errors;
        return;
    }
    snap_[snap_count_].id = id;
    snap_[snap_count_].tag = tag;
    snap_[snap_count_].value = value;
    ++snap_count_;
}

void VoxLinkClient::handle_state_end(const Frame &frame, uint32_t now_ms) {
    if (!in_snapshot_) return;
    if (frame.payload_len < 4) {
        ++counters_.parse_errors;
        return;
    }
    const uint32_t end_rev = get_u32(frame.payload);
    if (end_rev != snap_revision_ || snap_count_ != snap_expected_) {
        ++counters_.parse_errors;
        in_snapshot_ = false;
        return;
    }
    // Coherent snapshot: apply every entry, then mark Active.
    for (uint16_t i = 0; i < snap_count_; ++i) {
        push_event(EventType::ParamAuthoritative, snap_[i].id, snap_[i].value);
    }
    revision_ = snap_revision_;
    have_revision_ = true;
    in_snapshot_ = false;
    state_ = State::Active;
    backoff_index_ = 0;
    backoff_ms_ = 0;
    ++counters_.reconnects;
    last_heartbeat_ms_ = now_ms;
    last_flush_ms_ = now_ms;
    push_event(EventType::LinkActive, 0, 0.0f);
}

void VoxLinkClient::handle_param_changed(const Frame &frame) {
    if (frame.payload_len < 8) {
        ++counters_.parse_errors;
        return;
    }
    const uint16_t id = get_u16(frame.payload);
    const uint32_t rev = get_u32(frame.payload + 2);
    const ValueTag tag = static_cast<ValueTag>(frame.payload[7]);
    float value = 0.0f;
    size_t consumed = 0;
    if (!decode_value(tag, frame.payload + 8, frame.payload_len - 8, &value,
                      &consumed)) {
        ++counters_.parse_errors;
        return;
    }
    Pending *p = pending_find_seq(frame.seq);
    const bool direct = (p != nullptr && p->type == MsgType::SetParam && p->id == id);
    const bool newer = !have_revision_ || (int32_t)(rev - revision_) > 0;
    if (newer || direct) {
        if (newer) {
            revision_ = rev;
            have_revision_ = true;
        }
        push_event(EventType::ParamAuthoritative, id, value);
    }
    if (p != nullptr && p->id == id) pending_remove(p);
}

void VoxLinkClient::handle_param_value(const Frame &frame) {
    if (frame.payload_len < 7) {
        ++counters_.parse_errors;
        return;
    }
    const uint16_t id = get_u16(frame.payload);
    const uint32_t rev = get_u32(frame.payload + 2);
    const ValueTag tag = static_cast<ValueTag>(frame.payload[6]);
    float value = 0.0f;
    size_t consumed = 0;
    if (!decode_value(tag, frame.payload + 7, frame.payload_len - 7, &value,
                      &consumed)) {
        ++counters_.parse_errors;
        return;
    }
    const bool newer = !have_revision_ || (int32_t)(rev - revision_) > 0;
    if (newer) {
        revision_ = rev;
        have_revision_ = true;
    }
    push_event(EventType::ParamAuthoritative, id, value);
    Pending *p = pending_find_seq(frame.seq);
    if (p != nullptr && p->id == id) pending_remove(p);
}

void VoxLinkClient::handle_ack_nack(const Frame &frame, bool nack,
                                    uint32_t now_ms) {
    (void)now_ms;
    if (frame.payload_len < 4) {
        ++counters_.parse_errors;
        return;
    }
    const uint8_t ref_seq = frame.payload[1];
    const Code code = static_cast<Code>(get_u16(frame.payload + 2));
    Pending *p = pending_find_seq(ref_seq);
    if (!nack) {
        if (p != nullptr) pending_remove(p);
        return;
    }
    ++counters_.nacks;
    if (code == Code::QueueFull) ++counters_.queue_full;
    if (p != nullptr) {
        if (code == Code::QueueFull) {
            // Keep the latest desired value and resend next window.
            Coalesce *c = coalesce_find(p->id);
            if (c != nullptr) c->dirty = true;
        } else {
            push_event(EventType::ParamRevert, p->id, 0.0f);
        }
        pending_remove(p);
    }
}

void VoxLinkClient::push_event(EventType type, uint16_t id, float value) {
    const uint8_t next = static_cast<uint8_t>((ev_head_ + 1) % kEventCapacity);
    if (next == ev_tail_) return; // drop when full
    events_[ev_head_].type = type;
    events_[ev_head_].id = id;
    events_[ev_head_].value = value;
    ev_head_ = next;
}

} // namespace voxlink
