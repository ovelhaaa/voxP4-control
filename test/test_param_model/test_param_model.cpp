// Host-side tests for the pure parameter model and canonical state.
//
// Covers: formatters, registry-backed ranges/defaults, mode/switch gating,
// Delay/Chorus sync, Drive/Gate/Compressor metadata, Master routing, summaries,
// remote authority + rollback, and the mandatory UI coverage of all 71
// canonical parameters.
#include <unity.h>

#include <cmath>
#include <cstring>

#include "model/ParameterRegistry.h"
#include "ui/params/UiParamModel.h"
#include "ui/params/UiParamState.h"

namespace {

UiParamState g_state;

const UiParamMeta& meta(uint16_t wireId) {
    const UiParamMeta* m = ui_param_meta(wireId);
    TEST_ASSERT_NOT_NULL(m);
    return *m;
}

void format_into(uint16_t wireId, float value, char* out, size_t size) {
    ui_format_parameter_value(meta(wireId), value, out, size);
}

void summary(UiEffectId effect, char* main, size_t mainSize, char* metaOut,
             size_t metaSize) {
    ui_build_effect_summary(effect, g_state, main, mainSize, metaOut, metaSize);
}

const char* label_of(uint16_t wireId, int value) {
    const UiParamMeta& m = meta(wireId);
    TEST_ASSERT_TRUE(value >= 0 && value < (int)m.optionCount);
    return m.options[value];
}

} // namespace

void setUp(void) { g_state.resetToDefaults(); }
void tearDown(void) {}

