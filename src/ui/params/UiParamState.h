#ifndef UI_PARAM_STATE_H
#define UI_PARAM_STATE_H

// Canonical controller parameter state.
//
// This is the single value authority used by the UI. It is keyed by the wire
// (VoxLink) parameter ID and sized by the canonical ParameterRegistry, so it can
// represent every parameter the ESP32-P4 exposes. It is intentionally free of
// LVGL/Arduino and of any UI layout metadata: min/max/default/type come straight
// from ParameterRegistry (the one source of truth), while UiParamModel adds only
// presentation metadata on top.
//
// Authority model (unchanged from M6):
//   LocalDefault  -> boot / registry default, never P4-confirmed
//   LocalPending  -> optimistic local edit awaiting P4 confirmation
//   Authoritative -> last value confirmed/announced by the P4

#include <cstddef>
#include <cstdint>

#include "model/ParameterRegistry.h"

enum class UiValueAuthority : uint8_t {
    LocalDefault = 0,
    LocalPending = 1,
    Authoritative = 2
};

class UiParamState {
public:
    static constexpr size_t kCount = VOXP4_PARAM_COUNT;

    UiParamState() { resetToDefaults(); }

    // Fill every canonical parameter with its registry default.
    void resetToDefaults();

    // Current (possibly optimistic) value. Returns 0 when the wire id is not in
    // the registry.
    float get(uint16_t wireId) const;

    // Last value confirmed by the P4 (falls back to the current value when the
    // parameter has never been confirmed).
    float authoritative(uint16_t wireId) const;

    bool isValid(uint16_t wireId) const;
    UiValueAuthority authority(uint16_t wireId) const;

    // Optimistic local edit. Clamps/rounds to the canonical registry range/type
    // and marks the value LocalPending.
    void setLocal(uint16_t wireId, float value);

    // Adopt a value confirmed/announced by the P4. Clamps to the registry range
    // and marks the value Authoritative.
    void applyAuthoritative(uint16_t wireId, float value);

    // Roll a pending value back to the last authoritative value.
    void revert(uint16_t wireId);

    // Raw dense-index access for summaries/serialization. Index is the
    // ParameterRegistry dense index (0..70).
    const float* raw() const { return values_; }

private:
    static int dense(uint16_t wireId);

    float values_[kCount];
    float authoritative_[kCount];
    bool valid_[kCount];
    UiValueAuthority authority_[kCount];
};

#endif // UI_PARAM_STATE_H
