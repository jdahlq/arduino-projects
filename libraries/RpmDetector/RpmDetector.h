// Detects RPM for the given interrupt pin.

#ifndef RpmDetector_h
#define RpmDetector_h

#include "Arduino.h"

class RpmDetector {
 public:
  // min_rpm: assumed rpm when the wheel first starts rotating. If the current rpm ceiling
  //          drops below the min rpm, then the wheel is considered stopped.
  // nominal_rpm: the expected "normal" rpm when the wheel gets up to speed.
  RpmDetector(int min_rpm, int nominal_rpm);

  // Call this method when a blip is detected. This method can be called from within an ISR,
  // and the members it modifies are ISR-safe.
  void Blip();
  // Is the wheel stopped?
  bool IsStopped();
  // Get the current best approximation for the rpm. 
  int Rpm();
  // Get smoothed rpm, which takes acceleration into account.
  int SmoothedRpm();
  // Map the smoothed rpm onto the supplied range using the nominal rpm as reference.
  long MapSmoothedRpm(long min, long max);
  
  int GetNominalRpm() { return nominal_rpm_; }
  void SetNominalRpm(int rpm) { nominal_rpm_ = rpm; }

 private:
  // Gets the blips in a safe way.
  void GetBlips(long* blips);
  
  long ExpectedRpmAtTime(long time);

  bool IsBlipDeltaBelowMinRpm(long delta);
 
  const int min_rpm_;
  int nominal_rpm_;
  volatile long blips_[3];
};

#endif RpmDetector_h
