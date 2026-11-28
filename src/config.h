// ピン・閾値・色設定を集約する (docs/HANDOFF.md 5.2 節)。すべて実機で調整する。
#pragma once

#include <stdint.h>

#include "led_effects.h"
#include "swing_detector.h"

namespace config {

// --- LED テープ ---
constexpr uint8_t kLedDataPin = 9;          // Grove 黄線 (G9)
constexpr uint16_t kLedCount = 144;         // 手元のテープの実数
constexpr uint8_t kLedSupplyVolts = 5;
constexpr uint32_t kLedMaxMilliamps = 300;  // Grove 5V 出力の上限 0.38A に対する余裕。変更は実測後のみ
constexpr uint32_t kLedFrameMs = 20;        // LED の更新周期 (50Hz)

// --- スウィング検出 ---
constexpr float kSwingStartDps = 200.0f;  // スウィング開始閾値
constexpr float kSwingEndDps = 100.0f;    // 発光レベルがこれを下回ると待機に戻る (閾値付近のばたつき防止)
constexpr float kSwingMaxDps = 2000.0f;   // 最大色に達する角速度 (ジャイロのレンジ上限)
// 発光レベルは角速度の上昇に即座に追従し、下降時はこの半減期で減衰する。
constexpr SwingParams kSwing = {kSwingStartDps, kSwingEndDps, /*decayHalfLifeMs=*/400};

// --- 色 (色相は FastLED の 0〜255 表現: 160=青, 192=紫, 255≒赤) ---
constexpr uint8_t kHueBlue = 160;
constexpr uint8_t kHueRed = 255;
// 青 → 紫 → 赤。輝度も角速度とともに上げる。
constexpr SwingColorParams kSwingColor = {kSwingStartDps, kSwingMaxDps, kHueBlue, kHueRed,
                                          /*valMin=*/64, /*valMax=*/255};
// 暗い青でゆっくり明滅する。
constexpr IdleColorParams kIdleColor = {kHueBlue, /*valMin=*/8, /*valMax=*/40,
                                        /*periodMs=*/3000};

// --- 画面 ---
// 描画中は IMU サンプルを取りこぼすので、描画は IDLE との出入りのときと、IDLE 中の定期更新に限る。
constexpr uint32_t kDisplayRefreshMs = 5000;

}  // namespace config
