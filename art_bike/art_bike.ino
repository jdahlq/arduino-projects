#include "MusicPlayer.h"
#include "RpmDetector.h"
#include "WaveHC.h"
#include "WaveUtil.h"

const int kRpmDetectorInterrupt = 0;
const int kRpmDetectorInterruptMode = FALLING;

// Create an RpmDetector with a nominal rpm of 120 (~10mph) and a minimum/starting
// rpm of 10% of nominal.
RpmDetector rpm_detector(0.1, 120);
int d = 500;
float mult = 0.9;

MusicPlayer player;

void setup() {
  Serial.begin(9600);

  // RpmDetector setup.
  //attachInterrupt(kRpmDetectorInterrupt, MagneticSensorISR, kRpmDetectorInterruptMode);

  // MusicPlayer setup.
  player.Init();
  player.Play();
}

void loop() {

  // TEST
  if (d < 25) mult = 1.1;
  else if (d > 500) mult = 0.9;
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
}

void MagneticSensorISR() {
  rpm_detector.Blip();
}
