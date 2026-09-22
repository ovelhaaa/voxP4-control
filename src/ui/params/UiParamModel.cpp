#include "ui/params/UiParamModel.h"

#include <cmath>
#include <cstdio>
#include <cstring>

// ---------------------------------------------------------------------------
// Enum label tables
// ---------------------------------------------------------------------------
extern const char* const kUiHarmonyModeLabels[3] = {"FIXED", "DIATONIC",
                                                    "MIDI"};
extern const char* const kUiHarmonyKeyLabels[12] = {
    "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
extern const char* const kUiHarmonyScaleLabels[12] = {
    "MAJOR",      "NAT MIN",  "HARM MIN", "MEL MIN",  "DORIAN",   "PHRYGIAN",
    "LYDIAN",     "MIXOLYD",  "LOCRIAN",  "MAJ PENT", "MIN PENT", "BLUES MIN"};
extern const char* const kUiHarmonyScaleShortLabels[12] = {
    "MAJOR",   "NAT MIN",  "HARM MIN", "MEL MIN",  "DORIAN",   "PHRYGIAN",
    "LYDIAN",  "MIXOLYD",  "LOCRIAN",  "MAJ PENT", "MIN PENT", "BLUES MIN"};
extern const char* const kUiNonScalePolicyLabels[3] = {"NEAREST", "CHROMATIC",
                                                       "BYPASS"};
extern const char* const kUiModulationModeLabels[4] = {"CHORUS", "ENSEMBLE",
                                                       "DIMENSION",
                                                       "MICROSHIFT"};
extern const char* const kUiSubdivisionLabels[13] = {
    "1/1",  "1/2",  "1/4",   "1/8",   "1/16",   "1/32",  "1/2.",
    "1/4.", "1/8.", "1/16.", "1/4T",  "1/8T",   "1/16T"};
extern const char* const kUiDriveModeLabels[3] = {"WARM", "OVERDRIVE",
                                                  "MEGAPHONE"};
extern const char* const kUiSpatialRoutingLabels[2] = {"PARALLEL", "DLY>REV"};
extern const char* const kUiSpatialSourceLabels[3] = {"INPUT", "POST DYN",
                                                      "POST HARM"};

namespace {

// Harmony mode mask bits.
constexpr uint8_t kHarmonyFixed = 1u << 0;
constexpr uint8_t kHarmonyDiatonic = 1u << 1;
constexpr uint8_t kHarmonyMidi = 1u << 2;

// Modulation mode mask bits.
constexpr uint8_t kModChorus = 1u << 0;
constexpr uint8_t kModEnsemble = 1u << 1;
constexpr uint8_t kModDimension = 1u << 2;
constexpr uint8_t kModMicroshift = 1u << 3;
constexpr uint8_t kModLfo = kModChorus | kModEnsemble | kModDimension;

#define UIP(wire, lbl, sec, ctl, fmt, step, opts, optn, vis, cond, condwire, mask, wrap) \
    {wire, lbl, sec, UiControlType::ctl, UiValueFormat::fmt, step, opts, optn,          \
     UiVisibility::vis, UiCondition::cond, condwire, mask, wrap}

} // namespace

// ---------------------------------------------------------------------------
// Presentation metadata. Ranges/defaults intentionally live in the registry.
// ---------------------------------------------------------------------------

extern const UiParamMeta kUiGateDescriptors[] = {
    UIP(VOXP4_PARAM_GATE_THRESHOLD_DB, "THRESHOLD", "DETECTOR", Slider,
        Decibels, 0.5f, nullptr, 0, Basic, Always, 0, 0, false),
    UIP(VOXP4_PARAM_GATE_RANGE_DB, "RANGE", "DETECTOR", Slider, Decibels, 1.0f,
        nullptr, 0, Basic, Always, 0, 0, false),
    UIP(VOXP4_PARAM_GATE_ATTACK_MS, "ATTACK", "TIMING", Slider, Milliseconds,
        1.0f, nullptr, 0, Advanced, Always, 0, 0, false),
    UIP(VOXP4_PARAM_GATE_HOLD_MS, "HOLD", "TIMING", Slider, Milliseconds, 5.0f,
        nullptr, 0, Advanced, Always, 0, 0, false),
    UIP(VOXP4_PARAM_GATE_RELEASE_MS, "RELEASE", "TIMING", Slider, Milliseconds,
        5.0f, nullptr, 0, Advanced, Always, 0, 0, false),
};

extern const UiParamMeta kUiCompressorDescriptors[] = {
    UIP(VOXP4_PARAM_COMPRESSOR_THRESHOLD_DB, "THRESHOLD", "DETECTOR", Slider,
        Decibels, 0.5f, nullptr, 0, Basic, Always, 0, 0, false),
    UIP(VOXP4_PARAM_COMPRESSOR_RATIO, "RATIO", "DETECTOR", Slider, Ratio, 0.5f,
        nullptr, 0, Basic, Always, 0, 0, false),
    UIP(VOXP4_PARAM_COMPRESSOR_MAKEUP_DB, "MAKEUP", "OUTPUT", Slider, Decibels,
        0.5f, nullptr, 0, Basic, Always, 0, 0, false),
    UIP(VOXP4_PARAM_COMPRESSOR_ATTACK_MS, "ATTACK", "TIMING", Slider,
        Milliseconds, 1.0f, nullptr, 0, Advanced, Always, 0, 0, false),
    UIP(VOXP4_PARAM_COMPRESSOR_RELEASE_MS, "RELEASE", "TIMING", Slider,
        Milliseconds, 5.0f, nullptr, 0, Advanced, Always, 0, 0, false),
    UIP(VOXP4_PARAM_COMPRESSOR_KNEE_DB, "KNEE", "DETECTOR", Slider, Decibels,
        0.5f, nullptr, 0, Advanced, Always, 0, 0, false),
};

extern const UiParamMeta kUiHarmonyDescriptors[] = {
    UIP(VOXP4_PARAM_HARMONY_MODE, "MODE", "TARGET", Segmented, EnumLabel, 1.0f,
        kUiHarmonyModeLabels, 3, Basic, Always, 0, 0, false),
    UIP(VOXP4_PARAM_HARMONY_INTERVAL, "INTERVAL", "TARGET", Slider, Semitones,
        1.0f, nullptr, 0, Basic, Always, 0, kHarmonyFixed, false),
    UIP(VOXP4_PARAM_HARMONY_VOICE1_DEGREE, "DEGREE", "TARGET", Slider, Degrees,
        1.0f, nullptr, 0, Basic, Always, 0, kHarmonyDiatonic, false),
    UIP(VOXP4_PARAM_HARMONY_KEY, "KEY", "TARGET", Stepper, EnumLabel, 1.0f,
        kUiHarmonyKeyLabels, 12, Basic, Always, 0, kHarmonyDiatonic, true),
    UIP(VOXP4_PARAM_HARMONY_SCALE, "SCALE", "TARGET", Stepper, EnumLabel, 1.0f,
        kUiHarmonyScaleLabels, 12, Basic, Always, 0, kHarmonyDiatonic, true),
    UIP(VOXP4_PARAM_HARMONY_VOICE1_NON_SCALE_POLICY, "NON-SCALE", "TARGET",
        Segmented, EnumLabel, 1.0f, kUiNonScalePolicyLabels, 3, Basic, Always, 0,
        kHarmonyDiatonic, false),
    UIP(VOXP4_PARAM_HARMONY_VOICE1_VOICE_LEADING, "VOICE LEAD", "TARGET",
        Toggle, EnumLabel, 1.0f, nullptr, 0, Basic, Always, 0, kHarmonyDiatonic,
        false),
    UIP(VOXP4_PARAM_HARMONY_LEVEL, "LEVEL", "VOICE", Slider, Percent, 0.01f,
        nullptr, 0, Basic, Always, 0, 0, false),
    UIP(VOXP4_PARAM_HARMONY_VOICE1_PAN, "PAN", "VOICE", Slider, Pan, 0.01f,
        nullptr, 0, Basic, Always, 0, 0, false),
    UIP(VOXP4_PARAM_HARMONY_VOICE1_SMOOTHING_MS, "SMOOTHING", "VOICE", Slider,
        Milliseconds, 1.0f, nullptr, 0, Advanced, Always, 0, 0, false),
    UIP(VOXP4_PARAM_HARMONY_FORMANT_ENABLE, "FORMANT", "FORMANT", Toggle,
        EnumLabel, 1.0f, nullptr, 0, Advanced, Always, 0, 0, false),
    UIP(VOXP4_PARAM_HARMONY_FORMANT_AMOUNT, "AMOUNT", "FORMANT", Slider,
        Percent, 0.01f, nullptr, 0, Advanced, Always, 0, 0, false),
    UIP(VOXP4_PARAM_HARMONY_ATTACK_MS, "ATTACK", "DYNAMICS", Slider,
        Milliseconds, 0.1f, nullptr, 0, Advanced, Always, 0, 0, false),
    UIP(VOXP4_PARAM_HARMONY_RELEASE_MS, "RELEASE", "DYNAMICS", Slider,
        Milliseconds, 1.0f, nullptr, 0, Advanced, Always, 0, 0, false),
    UIP(VOXP4_PARAM_HARMONY_DRY_ALIGNMENT_ENABLE, "ENABLE", "DRY ALIGN",
        Toggle, EnumLabel, 1.0f, nullptr, 0, Advanced, Always, 0, 0, false),
    UIP(VOXP4_PARAM_HARMONY_DRY_ALIGNMENT_MS, "DELAY", "DRY ALIGN", Slider,
        Milliseconds, 1.0f, nullptr, 0, Advanced, BoolTrue,
        VOXP4_PARAM_HARMONY_DRY_ALIGNMENT_ENABLE, 0, false),
    UIP(VOXP4_PARAM_HARMONY_VOICE1_MIN_MIDI, "MIN NOTE", "MIDI RANGE", Slider,
        MidiNote, 1.0f, nullptr, 0, Advanced, Always, 0, kHarmonyMidi, false),
    UIP(VOXP4_PARAM_HARMONY_VOICE1_MAX_MIDI, "MAX NOTE", "MIDI RANGE", Slider,
        MidiNote, 1.0f, nullptr, 0, Advanced, Always, 0, kHarmonyMidi, false),
    UIP(VOXP4_PARAM_HARMONY_LIMITER_ENABLE, "ENABLE", "HARM LIMITER", Toggle,
        EnumLabel, 1.0f, nullptr, 0, Advanced, Always, 0, 0, false),
    UIP(VOXP4_PARAM_HARMONY_LIMITER_THRESHOLD_DB, "THRESHOLD", "HARM LIMITER",
        Slider, Decibels, 0.5f, nullptr, 0, Advanced, Always, 0, 0, false),
};

extern const UiParamMeta kUiDriveDescriptors[] = {
    UIP(VOXP4_PARAM_DRIVE_MODE, "MODE", "TONE", Segmented, EnumLabel, 1.0f,
        kUiDriveModeLabels, 3, Basic, Always, 0, 0, false),
    UIP(VOXP4_PARAM_DRIVE_DRIVE, "DRIVE", "TONE", Slider, Percent, 0.01f,
        nullptr, 0, Basic, Always, 0, 0, false),
    UIP(VOXP4_PARAM_DRIVE_TONE, "TONE", "TONE", Slider, Percent, 0.01f, nullptr,
        0, Basic, Always, 0, 0, false),
    UIP(VOXP4_PARAM_DRIVE_MIX, "MIX", "MIX", Slider, Percent, 0.01f, nullptr, 0,
        Basic, Always, 0, 0, false),
    UIP(VOXP4_PARAM_DRIVE_OUTPUT_LEVEL, "OUTPUT", "OUTPUT", Slider, Percent,
        0.01f, nullptr, 0, Advanced, Always, 0, 0, false),
};

extern const UiParamMeta kUiModulationDescriptors[] = {
    UIP(VOXP4_PARAM_CHORUS_MODE, "MODE", "MODE", Segmented, EnumLabel, 1.0f,
        kUiModulationModeLabels, 4, Basic, Always, 0, 0, false),
    UIP(VOXP4_PARAM_CHORUS_MIX, "MIX", "MODULATION", Slider, Percent, 0.01f,
        nullptr, 0, Basic, Always, 0, 0, false),
    UIP(VOXP4_PARAM_CHORUS_RATE_HZ, "RATE", "LFO", Slider, Hertz, 0.05f,
        nullptr, 0, Basic, BoolFalse, VOXP4_PARAM_CHORUS_SYNC_ENABLE, kModLfo,
        false),
    UIP(VOXP4_PARAM_CHORUS_SUBDIVISION, "DIVISION", "LFO", Stepper, EnumLabel,
        1.0f, kUiSubdivisionLabels, 13, Basic, BoolTrue,
        VOXP4_PARAM_CHORUS_SYNC_ENABLE, kModLfo, false),
    UIP(VOXP4_PARAM_CHORUS_DEPTH_MS, "DEPTH", "LFO", Slider, Milliseconds, 0.1f,
        nullptr, 0, Basic, Always, 0, kModLfo, false),
    UIP(VOXP4_PARAM_CHORUS_BASE_DELAY_MS, "BASE DELAY", "ADVANCED", Slider,
        Milliseconds, 0.5f, nullptr, 0, Advanced, Always, 0, kModLfo, false),
    UIP(VOXP4_PARAM_CHORUS_SYNC_ENABLE, "TEMPO SYNC", "ADVANCED", Toggle,
        EnumLabel, 1.0f, nullptr, 0, Advanced, Always, 0, kModLfo, false),
    UIP(VOXP4_PARAM_CHORUS_WIDTH, "WIDTH", "STEREO", Slider, Percent, 0.01f,
        nullptr, 0, Basic, Always, 0, 0, false),
    UIP(VOXP4_PARAM_CHORUS_MICROSHIFT_LEFT_CENTS, "LEFT DETUNE", "DETUNE",
        Slider, Cents, 1.0f, nullptr, 0, Basic, Always, 0, kModMicroshift,
        false),
    UIP(VOXP4_PARAM_CHORUS_MICROSHIFT_RIGHT_CENTS, "RIGHT DETUNE", "DETUNE",
        Slider, Cents, 1.0f, nullptr, 0, Basic, Always, 0, kModMicroshift,
        false),
    UIP(VOXP4_PARAM_CHORUS_MICROSHIFT_WINDOW_MS, "WINDOW", "ADVANCED", Slider,
        Milliseconds, 1.0f, nullptr, 0, Advanced, Always, 0, kModMicroshift,
        false),
};

extern const UiParamMeta kUiDelayDescriptors[] = {
    UIP(VOXP4_PARAM_DELAY_SYNC_ENABLE, "TEMPO SYNC", "SYNC", Toggle, EnumLabel,
        1.0f, nullptr, 0, Basic, Always, 0, 0, false),
    UIP(VOXP4_PARAM_DELAY_LEFT_MS, "LEFT", "TIME", Slider, Milliseconds, 1.0f,
        nullptr, 0, Basic, BoolFalse, VOXP4_PARAM_DELAY_SYNC_ENABLE, 0, false),
    UIP(VOXP4_PARAM_DELAY_RIGHT_MS, "RIGHT", "TIME", Slider, Milliseconds, 1.0f,
        nullptr, 0, Basic, BoolFalse, VOXP4_PARAM_DELAY_SYNC_ENABLE, 0, false),
    UIP(VOXP4_PARAM_DELAY_LEFT_SUBDIVISION, "LEFT DIV", "TIME", Stepper,
        EnumLabel, 1.0f, kUiSubdivisionLabels, 13, Basic, BoolTrue,
        VOXP4_PARAM_DELAY_SYNC_ENABLE, 0, false),
    UIP(VOXP4_PARAM_DELAY_RIGHT_SUBDIVISION, "RIGHT DIV", "TIME", Stepper,
        EnumLabel, 1.0f, kUiSubdivisionLabels, 13, Basic, BoolTrue,
        VOXP4_PARAM_DELAY_SYNC_ENABLE, 0, false),
    UIP(VOXP4_PARAM_DELAY_FEEDBACK, "FEEDBACK", "FEEDBACK", Slider,
        PercentSigned, 0.01f, nullptr, 0, Basic, Always, 0, 0, false),
    UIP(VOXP4_PARAM_DELAY_WET, "WET", "MIX", Slider, Percent, 0.01f, nullptr, 0,
        Basic, Always, 0, 0, false),
    UIP(VOXP4_PARAM_DELAY_DRY, "DRY", "MIX", Slider, Percent, 0.01f, nullptr, 0,
        Basic, Always, 0, 0, false),
    UIP(VOXP4_PARAM_DELAY_FEEDBACK_LOWPASS_HZ, "FILTER", "FEEDBACK", Slider,
        Hertz, 10.0f, nullptr, 0, Advanced, Always, 0, 0, false),
};

extern const UiParamMeta kUiReverbDescriptors[] = {
    UIP(VOXP4_PARAM_REVERB_WET, "MIX", "REVERB", Slider, Percent, 0.01f,
        nullptr, 0, Basic, Always, 0, 0, false),
    UIP(VOXP4_PARAM_REVERB_DECAY_S, "DECAY", "REVERB", Slider, Seconds, 0.05f,
        nullptr, 0, Basic, Always, 0, 0, false),
    UIP(VOXP4_PARAM_REVERB_DAMPING, "DAMPING", "REVERB", Slider, Percent,
        0.01f, nullptr, 0, Basic, Always, 0, 0, false),
};

extern const UiParamMeta kUiMasterDescriptors[] = {
    UIP(VOXP4_PARAM_TEMPO_BPM, "TEMPO", "TEMPO", Slider, Bpm, 1.0f, nullptr, 0,
        Global, Always, 0, 0, false),
    UIP(VOXP4_PARAM_LIMITER_CEILING, "CEILING", "OUTPUT", Slider, CeilingDb,
        0.01f, nullptr, 0, Global, Always, 0, 0, false),
    UIP(VOXP4_PARAM_OUTPUT_MUTE_DRY, "MUTE DRY", "OUTPUT", Toggle, EnumLabel,
        1.0f, nullptr, 0, Global, Always, 0, 0, false),
    UIP(VOXP4_PARAM_OUTPUT_SPATIAL_ROUTING, "ROUTING", "ROUTING", Segmented,
        EnumLabel, 1.0f, kUiSpatialRoutingLabels, 2, Global, Always, 0, 0,
        false),
    UIP(VOXP4_PARAM_OUTPUT_SPATIAL_SOURCE, "SOURCE", "ROUTING", Segmented,
        EnumLabel, 1.0f, kUiSpatialSourceLabels, 3, Global, Always, 0, 0, false),
};

// ---------------------------------------------------------------------------
// Lookup
// ---------------------------------------------------------------------------
const UiParamMeta* ui_effect_metadata(UiEffectId effect, size_t* count) {
    switch (effect) {
        case UiEffectId::Gate:
            if (count) *count = sizeof(kUiGateDescriptors) / sizeof(kUiGateDescriptors[0]);
            return kUiGateDescriptors;
        case UiEffectId::Compressor:
            if (count) *count = sizeof(kUiCompressorDescriptors) / sizeof(kUiCompressorDescriptors[0]);
            return kUiCompressorDescriptors;
        case UiEffectId::Harmony:
            if (count) *count = sizeof(kUiHarmonyDescriptors) / sizeof(kUiHarmonyDescriptors[0]);
            return kUiHarmonyDescriptors;
        case UiEffectId::Drive:
            if (count) *count = sizeof(kUiDriveDescriptors) / sizeof(kUiDriveDescriptors[0]);
            return kUiDriveDescriptors;
        case UiEffectId::Modulation:
            if (count) *count = sizeof(kUiModulationDescriptors) / sizeof(kUiModulationDescriptors[0]);
            return kUiModulationDescriptors;
        case UiEffectId::Delay:
            if (count) *count = sizeof(kUiDelayDescriptors) / sizeof(kUiDelayDescriptors[0]);
            return kUiDelayDescriptors;
        case UiEffectId::Reverb:
            if (count) *count = sizeof(kUiReverbDescriptors) / sizeof(kUiReverbDescriptors[0]);
            return kUiReverbDescriptors;
        default:
            if (count) *count = 0;
            return nullptr;
    }
}

const UiParamMeta* ui_master_metadata(size_t* count) {
    if (count) *count = sizeof(kUiMasterDescriptors) / sizeof(kUiMasterDescriptors[0]);
    return kUiMasterDescriptors;
}

const UiParamMeta* ui_param_meta(uint16_t wireId) {
    if (wireId == 0) return nullptr;
    for (size_t e = 0; e < kUiEffectCount; ++e) {
        size_t count = 0;
        const UiParamMeta* table =
            ui_effect_metadata(static_cast<UiEffectId>(e), &count);
        for (size_t i = 0; i < count; ++i) {
            if (table[i].wireId == wireId) return &table[i];
        }
    }
    size_t mcount = 0;
    const UiParamMeta* master = ui_master_metadata(&mcount);
    for (size_t i = 0; i < mcount; ++i) {
        if (master[i].wireId == wireId) return &master[i];
    }
    return nullptr;
}

UiEffectId ui_effect_of_wire(uint16_t wireId) {
    for (size_t e = 0; e < kUiEffectCount; ++e) {
        size_t count = 0;
        const UiParamMeta* table =
            ui_effect_metadata(static_cast<UiEffectId>(e), &count);
        for (size_t i = 0; i < count; ++i) {
            if (table[i].wireId == wireId) return static_cast<UiEffectId>(e);
        }
    }
    return UiEffectId::Count;
}

uint16_t ui_effect_enable_wire(UiEffectId effect) {
    switch (effect) {
        case UiEffectId::Gate: return VOXP4_PARAM_GATE_ENABLE;
        case UiEffectId::Compressor: return VOXP4_PARAM_COMPRESSOR_ENABLE;
        case UiEffectId::Harmony: return VOXP4_PARAM_HARMONY_ENABLE;
        case UiEffectId::Drive: return VOXP4_PARAM_DRIVE_ENABLE;
        case UiEffectId::Modulation: return VOXP4_PARAM_CHORUS_ENABLE;
        case UiEffectId::Delay: return VOXP4_PARAM_DELAY_ENABLE;
        case UiEffectId::Reverb: return VOXP4_PARAM_REVERB_ENABLE;
        default: return 0u;
    }
}

bool ui_effect_from_enable_wire(uint16_t wireId, UiEffectId* out) {
    for (size_t e = 0; e < kUiEffectCount; ++e) {
        const UiEffectId effect = static_cast<UiEffectId>(e);
        if (ui_effect_enable_wire(effect) == wireId) {
            if (out) *out = effect;
            return true;
        }
    }
    return false;
}

bool ui_is_effect_enable(uint16_t wireId) {
    UiEffectId ignored;
    return ui_effect_from_enable_wire(wireId, &ignored);
}

const char* ui_effect_name(UiEffectId effect) {
    switch (effect) {
        case UiEffectId::Gate: return "GATE";
        case UiEffectId::Compressor: return "COMPRESSOR";
        case UiEffectId::Harmony: return "HARMONY";
        case UiEffectId::Drive: return "DRIVE";
        case UiEffectId::Modulation: return "MODULATION";
        case UiEffectId::Delay: return "DELAY";
        case UiEffectId::Reverb: return "REVERB";
        default: return "--";
    }
}

const char* ui_effect_short_name(UiEffectId effect) {
    switch (effect) {
        case UiEffectId::Gate: return "GATE";
        case UiEffectId::Compressor: return "COMP";
        case UiEffectId::Harmony: return "HARM";
        case UiEffectId::Drive: return "DRIVE";
        case UiEffectId::Modulation: return "MOD";
        case UiEffectId::Delay: return "DLY";
        case UiEffectId::Reverb: return "REV";
        default: return "--";
    }
}

// ---------------------------------------------------------------------------
// Registry-backed accessors
// ---------------------------------------------------------------------------
float ui_param_min(uint16_t wireId) {
    const ParamDescriptor* d = ParameterRegistry::getByWireId(wireId);
    return d ? d->minValue : 0.0f;
}

float ui_param_max(uint16_t wireId) {
    const ParamDescriptor* d = ParameterRegistry::getByWireId(wireId);
    return d ? d->maxValue : 0.0f;
}

float ui_param_default(uint16_t wireId) {
    const ParamDescriptor* d = ParameterRegistry::getByWireId(wireId);
    return d ? d->defaultValue : 0.0f;
}

bool ui_meta_applies(const UiParamMeta& meta, uint8_t mode,
                     const UiParamState& state) {
    if (meta.modeMask != 0) {
        if (mode >= 8) return false;
        if ((meta.modeMask & (1u << mode)) == 0) return false;
    }
    switch (meta.condition) {
        case UiCondition::BoolTrue:
            if (state.get(meta.conditionWireId) < 0.5f) return false;
            break;
        case UiCondition::BoolFalse:
            if (state.get(meta.conditionWireId) >= 0.5f) return false;
            break;
        case UiCondition::Always:
        default:
            break;
    }
    return true;
}

float ui_clamp_parameter(const UiParamMeta& meta, float value) {
    const ParamDescriptor* d = ParameterRegistry::getByWireId(meta.wireId);
    if (d == nullptr) return value;
    if (!std::isfinite(value)) return d->defaultValue;

    float v = value;
    if (v < d->minValue) v = d->minValue;
    if (v > d->maxValue) v = d->maxValue;
    if (meta.uiStep > 0.0f) {
        v = d->minValue + std::round((v - d->minValue) / meta.uiStep) * meta.uiStep;
    }
    if (v < d->minValue) v = d->minValue;
    if (v > d->maxValue) v = d->maxValue;
    return v;
}

int ui_step_index(int index, int delta, int count, bool wraparound) {
    if (count <= 0) return 0;
    int i = index + delta;
    if (wraparound) {
        i %= count;
        if (i < 0) i += count;
    } else {
        if (i < 0) i = 0;
        if (i >= count) i = count - 1;
    }
    return i;
}

// ---------------------------------------------------------------------------
// Formatting (no heap)
// ---------------------------------------------------------------------------
namespace {

void trim_one_trailing_zero(char* number) {
    size_t len = std::strlen(number);
    if (len >= 2 && number[len - 1] == '0') {
        number[len - 1] = '\0';
    }
}

void format_value(UiValueFormat format, float value,
                  const char* const* options, uint8_t optionCount, char* out,
                  size_t size) {
    if (out == nullptr || size == 0) return;
    switch (format) {
        case UiValueFormat::Percent: {
            int pct = (int)std::lround(value * 100.0f);
            std::snprintf(out, size, "%d%%", pct);
            break;
        }
        case UiValueFormat::PercentSigned: {
            int pct = (int)std::lround(value * 100.0f);
            if (pct > 0)
                std::snprintf(out, size, "+%d%%", pct);
            else
                std::snprintf(out, size, "%d%%", pct);
            break;
        }
        case UiValueFormat::Milliseconds:
            std::snprintf(out, size, "%d ms", (int)std::lround(value));
            break;
        case UiValueFormat::Seconds: {
            char number[16];
            std::snprintf(number, sizeof(number), "%.2f", (double)value);
            trim_one_trailing_zero(number);
            std::snprintf(out, size, "%s s", number);
            break;
        }
        case UiValueFormat::Decibels:
            std::snprintf(out, size, "%.1f dB", (double)value);
            break;
        case UiValueFormat::Semitones: {
            int st = (int)std::lround(value);
            if (st > 0)
                std::snprintf(out, size, "+%d st", st);
            else
                std::snprintf(out, size, "%d st", st);
            break;
        }
        case UiValueFormat::Degrees: {
            int deg = (int)std::lround(value);
            if (deg > 0)
                std::snprintf(out, size, "+%d deg", deg);
            else
                std::snprintf(out, size, "%d deg", deg);
            break;
        }
        case UiValueFormat::Pan: {
            int mag = (int)std::lround(std::fabs(value) * 100.0f);
            if (mag == 0)
                std::snprintf(out, size, "C");
            else if (value < 0)
                std::snprintf(out, size, "L %d", mag);
            else
                std::snprintf(out, size, "R %d", mag);
            break;
        }
        case UiValueFormat::Hertz:
            if (std::fabs(value) >= 1000.0f)
                std::snprintf(out, size, "%.1f kHz", (double)(value / 1000.0f));
            else
                std::snprintf(out, size, "%d Hz", (int)std::lround(value));
            break;
        case UiValueFormat::EnumLabel: {
            int idx = (int)std::lround(value);
            if (options != nullptr && idx >= 0 && idx < optionCount)
                std::snprintf(out, size, "%s", options[idx]);
            else
                std::snprintf(out, size, "--");
            break;
        }
        case UiValueFormat::Cents: {
            int c = (int)std::lround(value);
            if (c > 0)
                std::snprintf(out, size, "+%d c", c);
            else
                std::snprintf(out, size, "%d c", c);
            break;
        }
        case UiValueFormat::Ratio:
            if (std::fabs(value - std::round(value)) < 0.05f)
                std::snprintf(out, size, "%d:1", (int)std::lround(value));
            else
                std::snprintf(out, size, "%.1f:1", (double)value);
            break;
        case UiValueFormat::Bpm:
            std::snprintf(out, size, "%d BPM", (int)std::lround(value));
            break;
        case UiValueFormat::MidiNote: {
            static const char* notes[12] = {"C",  "C#", "D",  "D#", "E",  "F",
                                            "F#", "G",  "G#", "A",  "A#", "B"};
            int n = (int)std::lround(value);
            if (n < 0 || n > 127) {
                std::snprintf(out, size, "--");
            } else {
                std::snprintf(out, size, "%s%d", notes[n % 12], n / 12 - 1);
            }
            break;
        }
        case UiValueFormat::CeilingDb: {
            if (value <= 0.0f) {
                std::snprintf(out, size, "-inf");
            } else {
                const double db = 20.0 * std::log10((double)value);
                std::snprintf(out, size, "%.1f dB", db);
            }
            break;
        }
    }
}

} // namespace

void ui_format_parameter_value(const UiParamMeta& meta, float value, char* out,
                               size_t size) {
    format_value(meta.format, value, meta.options, meta.optionCount, out, size);
}

// ---------------------------------------------------------------------------
// Summaries
// ---------------------------------------------------------------------------
namespace {

void label_of(const UiParamMeta* meta, float value, char* out, size_t size) {
    if (meta == nullptr) {
        std::snprintf(out, size, "--");
        return;
    }
    format_value(UiValueFormat::EnumLabel, value, meta->options, meta->optionCount,
                 out, size);
}

} // namespace

void ui_build_effect_summary(UiEffectId effect, const UiParamState& state,
                             char* mainValue, size_t mainSize, char* metadata,
                             size_t metaSize) {
    if (mainValue == nullptr || mainSize == 0 || metadata == nullptr ||
        metaSize == 0) {
        return;
    }
    mainValue[0] = '\0';
    metadata[0] = '\0';

    switch (effect) {
        case UiEffectId::Gate: {
            format_value(UiValueFormat::Decibels,
                         state.get(VOXP4_PARAM_GATE_THRESHOLD_DB), nullptr, 0,
                         mainValue, mainSize);
            std::snprintf(metadata, metaSize, "THR");
            break;
        }
        case UiEffectId::Compressor: {
            format_value(UiValueFormat::Ratio,
                         state.get(VOXP4_PARAM_COMPRESSOR_RATIO), nullptr, 0,
                         mainValue, mainSize);
            format_value(UiValueFormat::Decibels,
                         state.get(VOXP4_PARAM_COMPRESSOR_THRESHOLD_DB), nullptr,
                         0, metadata, metaSize);
            break;
        }
        case UiEffectId::Harmony: {
            const int mode = (int)std::lround(state.get(VOXP4_PARAM_HARMONY_MODE));
            if (mode == 1) {
                format_value(UiValueFormat::Degrees,
                             state.get(VOXP4_PARAM_HARMONY_VOICE1_DEGREE), nullptr,
                             0, mainValue, mainSize);
                const int key = (int)std::lround(state.get(VOXP4_PARAM_HARMONY_KEY));
                const int scale =
                    (int)std::lround(state.get(VOXP4_PARAM_HARMONY_SCALE));
                const char* keyLabel =
                    (key >= 0 && key < 12) ? kUiHarmonyKeyLabels[key] : "?";
                const char* scaleLabel =
                    (scale >= 0 && scale < 12) ? kUiHarmonyScaleShortLabels[scale]
                                               : "?";
                std::snprintf(metadata, metaSize, "%s %s", keyLabel, scaleLabel);
            } else if (mode == 2) {
                std::snprintf(mainValue, mainSize, "MIDI");
                std::snprintf(metadata, metaSize, "CHORD");
            } else {
                format_value(UiValueFormat::Semitones,
                             state.get(VOXP4_PARAM_HARMONY_INTERVAL), nullptr, 0,
                             mainValue, mainSize);
                std::snprintf(metadata, metaSize, "FIXED");
            }
            break;
        }
        case UiEffectId::Drive: {
            label_of(ui_param_meta(VOXP4_PARAM_DRIVE_MODE),
                     state.get(VOXP4_PARAM_DRIVE_MODE), mainValue, mainSize);
            format_value(UiValueFormat::Percent,
                         state.get(VOXP4_PARAM_DRIVE_DRIVE), nullptr, 0, metadata,
                         metaSize);
            break;
        }
        case UiEffectId::Modulation: {
            format_value(UiValueFormat::Percent, state.get(VOXP4_PARAM_CHORUS_MIX),
                         nullptr, 0, mainValue, mainSize);
            const int mode = (int)std::lround(state.get(VOXP4_PARAM_CHORUS_MODE));
            switch (mode) {
                case 1: std::snprintf(metadata, metaSize, "ENSEMBLE"); break;
                case 2: std::snprintf(metadata, metaSize, "DIMENSION"); break;
                case 3: {
                    const int left = (int)std::lround(
                        state.get(VOXP4_PARAM_CHORUS_MICROSHIFT_LEFT_CENTS));
                    const int right = (int)std::lround(
                        state.get(VOXP4_PARAM_CHORUS_MICROSHIFT_RIGHT_CENTS));
                    std::snprintf(metadata, metaSize, "%d/+%d c", left, right);
                    break;
                }
                default: std::snprintf(metadata, metaSize, "CHORUS"); break;
            }
            break;
        }
        case UiEffectId::Delay: {
            if (state.get(VOXP4_PARAM_DELAY_SYNC_ENABLE) >= 0.5f) {
                char left[8];
                char right[8];
                label_of(ui_param_meta(VOXP4_PARAM_DELAY_LEFT_SUBDIVISION),
                         state.get(VOXP4_PARAM_DELAY_LEFT_SUBDIVISION), left,
                         sizeof(left));
                label_of(ui_param_meta(VOXP4_PARAM_DELAY_RIGHT_SUBDIVISION),
                         state.get(VOXP4_PARAM_DELAY_RIGHT_SUBDIVISION), right,
                         sizeof(right));
                std::snprintf(mainValue, mainSize, "%s / %s", left, right);
                format_value(UiValueFormat::Percent,
                             state.get(VOXP4_PARAM_DELAY_FEEDBACK), nullptr, 0,
                             metadata, metaSize);
                const size_t len = std::strlen(metadata);
                if (len + 3 < metaSize) std::strcat(metadata, " FB");
            } else {
                format_value(UiValueFormat::Milliseconds,
                             state.get(VOXP4_PARAM_DELAY_LEFT_MS), nullptr, 0,
                             mainValue, mainSize);
                std::snprintf(metadata, metaSize, "R %d",
                              (int)std::lround(state.get(VOXP4_PARAM_DELAY_RIGHT_MS)));
            }
            break;
        }
        case UiEffectId::Reverb: {
            format_value(UiValueFormat::Percent,
                         state.get(VOXP4_PARAM_REVERB_WET), nullptr, 0, mainValue,
                         mainSize);
            format_value(UiValueFormat::Seconds,
                         state.get(VOXP4_PARAM_REVERB_DECAY_S), nullptr, 0,
                         metadata, metaSize);
            break;
        }
        default:
            std::snprintf(mainValue, mainSize, "--");
            break;
    }
}
