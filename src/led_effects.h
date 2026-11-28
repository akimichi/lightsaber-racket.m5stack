// 発光色の計算 (docs/HANDOFF.md 5.1・5.2 節)。ハードウェアに依存しない。
// 色相は FastLED と同じ 0〜255 表現 (160=青, 192=紫, 255≒赤)。
#pragma once

#include <stdint.h>

struct Hsv {
  uint8_t hue;
  uint8_t sat;
  uint8_t val;
};

struct SwingColorParams {
  float startDps;  // この角速度で hueStart / valMin
  float maxDps;    // この角速度以上で hueEnd / valMax
  uint8_t hueStart;
  uint8_t hueEnd;
  uint8_t valMin;
  uint8_t valMax;
};

struct IdleColorParams {
  uint8_t hue;
  uint8_t valMin;
  uint8_t valMax;
  uint32_t periodMs;  // 明滅の周期
};

// 角速度に応じたスウィング中の色。範囲外は両端に丸める。
Hsv swingColor(float gyroDps, const SwingColorParams& params);

// 待機中の色。valMin → valMax → valMin を periodMs 周期で往復する。
Hsv idleColor(uint32_t nowMs, const IdleColorParams& params);
