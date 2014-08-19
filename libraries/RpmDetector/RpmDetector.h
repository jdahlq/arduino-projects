// Detects RPM for the given interrupt pin.

#ifndef RpmDetector.h
#define RpmDetector.h

#include "Arduino.h"

class RpmDetector {
 public:
  // Pass the interrupt number, not the pin number.
  RpmDetector(int interrupt);
  // Get the smoothed, approximate RPM.
  int Rpm();

 private:
  int rpm_ = -1;
  int interrupt_ = -1;
};

#endif
