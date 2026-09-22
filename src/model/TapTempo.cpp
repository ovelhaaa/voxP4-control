#include "model/TapTempo.h"

namespace {

uint32_t median_in_place(uint32_t* values, size_t count) {
    // Small fixed array: insertion sort.
    for (size_t i = 1; i < count; ++i) {
        const uint32_t key = values[i];
        size_t j = i;
        while (j > 0 && values[j - 1] > key) {
            values[j] = values[j - 1];
            --j;
        }
        values[j] = key;
    }
    return values[count / 2];
}

} // namespace

void TapTempo::reset() {
    count_ = 0;
    bpm_ = 0.0f;
    has_estimate_ = false;
    timeout_ms_ = kDefaultTimeoutMs;
    for (size_t i = 0; i < kMaxTaps; ++i) taps_[i] = 0;
}

bool TapTempo::tap(uint32_t now_ms, float* out_bpm) {
    if (count_ > 0 && (now_ms - taps_[count_ - 1]) > timeout_ms_) {
        count_ = 0;
        has_estimate_ = false;
    }

    if (count_ < kMaxTaps) {
        taps_[count_++] = now_ms;
    } else {
        for (size_t i = 1; i < kMaxTaps; ++i) taps_[i - 1] = taps_[i];
        taps_[kMaxTaps - 1] = now_ms;
    }

    if (count_ < 2) return false;

    uint32_t intervals[kMaxTaps];
    size_t valid = 0;
    for (size_t i = 1; i < count_; ++i) {
        const uint32_t dt = taps_[i] - taps_[i - 1];
        if (dt >= kMinIntervalMs && dt <= kMaxIntervalMs) {
            intervals[valid++] = dt;
        }
    }
    if (valid == 0) return false;

    const uint32_t med = median_in_place(intervals, valid);
    if (med == 0) return false;

    float bpm = 60000.0f / static_cast<float>(med);
    if (bpm < kMinBpm) bpm = kMinBpm;
    if (bpm > kMaxBpm) bpm = kMaxBpm;

    bpm_ = bpm;
    has_estimate_ = true;
    if (out_bpm != nullptr) *out_bpm = bpm;
    return true;
}
