#include "Arduino.h"
#include "RpmDetector.h"

namespace {
  int BlipDeltaToRpm(long blip_delta_ms) {
    return 60000L / blip_delta_ms;
  }
}

RpmDetector::RpmDetector(int min_rpm, int nominal_rpm)
    : min_rpm_(min_rpm), nominal_rpm_(nominal_rpm), curr_blip_(-1), last_blip_(-1) {
}

int RpmDetector::Rpm() {
  if (IsStopped()) return 0;
  
  // Read the current and last blip.
  long curr_blip;
  long last_blip;
  GetBlips(&last_blip, &curr_blip);

  // If we only have a single blip, use the starting rpm.
  int last_measured_rpm = last_blip == -1 ? min_rpm_
                                          : BlipDeltaToRpm(curr_blip - last_blip);
  int current_rpm_ceiling = BlipDeltaToRpm(millis() - curr_blip);

  // If the last measured rpm is still feasible, use it. Otherwise use our best guess.
  return last_measured_rpm <= current_rpm_ceiling ? last_measured_rpm
                                                  : current_rpm_ceiling;
}

void RpmDetector::Blip() {
  last_blip_ = curr_blip_;
  curr_blip_ = millis();
}

bool RpmDetector::IsStopped() {
  long curr_blip;
  long last_blip;
  GetBlips(&last_blip, &curr_blip);  
  return curr_blip == -1 || IsBlipDeltaBelowMinRpm(millis() - curr_blip);
}

void RpmDetector::GetBlips(long* last_blip, long* curr_blip) {
  noInterrupts();
  *last_blip = last_blip_;
  *curr_blip = curr_blip_;
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
