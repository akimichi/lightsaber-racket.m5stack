// ピン・閾値・色設定を集約する (docs/HANDOFF.md 5.2 節)。
// 現時点では M1 (画面表示と IMU 値のシリアル出力) に必要な値のみ定義する。
#pragma once

#include <stdint.h>

namespace config {

// IMU 値をシリアル・画面へ出力する周期 [ms]
constexpr uint32_t kSerialIntervalMs = 20;    // 50Hz
constexpr uint32_t kDisplayIntervalMs = 200;  // 5Hz

}  // namespace config
