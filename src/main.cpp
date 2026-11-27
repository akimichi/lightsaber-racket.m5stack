// M2: Grove 給電で LED テープを点灯させ、R1 (データ信号レベル) と R2 (電流・電圧降下) を確認する。
// 消灯 → 赤 → 緑 → 青 → 白 → 流れる点 を繰り返す。
#include <M5Unified.h>

#include <FastLED.h>

#include "config.h"

namespace {

enum class TestStep : uint8_t { Off, Red, Green, Blue, White, Chase, Count };

const char* const kStepNames[] = {"OFF", "RED", "GREEN", "BLUE", "WHITE", "CHASE"};

CRGB leds[config::kLedCount];
TestStep step = TestStep::Off;
uint32_t stepStartMs = 0;
uint32_t lastFrameMs = 0;
uint32_t lastDisplayMs = 0;
uint16_t chasePos = 0;

void applyStep() {
  switch (step) {
    case TestStep::Off:   fill_solid(leds, config::kLedCount, CRGB::Black); break;
    case TestStep::Red:   fill_solid(leds, config::kLedCount, CRGB::Red);   break;
    case TestStep::Green: fill_solid(leds, config::kLedCount, CRGB::Green); break;
    case TestStep::Blue:  fill_solid(leds, config::kLedCount, CRGB::Blue);  break;
    case TestStep::White: fill_solid(leds, config::kLedCount, CRGB::White); break;
    case TestStep::Chase:
      fill_solid(leds, config::kLedCount, CRGB::Black);
      leds[chasePos] = CRGB::White;
      break;
    default: break;
  }
  FastLED.show();
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
  const bool extOk = M5.Power.setExtOutput(true);
  Serial.printf("setExtOutput: %s\n", extOk ? "ok" : "failed");

  stepStartMs = millis();
  applyStep();
}

void loop() {
  M5.update();
  const uint32_t now = millis();

  if (now - stepStartMs >= config::kTestStepMs) {
    stepStartMs = now;
    step = static_cast<TestStep>((static_cast<uint8_t>(step) + 1) %
                                 static_cast<uint8_t>(TestStep::Count));
    chasePos = 0;
    applyStep();
  } else if (step == TestStep::Chase && now - lastFrameMs >= config::kTestFrameMs) {
    lastFrameMs = now;
    chasePos = (chasePos + 1) % config::kLedCount;
    applyStep();
  }

  if (now - lastDisplayMs >= config::kDisplayIntervalMs) {
    lastDisplayMs = now;
    const char* name = kStepNames[static_cast<uint8_t>(step)];
    const float extMv = M5.Power.getExtVoltage(m5::ext_MAIN);
    const int batMv = M5.Power.getBatteryVoltage();
    const int batMa = M5.Power.getBatteryCurrent();
    const int bright = FastLED.getBrightness();
    M5.Display.setCursor(0, 0);
    M5.Display.printf("M2 LED test\n%-6s\n5V %4.0f mV\nbat %4d mV\n    %4d mA\n",
                      name, extMv, batMv, batMa);
    Serial.printf("%s,ext_mV=%.0f,bat_mV=%d,bat_mA=%d,brightness=%d\n",
                  name, extMv, batMv, batMa, bright);
  }
}
