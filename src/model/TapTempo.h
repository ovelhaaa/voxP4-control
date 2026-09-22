#ifndef TAP_TEMPO_H
#define TAP_TEMPO_H

// Pure Tap Tempo estimator (no Arduino/LVGL).
//
// The controller owns tap-tempo calculation: each footswitch tap is fed here,
// the estimator rejects clearly invalid intervals, averages/medians the recent
// taps to avoid jitter, clamps to the canonical TempoBpm range (30..300) and the
// result is then sent to the P4 as an ordinary TempoBpm parameter. There is no
// special protocol; the P4 remains the authority.

#include <cstddef>
#include <cstdint>

class TapTempo {
public:
    // Canonical TempoBpm range mirrored from the VoxLink registry.
    static constexpr float kMinBpm = 30.0f;
    static constexpr float kMaxBpm = 300.0f;
    // A tap shorter than 200 ms (>300 BPM) or longer than 2000 ms (<30 BPM) is
    // rejected as an invalid interval.
    static constexpr uint32_t kMinIntervalMs = 200;
    static constexpr uint32_t kMaxIntervalMs = 2000;
    // Sequence is forgotten after this idle period.
    static constexpr uint32_t kDefaultTimeoutMs = 2000;
    static constexpr size_t kMaxTaps = 6;

    TapTempo() { reset(); }

    void reset();

    // Register a tap at now_ms. Returns true when a fresh BPM estimate is
    // available (written to *out_bpm); false while the sequence is still being
    // primed or when no interval in the window is valid.
    bool tap(uint32_t now_ms, float* out_bpm);

    bool hasEstimate() const { return has_estimate_; }
    float bpm() const { return bpm_; }

    void setTimeoutMs(uint32_t ms) { timeout_ms_ = ms; }
    uint32_t timeoutMs() const { return timeout_ms_; }

private:
    uint32_t taps_[kMaxTaps];
    size_t count_;
    float bpm_;
    bool has_estimate_;
    uint32_t timeout_ms_;
};

#endif // TAP_TEMPO_H
