#include "Adafruit_NeoPixel.h"
#include "Bounce2.h"

const int kNeoPixelPin = 6;
const int kMagnitudePin = A0;
const int kLightModePin = 8;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(90, kNeoPixelPin, NEO_GRB + NEO_KHZ800);
int magnitude_16 = 0;
int magnitude_4 = 0;
byte wheel_position;

Bounce light_mode_button;

void setup() {
  Serial.begin(9600);

  // Buttons setup.
  pinMode(kLightModePin, INPUT_PULLUP);
  light_mode_button.attach(kLightModePin);
  light_mode_button.interval(1000);

  // NeoPixels setup.
  ChangeMode(light_mode_button.read());
}
void loop() {
  const long time = millis();

  CheckLightModeButtonUpdate();

  // Groove the strip to the beat.
  if (light_mode_button.read() == HIGH) {
    int magnitude = analogRead(kMagnitudePin);
    magnitude_4 = (magnitude_4 + magnitude) >> 1;
    magnitude_16 = (magnitude_16 * 15 + magnitude) >> 4;
    if (magnitude_4 > 1.1 * magnitude_16) {
      Serial.print("R7: ");
      Serial.print(magnitude_16);
      Serial.print(" ");
      Serial.println(magnitude_4);

      wheel_position += 3;
      byte current_position = wheel_position;
      for(int i = 0; i < strip.numPixels(); ++i) {
	strip.setPixelColor(i, Wheel(current_position));
	current_position += 3;
      }
      strip.show();
    }
    delay(1);
  }
  // Light show.
  else {
    if (rainbow(20)) return;
    if (rainbowCycle(20)) return;
    if (theaterChaseRainbow(50)) return;
  }
}

void ChangeMode(int mode) {
  strip.begin();
  // Depressed. Groove to the beat.
  if (mode == HIGH) {
    wheel_position = random(256);
    byte current_position = wheel_position;
    for(int i = 0; i < strip.numPixels(); ++i) {
      current_position += 3;
      strip.setPixelColor(i, Wheel(current_position));
    }
    strip.show();
  } 
  // Released. Do a pretty light show.
  else {
    strip.show();
    // The loop handles everything here.
  }
}

bool CheckLightModeButtonUpdate() {
  if (light_mode_button.update()) {
    ChangeMode(light_mode_button.read());
    return true;
  }
  return false;
}

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

bool rainbow(uint8_t wait) {
  uint16_t i, j;
  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
    if (CheckLightModeButtonUpdate()) return true;
  }
  return false;
}

bool rainbowCycle(uint8_t wait) {
  uint16_t i, j;
  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
    if (CheckLightModeButtonUpdate()) return true;
  }
  return false;
}

//Theatre-style crawling lights with rainbow effect
bool theaterChaseRainbow(uint8_t wait) {
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
      for (int i=0; i < strip.numPixels(); i=i+3) {
	strip.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
      }
      strip.show();
       
      delay(wait);
      if (CheckLightModeButtonUpdate()) return true;
       
      for (int i=0; i < strip.numPixels(); i=i+3) {
	strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
  return false;
}
