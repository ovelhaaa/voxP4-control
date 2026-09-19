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

namespace {

// Harmony mode mask bits.
constexpr uint8_t kModeFixed = 1u << 0;
constexpr uint8_t kModeDiatonic = 1u << 1;
constexpr uint8_t kModeMidi = 1u << 2;
constexpr uint8_t kModeAll = kModeFixed | kModeDiatonic | kModeMidi;

} // namespace

// ---------------------------------------------------------------------------
// Descriptor tables (VoxLink IDs mirror the ESP32-P4 registry; they are stored
// for the future transport mapping only and are never used as array indices).
// ---------------------------------------------------------------------------
extern const UiParamDescriptor kUiHarmonyDescriptors[] = {
    {UiParamId::HarmonyMode, "MODE", "TARGET", UiControlType::Segmented,
     UiValueFormat::EnumLabel, 0, 2, 1, 0, kUiHarmonyModeLabels, 3, VOXP4_PARAM_HARMONY_MODE,
     kModeAll, false},
    {UiParamId::HarmonyInterval, "INTERVAL", "TARGET", UiControlType::Slider,
     UiValueFormat::Semitones, -12, 12, 1, 0, nullptr, 0, VOXP4_PARAM_HARMONY_INTERVAL, kModeFixed,
     false},
    {UiParamId::HarmonyDegree, "DEGREE", "TARGET", UiControlType::Slider,
     UiValueFormat::Degrees, -7, 7, 1, 0, nullptr, 0, VOXP4_PARAM_HARMONY_VOICE1_DEGREE, kModeDiatonic,
     false},
    {UiParamId::HarmonyKey, "KEY", "TARGET", UiControlType::Stepper,
     UiValueFormat::EnumLabel, 0, 11, 1, 0, kUiHarmonyKeyLabels, 12, VOXP4_PARAM_HARMONY_KEY,
     kModeDiatonic, true},
    {UiParamId::HarmonyScale, "SCALE", "TARGET", UiControlType::Stepper,
     UiValueFormat::EnumLabel, 0, 11, 1, 0, kUiHarmonyScaleLabels, 12, VOXP4_PARAM_HARMONY_SCALE,
     kModeDiatonic, true},
    {UiParamId::HarmonyNonScalePolicy, "NON-SCALE", "TARGET",
     UiControlType::Segmented, UiValueFormat::EnumLabel, 0, 2, 1, 0,
     kUiNonScalePolicyLabels, 3, VOXP4_PARAM_HARMONY_VOICE1_NON_SCALE_POLICY, kModeDiatonic, false},
    {UiParamId::HarmonyVoiceLeading, "VOICE LEAD", "TARGET",
     UiControlType::Toggle, UiValueFormat::EnumLabel, 0, 1, 1, 0, nullptr, 0,
     VOXP4_PARAM_HARMONY_VOICE1_VOICE_LEADING, kModeDiatonic, false},
    {UiParamId::HarmonyLevel, "LEVEL", "VOICE", UiControlType::Slider,
     UiValueFormat::Percent, 0, 1, 0.01f, 1.0f, nullptr, 0, VOXP4_PARAM_HARMONY_LEVEL, kModeAll,
     false},
    {UiParamId::HarmonyPan, "PAN", "VOICE", UiControlType::Slider,
     UiValueFormat::Pan, -1, 1, 0.01f, 0, nullptr, 0, VOXP4_PARAM_HARMONY_VOICE1_PAN, kModeAll, false},
    {UiParamId::HarmonySmoothingMs, "SMOOTHING", "VOICE",
     UiControlType::Slider, UiValueFormat::Milliseconds, 1, 500, 1, 30,
     nullptr, 0, VOXP4_PARAM_HARMONY_VOICE1_SMOOTHING_MS, kModeAll, false},
    {UiParamId::FormantEnabled, "MODE", "FORMANT", UiControlType::Toggle,
     UiValueFormat::EnumLabel, 0, 1, 1, 0, nullptr, 0, VOXP4_PARAM_HARMONY_FORMANT_ENABLE, kModeAll, false},
    {UiParamId::FormantAmount, "AMOUNT", "FORMANT", UiControlType::Slider,
     UiValueFormat::Percent, 0, 1, 0.01f, 1.0f, nullptr, 0, VOXP4_PARAM_HARMONY_FORMANT_AMOUNT, kModeAll,
     false},
    {UiParamId::HarmonyAttackMs, "ATTACK", "DYNAMICS", UiControlType::Slider,
     UiValueFormat::Milliseconds, 0.1f, 100, 0.1f, 4, nullptr, 0, VOXP4_PARAM_HARMONY_ATTACK_MS,
     kModeAll, false},
    {UiParamId::HarmonyReleaseMs, "RELEASE", "DYNAMICS", UiControlType::Slider,
     UiValueFormat::Milliseconds, 1, 500, 1, 20, nullptr, 0, VOXP4_PARAM_HARMONY_RELEASE_MS, kModeAll,
     false},
};
extern const size_t kUiHarmonyDescriptorCount =
    sizeof(kUiHarmonyDescriptors) / sizeof(kUiHarmonyDescriptors[0]);

