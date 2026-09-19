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
    counters_ = Counters{};
    state_ = State::Disconnected;
    caps_ = CapsSnapshot{};
    hello_ = HelloInfo{};
    tx_head_ = tx_tail_ = 0;
    ev_head_ = ev_tail_ = 0;
    for (auto &p : pending_) p = Pending{};
    for (auto &c : coalesce_) c = Coalesce{};
    for (auto &r : retries_) r = Retry{};
    next_seq_ = 0;
    last_rx_ms_ = now_ms;
    last_heartbeat_ms_ = now_ms;
    last_flush_ms_ = now_ms;
    revision_ = 0;
    have_revision_ = false;
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
    // Drop any stale capability inventory so a reconnect never uses old caps.
    caps_ = CapsSnapshot{};
    hello_ = HelloInfo{};
    caps_receiving_ = false;
    caps_received_ = false;
    caps_received_count_ = 0;
    for (auto &p : pending_) p = Pending{};
    for (auto &c : coalesce_) {
        c.dirty = false;
        c.retries = 0;
    }
    for (auto &r : retries_) r = Retry{};

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
    for (auto &r : retries_) r = Retry{};

    static const uint16_t kBackoff[] = {250, 500, 1000, 2000, 2000};
    backoff_ms_ = kBackoff[backoff_index_];
    if (backoff_index_ < 4) ++backoff_index_;
    next_attempt_ms_ = now_ms + backoff_ms_;
    // Link only: never mutate DSP/UI parameter state here.
    push_event(EventType::LinkDown, 0, 0.0f);
}

void VoxLinkClient::feed(const uint8_t *data, size_t len, uint32_t now_ms) {
    if (data == nullptr || len == 0) return;
    counters_.bytes_rx += static_cast<uint32_t>(len);
    // Liveness is refreshed only for a valid frame (inside handle_frame), never
    // by raw bytes, garbage or bad-CRC frames.
    parser_.feed(data, len, [this, now_ms](const Frame &f) {
        handle_frame(f, now_ms);
    });
    const ParserCounters &pc = parser_.counters();
    counters_.frames_rx = static_cast<uint32_t>(pc.frames);
    counters_.crc_errors = static_cast<uint32_t>(pc.crc_errors);
    counters_.length_errors = static_cast<uint32_t>(pc.length_errors);
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
        if ((int32_t)(now_ms - last_heartbeat_ms_) >=
            (int32_t)kHeartbeatIntervalMs) {
            send_heartbeat(now_ms);
        }
        if ((int32_t)(now_ms - last_flush_ms_) >= (int32_t)kCoalesceIntervalMs) {
            flush_coalesced(now_ms);
        }
        retry_tick(now_ms);
        expire_pending(now_ms);
        return;
    }
    if (state_ == State::Disconnected) {
        if ((int32_t)(now_ms - next_attempt_ms_) >= 0) start_handshake(now_ms);
        return;
    }
    if ((int32_t)(now_ms - deadline_ms_) >= 0) disconnect(now_ms, true);
}

// ---------------------------------------------------------------------------
// TX ring (head == tail means empty; usable capacity is kTxCapacity - 1)
// ---------------------------------------------------------------------------
size_t VoxLinkClient::tx_used() const {
    return (tx_head_ + kTxCapacity - tx_tail_) % kTxCapacity;
}
size_t VoxLinkClient::tx_free() const { return kTxCapacity - 1 - tx_used(); }

