#include "Arduino.h"
#include "RpmDetector.h"

namespace {
  int BlipDeltaToRpm(long blip_delta_ms) {
    return 60000L / blip_delta_ms;
  }
}

RpmDetector::RpmDetector(int min_rpm, int nominal_rpm)
    : min_rpm_(min_rpm), nominal_rpm_(nominal_rpm), blips_({-1}) {
}

int RpmDetector::Rpm() {
  if (IsStopped()) return 0;
  
  // Read the current and last blip.
  long blips[3];
  GetBlips(blips);

  // If we only have a single blip, use the starting rpm.
  int last_measured_rpm = last_blip == -1 ? min_rpm_
                                          : BlipDeltaToRpm(curr_blip - last_blip);
  int current_rpm_ceiling = BlipDeltaToRpm(millis() - curr_blip);

  // If the last measured rpm is still feasible, use it. Otherwise use our best guess.
  return last_measured_rpm <= current_rpm_ceiling ? last_measured_rpm
                                                  : current_rpm_ceiling;
}

void RpmDetector::Blip() {
  blips_[2] = blips_[1];
  blips_[1] = blips_[0];
  blips_[0] = millis();
}

bool RpmDetector::IsStopped() {
  long blips[3];
  GetBlips(blips);  
  return blips[0] == -1 || IsBlipDeltaBelowMinRpm(millis() - curr_blip);
}

void RpmDetector::GetBlips(long* blips) {
  noInterrupts();
  memcpy(blips_, blips, 3);
  interrupts();
}

int RpmDetector::SmoothedRpm() {
  // TODO: Implement
  return -1;
}

long RpmDetector::MapSmoothedRpm(long min, long max) {
  return map(SmoothedRpm(), 0, nominal_rpm_, min, max + 1);
}

bool RpmDetector::IsBlipDeltaBelowMinRpm(long delta) {
  return BlipDeltaToRpm(delta) < min_rpm_;
}

void RpmDetector::ClearOldBlips() {
  noInterrupts();
  int start_clear = 3;
  if (IsBlipDeltaBelowMinRpm(millis() - blips_[0])) start_clear = 0;
  else if (IsBlipDeltaBelowMinRpm(blips_[0] - blips_[1])) start_clear = 1;
  else if (IsBlipDeltaBelowMinRpm(blips_[1] - blips_[2])) start_clear = 2;
  
  for (int i = start_clear; i < 3; ++i) blips_[i] = -1;
  interrupts();
}

// The expected rpm at the given time based on the last two calculated rpm's,
// i.e. taking acceleration into account. Projected rpm may be negative, and it doesn't
// consider the ceiling rpm based on the time elapsed since the last blip.
int RpmDetector::ProjectedRpmAtTime(long time, const long* blips) {
  // No blips yet...
  if (blips[0] == -1) return 0;
  // Only one blip...
  if (blips[1] == -1) return min_rpm_;
  
  const int velocity_0 = BlipDeltaToRpm(blips[0] - blips[1]);
  // Only two blips...
  if (blips[2]) == -1) return velocity_0;
  
  // Three blips, project velocity with acceleration.
  const int velocity_1 = BlipDeltaToRpm(blips[1] - blips[2]);
  const int acceleration = velocity_1 - velocity_0;
  return velocity_time = velocity_1 + (acceleration * (time - blips[0]));
}
