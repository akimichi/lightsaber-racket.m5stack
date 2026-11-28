#include "led_effects.h"

namespace {

// NaN と 0 未満は 0、1 超は 1 に丸める。
float clamp01(float t) {
  if (!(t > 0.0f)) {
    return 0.0f;
  }
  return t > 1.0f ? 1.0f : t;
}

uint8_t lerp8(uint8_t from, uint8_t to, float t) {
  const float v = static_cast<float>(from) +
                  (static_cast<float>(to) - static_cast<float>(from)) * t;
  return static_cast<uint8_t>(v + 0.5f);
}

}  // namespace

Hsv swingColor(float gyroDps, const SwingColorParams& params) {
  float t;
  if (params.maxDps > params.startDps) {
    t = clamp01((gyroDps - params.startDps) / (params.maxDps - params.startDps));
  } else {
    t = gyroDps >= params.maxDps ? 1.0f : 0.0f;
  }
  return {lerp8(params.hueStart, params.hueEnd, t), 255,
          lerp8(params.valMin, params.valMax, t)};
}

Hsv idleColor(uint32_t nowMs, const IdleColorParams& params) {
  if (params.periodMs == 0) {
    return {params.hue, 255, params.valMin};
  }
  const float phase = static_cast<float>(nowMs % params.periodMs) /
                      static_cast<float>(params.periodMs);
  const float t = phase < 0.5f ? phase * 2.0f : (1.0f - phase) * 2.0f;  // 三角波
  return {params.hue, 255, lerp8(params.valMin, params.valMax, t)};
}