bool VoxLinkClient::tx_push(const uint8_t *data, size_t len) {
    if (data == nullptr) return false;
    if (len > tx_free()) {
        ++counters_.tx_drops;
        return false;
    }
    for (size_t i = 0; i < len; ++i) {
        tx_[tx_head_] = data[i];
        tx_head_ = (tx_head_ + 1) % kTxCapacity;
    }
    return true;
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

// ---------------------------------------------------------------------------
// Events
// ---------------------------------------------------------------------------
void VoxLinkClient::push_event(EventType type, uint16_t id, float value) {
    const uint8_t next = static_cast<uint8_t>((ev_head_ + 1) % kEventCapacity);
    if (next == ev_tail_) {
        ++counters_.event_drops; // never overwrite an older event silently
        return;
    }
    events_[ev_head_].type = type;
    events_[ev_head_].id = id;
    events_[ev_head_].value = value;
    ev_head_ = next;
}

bool VoxLinkClient::take_event(Event *out) {
    if (out == nullptr || ev_head_ == ev_tail_) return false;
    *out = events_[ev_tail_];
    ev_tail_ = static_cast<uint8_t>((ev_tail_ + 1) % kEventCapacity);
    return true;
}

// ---------------------------------------------------------------------------
// Pending / retry bookkeeping
// ---------------------------------------------------------------------------
VoxLinkClient::Pending *VoxLinkClient::pending_find(MsgType type, uint8_t seq) {
    for (auto &p : pending_)
        if (p.used && p.type == type && p.seq == seq) return &p;
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

VoxLinkClient::Pending *VoxLinkClient::allocate_pending(MsgType type, uint16_t id,
                                                        ValueTag tag, float value,
                                                        uint8_t retries,
                                                        uint32_t now_ms) {
    Pending *slot = nullptr;
    for (auto &p : pending_) {
        if (!p.used) {
            slot = &p;
            break;
        }
    }
    if (slot == nullptr) {
        ++counters_.pending_full;
        return nullptr;
    }
    uint8_t seq = 0;
    bool found = false;
    for (int attempt = 0; attempt < 256; ++attempt) {
        seq = next_seq_++;
        bool in_use = false;
        for (const auto &p : pending_)
            if (p.used && p.seq == seq) {
                in_use = true;
                break;
            }
        if (!in_use) {
            found = true;
            break;
        }
    }
    if (!found) {
        ++counters_.pending_full;
        return nullptr;
    }
    slot->used = true;
    slot->acked = false;
    slot->seq = seq;
    slot->type = type;
    slot->id = id;
    slot->tag = tag;
    slot->value = value;
    slot->sent_ms = now_ms;
    slot->retries = retries;
    return slot;
}

VoxLinkClient::Coalesce *VoxLinkClient::coalesce_find(uint16_t id) {
    for (auto &c : coalesce_)
        if (c.used && c.id == id) return &c;
    return nullptr;
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
            c.retries = 0;
            return;
        }
    }
}

VoxLinkClient::Retry *VoxLinkClient::retry_find(uint16_t id) {
    for (auto &r : retries_)
        if (r.used && r.id == id) return &r;
    return nullptr;
}

VoxLinkClient::Retry *VoxLinkClient::retry_alloc() {
    for (auto &r : retries_)
        if (!r.used) return &r;
    return nullptr;
}

VoxLinkClient::Retry *VoxLinkClient::retry_put(uint16_t id, ValueTag tag,
                                               float value, uint32_t now_ms) {
    Retry *slot = retry_find(id);
    if (slot == nullptr) {
        slot = retry_alloc();
        if (slot == nullptr) return nullptr;
        slot->used = true;
        slot->id = id;
        slot->retries = 0;
    }
    slot->tag = tag;
    slot->value = value; // latest wins
    ++slot->retries;
    slot->next_ms = now_ms + kRetryBackoffMs;
    return slot;
}

// ---------------------------------------------------------------------------
// Sending
// ---------------------------------------------------------------------------
bool VoxLinkClient::send_simple(MsgType type, uint8_t seq,
                                const uint8_t *payload, size_t len) {
    uint8_t scratch[kMaxFrame];
    const size_t n = encode_frame(type, 0, seq, payload, len, scratch,
                                  sizeof(scratch));
    if (n == 0) return false;
    if (!tx_push(scratch, n)) return false;
    ++counters_.frames_tx;
    counters_.bytes_tx += static_cast<uint32_t>(n);
    return true;
}

bool VoxLinkClient::send_set_now(uint16_t id, ValueTag tag, float value,
                                 uint8_t retries, uint32_t now_ms) {
    const size_t size = value_tag_size(tag);
    if (size == 0) return false;
    Pending *p = allocate_pending(MsgType::SetParam, id, tag, value, retries,
                                  now_ms);
    if (p == nullptr) return false; // no untracked requests
    uint8_t body[8];
    put_u16(body, id);
    body[2] = static_cast<uint8_t>(tag);
    const size_t vn = encode_value(tag, value, body + 3, sizeof(body) - 3);
    if (vn == 0 || !send_simple(MsgType::SetParam, p->seq, body, 3 + vn)) {
        pending_remove(p); // never leave a ghost request for timeout
        return false;
    }
    return true;
}

void VoxLinkClient::send_heartbeat(uint32_t now_ms) {
    last_heartbeat_ms_ = now_ms;
    send_simple(MsgType::Heartbeat, 0, nullptr, 0);
}

