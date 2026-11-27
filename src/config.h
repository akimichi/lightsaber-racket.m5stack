// ピン・閾値・色設定を集約する (docs/HANDOFF.md 5.2 節)。
#pragma once

#include <stdint.h>

namespace config {

// --- LED テープ ---
constexpr uint8_t kLedDataPin = 9;        // Grove 黄線 (G9)
constexpr uint16_t kLedCount = 144;       // 手元のテープの実数
constexpr uint8_t kLedSupplyVolts = 5;
constexpr uint32_t kLedMaxMilliamps = 300;  // Grove 5V 出力の上限 0.38A に対する余裕。変更は実測後のみ

// --- M2: LED 点灯テスト ---
constexpr uint32_t kTestStepMs = 3000;      // 1 パターンの表示時間
constexpr uint32_t kTestFrameMs = 20;       // 流れる点の更新周期
constexpr uint32_t kDisplayIntervalMs = 500;

}  // namespace config
