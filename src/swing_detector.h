// 角速度の大きさからスウィングの状態と発光レベルを求める (docs/HANDOFF.md 5.1 節)。
// ハードウェアに依存しない。IMPACT は M4 で追加する。
//
// 発光レベルは「ピーク保持 + 減衰」で決める。角速度が上がれば即座に追従し、
// 下がるときは半減期 decayHalfLifeMs で指数的に減衰する。
#pragma once

#include <stdint.h>

enum class SwingState : uint8_t {
  Idle,       // 発光していない
  Swing,      // 角速度が開始閾値以上
  Afterglow,  // 角速度は開始閾値未満だが、発光レベルが減衰中で終了閾値以上
};

struct SwingParams {
  float startDps;             // 角速度がこの値以上で Swing に入る
  float endDps;               // 発光レベルがこの値未満で Idle に戻る。startDps より大きい値は startDps として扱う
  uint32_t decayHalfLifeMs;   // 発光レベルの半減期。0 なら保持せず即座に追従する
};

class SwingDetector {
 public:
  explicit SwingDetector(const SwingParams& params);

  // 新しい角速度サンプルで更新し、更新後の状態を返す。
  SwingState update(float gyroDps, uint32_t nowMs);

  SwingState state() const { return state_; }

  // 発光レベル [dps]。色の計算にはこの値を使う。
  float levelDps() const { return levelDps_; }

  // 現在の、または直近のスウィングのピーク角速度。一度もスウィングしていなければ 0。
  float peakDps() const { return peakDps_; }

 private:
  SwingParams params_;
  SwingState state_ = SwingState::Idle;
  float levelDps_ = 0.0f;
  float peakDps_ = 0.0f;
  uint32_t lastMs_ = 0;
  bool hasLast_ = false;
};
