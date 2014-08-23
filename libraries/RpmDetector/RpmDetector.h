// Detects RPM for the given interrupt pin.
//
// Note: Unless otherwise specified, periods are expressed in milliseconds.

#ifndef RpmDetector_h
#define RpmDetector_h

#include "Arduino.h"

class RpmDetector {
 public:
  // min_rpm: assumed rpm when the wheel first starts rotating. If the current rpm ceiling
  //          drops below the min rpm, then the wheel is considered stopped.
  // nominal_rpm: the expected "normal" rpm when the wheel gets up to speed.
  RpmDetector(float min_rpm_pct_of_nominal, int nominal_rpm);

  // Call this method when a blip is detected. This method can be called from within an ISR,
  // and the members it modifies are ISR-safe.
  void Blip();
  // Get the current best approximation for the rpm. 
  int Rpm();
  // Map the smoothed rpm onto the supplied range using the minimum and nominal rpm as reference.
  long MapRpm(long min, long max);
  void SetNominalRpm(int rpm);

 private:
  // Gets the blips in a safe way.
  void GetBlips(long* blips);
  // The projected period at a certain time based on the last three blips.
  long ProjectedPeriodAtTime(long time, const long* blips);
  bool PeriodIsAboveMax(long period);
 
  long max_period_;
  int nominal_period_;
  const float min_rpm_pct_of_nominal_;
  volatile long blips_[3];
};

#endif RpmDetector_h
