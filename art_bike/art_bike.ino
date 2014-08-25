#include "Adafruit_NeoPixel.h"
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

// NeoPixel settings
const int kNeoPixelPin = 6;
const long kBeatDetectPeriod = 25;
long last_beat_detect = 0;

// Create an RpmDetector with a nominal rpm of 120 (~10mph) and a minimum/starting
// rpm of 10% of nominal.
RpmDetector rpm_detector(0.1, 120);
int d = 500;
float mult = 0.8;

MusicPlayer player;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(90, kNeoPixelPin, NEO_GRB + NEO_KHZ800);
int magnitude_10 = 0;
int magnitude_4 = 0;

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
    return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
    WheelPos -= 170;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}

int mod(int a, int b) {
  int r = a % b;
  return r >= 0 ? r : r + b;
}

byte wheel_position;

void setup() {
  Serial.begin(9600);

  // RpmDetector setup.
  pinMode(kRpmDetectorPin, INPUT_PULLUP);
  attachInterrupt(kRpmDetectorInterrupt, MagneticSensorISR, kRpmDetectorInterruptMode);

  // MusicPlayer setup.
  player.Init();
  player.Play();

  // NeoPixels setup.
  strip.begin();
  wheel_position = random(256);
  byte current_position = wheel_position;
  for(int i = 0; i < strip.numPixels(); ++i) {
    current_position += 3;
    strip.setPixelColor(i, Wheel(current_position));
  }
  strip.show();
}


void loop() {
  const long time = millis();

  // Groove the strip to the beat.
  if (time - last_beat_detect > kBeatDetectPeriod) {
    last_beat_detect = time;
    int magnitude = analogRead(A0);
    magnitude_4 = (magnitude_4 + magnitude) >> 1;
    magnitude_10 = (magnitude_10 * 15 + magnitude) >> 4;
    // magnitude_10 = 0.9375 * magnitude_10 + 0.0625 * magnitude;
    if (magnitude_4 > 1.0625 * magnitude_10) {
      Serial.print("R7: ");
      Serial.print(magnitude_10);
      Serial.print(" ");
      Serial.println(magnitude_4);

      wheel_position += 3;
      byte current_position = wheel_position;
      for(int i = 0; i < strip.numPixels(); ++i) {
	strip.setPixelColor(i, Wheel(current_position));
	current_position += 6;
      }
    }
  }

  // Adjust playback rate.
  if (time - last_play_rate_update > kPlayRateUpdatePeriod) {
    strip.show();
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
