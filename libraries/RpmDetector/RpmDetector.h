// Detects RPM for the given interrupt pin.

#ifndef RpmDetector_h
#define RpmDetector_h

#include "Arduino.h"

class RpmDetector {
 public:
  // Pass the interrupt number, not the pin number.
  RpmDetector(int interrupt);
  // Get the smoothed, approximate RPM.
  int Rpm();

 private:
  const int interrupt_;
  int rpm_;
};

#endif RpmDetector_h