void VoxLinkClient::flush_coalesced(uint32_t now_ms) {
    last_flush_ms_ = now_ms;
    for (auto &c : coalesce_) {
        if (!c.used || !c.dirty) continue;
        CapsParam *cp = caps_find(c.id);
        if (cp == nullptr || cp->tag != ValueTag::Float32) {
            c.dirty = false;
            continue;
        }
        if (send_set_now(c.id, cp->tag, c.value, c.retries, now_ms)) {
            c.dirty = false;
        }
    }
}

void VoxLinkClient::retry_tick(uint32_t now_ms) {
    for (auto &r : retries_) {
        if (!r.used) continue;
        if ((int32_t)(now_ms - r.next_ms) < 0) continue;
        if (r.retries > kMaxQueueFullRetries) {
            push_event(EventType::ParamRevert, r.id, 0.0f);
            r = Retry{};
            ++counters_.queue_full_exhausted;
            continue;
        }
        if (send_set_now(r.id, r.tag, r.value, r.retries, now_ms)) {
            r = Retry{};
        } else {
            ++r.retries;
            r.next_ms = now_ms + kRetryBackoffMs;
            ++counters_.queue_full_retries;
            if (r.retries > kMaxQueueFullRetries) {
                push_event(EventType::ParamRevert, r.id, 0.0f);
                r = Retry{};
                ++counters_.queue_full_exhausted;
            }
        }
    }
}

