#include "Arduino.h"
#include "RpmDetector.h"

RpmDetector::RpmDetector(int interrupt)
    : interrupt_(interrupt), rpm_(-1) {
  // do stuff
}

int RpmDetector::Rpm() {
  return rpm_;
}
