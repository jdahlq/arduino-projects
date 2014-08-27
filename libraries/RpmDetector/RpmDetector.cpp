#include "Arduino.h"
#include "limits.h"
#include "RpmDetector.h"

namespace {
  const long kMillisPerMinute = 60000L;

  int PeriodToRpm(long period) {
    if (period < 2) return INT_MAX;
    return kMillisPerMinute / period;
  }

  long RpmToPeriod(int rpm) {
    return kMillisPerMinute / rpm;
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
  // Easy peasy debouncing.
  if (blips_[0] != -1 && millis() - blips_[0] < 120) return;
  blips_[2] = blips_[1];
  blips_[1] = blips_[0];
  blips_[0] = millis();
}

void RpmDetector::SetNominalRpm(int rpm) {
  nominal_period_ = RpmToPeriod(rpm);
  max_period_ = RpmToPeriod(min_rpm_pct_of_nominal_ * rpm);
}

int RpmDetector::Rpm() {
  long blips[3];
  GetBlips(blips);

  const long projected_period = ProjectedPeriodAtTime(millis(), blips);
  if (projected_period == LONG_MAX) return 0;
  const long current_period_floor = millis() - blips[0];
  const long estimated_period = max(projected_period, current_period_floor);
  return PeriodIsAboveMax(estimated_period) ? 0 : PeriodToRpm(estimated_period);
}

void RpmDetector::GetBlips(long* blips) {
  noInterrupts();
  // First, clear old blips.
  int start_clear = 3;
  if (PeriodIsAboveMax(millis() - blips_[0])) start_clear = 0;
  if (PeriodIsAboveMax(blips_[0] - blips_[1])) start_clear = 1;
  else if (PeriodIsAboveMax(blips_[1] - blips_[2])) start_clear = 2;
  for (int i = start_clear; i < 3; ++i) blips_[i] = -1;

  for (int i = 0; i < 3; ++i) {
    blips[i] = blips_[i];
  }
  interrupts();
}

long RpmDetector::MapRpm(int rpm, long min, long max) {
  return map(rpm, PeriodToRpm(max_period_), PeriodToRpm(nominal_period_), min, max + 1);
}

bool RpmDetector::PeriodIsAboveMax(long period) {
  return period > max_period_;
}

// The expected period at the given time based on the last two periods,
// i.e. taking acceleration into account. Projected rpm may be negative, and it doesn't
// consider the ceiling rpm based on the time elapsed since the last blip.
long RpmDetector::ProjectedPeriodAtTime(long time, const long* blips) {
  // No blips yet...
  if (blips[0] == -1) return LONG_MAX;
  // Only one blip...
  if (blips[1] == -1) return max_period_;
  
  const long T_0_long = blips[0] - blips[1];
  // Only two blips...
  if (blips[2] == -1) return T_0_long;
  
  // Three blips, project velocity with acceleration. Division is expensive, so we avoid
  // it as much as possible. Floats are used due to the cubic part of the expression, which
  // uint32_t (unsigned long) cannot accomodate.
  const float T_0 = T_0_long;
  const float T_1 = blips[1] - blips[2];
  const float T_c = time - blips[0];

  const float denominator = T_0 * T_1 + T_c * (T_1 - T_0);
  // Handle the discontinuity.
  if (denominator == 0) return LONG_MAX;
  return T_0 * T_0 * T_1 / denominator;
}
