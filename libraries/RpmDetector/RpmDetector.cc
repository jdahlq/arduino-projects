#include "Arduino.h"
#include "RpmDetector.h"

RpmDetector::RpmDetector(int interrupt) : interrupt_(interrupt) {
  // do stuff
}

int RpmDetector::Rpm() {
  return rpm_;
}
