#ifndef UI_PARAM_MODEL_H
#define UI_PARAM_MODEL_H

// Presentation model for the effect editors and the Performance/FX screens.
//
// IMPORTANT ARCHITECTURE RULE
// --------------------------
// ParameterRegistry (src/model/ParameterRegistry.{h,cpp}) is the ONE canonical
// source of truth for parameter ID, type, range and default. This file adds ONLY
// presentation metadata on top of it:
//   * short musical label
//   * visual section header
//   * widget type
//   * uiStep (touch ergonomics, NOT the wire step)
//   * value formatting
//   * enum labels
//   * Basic / Advanced / Global visibility
//   * conditional display rules (mode + switch gating)
//
// It deliberately does NOT store min/max/default. Those are read from the
// registry through ui_param_min()/ui_param_max()/ui_param_default(). The native
// test suite asserts that every canonical parameter has exactly one UI
// placement, so adding a parameter to the VoxLink contract without a UI home
// fails the build.
//
// This header is free of LVGL/Arduino so it can be unit-tested on a host.

#include <cstddef>
#include <cstdint>

#include "model/ParameterRegistry.h"
#include "ui/params/UiParamState.h"
#include "voxlink/VoxLinkContract.h"

// Signal-chain order used by the FX rack and the Performance indicators.
enum class UiEffectId : uint8_t {
    Gate = 0,
    Compressor,
    Harmony,
    Drive,
    Modulation,
    Delay,
    Reverb,
    Count
};

constexpr size_t kUiEffectCount = static_cast<size_t>(UiEffectId::Count);

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
    Cents,          // "-7 c" / "+9 c"
    Ratio,          // "3:1"
    Bpm,            // "120 BPM"
    MidiNote,       // "C4"
    CeilingDb       // amplitude 0..1 -> "-0.4 dB"
};

// Runtime generation position, used both for area layout and for reporting.
enum class UiVisibility : uint8_t {
    Basic = 0,
    Advanced = 1,
    Global = 2
};

// Conditional display rule for a control.
enum class UiCondition : uint8_t {
    Always = 0,
    BoolTrue = 1,  // visible when state.get(conditionWireId) >= 0.5
    BoolFalse = 2  // visible when state.get(conditionWireId) <  0.5
};

struct UiParamMeta {
    uint16_t wireId;
    const char* label;
    const char* section;
    UiControlType type;
    UiValueFormat format;
    float uiStep;
    const char* const* options;  // enum labels for Segmented/Stepper/EnumLabel
    uint8_t optionCount;
    UiVisibility visibility;
    UiCondition condition;
    uint16_t conditionWireId;  // controlling switch for Bool* conditions
    uint8_t modeMask;          // bit(mode); 0 = mode-agnostic
    bool wraparound;           // stepper wrap (KEY / SCALE)
};

// Enum label tables.
extern const char* const kUiHarmonyModeLabels[3];
extern const char* const kUiHarmonyKeyLabels[12];
extern const char* const kUiHarmonyScaleLabels[12];
extern const char* const kUiHarmonyScaleShortLabels[12];
extern const char* const kUiNonScalePolicyLabels[3];
extern const char* const kUiModulationModeLabels[4];
extern const char* const kUiSubdivisionLabels[13];
extern const char* const kUiDriveModeLabels[3];
extern const char* const kUiSpatialRoutingLabels[2];
extern const char* const kUiSpatialSourceLabels[3];

// Presentation metadata tables (static, no heap).
extern const UiParamMeta kUiGateDescriptors[];
extern const UiParamMeta kUiCompressorDescriptors[];
extern const UiParamMeta kUiHarmonyDescriptors[];
extern const UiParamMeta kUiDriveDescriptors[];
extern const UiParamMeta kUiModulationDescriptors[];
extern const UiParamMeta kUiDelayDescriptors[];
extern const UiParamMeta kUiReverbDescriptors[];
extern const UiParamMeta kUiMasterDescriptors[];

// Metadata array for one processing effect. Master/global parameters are not an
// effect; use ui_master_metadata() for those.
const UiParamMeta* ui_effect_metadata(UiEffectId effect, size_t* count);
const UiParamMeta* ui_master_metadata(size_t* count);

// Lookup across every effect and master. Returns nullptr when the wire id has no
// metadata (e.g. it is an effect enable, which is represented as a header
// toggle rather than a row).
const UiParamMeta* ui_param_meta(uint16_t wireId);

// Owning effect of a wire id (UiEffectId::Count for master/global/enable/unknown).
UiEffectId ui_effect_of_wire(uint16_t wireId);

// Effect enable mapping (the header ON/OFF, not a descriptor row).
uint16_t ui_effect_enable_wire(UiEffectId effect);
bool ui_effect_from_enable_wire(uint16_t wireId, UiEffectId* out);
bool ui_is_effect_enable(uint16_t wireId);

// Canonical display names.
const char* ui_effect_name(UiEffectId effect);
const char* ui_effect_short_name(UiEffectId effect);

// Registry-backed accessors. Never duplicate these values in UI metadata.
float ui_param_min(uint16_t wireId);
float ui_param_max(uint16_t wireId);
float ui_param_default(uint16_t wireId);

// Conditional display. `mode` is the owning effect's mode value (HarmonyMode for
// HARMONY, ChorusMode for MODULATION, otherwise 0).
bool ui_meta_applies(const UiParamMeta& meta, uint8_t mode,
                     const UiParamState& state);

// Clamp + quantize to the registry range and the descriptor uiStep.
float ui_clamp_parameter(const UiParamMeta& meta, float value);

// Wrapped stepper index helper (KEY / SCALE and subdivision steppers).
int ui_step_index(int index, int delta, int count, bool wraparound);

// Format a value into a fixed buffer. No heap.
void ui_format_parameter_value(const UiParamMeta& meta, float value, char* out,
                               size_t size);

// Build the Performance/FX Chain summary for one effect from the canonical
// state. Both screens share this single function.
void ui_build_effect_summary(UiEffectId effect, const UiParamState& state,
                             char* mainValue, size_t mainSize, char* metadata,
                             size_t metaSize);

#endif // UI_PARAM_MODEL_H