// --- Formatters ------------------------------------------------------------
void test_percent_format(void) {
    char buf[24];
    format_into(VOXP4_PARAM_REVERB_WET, 0.18f, buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING("18%", buf);
    format_into(VOXP4_PARAM_REVERB_WET, 1.0f, buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING("100%", buf);
}

void test_db_format(void) {
    char buf[24];
    format_into(VOXP4_PARAM_HARMONY_LIMITER_THRESHOLD_DB, -3.0f, buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING("-3.0 dB", buf);
}

void test_pan_format(void) {
    char buf[24];
    format_into(VOXP4_PARAM_HARMONY_VOICE1_PAN, 0.0f, buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING("C", buf);
    format_into(VOXP4_PARAM_HARMONY_VOICE1_PAN, -0.25f, buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING("L 25", buf);
    format_into(VOXP4_PARAM_HARMONY_VOICE1_PAN, 0.5f, buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING("R 50", buf);
}

void test_semitone_format(void) {
    char buf[24];
    format_into(VOXP4_PARAM_HARMONY_INTERVAL, 4.0f, buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING("+4 st", buf);
    format_into(VOXP4_PARAM_HARMONY_INTERVAL, -5.0f, buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING("-5 st", buf);
}

void test_seconds_format(void) {
    char buf[24];
    format_into(VOXP4_PARAM_REVERB_DECAY_S, 2.0f, buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING("2.0 s", buf);
    format_into(VOXP4_PARAM_REVERB_DECAY_S, 4.5f, buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING("4.5 s", buf);
}

void test_hertz_format(void) {
    char buf[24];
    format_into(VOXP4_PARAM_DELAY_FEEDBACK_LOWPASS_HZ, 6000.0f, buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING("6.0 kHz", buf);
    format_into(VOXP4_PARAM_DELAY_FEEDBACK_LOWPASS_HZ, 200.0f, buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING("200 Hz", buf);
}

void test_cents_format(void) {
    char buf[24];
    format_into(VOXP4_PARAM_CHORUS_MICROSHIFT_LEFT_CENTS, -7.0f, buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING("-7 c", buf);
    format_into(VOXP4_PARAM_CHORUS_MICROSHIFT_RIGHT_CENTS, 9.0f, buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING("+9 c", buf);
}

void test_ratio_format(void) {
    char buf[24];
    format_into(VOXP4_PARAM_COMPRESSOR_RATIO, 3.0f, buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING("3:1", buf);
    format_into(VOXP4_PARAM_COMPRESSOR_RATIO, 4.5f, buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING("4.5:1", buf);
}

void test_bpm_format(void) {
    char buf[24];
    format_into(VOXP4_PARAM_TEMPO_BPM, 120.0f, buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING("120 BPM", buf);
}

void test_midi_note_format(void) {
    char buf[24];
    format_into(VOXP4_PARAM_HARMONY_VOICE1_MIN_MIDI, 60.0f, buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING("C4", buf);
    format_into(VOXP4_PARAM_HARMONY_VOICE1_MAX_MIDI, 69.0f, buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING("A4", buf);
}

// --- Registry-backed ranges / defaults -------------------------------------
void test_ui_ranges_match_registry(void) {
    for (size_t e = 0; e < kUiEffectCount; ++e) {
        size_t count = 0;
        const UiParamMeta* table =
            ui_effect_metadata(static_cast<UiEffectId>(e), &count);
        TEST_ASSERT_NOT_NULL(table);
        TEST_ASSERT_TRUE(count > 0);
        for (size_t i = 0; i < count; ++i) {
            const ParamDescriptor* d =
                ParameterRegistry::getByWireId(table[i].wireId);
            TEST_ASSERT_NOT_NULL(d);
            // The UI exposes exactly the registry range/default.
            TEST_ASSERT_EQUAL_FLOAT(d->minValue, ui_param_min(table[i].wireId));
            TEST_ASSERT_EQUAL_FLOAT(d->maxValue, ui_param_max(table[i].wireId));
            TEST_ASSERT_EQUAL_FLOAT(d->defaultValue,
                                    ui_param_default(table[i].wireId));
        }
    }
    size_t mcount = 0;
    const UiParamMeta* master = ui_master_metadata(&mcount);
    for (size_t i = 0; i < mcount; ++i) {
        const ParamDescriptor* d = ParameterRegistry::getByWireId(master[i].wireId);
        TEST_ASSERT_NOT_NULL(d);
        TEST_ASSERT_EQUAL_FLOAT(d->minValue, ui_param_min(master[i].wireId));
        TEST_ASSERT_EQUAL_FLOAT(d->maxValue, ui_param_max(master[i].wireId));
        TEST_ASSERT_EQUAL_FLOAT(d->defaultValue, ui_param_default(master[i].wireId));
    }
}

void test_known_registry_defaults(void) {
    TEST_ASSERT_EQUAL_FLOAT(120.0f, ui_param_default(VOXP4_PARAM_TEMPO_BPM));
    TEST_ASSERT_EQUAL_FLOAT(0.30f, ui_param_default(VOXP4_PARAM_CHORUS_MIX));
    TEST_ASSERT_EQUAL_FLOAT(0.75f, ui_param_default(VOXP4_PARAM_CHORUS_RATE_HZ));
    TEST_ASSERT_EQUAL_FLOAT(0.0f, ui_param_min(VOXP4_PARAM_DELAY_LEFT_MS));
    TEST_ASSERT_EQUAL_FLOAT(-50.0f,
                            ui_param_min(VOXP4_PARAM_CHORUS_MICROSHIFT_LEFT_CENTS));
    TEST_ASSERT_EQUAL_FLOAT(50.0f,
                            ui_param_max(VOXP4_PARAM_CHORUS_MICROSHIFT_LEFT_CENTS));
    TEST_ASSERT_EQUAL_FLOAT(8.0f,
                            ui_param_default(VOXP4_PARAM_DELAY_RIGHT_SUBDIVISION));
}

// --- Clamping / quantization -----------------------------------------------
void test_clamp(void) {
    TEST_ASSERT_EQUAL_FLOAT(12.0f,
        ui_clamp_parameter(meta(VOXP4_PARAM_HARMONY_INTERVAL), 99.0f));
    TEST_ASSERT_EQUAL_FLOAT(-12.0f,
        ui_clamp_parameter(meta(VOXP4_PARAM_HARMONY_INTERVAL), -99.0f));
    TEST_ASSERT_EQUAL_FLOAT(-50.0f,
        ui_clamp_parameter(meta(VOXP4_PARAM_CHORUS_MICROSHIFT_LEFT_CENTS), -99.0f));
    TEST_ASSERT_EQUAL_FLOAT(50.0f,
        ui_clamp_parameter(meta(VOXP4_PARAM_CHORUS_MICROSHIFT_LEFT_CENTS), 99.0f));
}

void test_uistep_quantization(void) {
    const UiParamMeta& level = meta(VOXP4_PARAM_HARMONY_LEVEL);
    TEST_ASSERT_EQUAL_FLOAT(0.01f, level.uiStep);
    TEST_ASSERT_EQUAL_FLOAT(0.12f, ui_clamp_parameter(level, 0.123f));
    TEST_ASSERT_EQUAL_FLOAT(1.0f, ui_clamp_parameter(level, 1.5f));
}

// --- State authority / rollback --------------------------------------------
void test_state_local_and_authoritative(void) {
    UiParamState s;
    s.resetToDefaults();
    TEST_ASSERT_EQUAL_FLOAT(120.0f, s.get(VOXP4_PARAM_TEMPO_BPM));
    TEST_ASSERT_EQUAL_INT((int)UiValueAuthority::LocalDefault,
                          (int)s.authority(VOXP4_PARAM_TEMPO_BPM));

    // Local edit clamps to the registry range and is pending.
    s.setLocal(VOXP4_PARAM_TEMPO_BPM, 900.0f);
    TEST_ASSERT_EQUAL_FLOAT(300.0f, s.get(VOXP4_PARAM_TEMPO_BPM));
    TEST_ASSERT_EQUAL_INT((int)UiValueAuthority::LocalPending,
                          (int)s.authority(VOXP4_PARAM_TEMPO_BPM));

    // Rejected SET -> rollback to the last authoritative value.
    s.revert(VOXP4_PARAM_TEMPO_BPM);
    TEST_ASSERT_EQUAL_FLOAT(120.0f, s.get(VOXP4_PARAM_TEMPO_BPM));
    TEST_ASSERT_EQUAL_INT((int)UiValueAuthority::Authoritative,
                          (int)s.authority(VOXP4_PARAM_TEMPO_BPM));

    // Remote PARAM_CHANGED wins.
    s.setLocal(VOXP4_PARAM_TEMPO_BPM, 130.0f);
    s.applyAuthoritative(VOXP4_PARAM_TEMPO_BPM, 96.0f);
    TEST_ASSERT_EQUAL_FLOAT(96.0f, s.get(VOXP4_PARAM_TEMPO_BPM));
    TEST_ASSERT_EQUAL_FLOAT(96.0f, s.authoritative(VOXP4_PARAM_TEMPO_BPM));
    TEST_ASSERT_EQUAL_INT((int)UiValueAuthority::Authoritative,
                          (int)s.authority(VOXP4_PARAM_TEMPO_BPM));
}

void test_state_bool_clamp(void) {
    UiParamState s;
    s.resetToDefaults();
    s.setLocal(VOXP4_PARAM_DRIVE_ENABLE, 1.0f);
    TEST_ASSERT_EQUAL_FLOAT(1.0f, s.get(VOXP4_PARAM_DRIVE_ENABLE));
    s.setLocal(VOXP4_PARAM_DRIVE_ENABLE, 0.0f);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, s.get(VOXP4_PARAM_DRIVE_ENABLE));
}

// --- Mode / switch gating --------------------------------------------------
void test_harmony_mode_gating(void) {
    const UiParamMeta& interval = meta(VOXP4_PARAM_HARMONY_INTERVAL);
    const UiParamMeta& degree = meta(VOXP4_PARAM_HARMONY_VOICE1_DEGREE);
    const UiParamMeta& key = meta(VOXP4_PARAM_HARMONY_KEY);
    const UiParamMeta& minMidi = meta(VOXP4_PARAM_HARMONY_VOICE1_MIN_MIDI);
    const UiParamMeta& level = meta(VOXP4_PARAM_HARMONY_LEVEL);

    TEST_ASSERT_TRUE(ui_meta_applies(interval, 0, g_state));
    TEST_ASSERT_FALSE(ui_meta_applies(degree, 0, g_state));
    TEST_ASSERT_FALSE(ui_meta_applies(interval, 1, g_state));
    TEST_ASSERT_TRUE(ui_meta_applies(degree, 1, g_state));
    TEST_ASSERT_TRUE(ui_meta_applies(key, 1, g_state));
    TEST_ASSERT_FALSE(ui_meta_applies(degree, 2, g_state));
    TEST_ASSERT_TRUE(ui_meta_applies(minMidi, 2, g_state));
    TEST_ASSERT_FALSE(ui_meta_applies(minMidi, 0, g_state));
    // Shared voice parameter applies in every mode.
    TEST_ASSERT_TRUE(ui_meta_applies(level, 0, g_state));
    TEST_ASSERT_TRUE(ui_meta_applies(level, 1, g_state));
    TEST_ASSERT_TRUE(ui_meta_applies(level, 2, g_state));
}

void test_delay_sync_gating(void) {
    UiParamState s;
    s.resetToDefaults();
    const UiParamMeta& leftMs = meta(VOXP4_PARAM_DELAY_LEFT_MS);
    const UiParamMeta& leftDiv = meta(VOXP4_PARAM_DELAY_LEFT_SUBDIVISION);

    TEST_ASSERT_FALSE(s.get(VOXP4_PARAM_DELAY_SYNC_ENABLE) >= 0.5f);
    TEST_ASSERT_TRUE(ui_meta_applies(leftMs, 0, s));
    TEST_ASSERT_FALSE(ui_meta_applies(leftDiv, 0, s));

    s.setLocal(VOXP4_PARAM_DELAY_SYNC_ENABLE, 1.0f);
    TEST_ASSERT_FALSE(ui_meta_applies(leftMs, 0, s));
    TEST_ASSERT_TRUE(ui_meta_applies(leftDiv, 0, s));
}

void test_chorus_sync_and_mode_gating(void) {
    UiParamState s;
    s.resetToDefaults();
    const UiParamMeta& rate = meta(VOXP4_PARAM_CHORUS_RATE_HZ);
    const UiParamMeta& div = meta(VOXP4_PARAM_CHORUS_SUBDIVISION);
    const UiParamMeta& depth = meta(VOXP4_PARAM_CHORUS_DEPTH_MS);
    const UiParamMeta& width = meta(VOXP4_PARAM_CHORUS_WIDTH);
    const UiParamMeta& left = meta(VOXP4_PARAM_CHORUS_MICROSHIFT_LEFT_CENTS);

    // CHORUS (0), sync off -> rate + depth + width; microshift hidden.
    TEST_ASSERT_TRUE(ui_meta_applies(rate, 0, s));
    TEST_ASSERT_FALSE(ui_meta_applies(div, 0, s));
    TEST_ASSERT_TRUE(ui_meta_applies(depth, 0, s));
    TEST_ASSERT_TRUE(ui_meta_applies(width, 0, s));
    TEST_ASSERT_FALSE(ui_meta_applies(left, 0, s));

    // Sync on -> division instead of rate.
    s.setLocal(VOXP4_PARAM_CHORUS_SYNC_ENABLE, 1.0f);
    TEST_ASSERT_FALSE(ui_meta_applies(rate, 0, s));
    TEST_ASSERT_TRUE(ui_meta_applies(div, 0, s));

    // ENSEMBLE (1) and DIMENSION (2) still use rate/depth/base delay.
    TEST_ASSERT_TRUE(ui_meta_applies(depth, 1, s));
    TEST_ASSERT_TRUE(ui_meta_applies(depth, 2, s));
    TEST_ASSERT_FALSE(ui_meta_applies(left, 1, s));

    // MICROSHIFT (3) uses detune + width, not LFO.
    TEST_ASSERT_FALSE(ui_meta_applies(depth, 3, s));
    TEST_ASSERT_TRUE(ui_meta_applies(left, 3, s));
    TEST_ASSERT_TRUE(ui_meta_applies(width, 3, s));
}

// --- Enum labels -----------------------------------------------------------
void test_enum_labels(void) {
    TEST_ASSERT_EQUAL_STRING("WARM", label_of(VOXP4_PARAM_DRIVE_MODE, 0));
    TEST_ASSERT_EQUAL_STRING("MEGAPHONE", label_of(VOXP4_PARAM_DRIVE_MODE, 2));
    TEST_ASSERT_EQUAL_STRING("PARALLEL",
                             label_of(VOXP4_PARAM_OUTPUT_SPATIAL_ROUTING, 0));
    TEST_ASSERT_EQUAL_STRING("DLY>REV",
                             label_of(VOXP4_PARAM_OUTPUT_SPATIAL_ROUTING, 1));
    TEST_ASSERT_EQUAL_STRING("POST HARM",
                             label_of(VOXP4_PARAM_OUTPUT_SPATIAL_SOURCE, 2));
    TEST_ASSERT_EQUAL_STRING("1/8", label_of(VOXP4_PARAM_DELAY_LEFT_SUBDIVISION, 3));
    TEST_ASSERT_EQUAL_STRING("1/8.",
                             label_of(VOXP4_PARAM_DELAY_RIGHT_SUBDIVISION, 8));
}

void test_stepper_wrap(void) {
    TEST_ASSERT_EQUAL_INT(0, ui_step_index(11, 1, 12, true));
    TEST_ASSERT_EQUAL_INT(11, ui_step_index(0, -1, 12, true));
    TEST_ASSERT_EQUAL_INT(0, ui_step_index(0, -1, 12, false));
    TEST_ASSERT_EQUAL_INT(11, ui_step_index(11, 1, 12, false));
}

// --- Effect mapping / enables ----------------------------------------------
void test_effect_lookup_and_enable(void) {
    TEST_ASSERT_EQUAL_INT((int)UiEffectId::Harmony,
                          (int)ui_effect_of_wire(VOXP4_PARAM_HARMONY_VOICE1_PAN));
    TEST_ASSERT_EQUAL_INT((int)UiEffectId::Delay,
                          (int)ui_effect_of_wire(VOXP4_PARAM_DELAY_WET));
    TEST_ASSERT_EQUAL_INT((int)UiEffectId::Drive,
                          (int)ui_effect_of_wire(VOXP4_PARAM_DRIVE_MIX));
    TEST_ASSERT_EQUAL_INT((int)UiEffectId::Gate,
                          (int)ui_effect_of_wire(VOXP4_PARAM_GATE_THRESHOLD_DB));
    TEST_ASSERT_EQUAL_INT((int)UiEffectId::Compressor,
                          (int)ui_effect_of_wire(VOXP4_PARAM_COMPRESSOR_RATIO));
    TEST_ASSERT_EQUAL_INT((int)UiEffectId::Modulation,
                          (int)ui_effect_of_wire(VOXP4_PARAM_CHORUS_MIX));
    TEST_ASSERT_EQUAL_INT((int)UiEffectId::Reverb,
                          (int)ui_effect_of_wire(VOXP4_PARAM_REVERB_WET));

    TEST_ASSERT_EQUAL_HEX16(VOXP4_PARAM_HARMONY_ENABLE,
                            ui_effect_enable_wire(UiEffectId::Harmony));
    TEST_ASSERT_EQUAL_HEX16(VOXP4_PARAM_GATE_ENABLE,
                            ui_effect_enable_wire(UiEffectId::Gate));
    TEST_ASSERT_EQUAL_HEX16(VOXP4_PARAM_COMPRESSOR_ENABLE,
                            ui_effect_enable_wire(UiEffectId::Compressor));
    TEST_ASSERT_EQUAL_HEX16(VOXP4_PARAM_DRIVE_ENABLE,
                            ui_effect_enable_wire(UiEffectId::Drive));
    TEST_ASSERT_EQUAL_HEX16(VOXP4_PARAM_CHORUS_ENABLE,
                            ui_effect_enable_wire(UiEffectId::Modulation));
    TEST_ASSERT_EQUAL_HEX16(VOXP4_PARAM_DELAY_ENABLE,
                            ui_effect_enable_wire(UiEffectId::Delay));
    TEST_ASSERT_EQUAL_HEX16(VOXP4_PARAM_REVERB_ENABLE,
                            ui_effect_enable_wire(UiEffectId::Reverb));

    // The harmony bus limiter is a Harmony Advanced parameter, never the master
    // ceiling.
    TEST_ASSERT_FALSE(ui_is_effect_enable(VOXP4_PARAM_LIMITER_CEILING));
    TEST_ASSERT_EQUAL_INT((int)UiEffectId::Harmony,
                          (int)ui_effect_of_wire(VOXP4_PARAM_HARMONY_LIMITER_ENABLE));
}

// --- UI coverage of all canonical parameters -------------------------------
void test_ui_coverage_all_parameters(void) {
    bool seen[VOXP4_PARAM_COUNT];
    for (size_t i = 0; i < VOXP4_PARAM_COUNT; ++i) seen[i] = false;

    auto mark = [&](uint16_t wireId) {
        const int idx = ParameterRegistry::wireIdToDenseIndex(wireId);
        TEST_ASSERT_TRUE(idx >= 0);
        TEST_ASSERT_FALSE_MESSAGE(seen[idx], "duplicate UI placement for parameter");
        seen[idx] = true;
    };

    // Effect enables are represented as the editor/card header toggle.
    for (size_t e = 0; e < kUiEffectCount; ++e) {
        mark(ui_effect_enable_wire(static_cast<UiEffectId>(e)));
    }
    // Effect descriptors.
    for (size_t e = 0; e < kUiEffectCount; ++e) {
        size_t count = 0;
        const UiParamMeta* table =
            ui_effect_metadata(static_cast<UiEffectId>(e), &count);
        for (size_t i = 0; i < count; ++i) {
            mark(table[i].wireId);
            TEST_ASSERT_FALSE(ui_is_effect_enable(table[i].wireId));
            TEST_ASSERT_TRUE(table[i].visibility == UiVisibility::Basic ||
                             table[i].visibility == UiVisibility::Advanced);
        }
    }
    // Master / global descriptors.
    size_t mcount = 0;
    const UiParamMeta* master = ui_master_metadata(&mcount);
    for (size_t i = 0; i < mcount; ++i) {
        mark(master[i].wireId);
        TEST_ASSERT_TRUE(master[i].visibility == UiVisibility::Global);
    }

    // Every canonical parameter must have exactly one UI home.
    for (size_t i = 0; i < VOXP4_PARAM_COUNT; ++i) {
        TEST_ASSERT_TRUE_MESSAGE(seen[i], "canonical parameter has no UI placement");
    }
}

void test_basic_advanced_split(void) {
    // Gate/Compressor keep the default editor simple.
    size_t count = 0;
    const UiParamMeta* gate = ui_effect_metadata(UiEffectId::Gate, &count);
    int basic = 0, advanced = 0;
    for (size_t i = 0; i < count; ++i) {
        if (gate[i].visibility == UiVisibility::Basic) ++basic;
        if (gate[i].visibility == UiVisibility::Advanced) ++advanced;
    }
    TEST_ASSERT_EQUAL_INT(2, basic);
    TEST_ASSERT_EQUAL_INT(3, advanced);

    const UiParamMeta* comp = ui_effect_metadata(UiEffectId::Compressor, &count);
    basic = advanced = 0;
    for (size_t i = 0; i < count; ++i) {
        if (comp[i].visibility == UiVisibility::Basic) ++basic;
        if (comp[i].visibility == UiVisibility::Advanced) ++advanced;
    }
    TEST_ASSERT_EQUAL_INT(3, basic);
    TEST_ASSERT_EQUAL_INT(3, advanced);
}

// --- Summaries -------------------------------------------------------------
void test_summary_reverb(void) {
    g_state.setLocal(VOXP4_PARAM_REVERB_WET, 0.35f);
    g_state.setLocal(VOXP4_PARAM_REVERB_DECAY_S, 4.5f);
    char main[24], m[24];
    summary(UiEffectId::Reverb, main, sizeof(main), m, sizeof(m));
    TEST_ASSERT_EQUAL_STRING("35%", main);
    TEST_ASSERT_EQUAL_STRING("4.5 s", m);
}

void test_summary_delay_free(void) {
    g_state.setLocal(VOXP4_PARAM_DELAY_SYNC_ENABLE, 0.0f);
    g_state.setLocal(VOXP4_PARAM_DELAY_LEFT_MS, 420.0f);
    g_state.setLocal(VOXP4_PARAM_DELAY_RIGHT_MS, 510.0f);
    char main[24], m[24];
    summary(UiEffectId::Delay, main, sizeof(main), m, sizeof(m));
    TEST_ASSERT_EQUAL_STRING("420 ms", main);
    TEST_ASSERT_EQUAL_STRING("R 510", m);
}

void test_summary_delay_sync(void) {
    g_state.setLocal(VOXP4_PARAM_DELAY_SYNC_ENABLE, 1.0f);
    g_state.setLocal(VOXP4_PARAM_DELAY_LEFT_SUBDIVISION, 3.0f);   // 1/8
    g_state.setLocal(VOXP4_PARAM_DELAY_RIGHT_SUBDIVISION, 8.0f);  // 1/8.
    g_state.setLocal(VOXP4_PARAM_DELAY_FEEDBACK, 0.25f);
    char main[24], m[24];
    summary(UiEffectId::Delay, main, sizeof(main), m, sizeof(m));
    TEST_ASSERT_EQUAL_STRING("1/8 / 1/8.", main);
    TEST_ASSERT_EQUAL_STRING("25% FB", m);
}

void test_summary_harmony_modes(void) {
    char main[24], m[24];
    g_state.setLocal(VOXP4_PARAM_HARMONY_MODE, 0.0f);
    g_state.setLocal(VOXP4_PARAM_HARMONY_INTERVAL, 4.0f);
    summary(UiEffectId::Harmony, main, sizeof(main), m, sizeof(m));
    TEST_ASSERT_EQUAL_STRING("+4 st", main);
    TEST_ASSERT_EQUAL_STRING("FIXED", m);

    g_state.setLocal(VOXP4_PARAM_HARMONY_MODE, 1.0f);
    g_state.setLocal(VOXP4_PARAM_HARMONY_VOICE1_DEGREE, 2.0f);
    g_state.setLocal(VOXP4_PARAM_HARMONY_KEY, 2.0f);
    g_state.setLocal(VOXP4_PARAM_HARMONY_SCALE, 4.0f);
    summary(UiEffectId::Harmony, main, sizeof(main), m, sizeof(m));
    TEST_ASSERT_EQUAL_STRING("+2 deg", main);
    TEST_ASSERT_EQUAL_STRING("D DORIAN", m);

    g_state.setLocal(VOXP4_PARAM_HARMONY_MODE, 2.0f);
    summary(UiEffectId::Harmony, main, sizeof(main), m, sizeof(m));
    TEST_ASSERT_EQUAL_STRING("MIDI", main);
}

void test_summary_drive(void) {
    g_state.setLocal(VOXP4_PARAM_DRIVE_MODE, 0.0f);
    g_state.setLocal(VOXP4_PARAM_DRIVE_DRIVE, 0.4f);
    char main[24], m[24];
    summary(UiEffectId::Drive, main, sizeof(main), m, sizeof(m));
    TEST_ASSERT_EQUAL_STRING("WARM", main);
    TEST_ASSERT_EQUAL_STRING("40%", m);
}

void test_summary_compressor_gate(void) {
    g_state.setLocal(VOXP4_PARAM_COMPRESSOR_RATIO, 3.0f);
    g_state.setLocal(VOXP4_PARAM_COMPRESSOR_THRESHOLD_DB, -18.0f);
    char main[24], m[24];
    summary(UiEffectId::Compressor, main, sizeof(main), m, sizeof(m));
    TEST_ASSERT_EQUAL_STRING("3:1", main);
    TEST_ASSERT_EQUAL_STRING("-18.0 dB", m);

    g_state.setLocal(VOXP4_PARAM_GATE_THRESHOLD_DB, -55.0f);
    summary(UiEffectId::Gate, main, sizeof(main), m, sizeof(m));
    TEST_ASSERT_EQUAL_STRING("-55.0 dB", main);
}

void test_summary_modulation(void) {
    char main[24], m[24];
    g_state.setLocal(VOXP4_PARAM_CHORUS_MODE, 0.0f);
    g_state.setLocal(VOXP4_PARAM_CHORUS_MIX, 0.3f);
    summary(UiEffectId::Modulation, main, sizeof(main), m, sizeof(m));
    TEST_ASSERT_EQUAL_STRING("30%", main);
    TEST_ASSERT_EQUAL_STRING("CHORUS", m);

    g_state.setLocal(VOXP4_PARAM_CHORUS_MODE, 3.0f);
    g_state.setLocal(VOXP4_PARAM_CHORUS_MICROSHIFT_LEFT_CENTS, -7.0f);
    g_state.setLocal(VOXP4_PARAM_CHORUS_MICROSHIFT_RIGHT_CENTS, 9.0f);
    summary(UiEffectId::Modulation, main, sizeof(main), m, sizeof(m));
    TEST_ASSERT_EQUAL_STRING("30%", main);
    TEST_ASSERT_EQUAL_STRING("-7/+9 c", m);
}

// --- Generated contract ----------------------------------------------------
void test_generated_constants(void) {
    TEST_ASSERT_EQUAL_INT(71, (int)VOXP4_PARAM_COUNT);
    TEST_ASSERT_EQUAL_INT(71, (int)ParameterRegistry::kParamCount);
    TEST_ASSERT_EQUAL_HEX16(0x10u, VOXP4_VOXLINK_VERSION);
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_percent_format);
    RUN_TEST(test_db_format);
    RUN_TEST(test_pan_format);
    RUN_TEST(test_semitone_format);
    RUN_TEST(test_seconds_format);
    RUN_TEST(test_hertz_format);
    RUN_TEST(test_cents_format);
    RUN_TEST(test_ratio_format);
    RUN_TEST(test_bpm_format);
    RUN_TEST(test_midi_note_format);
    RUN_TEST(test_ui_ranges_match_registry);
    RUN_TEST(test_known_registry_defaults);
    RUN_TEST(test_clamp);
    RUN_TEST(test_uistep_quantization);
    RUN_TEST(test_state_local_and_authoritative);
    RUN_TEST(test_state_bool_clamp);
    RUN_TEST(test_harmony_mode_gating);
    RUN_TEST(test_delay_sync_gating);
    RUN_TEST(test_chorus_sync_and_mode_gating);
    RUN_TEST(test_enum_labels);
    RUN_TEST(test_stepper_wrap);
    RUN_TEST(test_effect_lookup_and_enable);
    RUN_TEST(test_ui_coverage_all_parameters);
    RUN_TEST(test_basic_advanced_split);
    RUN_TEST(test_summary_reverb);
    RUN_TEST(test_summary_delay_free);
    RUN_TEST(test_summary_delay_sync);
    RUN_TEST(test_summary_harmony_modes);
    RUN_TEST(test_summary_drive);
    RUN_TEST(test_summary_compressor_gate);
    RUN_TEST(test_summary_modulation);
    RUN_TEST(test_generated_constants);
    return UNITY_END();
}
