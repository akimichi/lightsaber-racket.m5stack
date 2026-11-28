#include <math.h>
#include <unity.h>

#include "led_effects.h"

namespace {
// 青(160) → 赤(255)、輝度 64 → 255
const SwingColorParams kSwing = {200.0f, 2000.0f, 160, 255, 64, 255};
const IdleColorParams kIdle = {160, 8, 40, 3000};

void assertHsv(uint8_t hue, uint8_t sat, uint8_t val, const Hsv& actual) {
  TEST_ASSERT_EQUAL_UINT8(hue, actual.hue);
  TEST_ASSERT_EQUAL_UINT8(sat, actual.sat);
  TEST_ASSERT_EQUAL_UINT8(val, actual.val);
}
}  // namespace

void setUp() {}
void tearDown() {}

void test_swing_color_at_start_threshold_is_start_hue_and_min_value() {
  assertHsv(160, 255, 64, swingColor(200.0f, kSwing));
}

void test_swing_color_at_max_is_end_hue_and_max_value() {
  assertHsv(255, 255, 255, swingColor(2000.0f, kSwing));
}

void test_swing_color_at_midpoint_is_linear() {
  // t = 0.5: hue 160 + 95*0.5 = 207.5 → 208、val 64 + 191*0.5 = 159.5 → 160
  assertHsv(208, 255, 160, swingColor(1100.0f, kSwing));
}

void test_swing_color_below_start_clamps_to_start() {
  assertHsv(160, 255, 64, swingColor(0.0f, kSwing));
  assertHsv(160, 255, 64, swingColor(-100.0f, kSwing));
}

void test_swing_color_above_max_clamps_to_end() {
  assertHsv(255, 255, 255, swingColor(5000.0f, kSwing));
}

void test_swing_color_nan_is_treated_as_start() {
  assertHsv(160, 255, 64, swingColor(NAN, kSwing));
}

void test_swing_color_with_degenerate_range_does_not_divide_by_zero() {
  const SwingColorParams same = {200.0f, 200.0f, 160, 255, 64, 255};
  assertHsv(160, 255, 64, swingColor(199.0f, same));
  assertHsv(255, 255, 255, swingColor(200.0f, same));
}

void test_swing_color_supports_decreasing_hue() {
  const SwingColorParams down = {0.0f, 100.0f, 200, 100, 0, 255};
  assertHsv(150, 255, 128, swingColor(50.0f, down));
}

void test_idle_color_breathes_between_min_and_max() {
  assertHsv(160, 255, 8, idleColor(0, kIdle));
  assertHsv(160, 255, 24, idleColor(750, kIdle));   // 上りの中間
  assertHsv(160, 255, 40, idleColor(1500, kIdle));  // 頂点
  assertHsv(160, 255, 24, idleColor(2250, kIdle));  // 下りの中間
  assertHsv(160, 255, 8, idleColor(3000, kIdle));   // 1 周して最小に戻る
}

void test_idle_color_with_zero_period_stays_at_min() {
  const IdleColorParams still = {160, 8, 40, 0};
  assertHsv(160, 255, 8, idleColor(12345, still));
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_swing_color_at_start_threshold_is_start_hue_and_min_value);
  RUN_TEST(test_swing_color_at_max_is_end_hue_and_max_value);
  RUN_TEST(test_swing_color_at_midpoint_is_linear);
  RUN_TEST(test_swing_color_below_start_clamps_to_start);
  RUN_TEST(test_swing_color_above_max_clamps_to_end);
  RUN_TEST(test_swing_color_nan_is_treated_as_start);
  RUN_TEST(test_swing_color_with_degenerate_range_does_not_divide_by_zero);
  RUN_TEST(test_swing_color_supports_decreasing_hue);
  RUN_TEST(test_idle_color_breathes_between_min_and_max);
  RUN_TEST(test_idle_color_with_zero_period_stays_at_min);
  return UNITY_END();
}
