// M1: StickS3 で画面表示と IMU 値のシリアル出力を行う。
#include <M5Unified.h>

#include <math.h>

#include "config.h"

namespace {

uint32_t lastSerialMs = 0;
uint32_t lastDisplayMs = 0;

}  // namespace

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);

  M5.Display.setRotation(1);
  M5.Display.setTextSize(2);
  M5.Display.fillScreen(TFT_BLACK);

  if (!M5.Imu.isEnabled()) {
    M5.Display.println("IMU not found");
    Serial.println("IMU not found");
  }
  Serial.println("ax,ay,az,gx,gy,gz,gyro_mag");
}

void loop() {
  M5.update();
  if (!M5.Imu.update()) {
    return;
  }
  const auto data = M5.Imu.getImuData();
  const float gyroMag = sqrtf(data.gyro.x * data.gyro.x +
                              data.gyro.y * data.gyro.y +
                              data.gyro.z * data.gyro.z);

  const uint32_t now = millis();
  if (now - lastSerialMs >= config::kSerialIntervalMs) {
    lastSerialMs = now;
    Serial.printf("%.3f,%.3f,%.3f,%.1f,%.1f,%.1f,%.1f\n",
                  data.accel.x, data.accel.y, data.accel.z,
                  data.gyro.x, data.gyro.y, data.gyro.z, gyroMag);
  }
  if (now - lastDisplayMs >= config::kDisplayIntervalMs) {
    lastDisplayMs = now;
    M5.Display.setCursor(0, 0);
    M5.Display.printf("M1 IMU test\n");
    M5.Display.printf("gyro %6.0f dps\n", gyroMag);
    M5.Display.printf("acc %5.2f %5.2f\n    %5.2f g\n",
                      data.accel.x, data.accel.y, data.accel.z);
    M5.Display.printf("bat %3d %%\n", (int)M5.Power.getBatteryLevel());
  }
}
