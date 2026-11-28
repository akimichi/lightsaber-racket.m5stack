#include <math.h>
#include <unity.h>

#include "swing_detector.h"

namespace {
// 開始閾値 = 終了閾値 = 200dps (ヒステリシスなし)、半減期 150ms
const SwingParams kParams = {200.0f, 200.0f, 150};
// 開始閾値 200dps、終了閾値 100dps、半減期 150ms
const SwingParams kHysteresis = {200.0f, 100.0f, 150};
}

void setUp() {}
void tearDown() {}

void test_initial_state_is_idle_with_zero_level_and_peak() {
  SwingDetector d(kParams);
  TEST_ASSERT_EQUAL(SwingState::Idle, d.state());
  TEST_ASSERT_EQUAL_FLOAT(0.0f, d.levelDps());
  TEST_ASSERT_EQUAL_FLOAT(0.0f, d.peakDps());
}

void test_below_threshold_stays_idle_and_does_not_set_peak() {
  SwingDetector d(kParams);
  TEST_ASSERT_EQUAL(SwingState::Idle, d.update(199.9f, 0));
  TEST_ASSERT_EQUAL_FLOAT(199.9f, d.levelDps());
  TEST_ASSERT_EQUAL_FLOAT(0.0f, d.peakDps());
}

void test_exactly_threshold_enters_swing() {
  SwingDetector d(kParams);
  TEST_ASSERT_EQUAL(SwingState::Swing, d.update(200.0f, 0));
  TEST_ASSERT_EQUAL_FLOAT(200.0f, d.levelDps());
  TEST_ASSERT_EQUAL_FLOAT(200.0f, d.peakDps());
}

void test_level_follows_rising_gyro_immediately() {
  SwingDetector d(kParams);
  d.update(300.0f, 0);
  d.update(900.0f, 5);
  TEST_ASSERT_EQUAL_FLOAT(900.0f, d.levelDps());
  TEST_ASSERT_EQUAL_FLOAT(900.0f, d.peakDps());
}

void test_level_halves_after_one_half_life_when_gyro_drops() {
  SwingDetector d(kParams);
  d.update(800.0f, 0);
  TEST_ASSERT_EQUAL(SwingState::Afterglow, d.update(0.0f, 150));
  TEST_ASSERT_EQUAL_FLOAT(400.0f, d.levelDps());
}

void test_decay_does_not_depend_on_sample_interval() {
  SwingDetector d(kParams);
  d.update(800.0f, 0);
  d.update(0.0f, 50);
  d.update(0.0f, 75);
  d.update(0.0f, 150);
  TEST_ASSERT_FLOAT_WITHIN(0.01f, 400.0f, d.levelDps());
}

void test_afterglow_ends_when_level_falls_below_threshold() {
  SwingDetector d(kParams);
  d.update(800.0f, 0);
  d.update(0.0f, 150);  // 400
  TEST_ASSERT_EQUAL(SwingState::Afterglow, d.update(0.0f, 300));  // ちょうど 200 = 閾値
  TEST_ASSERT_EQUAL_FLOAT(200.0f, d.levelDps());
  TEST_ASSERT_EQUAL(SwingState::Idle, d.update(0.0f, 301));  // 閾値をわずかに下回る
}

void test_gyro_between_threshold_and_decayed_level_keeps_swing_and_level() {
  SwingDetector d(kParams);
  d.update(800.0f, 0);
  TEST_ASSERT_EQUAL(SwingState::Swing, d.update(300.0f, 150));
  TEST_ASSERT_EQUAL_FLOAT(400.0f, d.levelDps());  // 減衰後のレベルのほうが高い
}

void test_gyro_above_decayed_level_overrides_level() {
  SwingDetector d(kParams);
  d.update(800.0f, 0);
  TEST_ASSERT_EQUAL(SwingState::Swing, d.update(600.0f, 150));
  TEST_ASSERT_EQUAL_FLOAT(600.0f, d.levelDps());
  TEST_ASSERT_EQUAL_FLOAT(800.0f, d.peakDps());
}

void test_peak_is_kept_after_returning_to_idle() {
  SwingDetector d(kParams);
  d.update(900.0f, 0);
  d.update(0.0f, 5000);
  TEST_ASSERT_EQUAL(SwingState::Idle, d.state());
  TEST_ASSERT_EQUAL_FLOAT(900.0f, d.peakDps());
}

void test_new_swing_from_idle_resets_peak() {
  SwingDetector d(kParams);
  d.update(900.0f, 0);
  d.update(0.0f, 5000);
  TEST_ASSERT_EQUAL(SwingState::Swing, d.update(300.0f, 5005));
  TEST_ASSERT_EQUAL_FLOAT(300.0f, d.peakDps());
}

void test_reentering_swing_during_afterglow_keeps_peak() {
  SwingDetector d(kParams);
  d.update(900.0f, 0);
  TEST_ASSERT_EQUAL(SwingState::Afterglow, d.update(100.0f, 10));
  TEST_ASSERT_EQUAL(SwingState::Swing, d.update(300.0f, 20));
  TEST_ASSERT_EQUAL_FLOAT(900.0f, d.peakDps());
}

void test_decay_survives_millis_wraparound() {
  SwingDetector d(kParams);
  const uint32_t start = 0xFFFFFFB5u;  // 折り返しの 75ms 前
  d.update(800.0f, start);
  TEST_ASSERT_EQUAL(SwingState::Afterglow, d.update(0.0f, start + 150u));  // 折り返し後
  TEST_ASSERT_EQUAL_FLOAT(400.0f, d.levelDps());
}