extern const UiParamDescriptor kUiReverbDescriptors[] = {
    {UiParamId::ReverbWet, "MIX", "REVERB", UiControlType::Slider,
     UiValueFormat::Percent, 0, 1, 0.01f, 0.18f, nullptr, 0, VOXP4_PARAM_REVERB_WET, 0, false},
    {UiParamId::ReverbDecaySeconds, "DECAY", "REVERB", UiControlType::Slider,
     UiValueFormat::Seconds, 0.15f, 20, 0.05f, 2.0f, nullptr, 0, VOXP4_PARAM_REVERB_DECAY_S, 0,
     false},
    {UiParamId::ReverbDamping, "DAMPING", "REVERB", UiControlType::Slider,
     UiValueFormat::Percent, 0, 1, 0.01f, 0.45f, nullptr, 0, VOXP4_PARAM_REVERB_DAMPING, 0, false},
};
extern const size_t kUiReverbDescriptorCount =
    sizeof(kUiReverbDescriptors) / sizeof(kUiReverbDescriptors[0]);

extern const UiParamDescriptor kUiDelayDescriptors[] = {
    {UiParamId::DelayLeftMs, "LEFT", "TIME", UiControlType::Slider,
     UiValueFormat::Milliseconds, 1, 2000, 1, 250, nullptr, 0, VOXP4_PARAM_DELAY_LEFT_MS, 0,
     false},
    {UiParamId::DelayRightMs, "RIGHT", "TIME", UiControlType::Slider,
     UiValueFormat::Milliseconds, 1, 2000, 1, 375, nullptr, 0, VOXP4_PARAM_DELAY_RIGHT_MS, 0,
     false},
    {UiParamId::DelayFeedback, "FEEDBACK", "FEEDBACK", UiControlType::Slider,
     UiValueFormat::PercentSigned, -0.95f, 0.95f, 0.01f, 0.25f, nullptr, 0,
     VOXP4_PARAM_DELAY_FEEDBACK, 0, false},
    {UiParamId::DelayFeedbackLowpassHz, "FILTER", "FEEDBACK",
     UiControlType::Slider, UiValueFormat::Hertz, 200, 20000, 10, 6000, nullptr,
     0, VOXP4_PARAM_DELAY_FEEDBACK_LOWPASS_HZ, 0, false},
    {UiParamId::DelayWet, "WET", "MIX", UiControlType::Slider,
     UiValueFormat::Percent, 0, 1, 0.01f, 0.20f, nullptr, 0, VOXP4_PARAM_DELAY_WET, 0, false},
    {UiParamId::DelayDry, "DRY", "MIX", UiControlType::Slider,
     UiValueFormat::Percent, 0, 1, 0.01f, 1.0f, nullptr, 0, VOXP4_PARAM_DELAY_DRY, 0, false},
};
extern const size_t kUiDelayDescriptorCount =
    sizeof(kUiDelayDescriptors) / sizeof(kUiDelayDescriptors[0]);

extern const UiParamDescriptor kUiLimiterDescriptors[] = {
    {UiParamId::LimiterThresholdDb, "THRESHOLD", "HARMONY BUS",
     UiControlType::Slider, UiValueFormat::Decibels, -24, 0, 0.5f, -3.0f,
     nullptr, 0, VOXP4_PARAM_HARMONY_LIMITER_THRESHOLD_DB, 0, false},
};
extern const size_t kUiLimiterDescriptorCount =
    sizeof(kUiLimiterDescriptors) / sizeof(kUiLimiterDescriptors[0]);

// ---------------------------------------------------------------------------
// Lookup / gating
// ---------------------------------------------------------------------------
const UiParamDescriptor* ui_effect_descriptors(UiEffectId effect, size_t* count) {
    switch (effect) {
        case UiEffectId::Harmony:
            if (count) *count = kUiHarmonyDescriptorCount;
            return kUiHarmonyDescriptors;
        case UiEffectId::Reverb:
            if (count) *count = kUiReverbDescriptorCount;
            return kUiReverbDescriptors;
        case UiEffectId::Delay:
            if (count) *count = kUiDelayDescriptorCount;
            return kUiDelayDescriptors;
        case UiEffectId::Limiter:
            if (count) *count = kUiLimiterDescriptorCount;
            return kUiLimiterDescriptors;
        default:
            if (count) *count = 0;
            return nullptr;
    }
}

bool ui_descriptor_applies(const UiParamDescriptor& descriptor,
                           uint8_t harmonyMode) {
    if (descriptor.harmonyModeMask == 0) return true;
    if (harmonyMode > 2) harmonyMode = 0;
    return (descriptor.harmonyModeMask & (1u << harmonyMode)) != 0;
}

const UiParamDescriptor* ui_param_descriptor(UiParamId id) {
    for (size_t e = 0; e < kUiEffectCount; ++e) {
        size_t count = 0;
        const UiParamDescriptor* table =
            ui_effect_descriptors(static_cast<UiEffectId>(e), &count);
        for (size_t i = 0; i < count; ++i) {
            if (table[i].id == id) return &table[i];
        }
    }
    return nullptr;
}

