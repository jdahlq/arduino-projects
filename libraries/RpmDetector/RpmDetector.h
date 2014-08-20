// Detects RPM for the given interrupt pin.

#ifndef RpmDetector_h
#define RpmDetector_h

#include "Arduino.h"

class RpmDetector {
 public:
  // interrupt: pass the interrupt number, not the pin number.
  // starting_rpm: assumed rpm when the wheel first starts rotating.
  // nominal_rpm: the expected "normal" rpm when the wheel gets up to speed.
  RpmDetector(int interrupt, int starting_rpm, int nominal_rpm);
  
  boolean IsStopped() { return stopped_; }
  // Get the current best approximation for the rpm. 
  int Rpm();
  
  // Get smoothed rpm.
  int SmoothedRpm();
  // Map the smoothed rpm onto the supplied range using the nominal rpm as reference.
  long MapSmoothedRpm(long min, long max) {
    return map(SmoothedRpm(), 0, nominal_rpm_, min, max + 1);
  }
  
  int GetNominalRpm() { return nominal_rpm_; }
  void SetNominalRpm(int rpm) { nominal_rpm_ = rpm; }

 private:
  void OnFirstBlip();
  void OnBlip();
 
  const int interrupt_;
  const int starting_rpm_;
  int nominal_rpm_;
  bool stopped_ = true;
  volatile long curr_blip_ = -1;
  volatile long last_blip_ = -1;
};

#endif RpmDetector_h
