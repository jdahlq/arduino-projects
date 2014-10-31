#include "Bounce2.h"
#include "MusicPlayer.h"
#include "RpmDetector.h"
#include "WaveHC.h"
#include "WaveUtil.h"

// RpmDetector settings
const int kRpmDetectorPin = 2;
// const int kRpmDetectorInterrupt = 0;
// const int kRpmDetectorInterruptMode = FALLING;
int avg_rpm = 0;
bool disable_play_rate_changes = false;

// MusicPlayer settings
const int kPlayRateUpdatePeriod = 250;
long last_play_rate_update = 0;
long current_play_rate_rpm = 0;

// Button settings
const int kSetNominalRatePin = A0;
const int kNextSongPin = A1;

// Create an RpmDetector with a nominal rpm of 120 (~10mph) and a minimum/starting
// rpm of 10% of nominal.
RpmDetector rpm_detector(0.1, 120);
int d = 500;
float mult = 0.8;

MusicPlayer player;

Bounce button_set_rate;
long button_set_rate_time;
Bounce button_next_song;
long button_next_song_time;
Bounce magnetic_sensor;
long magnetic_sensor_time;

void setup() {
  Serial.begin(9600);

  // RpmDetector setup.
  pinMode(kRpmDetectorPin, INPUT_PULLUP);
  // attachInterrupt(kRpmDetectorInterrupt, MagneticSensorISR, kRpmDetectorInterruptMode);
  magnetic_sensor.attach(kRpmDetectorPin);
  magnetic_sensor.interval(2);

  // MusicPlayer setup.
  player.Init();
  player.Play();

  // Button setup.
  pinMode(kSetNominalRatePin, INPUT_PULLUP);
  button_set_rate.attach(kSetNominalRatePin);
  pinMode(kNextSongPin, INPUT_PULLUP);
  button_next_song.attach(kNextSongPin);
}


void loop() {
  const long time = millis();

  if (magnetic_sensor.update()) {
    if (magnetic_sensor.read() == LOW) {
      MagneticSensorISR();
    }
  }

  if (button_set_rate.update()) {
    if (button_set_rate.read() == LOW) {
      button_set_rate_time = time;
    } else {
      if (time - button_set_rate_time > 1000) {
	disable_play_rate_changes = !disable_play_rate_changes;
      } else {
	disable_play_rate_changes = false;
	if (avg_rpm != 0) rpm_detector.SetNominalRpm(avg_rpm);
      }
      if (disable_play_rate_changes) {
	player.Resume();
	player.SetPlaybackRate(player.GetNormalPlaybackRate());
      }
    }
  }
  
  if (button_next_song.update()) {
    if (button_next_song.read() == LOW) button_next_song_time = time;
    else {
      if (time - button_next_song_time > 1000) player.NextPlaylist();
      else player.NextSong();
    }
  }

  AdjustPlaybackRate(time);
}

void AdjustPlaybackRate(long time) {
  // Adjust playback rate.
  if (time - last_play_rate_update > kPlayRateUpdatePeriod) {
    player.Play();
    last_play_rate_update = time;
    int rpm = rpm_detector.Rpm();
    if (rpm == 0) avg_rpm = 0;
    else avg_rpm = (avg_rpm == 0) ? rpm : 0.1 * rpm + 0.9 * avg_rpm;
    // Serial.print("rpm: ");
    // Serial.println(rpm);

    // Use normal playrate.
    if (disable_play_rate_changes) return;

    // Adjust playrate based on rpm.
    if (avg_rpm == 0) {
      if (!player.IsPaused()) player.Pause();
      return;
    }
    if (player.IsPaused()) player.Resume();
    float rpm_ratio = float(current_play_rate_rpm) / avg_rpm;
    if (rpm_ratio > 1.04 || rpm_ratio < 0.96) {
      current_play_rate_rpm = avg_rpm;
      uint32_t normal_rate = player.GetNormalPlaybackRate();
      player.SetPlaybackRate(rpm_detector.MapRpm(current_play_rate_rpm,
						 normal_rate >> 2,
						 normal_rate));
    }
    // player.SetPlaybackRate(rpm_detector.MapRpm(normal_rate >> 2, normal_rate));
  }
}

void MagneticSensorISR() {
  rpm_detector.Blip();
}