UiEffectId ui_effect_of(UiParamId id) {
    for (size_t e = 0; e < kUiEffectCount; ++e) {
        size_t count = 0;
        const UiParamDescriptor* table =
            ui_effect_descriptors(static_cast<UiEffectId>(e), &count);
        for (size_t i = 0; i < count; ++i) {
            if (table[i].id == id) return static_cast<UiEffectId>(e);
        }
    }
    return UiEffectId::Count;
}

uint16_t ui_param_voxlink_id(UiParamId id) {
    const UiParamDescriptor* d = ui_param_descriptor(id);
    return d ? d->voxlinkId : 0u;
}

uint16_t ui_effect_enable_voxlink_id(UiEffectId effect) {
    switch (effect) {
        case UiEffectId::Harmony: return VOXP4_PARAM_HARMONY_ENABLE;
        case UiEffectId::Reverb: return VOXP4_PARAM_REVERB_ENABLE;
        case UiEffectId::Delay: return VOXP4_PARAM_DELAY_ENABLE;
        // LIMITER is the harmony bus limiter, never the master ceiling.
        case UiEffectId::Limiter: return VOXP4_PARAM_HARMONY_LIMITER_ENABLE;
        default: return 0u;
    }
}

bool ui_effect_default_enabled(UiEffectId effect) {
    // Mirrors the VoxLink registry *.enable defaults. These are a LOCAL
    // simulation until M6 GET_STATE hydrates authoritative state.
    switch (effect) {
        case UiEffectId::Harmony: return false; // harmony.enable default 0
        case UiEffectId::Reverb: return true;   // reverb.enable default 1
        case UiEffectId::Delay: return true;    // delay.enable default 1
        case UiEffectId::Limiter: return true;  // harmony.limiter.enable default 1
        default: return false;
    }
}

float ui_clamp_parameter(const UiParamDescriptor& descriptor, float value) {
    if (!std::isfinite(value)) return descriptor.defaultValue;
    float v = value;
    if (v < descriptor.minValue) v = descriptor.minValue;
    if (v > descriptor.maxValue) v = descriptor.maxValue;
    if (descriptor.uiStep > 0.0f) {
        v = descriptor.minValue +
            std::round((v - descriptor.minValue) / descriptor.uiStep) *
                descriptor.uiStep;
    }
    if (v < descriptor.minValue) v = descriptor.minValue;
    if (v > descriptor.maxValue) v = descriptor.maxValue;
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

// Trim a single trailing zero so "2.00" -> "2.0" while keeping "0.15".
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
    }
}

float value_of(const float* values, UiParamId id) {
    return values[static_cast<size_t>(id)];
}

} // namespace

void ui_format_parameter_value(const UiParamDescriptor& descriptor,
                               float value, char* out, size_t size) {
    format_value(descriptor.format, value, descriptor.options,
                 descriptor.optionCount, out, size);
}

void ui_build_effect_summary(UiEffectId effect, const float* values,
                             char* mainValue, size_t mainSize, char* metadata,
                             size_t metaSize) {
    if (mainValue == nullptr || mainSize == 0 || metadata == nullptr ||
        metaSize == 0 || values == nullptr) {
        return;
    }
    mainValue[0] = '\0';
    metadata[0] = '\0';

    switch (effect) {
        case UiEffectId::Harmony: {
            const int mode = (int)std::lround(value_of(values, UiParamId::HarmonyMode));
            if (mode == 1) {
                format_value(UiValueFormat::Degrees,
                             value_of(values, UiParamId::HarmonyDegree), nullptr,
                             0, mainValue, mainSize);
                const int key = (int)std::lround(value_of(values, UiParamId::HarmonyKey));
                const int scale = (int)std::lround(value_of(values, UiParamId::HarmonyScale));
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
                             value_of(values, UiParamId::HarmonyInterval), nullptr,
                             0, mainValue, mainSize);
                std::snprintf(metadata, metaSize, "FIXED");
            }
            break;
        }
        case UiEffectId::Reverb:
            format_value(UiValueFormat::Percent,
                         value_of(values, UiParamId::ReverbWet), nullptr, 0,
                         mainValue, mainSize);
            format_value(UiValueFormat::Seconds,
                         value_of(values, UiParamId::ReverbDecaySeconds), nullptr,
                         0, metadata, metaSize);
            break;
        case UiEffectId::Delay:
            format_value(UiValueFormat::Milliseconds,
                         value_of(values, UiParamId::DelayLeftMs), nullptr, 0,
                         mainValue, mainSize);
            std::snprintf(metadata, metaSize, "R %d",
                          (int)std::lround(value_of(values, UiParamId::DelayRightMs)));
            break;
        case UiEffectId::Limiter:
            format_value(UiValueFormat::Decibels,
                         value_of(values, UiParamId::LimiterThresholdDb), nullptr,
                         0, mainValue, mainSize);
            std::snprintf(metadata, metaSize, "HARM BUS");
            break;
        default:
            std::snprintf(mainValue, mainSize, "--");
            break;
    }
}
