#include "Arduino.h"
#include "RpmDetector.h"

RpmDetector::RpmDetector(int interrupt, int starting_rpm, int nominal_rpm)
    : interrupt_(interrupt), starting_rpm_(starting_rpm), nominal_rpm_(nominal_rpm) {
  // TODO: Attach interrupt.
}

int RpmDetector::Rpm() {
  // TODO: Implement.
  return -1;
}

  
boolean RpmDetector::IsStopped() {
  return Rpm() == 0;
}

int RpmDetector::SmoothedRpm() {
  // TODO: Implement
  return -1;
}

long RpmDetector::MapSmoothedRpm(long min, long max) {
  return map(SmoothedRpm(), 0, nominal_rpm_, min, max + 1);
}
