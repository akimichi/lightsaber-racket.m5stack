// M3: ジャイロの角速度でスウィングを検出し、LED の色を変える。
#include <M5Unified.h>

#include <FastLED.h>
#include <math.h>

#include "config.h"
#include "led_effects.h"
#include "swing_detector.h"

namespace {

CRGB leds[config::kLedCount];
SwingDetector detector(config::kSwing);
uint32_t lastFrameMs = 0;
uint32_t lastDisplayMs = 0;
bool displayDirty = true;

const char* stateName(SwingState state) {
  switch (state) {
    case SwingState::Idle:      return "IDLE";
    case SwingState::Swing:     return "SWING";
    case SwingState::Afterglow: return "AFTERGLOW";
  }
  return "?";
}

Hsv currentColor(uint32_t now) {
  if (detector.state() == SwingState::Idle) {
    return idleColor(now, config::kIdleColor);
  }
  return swingColor(detector.levelDps(), config::kSwingColor);
}

// 画面の状態表示は IDLE / SWING の 2 種類。AFTERGLOW は SWING の続きとして扱う。
void drawState() {
  M5.Display.setCursor(0, 0);
  M5.Display.printf("%-5s\n", detector.state() == SwingState::Idle ? "IDLE" : "SWING");
}

void drawAll() {
  drawState();
  M5.Display.printf("peak %4.0f dps\nbat %3d %%\n", detector.peakDps(),
                    static_cast<int>(M5.Power.getBatteryLevel()));
}

}  // namespace

void setup() {
  auto cfg = M5.config();
  cfg.serial_baudrate = 115200;  // 既定値 0 では Serial.begin が呼ばれない
  M5.begin(cfg);

  M5.Display.setRotation(1);
  M5.Display.setTextSize(2);
  M5.Display.fillScreen(TFT_BLACK);

  FastLED.addLeds<WS2812B, config::kLedDataPin, GRB>(leds, config::kLedCount);
  FastLED.setMaxPowerInVoltsAndMilliamps(config::kLedSupplyVolts, config::kLedMaxMilliamps);
  FastLED.clear(true);

  // EXT_5V を出力にする。この状態で Grove 側から給電してはならない。
  if (!M5.Power.setExtOutput(true)) {
    Serial.println("setExtOutput failed");
  }
}

void loop() {
  M5.update();
  const uint32_t now = millis();

  if (M5.Imu.update() & m5::IMU_Class::sensor_mask_gyro) {
    const auto data = M5.Imu.getImuData();
    const float gyroDps = sqrtf(data.gyro.x * data.gyro.x + data.gyro.y * data.gyro.y +
                    data.gyro.z * data.gyro.z);
    const SwingState before = detector.state();
    const SwingState after = detector.update(gyroDps, now);
    if (after != before) {
      Serial.printf("%lu,%s,peak=%.0f\n", static_cast<unsigned long>(now), stateName(after),
                    detector.peakDps());
      if (after == SwingState::Idle) {
        displayDirty = true;
      } else if (before == SwingState::Idle) {
        drawState();
      }
    }
  }

  if (now - lastFrameMs >= config::kLedFrameMs) {
    lastFrameMs = now;
    const Hsv c = currentColor(now);
    fill_solid(leds, config::kLedCount, CHSV(c.hue, c.sat, c.val));
    FastLED.show();
  }

  if (detector.state() == SwingState::Idle &&
      (displayDirty || now - lastDisplayMs >= config::kDisplayRefreshMs)) {
    displayDirty = false;
    lastDisplayMs = now;
    drawAll();
  }
}
