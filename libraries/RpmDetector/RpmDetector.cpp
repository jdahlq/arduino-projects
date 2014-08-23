#include "Arduino.h"
#include "limits.h"
#include "RpmDetector.h"

namespace {
  int BlipDeltaToRpm(long blip_delta_ms) {
    if (blip_delta_ms < 2) return INT_MAX;
    return 60000L / blip_delta_ms;
  }
}

RpmDetector::RpmDetector(float min_rpm_pct_of_nominal, int nominal_rpm)
    : min_rpm_pct_of_nominal_(min_rpm_pct_of_nominal) {
  blips_[0] = -1;
  blips_[1] = -1;
  blips_[2] = -1;
  SetNominalRpm(nominal_rpm);
}

void RpmDetector::Blip() {
  blips_[2] = blips_[1];
  blips_[1] = blips_[0];
  blips_[0] = millis();
}

int RpmDetector::Rpm() {
  long blips[3];
  GetBlips(blips);

  int projected_rpm = ProjectedRpmAtTime(millis(), blips);
  if (projected_rpm == 0) return projected_rpm;
  int current_rpm_ceiling = BlipDeltaToRpm(millis() - blips[0]);
  int estimated_rpm = min(projected_rpm, current_rpm_ceiling);
  return estimated_rpm >= min_rpm_ ? estimated_rpm : 0;
}

void RpmDetector::GetBlips(long* blips) {
  noInterrupts();
  // First, clear old blips.
  int start_clear = 3;
  if (IsBlipDeltaBelowMinRpm(millis() - blips_[0])) start_clear = 0;
  if (IsBlipDeltaBelowMinRpm(blips_[0] - blips_[1])) start_clear = 1;
  else if (IsBlipDeltaBelowMinRpm(blips_[1] - blips_[2])) start_clear = 2;
  for (int i = start_clear; i < 3; ++i) blips_[i] = -1;

  for (int i = 0; i < 3; ++i) {
    blips[i] = blips_[i];
  }
  interrupts();
}

long RpmDetector::MapRpm(long min, long max) {
  return map(Rpm(), min_rpm_, nominal_rpm_, min, max + 1);
}

bool RpmDetector::IsBlipDeltaBelowMinRpm(long delta) {
  return BlipDeltaToRpm(delta) < min_rpm_;
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
  if (blips[2] == -1) return velocity_0;
  
  // Three blips, project velocity with acceleration.
  const int velocity_1 = BlipDeltaToRpm(blips[1] - blips[2]);
  const float acceleration = float(velocity_0 - velocity_1) / (blips[0] - blips[1]);
  return velocity_1 + (acceleration * (time - blips[0]));
}