void test_zero_half_life_follows_gyro_without_hold() {
  SwingDetector d({200.0f, 200.0f, 0});
  d.update(800.0f, 0);
  TEST_ASSERT_EQUAL(SwingState::Idle, d.update(0.0f, 1));
  TEST_ASSERT_EQUAL_FLOAT(0.0f, d.levelDps());
}

void test_nan_and_negative_input_are_treated_as_zero() {
  SwingDetector d(kParams);
  TEST_ASSERT_EQUAL(SwingState::Idle, d.update(NAN, 0));
  TEST_ASSERT_EQUAL(SwingState::Idle, d.update(-500.0f, 5));
  TEST_ASSERT_EQUAL_FLOAT(0.0f, d.levelDps());
  TEST_ASSERT_EQUAL_FLOAT(0.0f, d.peakDps());
  d.update(800.0f, 10);
  TEST_ASSERT_EQUAL(SwingState::Afterglow, d.update(NAN, 160));
  TEST_ASSERT_EQUAL_FLOAT(400.0f, d.levelDps());
  TEST_ASSERT_EQUAL_FLOAT(800.0f, d.peakDps());
}

void test_hysteresis_keeps_afterglow_until_level_falls_below_end_threshold() {
  SwingDetector d(kHysteresis);
  d.update(800.0f, 0);
  TEST_ASSERT_EQUAL(SwingState::Afterglow, d.update(0.0f, 300));  // 200: 開始閾値ちょうど
  TEST_ASSERT_EQUAL(SwingState::Afterglow, d.update(0.0f, 301));  // 開始閾値未満でも継続
  TEST_ASSERT_EQUAL(SwingState::Afterglow, d.update(0.0f, 450));  // 100: 終了閾値ちょうど
  TEST_ASSERT_EQUAL_FLOAT(100.0f, d.levelDps());
  TEST_ASSERT_EQUAL(SwingState::Idle, d.update(0.0f, 451));       // 終了閾値未満
}

void test_hysteresis_does_not_start_swing_between_end_and_start_threshold() {
  SwingDetector d(kHysteresis);
  TEST_ASSERT_EQUAL(SwingState::Idle, d.update(150.0f, 0));
  TEST_ASSERT_EQUAL(SwingState::Idle, d.update(199.9f, 5));
  TEST_ASSERT_EQUAL_FLOAT(0.0f, d.peakDps());
}

void test_hysteresis_gyro_between_thresholds_sustains_afterglow_without_new_peak() {
  SwingDetector d(kHysteresis);
  d.update(250.0f, 0);
  // 角速度 150 は開始閾値未満だが、発光レベルを 150 に保つので Idle に落ちない。
  TEST_ASSERT_EQUAL(SwingState::Afterglow, d.update(150.0f, 1000));
  TEST_ASSERT_EQUAL_FLOAT(150.0f, d.levelDps());
  TEST_ASSERT_EQUAL(SwingState::Swing, d.update(210.0f, 1005));
  TEST_ASSERT_EQUAL_FLOAT(250.0f, d.peakDps());  // 同じスウィングの続きなのでピークを保つ
}

void test_end_threshold_above_start_is_treated_as_start_threshold() {
  SwingDetector d({200.0f, 500.0f, 150});
  d.update(800.0f, 0);
  TEST_ASSERT_EQUAL(SwingState::Afterglow, d.update(0.0f, 150));  // 400: 500 未満だが継続
  TEST_ASSERT_EQUAL(SwingState::Afterglow, d.update(0.0f, 300));  // 200
  TEST_ASSERT_EQUAL(SwingState::Idle, d.update(0.0f, 301));
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_initial_state_is_idle_with_zero_level_and_peak);
  RUN_TEST(test_below_threshold_stays_idle_and_does_not_set_peak);
  RUN_TEST(test_exactly_threshold_enters_swing);
  RUN_TEST(test_level_follows_rising_gyro_immediately);
  RUN_TEST(test_level_halves_after_one_half_life_when_gyro_drops);
  RUN_TEST(test_decay_does_not_depend_on_sample_interval);
  RUN_TEST(test_afterglow_ends_when_level_falls_below_threshold);
  RUN_TEST(test_gyro_between_threshold_and_decayed_level_keeps_swing_and_level);
  RUN_TEST(test_gyro_above_decayed_level_overrides_level);
  RUN_TEST(test_peak_is_kept_after_returning_to_idle);
  RUN_TEST(test_new_swing_from_idle_resets_peak);
  RUN_TEST(test_reentering_swing_during_afterglow_keeps_peak);
  RUN_TEST(test_decay_survives_millis_wraparound);
  RUN_TEST(test_zero_half_life_follows_gyro_without_hold);
  RUN_TEST(test_nan_and_negative_input_are_treated_as_zero);
  RUN_TEST(test_hysteresis_keeps_afterglow_until_level_falls_below_end_threshold);
  RUN_TEST(test_hysteresis_does_not_start_swing_between_end_and_start_threshold);
  RUN_TEST(test_hysteresis_gyro_between_thresholds_sustains_afterglow_without_new_peak);
  RUN_TEST(test_end_threshold_above_start_is_treated_as_start_threshold);
  return UNITY_END();
}
