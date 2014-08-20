#include "Arduino.h"
#include "RpmDetector.h"

namespace {
  int BlipDeltaToRpm(long blip_delta_ms) {
    return 60000L / blip_delta_ms;
  }
}

RpmDetector::RpmDetector(int interrupt, int starting_rpm, int nominal_rpm)
    : interrupt_(interrupt), starting_rpm_(starting_rpm), nominal_rpm_(nominal_rpm) {
  // TODO: Attach interrupt.
}

int RpmDetector::Rpm() {
  if (IsStopped()) return 0;
  // If we only have a single blip, use the starting rpm.
  int last_measured_rpm = last_blip_ == -1 ? starting_rpm_
                                        : BlipDeltaToRpm(curr_blip_ - last_blip_);
  int current_rpm_ceiling = BlipDeltaToRpm(millis() - curr_blip_);

  // If the last measured rpm is still feasible, use it. Otherwise use our best guess.
  return last_measured_rpm <= current_rpm_ceiling ? last_measured_rpm
                                                  : current_rpm_ceiling;
}

int RpmDetector::SmoothedRpm() {
  // TODO: Implement
  return -1;
}

long RpmDetector::MapSmoothedRpm(long min, long max) {
  return map(SmoothedRpm(), 0, nominal_rpm_, min, max + 1);
}
