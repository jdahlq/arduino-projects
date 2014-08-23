// Detects RPM for the given interrupt pin.

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
  void SetNominalRpm(int rpm) {
    nominal_rpm_ = rpm;
    min_rpm_ = min_rpm_pct_of_nominal_ * nominal_rpm_;
  }

 private:
  // Gets the blips in a safe way.
  void GetBlips(long* blips);
  int ProjectedRpmAtTime(long time, const long* blips);
  bool IsBlipDeltaBelowMinRpm(long delta);
 
  int min_rpm_;
  int nominal_rpm_;
  const float min_rpm_pct_of_nominal_;
  volatile long blips_[3];
};

#endif RpmDetector_h
