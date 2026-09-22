// Host-side tests for the pure TapTempo estimator.
#include <unity.h>

#include <cmath>

#include "model/TapTempo.h"

static bool approx(float a, float b, float eps = 0.5f) {
    return std::fabs(a - b) <= eps;
}

void setUp(void) {}
void tearDown(void) {}

void test_two_taps(void) {
    TapTempo t;
    float bpm = 0.0f;
    TEST_ASSERT_FALSE(t.tap(0, &bpm));
    TEST_ASSERT_TRUE(t.tap(500, &bpm));
    TEST_ASSERT_TRUE(approx(bpm, 120.0f));
    TEST_ASSERT_TRUE(t.hasEstimate());
}

void test_median_of_recent_intervals(void) {
    TapTempo t;
    float bpm = 0.0f;
    t.tap(0, &bpm);
    t.tap(500, &bpm);   // 500
    t.tap(1010, &bpm);  // 510
    t.tap(1500, &bpm);  // 490
    // Median interval 500 -> 120 BPM despite jitter.
    TEST_ASSERT_TRUE(t.hasEstimate());
    TEST_ASSERT_TRUE(approx(t.bpm(), 120.0f));
}

void test_invalid_interval_rejected(void) {
    TapTempo t;
    float bpm = 0.0f;
    t.reset();
    TEST_ASSERT_FALSE(t.tap(0, &bpm));
    // 100 ms (>300 BPM) is outside the accepted window.
    TEST_ASSERT_FALSE(t.tap(100, &bpm));
    // 2400 ms (<30 BPM) is also rejected.
    TEST_ASSERT_FALSE(t.tap(2500, &bpm));
}

void test_timeout_resets_sequence(void) {
    TapTempo t;
    float bpm = 0.0f;
    t.tap(0, &bpm);
    t.tap(500, &bpm);
    TEST_ASSERT_TRUE(approx(bpm, 120.0f));

    // A tap after a long idle period starts a new sequence: no estimate yet.
    TEST_ASSERT_FALSE(t.tap(5000, &bpm));
    TEST_ASSERT_TRUE(t.tap(5500, &bpm));
    TEST_ASSERT_TRUE(approx(bpm, 120.0f));
}

void test_bpm_clamped_to_range(void) {
    TapTempo t;
    float bpm = 0.0f;
    // Intervals at the edge of the accepted window map to the clamp bounds.
    t.tap(0, &bpm);
    t.tap(200, &bpm);
    TEST_ASSERT_TRUE(approx(bpm, 300.0f));
    t.reset();
    t.tap(0, &bpm);
    t.tap(2000, &bpm);
    TEST_ASSERT_TRUE(approx(bpm, 30.0f));
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_two_taps);
    RUN_TEST(test_median_of_recent_intervals);
    RUN_TEST(test_invalid_interval_rejected);
    RUN_TEST(test_timeout_resets_sequence);
    RUN_TEST(test_bpm_clamped_to_range);
    return UNITY_END();
}
