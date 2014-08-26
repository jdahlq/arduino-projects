#include "MusicPlayer.h"
#include "RpmDetector.h"
#include "WaveHC.h"
#include "WaveUtil.h"

// RpmDetector settings
const int kRpmDetectorPin = 2;
const int kRpmDetectorInterrupt = 0;
const int kRpmDetectorInterruptMode = FALLING;
int avg_rpm = 0;

// MusicPlayer settings
const int kPlayRateUpdatePeriod = 250;
long last_play_rate_update = 0;
long current_play_rate_rpm = 0;

// Create an RpmDetector with a nominal rpm of 120 (~10mph) and a minimum/starting
// rpm of 10% of nominal.
RpmDetector rpm_detector(0.1, 120);
int d = 500;
float mult = 0.8;

MusicPlayer player;

void setup() {
  Serial.begin(9600);

  // RpmDetector setup.
  pinMode(kRpmDetectorPin, INPUT_PULLUP);
  attachInterrupt(kRpmDetectorInterrupt, MagneticSensorISR, kRpmDetectorInterruptMode);

  // MusicPlayer setup.
  player.Init();
  player.Play();
}


void loop() {
  const long time = millis();

  // Adjust playback rate.
  if (time - last_play_rate_update > kPlayRateUpdatePeriod) {
    last_play_rate_update = time;
    uint32_t normal_rate = player.GetNormalPlaybackRate();
    int rpm = rpm_detector.Rpm();
    // Serial.print("rpm: ");
    // Serial.println(rpm);
    avg_rpm = 0.1 * rpm + 0.9 * avg_rpm;
    if (avg_rpm < 12) player.Pause();
    else {
      if (player.IsPaused()) player.Resume();
      float rpm_ratio = float(current_play_rate_rpm) / avg_rpm;
      if (rpm_ratio > 1.04 || rpm_ratio < 0.96) {
	current_play_rate_rpm = avg_rpm;
	player.SetPlaybackRate(map(current_play_rate_rpm, 12, 120, normal_rate >> 2, normal_rate));
      }
      // player.SetPlaybackRate(rpm_detector.MapRpm(normal_rate >> 2, normal_rate));
    }
  }
  // TEST
  /*
  if (d < 200) mult = 1.02;
  else if (d > 500) mult = 0.8;
  noInterrupts();
  MagneticSensorISR();
  interrupts();
  Serial.print("RPM: ");
  int rpm = rpm_detector.Rpm();
  Serial.println(rpm);
  uint32_t normal_rate = player.GetNormalPlaybackRate();
  player.SetPlaybackRate(rpm_detector.MapRpm(normal_rate >> 2, normal_rate));
  delay(d);

  Serial.print("RPM: ");
  Serial.println(rpm_detector.Rpm());
  normal_rate = player.GetNormalPlaybackRate();
  player.SetPlaybackRate(rpm_detector.MapRpm(normal_rate >> 2, normal_rate));

  delay(d);
  d *= mult;
  // END TEST
  */
}

void MagneticSensorISR() {
  rpm_detector.Blip();
}
