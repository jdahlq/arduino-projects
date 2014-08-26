#include "Adafruit_NeoPixel.h"

const int kNeoPixelPin = 6;
const int kMagnitudePin = A0;
const long kBeatDetectPeriod = 25;
long last_beat_detect = 0;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(90, kNeoPixelPin, NEO_GRB + NEO_KHZ800);
int magnitude_10 = 0;
int magnitude_4 = 0;

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
    return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
    WheelPos -= 170;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}

int mod(int a, int b) {
  int r = a % b;
  return r >= 0 ? r : r + b;
}

byte wheel_position;

void setup() {
  Serial.begin(9600);

  // NeoPixels setup.
  strip.begin();
  wheel_position = random(256);
  byte current_position = wheel_position;
  for(int i = 0; i < strip.numPixels(); ++i) {
    current_position += 3;
    strip.setPixelColor(i, Wheel(current_position));
  }
  strip.show();
}


void loop() {
  const long time = millis();

  // Groove the strip to the beat.
  if (time - last_beat_detect > kBeatDetectPeriod) {
    last_beat_detect = time;
    int magnitude = analogRead(kMagnitudePin);
    magnitude_4 = (magnitude_4 + magnitude) >> 1;
    magnitude_10 = (magnitude_10 * 15 + magnitude) >> 4;
    // magnitude_10 = 0.9375 * magnitude_10 + 0.0625 * magnitude;
    if (magnitude_4 > 1.0625 * magnitude_10) {
      Serial.print("R7: ");
      Serial.print(magnitude_10);
      Serial.print(" ");
      Serial.println(magnitude_4);

      wheel_position += 3;
      byte current_position = wheel_position;
      for(int i = 0; i < strip.numPixels(); ++i) {
	strip.setPixelColor(i, Wheel(current_position));
	current_position += 6;
      }
    }
  }
}
