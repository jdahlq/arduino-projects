#include "RpmDetector.h"

static const int kRpmDetectorInterrupt = 0;

RpmDetector rpm_detector(0.1, 100);
int d = 1000;
float mult = 0.9;

void setup() {
  Serial.begin(9600);
}

void loop() {
  if (d < 25) mult = 1.1;
  else if (d > 1000) mult = 0.9;
  rpm_detector.Blip();
  Serial.print("RPM: ");
  Serial.println(rpm_detector.Rpm());
  delay(d);

  Serial.print("RPM: ");
  Serial.println(rpm_detector.Rpm());
  delay(d);
  d *= mult;
}
