// Host-side tests for the pure parameter model (no LVGL / Arduino).
#include <unity.h>

#include <cmath>
#include <cstring>

#include "ui/params/UiParamModel.h"

namespace {

float g_values[kUiParamCount];

const UiParamDescriptor& desc(UiParamId id) {
    const UiParamDescriptor* d = ui_param_descriptor(id);
    TEST_ASSERT_NOT_NULL(d);
    return *d;
}

void set(UiParamId id, float value) { g_values[(size_t)id] = value; }

void format_into(UiParamId id, float value, char* out, size_t size) {
    ui_format_parameter_value(desc(id), value, out, size);
}

void init_defaults() {
    for (size_t i = 0; i < kUiParamCount; i++) {
        const UiParamDescriptor* d = ui_param_descriptor((UiParamId)i);
        g_values[i] = d ? d->defaultValue : 0.0f;
    }
}

void summary(UiEffectId effect, char* main, size_t mainSize, char* meta,
             size_t metaSize) {
    ui_build_effect_summary(effect, g_values, main, mainSize, meta, metaSize);
}

} // namespace

void setUp(void) { init_defaults(); }
void tearDown(void) {}

// --- Formatters ------------------------------------------------------------
void test_percent_format(void) {
    char buf[24];
    format_into(UiParamId::ReverbWet, 0.18f, buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING("18%", buf);
    format_into(UiParamId::ReverbWet, 0.35f, buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING("35%", buf);
    format_into(UiParamId::ReverbWet, 1.0f, buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING("100%", buf);
}

void test_db_format(void) {
    char buf[24];
    format_into(UiParamId::LimiterThresholdDb, -3.0f, buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING("-3.0 dB", buf);
}

void test_pan_format(void) {
    char buf[24];
    format_into(UiParamId::HarmonyPan, 0.0f, buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING("C", buf);
    format_into(UiParamId::HarmonyPan, -0.25f, buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING("L 25", buf);
    format_into(UiParamId::HarmonyPan, 0.5f, buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING("R 50", buf);
}

void test_semitone_format(void) {
    char buf[24];
    format_into(UiParamId::HarmonyInterval, 4.0f, buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING("+4 st", buf);
    format_into(UiParamId::HarmonyInterval, -5.0f, buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING("-5 st", buf);
    format_into(UiParamId::HarmonyInterval, 0.0f, buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING("0 st", buf);
}

void test_seconds_format(void) {
    char buf[24];
    format_into(UiParamId::ReverbDecaySeconds, 2.0f, buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING("2.0 s", buf);
    format_into(UiParamId::ReverbDecaySeconds, 4.5f, buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING("4.5 s", buf);
}

void test_hertz_format(void) {
    char buf[24];
    format_into(UiParamId::DelayFeedbackLowpassHz, 6000.0f, buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING("6.0 kHz", buf);
    format_into(UiParamId::DelayFeedbackLowpassHz, 200.0f, buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING("200 Hz", buf);
}

// --- Stepper wrap ----------------------------------------------------------
void test_enum_wrap(void) {
    TEST_ASSERT_EQUAL_INT(0, ui_step_index(11, 1, 12, true));
    TEST_ASSERT_EQUAL_INT(11, ui_step_index(0, -1, 12, true));
    TEST_ASSERT_EQUAL_INT(0, ui_step_index(0, -1, 12, false));
    TEST_ASSERT_EQUAL_INT(11, ui_step_index(11, 1, 12, false));
}

// --- Clamping --------------------------------------------------------------
void test_clamp(void) {
    TEST_ASSERT_EQUAL_FLOAT(12.0f,
                            ui_clamp_parameter(desc(UiParamId::HarmonyInterval), 99.0f));
    TEST_ASSERT_EQUAL_FLOAT(-12.0f,
                            ui_clamp_parameter(desc(UiParamId::HarmonyInterval), -99.0f));
    TEST_ASSERT_EQUAL_FLOAT(1.0f,
                            ui_clamp_parameter(desc(UiParamId::ReverbWet), 2.0f));
    TEST_ASSERT_EQUAL_FLOAT(-7.0f,
                            ui_clamp_parameter(desc(UiParamId::HarmonyDegree), -99.0f));
}

// --- Mode-dependent descriptor selection -----------------------------------
void test_mode_descriptor_selection(void) {
    const UiParamDescriptor& interval = desc(UiParamId::HarmonyInterval);
    const UiParamDescriptor& degree = desc(UiParamId::HarmonyDegree);
    const UiParamDescriptor& key = desc(UiParamId::HarmonyKey);
    const UiParamDescriptor& level = desc(UiParamId::HarmonyLevel);

    // FIXED = 0
    TEST_ASSERT_TRUE(ui_descriptor_applies(interval, 0));
    TEST_ASSERT_FALSE(ui_descriptor_applies(degree, 0));
    TEST_ASSERT_FALSE(ui_descriptor_applies(key, 0));
    // DIATONIC = 1
    TEST_ASSERT_FALSE(ui_descriptor_applies(interval, 1));
    TEST_ASSERT_TRUE(ui_descriptor_applies(degree, 1));
    TEST_ASSERT_TRUE(ui_descriptor_applies(key, 1));
    // MIDI = 2
    TEST_ASSERT_FALSE(ui_descriptor_applies(interval, 2));
    TEST_ASSERT_FALSE(ui_descriptor_applies(degree, 2));
    // Shared voice parameter applies in every mode.
    TEST_ASSERT_TRUE(ui_descriptor_applies(level, 0));
    TEST_ASSERT_TRUE(ui_descriptor_applies(level, 1));
    TEST_ASSERT_TRUE(ui_descriptor_applies(level, 2));
}

void test_scale_options(void) {
    const UiParamDescriptor& scale = desc(UiParamId::HarmonyScale);
    TEST_ASSERT_EQUAL_INT(12, scale.optionCount);
    TEST_ASSERT_EQUAL_STRING("DORIAN", scale.options[4]);
    TEST_ASSERT_EQUAL_STRING("BLUES MIN", scale.options[11]);
}

void test_effect_lookup(void) {
    TEST_ASSERT_EQUAL_INT((int)UiEffectId::Harmony,
                          (int)ui_effect_of(UiParamId::HarmonyPan));
    TEST_ASSERT_EQUAL_INT((int)UiEffectId::Delay,
                          (int)ui_effect_of(UiParamId::DelayWet));
    TEST_ASSERT_EQUAL_INT((int)UiEffectId::Limiter,
                          (int)ui_effect_of(UiParamId::LimiterThresholdDb));
}

// --- Effect summaries ------------------------------------------------------
void test_summary_reverb(void) {
    set(UiParamId::ReverbWet, 0.35f);
    set(UiParamId::ReverbDecaySeconds, 4.5f);
    char main[24];
    char meta[24];
    summary(UiEffectId::Reverb, main, sizeof(main), meta, sizeof(meta));
    TEST_ASSERT_EQUAL_STRING("35%", main);
    TEST_ASSERT_EQUAL_STRING("4.5 s", meta);
}

void test_summary_delay(void) {
    set(UiParamId::DelayLeftMs, 420.0f);
    set(UiParamId::DelayRightMs, 510.0f);
    char main[24];
    char meta[24];
    summary(UiEffectId::Delay, main, sizeof(main), meta, sizeof(meta));
    TEST_ASSERT_EQUAL_STRING("420 ms", main);
    TEST_ASSERT_EQUAL_STRING("R 510", meta);
}

void test_summary_harmony_fixed(void) {
    set(UiParamId::HarmonyMode, 0.0f);
    set(UiParamId::HarmonyInterval, 4.0f);
    char main[24];
    char meta[24];
    summary(UiEffectId::Harmony, main, sizeof(main), meta, sizeof(meta));
    TEST_ASSERT_EQUAL_STRING("+4 st", main);
    TEST_ASSERT_EQUAL_STRING("FIXED", meta);
}

void test_summary_harmony_diatonic(void) {
    set(UiParamId::HarmonyMode, 1.0f);
    set(UiParamId::HarmonyDegree, 2.0f);
    set(UiParamId::HarmonyKey, 2.0f);   // D
    set(UiParamId::HarmonyScale, 4.0f); // Dorian
    char main[24];
    char meta[24];
    summary(UiEffectId::Harmony, main, sizeof(main), meta, sizeof(meta));
    TEST_ASSERT_EQUAL_STRING("+2 deg", main);
    TEST_ASSERT_EQUAL_STRING("D DORIAN", meta);
}

void test_summary_harmony_midi(void) {
    set(UiParamId::HarmonyMode, 2.0f);
    char main[24];
    char meta[24];
    summary(UiEffectId::Harmony, main, sizeof(main), meta, sizeof(meta));
    TEST_ASSERT_EQUAL_STRING("MIDI", main);
    TEST_ASSERT_EQUAL_STRING("CHORD", meta);
}

void test_summary_limiter(void) {
    set(UiParamId::LimiterThresholdDb, -3.0f);
    char main[24];
    char meta[24];
    summary(UiEffectId::Limiter, main, sizeof(main), meta, sizeof(meta));
    TEST_ASSERT_EQUAL_STRING("-3.0 dB", main);
    TEST_ASSERT_EQUAL_STRING("HARM BUS", meta);
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_percent_format);
    RUN_TEST(test_db_format);
    RUN_TEST(test_pan_format);
    RUN_TEST(test_semitone_format);
    RUN_TEST(test_seconds_format);
    RUN_TEST(test_hertz_format);
    RUN_TEST(test_enum_wrap);
    RUN_TEST(test_clamp);
    RUN_TEST(test_mode_descriptor_selection);
    RUN_TEST(test_scale_options);
    RUN_TEST(test_effect_lookup);
    RUN_TEST(test_summary_reverb);
    RUN_TEST(test_summary_delay);
    RUN_TEST(test_summary_harmony_fixed);
    RUN_TEST(test_summary_harmony_diatonic);
    RUN_TEST(test_summary_harmony_midi);
    RUN_TEST(test_summary_limiter);
    return UNITY_END();
}