void VoxLinkClient::expire_pending(uint32_t now_ms) {
    for (auto &p : pending_) {
        if (!p.used) continue;
        if ((int32_t)(now_ms - p.sent_ms) > (int32_t)kRequestTimeoutMs) {
            ++counters_.timeouts;
            if (p.type == MsgType::SetParam)
                push_event(EventType::ParamRevert, p.id, 0.0f);
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
    // If a QUEUE_FULL retry is pending for this id, update its latest value.
    Retry *r = retry_find(id);
    if (r != nullptr) {
        r->tag = cp->tag;
        r->value = value;
        return true;
    }
    if (cp->tag == ValueTag::Float32) {
        coalesce_put(id, value);
        return true;
    }
    return send_set_now(id, cp->tag, value, 0, now_ms);
}

bool VoxLinkClient::get_parameter(uint16_t id, uint32_t now_ms) {
    if (state_ != State::Active) return false;
    if (caps_find(id) == nullptr) return false;
    Pending *p = allocate_pending(MsgType::GetParam, id, ValueTag::Float32, 0.0f,
                                  0, now_ms);
    if (p == nullptr) return false;
    uint8_t body[2];
    put_u16(body, id);
    if (!send_simple(MsgType::GetParam, p->seq, body, sizeof(body))) {
        pending_remove(p);
        return false;
    }
    return true;
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
    // Before any complete CAPS snapshot: offline/local development is allowed,
    // but during an active handshake we must not pretend to know capabilities.
    if (!caps_.valid) return !handshake_in_progress();
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
    // A valid frame is the only thing that keeps the link alive.
    last_rx_ms_ = now_ms;
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
    hello_.valid = true;
    hello_.version = p[0];
    hello_.caps = get_u32(p + 1);
    hello_.sample_rate = get_u32(p + 5);
    hello_.block_size = get_u16(p + 9);
    hello_.harmony_voices = p[11];
    // Parameter inventory is NOT valid yet; wait for CAPS_END.
    caps_.valid = false;
    send_simple(MsgType::CapsRequest, 0, nullptr, 0);
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
    const uint16_t count = get_u16(p + 5);
    caps_receiving_ = false;
    caps_received_ = false;
    caps_received_count_ = 0;
    caps_.valid = false;
    for (auto &param : caps_.params) param = CapsParam{};
    if (count > kMaxCapsParams) {
        // Reject an oversized inventory instead of silently truncating it.
        ++counters_.caps_overflow;
        ++counters_.parse_errors;
        return;
    }
    caps_.version = p[0];
    caps_.caps = get_u32(p + 1);
    caps_.param_count = count;
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
    if (value_tag_size(cp.tag) == 0) {
        ++counters_.parse_errors;
        return;
    }
    caps_.params[caps_received_count_++] = cp;
}

void VoxLinkClient::handle_caps_end(const Frame &frame, uint32_t now_ms) {
    if (state_ != State::CapsReceiving || !caps_receiving_) return;
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
    // Snapshot is now a complete, authoritative inventory.
    caps_.version = hello_.version;
    caps_.caps = hello_.caps;
    caps_.sample_rate = hello_.sample_rate;
    caps_.block_size = hello_.block_size;
    caps_.harmony_voices = hello_.harmony_voices;
    caps_.valid = true;
    send_simple(MsgType::GetState, 0, nullptr, 0);
    state_ = State::StateReceiving;
    deadline_ms_ = now_ms + kStateTimeoutMs;
}

void VoxLinkClient::handle_state_begin(const Frame &frame) {
    if (state_ != State::StateReceiving && state_ != State::Active) return;
    if (frame.payload_len < 6) {
        ++counters_.parse_errors;
        return;
    }
    const uint16_t count = get_u16(frame.payload + 4);
    if (count > kMaxSnapshotParams) {
        ++counters_.snapshot_overflow;
        ++counters_.parse_errors;
        in_snapshot_ = false;
        return;
    }
    snap_revision_ = get_u32(frame.payload);
    snap_expected_ = count;
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
    // Coherent snapshot: emit values first, LinkActive last.
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
    Pending *p = pending_find(MsgType::SetParam, frame.seq);
    const bool direct = (p != nullptr && p->id == id);
    const bool newer = !have_revision_ || (int32_t)(rev - revision_) > 0;
    if (newer) {
        revision_ = rev;
        have_revision_ = true;
        push_event(EventType::ParamAuthoritative, id, value);
    } else if (direct) {
        // Direct response to our SET: accept even at the same (no-op) revision.
        push_event(EventType::ParamAuthoritative, id, value);
    }
    // else: stale asynchronous notification -> ignore value.
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
    Pending *p = pending_find(MsgType::GetParam, frame.seq);
    const bool direct = (p != nullptr && p->id == id);
    const bool newer = !have_revision_ || (int32_t)(rev - revision_) > 0;
    const bool same = have_revision_ && rev == revision_;
    if (newer) {
        revision_ = rev;
        have_revision_ = true;
        push_event(EventType::ParamAuthoritative, id, value);
    } else if (direct && same) {
        push_event(EventType::ParamAuthoritative, id, value);
    }
    // Stale (older) value: do not regress state.
    if (p != nullptr && p->id == id) pending_remove(p);
}

void VoxLinkClient::handle_ack_nack(const Frame &frame, bool nack,
                                    uint32_t now_ms) {
    if (frame.payload_len < 4) {
        ++counters_.parse_errors;
        return;
    }
    const MsgType ref_type = static_cast<MsgType>(frame.payload[0]);
    const uint8_t ref_seq = frame.payload[1];
    const Code code = static_cast<Code>(get_u16(frame.payload + 2));

    // Matching requires BOTH reference type and sequence so a heartbeat ACK
    // (seq 0) can never touch a SET/GET request that also used seq 0.
    Pending *p = pending_find(ref_type, ref_seq);

    if (!nack) {
        // ACK only means protocol-level acceptance. For SET_PARAM the request
        // stays pending until PARAM_CHANGED (or NACK/timeout).
        if (p != nullptr && p->type == MsgType::SetParam) p->acked = true;
        return;
    }

    ++counters_.nacks;
    if (p == nullptr) return;

    if (code == Code::QueueFull) {
        ++counters_.queue_full;
        if (p->tag == ValueTag::Float32) {
            Coalesce *c = coalesce_find(p->id);
            if (c != nullptr) {
                c->dirty = true; // keep latest; retry next window
                ++c->retries;
                if (c->retries > kMaxQueueFullRetries) {
                    push_event(EventType::ParamRevert, p->id, 0.0f);
                    *c = Coalesce{};
                    ++counters_.queue_full_exhausted;
                }
            } else {
                Retry *r = retry_put(p->id, p->tag, p->value, now_ms);
                if (r != nullptr && r->retries > kMaxQueueFullRetries) {
                    push_event(EventType::ParamRevert, p->id, 0.0f);
                    *r = Retry{};
                    ++counters_.queue_full_exhausted;
                }
            }
        } else {
            const uint8_t next = static_cast<uint8_t>(p->retries + 1);
            if (next > kMaxQueueFullRetries) {
                push_event(EventType::ParamRevert, p->id, 0.0f);
                ++counters_.queue_full_exhausted;
            } else {
                Retry *r = retry_put(p->id, p->tag, p->value, now_ms);
                if (r == nullptr) {
                    // Retry table full: treat as definitive failure.
                    push_event(EventType::ParamRevert, p->id, 0.0f);
                    ++counters_.queue_full_exhausted;
                } else {
                    r->retries = next;
                }
            }
        }
        pending_remove(p);
        return;
    }

    // Definitive failure: roll back and finalize.
    push_event(EventType::ParamRevert, p->id, 0.0f);
    pending_remove(p);
}

} // namespace voxlink
