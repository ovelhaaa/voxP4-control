#ifndef VOXLINK_PARSER_H
#define VOXLINK_PARSER_H

// Bounded, allocation-free streaming VoxLink parser for the CYD client. Accepts
// arbitrary byte chunks and recovers automatically from corruption.
#include "voxlink/VoxLinkCodec.h"
#include "voxlink/VoxLinkCrc.h"
#include <cstddef>
#include <cstdint>

namespace voxlink {

struct ParserCounters {
    uint64_t bytes = 0;
    uint64_t frames = 0;
    uint64_t crc_errors = 0;
    uint64_t length_errors = 0;
    uint64_t version_errors = 0;
    uint64_t resyncs = 0;
};

class Parser {
public:
    // Resets the framing state only. Counters are cumulative across reconnects
    // and cleared explicitly with reset_counters().
    void reset() {
        state_ = State::Sof0;
        header_pos_ = 0;
        payload_pos_ = 0;
        crc_pos_ = 0;
        payload_len_ = 0;
        working_ = Frame{};
    }

    void reset_counters() { counters_ = ParserCounters{}; }

    template <typename Emit>
    void feed(const uint8_t *data, size_t len, Emit &&emit) {
        if (data == nullptr) return;
        counters_.bytes += len;
        for (size_t i = 0; i < len; ++i) handle(data[i], emit);
    }

    const ParserCounters &counters() const { return counters_; }

private:
    enum class State : uint8_t { Sof0, Sof1, Header, Payload, Crc };

    void resync() {
        ++counters_.resyncs;
        state_ = State::Sof0;
        header_pos_ = 0;
        payload_pos_ = 0;
        crc_pos_ = 0;
        payload_len_ = 0;
    }

    template <typename Emit> void complete(Emit &emit) {
        const uint16_t expected =
            static_cast<uint16_t>(static_cast<uint16_t>(crc_[0]) |
                                  (static_cast<uint16_t>(crc_[1]) << 8));
        uint16_t actual = crc16_ccitt_false(header_, sizeof(header_));
        actual = crc_accumulate_(actual, working_.payload, working_.payload_len);
        if (expected != actual) {
            ++counters_.crc_errors;
            resync();
            return;
        }
        ++counters_.frames;
        emit(working_);
        state_ = State::Sof0;
        header_pos_ = 0;
        payload_pos_ = 0;
        crc_pos_ = 0;
        payload_len_ = 0;
    }

    static uint16_t crc_accumulate_(uint16_t crc, const uint8_t *data, size_t len) {
        for (size_t i = 0; i < len; ++i) {
            crc ^= static_cast<uint16_t>(data[i]) << 8;
            for (int bit = 0; bit < 8; ++bit)
                crc = (crc & 0x8000u) ? static_cast<uint16_t>((crc << 1) ^ 0x1021u)
                                      : static_cast<uint16_t>(crc << 1);
        }
        return crc;
    }

    template <typename Emit> void handle(uint8_t b, Emit &emit) {
        switch (state_) {
            case State::Sof0:
                if (b == kSof0) state_ = State::Sof1;
                break;
            case State::Sof1:
                if (b == kSof1) state_ = State::Header;
                else if (b == kSof0) state_ = State::Sof1;
                else state_ = State::Sof0;
                break;
            case State::Header:
                header_[header_pos_++] = b;
                if (header_pos_ < sizeof(header_)) break;
                working_.version = header_[0];
                working_.type = static_cast<MsgType>(header_[1]);
                working_.flags = header_[2];
                working_.seq = header_[3];
                payload_len_ = get_u16(header_ + 4);
                working_.payload_len = payload_len_;
                if ((working_.version >> 4) != kVersionMajor) {
                    ++counters_.version_errors;
                    resync();
                    break;
                }
                if (payload_len_ > kMaxPayload) {
                    ++counters_.length_errors;
                    resync();
                    break;
                }
                payload_pos_ = 0;
                crc_pos_ = 0;
                state_ = (payload_len_ == 0) ? State::Crc : State::Payload;
                break;
            case State::Payload:
                working_.payload[payload_pos_++] = b;
                if (payload_pos_ >= payload_len_) state_ = State::Crc;
                break;
            case State::Crc:
                crc_[crc_pos_++] = b;
                if (crc_pos_ >= sizeof(crc_)) complete(emit);
                break;
        }
    }

    State state_ = State::Sof0;
    uint8_t header_[6] = {0};
    size_t header_pos_ = 0;
    size_t payload_pos_ = 0;
    uint8_t crc_[2] = {0};
    size_t crc_pos_ = 0;
    uint16_t payload_len_ = 0;
    Frame working_{};
    ParserCounters counters_{};
};

} // namespace voxlink

#endif // VOXLINK_PARSER_H
