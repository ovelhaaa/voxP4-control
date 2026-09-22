#ifndef UI_PARAM_MODEL_H
#define UI_PARAM_MODEL_H

// Data-driven parameter model for the effect editor.
//
// This file is intentionally free of LVGL and Arduino dependencies so the pure
// logic (descriptors, clamping, formatting, summaries) can be unit-tested on a
// host and reused by the future VoxLink layer without redesign.
//
// The controller keeps its own logical IDs (UiParamId). Those IDs are NOT the
// backend's VocalFxParameter ordinals. `voxlinkId` records the intended mapping
// to the ESP32-P4 VoxLink parameter IDs and will be used by the transport layer
// in a later milestone. No voxP4 headers are included here on purpose.

#include <cstddef>
#include <cstdint>

// Versioned VoxLink ABI constants (parameter IDs, count, golden vectors).
// Synchronized from the ESP32-P4 firmware; see src/voxlink/VoxLinkContract.h.
#include "voxlink/VoxLinkContract.h"

enum class UiEffectId : uint8_t {
    Harmony = 0,
    Reverb = 1,
    Delay = 2,
    Limiter = 3,
    Modulation = 4,
    Count
};

constexpr size_t kUiEffectCount = static_cast<size_t>(UiEffectId::Count);

// Logical parameter IDs. Order is local to the controller and must not be used
// as a wire ordinal.
enum class UiParamId : uint16_t {
    HarmonyMode = 0,
    HarmonyInterval,
    HarmonyDegree,
    HarmonyKey,
    HarmonyScale,
    HarmonyNonScalePolicy,
    HarmonyVoiceLeading,
    HarmonyLevel,
    HarmonyPan,
    HarmonySmoothingMs,
    HarmonyAttackMs,
    HarmonyReleaseMs,
    FormantEnabled,
    FormantAmount,

    ReverbWet,
    ReverbDecaySeconds,
    ReverbDamping,

    DelayLeftMs,
    DelayRightMs,
    DelayFeedback,
    DelayWet,
    DelayDry,
    DelayFeedbackLowpassHz,

    LimiterThresholdDb,

    ModulationMode,
    ModulationMix,
    ModulationRateHz,
    ModulationDepthMs,
    ModulationWidth,
    MicroshiftLeftCents,
    MicroshiftRightCents,
    MicroshiftWindowMs,

    Count
};

constexpr size_t kUiParamCount = static_cast<size_t>(UiParamId::Count);

enum class UiControlType : uint8_t {
    Slider,
    Toggle,
    Segmented,
    Stepper
};

enum class UiValueFormat : uint8_t {
    Percent,        // 0..1        -> "18%"
    PercentSigned,  // -1..1       -> "+25%" / "-25%"
    Milliseconds,   // ms
    Seconds,        // "2.0 s"
    Decibels,       // "-3.0 dB"
    Semitones,      // "+4 st"
    Degrees,        // "+2 deg"
    Pan,            // "L 25" / "C" / "R 25"
    Hertz,          // "6.0 kHz"
    EnumLabel,      // options[(int)value]
    Cents           // "-7 c" / "+9 c"
};

struct UiParamDescriptor {
    UiParamId id;
    const char* label;
    const char* section;
    UiControlType type;
    UiValueFormat format;
    // uiStep is the UI/touch ergonomics resolution, NOT the wire step. The
    // VoxLink schema (integration/voxlink_params.json) owns the authoritative
    // wire step/range; the UI step may be coarser and stays within the wire
    // range. See docs/m5_1_voxlink_contract.md.
    float minValue;
    float maxValue;
    float uiStep;
    float defaultValue;
    const char* const* options; // enum labels for Segmented/Stepper/EnumLabel
    uint8_t optionCount;
    uint16_t voxlinkId;        // future ESP32-P4 VoxLink parameter ID (0 = none)
    uint8_t harmonyModeMask;   // bit(mode) for mode-gated params; 0 = effect-agnostic
    bool wraparound;           // stepper wrap (KEY / SCALE)
};

// Enum label tables.
extern const char* const kUiHarmonyModeLabels[3];
extern const char* const kUiHarmonyKeyLabels[12];
extern const char* const kUiHarmonyScaleLabels[12];
extern const char* const kUiHarmonyScaleShortLabels[12];
extern const char* const kUiNonScalePolicyLabels[3];
extern const char* const kUiModulationModeLabels[4];

// Descriptor tables (static const, no heap).
extern const UiParamDescriptor kUiHarmonyDescriptors[];
extern const size_t kUiHarmonyDescriptorCount;
extern const UiParamDescriptor kUiReverbDescriptors[];
extern const size_t kUiReverbDescriptorCount;
extern const UiParamDescriptor kUiDelayDescriptors[];
extern const size_t kUiDelayDescriptorCount;
extern const UiParamDescriptor kUiLimiterDescriptors[];
extern const size_t kUiLimiterDescriptorCount;
extern const UiParamDescriptor kUiModulationDescriptors[];
extern const size_t kUiModulationDescriptorCount;

// Descriptor array for an effect.
const UiParamDescriptor* ui_effect_descriptors(UiEffectId effect, size_t* count);

// Harmony mode gating. For non-harmony descriptors this is always true.
bool ui_descriptor_applies(const UiParamDescriptor& descriptor, uint8_t harmonyMode);

// Lookup across all effects. Returns nullptr when unknown.
const UiParamDescriptor* ui_param_descriptor(UiParamId id);

// Owning effect of a parameter (UiEffectId::Count when unknown).
UiEffectId ui_effect_of(UiParamId id);

// Wire ABI mapping. ui_param_voxlink_id returns 0 when a parameter is not bound
// to the VoxLink schema. ui_effect_enable_voxlink_id maps the four visual module
// enables to their backend parameter (LIMITER == harmony bus limiter).
uint16_t ui_param_voxlink_id(UiParamId id);
uint16_t ui_effect_enable_voxlink_id(UiEffectId effect);

// Reverse lookup used by the VoxLink client glue to map authoritative wire IDs
// back to UI parameters / effect enables.
bool ui_param_from_voxlink_id(uint16_t voxlinkId, UiParamId* out);
bool ui_effect_from_enable_voxlink_id(uint16_t id, UiEffectId* out);

// Local enable defaults mirrored from the VoxLink registry (*.enable default).
// Used only until M6 GET_STATE provides the authoritative snapshot.
bool ui_effect_default_enabled(UiEffectId effect);

// Clamp + quantize to the descriptor range/step.
float ui_clamp_parameter(const UiParamDescriptor& descriptor, float value);

// Wrapped stepper index helper (used by KEY/SCALE steppers and tested).
int ui_step_index(int index, int delta, int count, bool wraparound);

// Format a value into a fixed buffer. No heap.
void ui_format_parameter_value(const UiParamDescriptor& descriptor, float value,
                               char* out, size_t size);

// Build the Performance/FX Chain summary for one effect from the parameter
// value array (indexed by UiParamId). Both screens share this single function.
void ui_build_effect_summary(UiEffectId effect, const float* values,
                             char* mainValue, size_t mainSize,
                             char* metadata, size_t metaSize);

#endif // UI_PARAM_MODEL_H
