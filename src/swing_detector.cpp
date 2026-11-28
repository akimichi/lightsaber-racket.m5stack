#include "swing_detector.h"

#include <math.h>

SwingDetector::SwingDetector(const SwingParams& params) : params_(params) {}

SwingState SwingDetector::update(float gyroDps, uint32_t nowMs) {
  // NaN と負値は 0 として扱う (NaN は比較が偽になる)。
  const float gyro = gyroDps > 0.0f ? gyroDps : 0.0f;

  // 前回からの経過時間ぶんだけ発光レベルを減衰させる。符号なし減算で millis の折り返しに耐える。
  if (hasLast_) {
    const uint32_t dt = nowMs - lastMs_;
    if (params_.decayHalfLifeMs == 0) {
      levelDps_ = 0.0f;
    } else if (dt > 0) {
      levelDps_ *= powf(0.5f, static_cast<float>(dt) /
                                  static_cast<float>(params_.decayHalfLifeMs));
    }
  }
  lastMs_ = nowMs;
  hasLast_ = true;
  if (gyro > levelDps_) {
    levelDps_ = gyro;
  }

  const float endDps = params_.endDps < params_.startDps ? params_.endDps : params_.startDps;
  if (gyro >= params_.startDps) {
    if (state_ == SwingState::Idle) {
      peakDps_ = 0.0f;  // 新しいスウィング
    }
    state_ = SwingState::Swing;
    if (gyro > peakDps_) {
      peakDps_ = gyro;
    }
  } else if (state_ != SwingState::Idle && levelDps_ >= endDps) {
    state_ = SwingState::Afterglow;
  } else {
    state_ = SwingState::Idle;
  }
  return state_;
}
