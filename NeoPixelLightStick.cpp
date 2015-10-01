#include "NeoPixelLightStick.h"

#define MAX_PIXEL_COUNT 288
#define PIXEL_PIN 2


NeoPixelLightStickClass::NeoPixelLightStickClass(int maxPixelCount, int pixelPin)
: _maxPixelCount(maxPixelCount)
, _pixelPin(pixelPin)
#if NEOPIXELLIBRARY == NEOPIXELBUS
, _strip(NeoPixelBus(maxPixelCount, pixelPin))
#elif NEOPIXELLIBRARY == ADAFRUIT
, _strip(Adafruit_NeoPixel(maxPixelCount, pixelPin, NEO_GRB + NEO_KHZ800))
#endif
{  
  #if NEOPIXELLIBRARY == NEOPIXELBUS
    _strip.Begin();
  #elif NEOPIXELLIBRARY == ADAFRUIT
    _strip.begin();
  #endif
}


void NeoPixelLightStickClass::setColor(int index, byte r, byte g, byte b) {
  #if NEOPIXELLIBRARY == NEOPIXELBUS
    RgbColor color = RgbColor(r, g, b);
    _strip.SetPixelColor(index, color);  
  #elif NEOPIXELLIBRARY == ADAFRUIT
    _strip.setPixelColor(index, r, g, b);  
  #endif
}


void NeoPixelLightStickClass::show() {
  #if NEOPIXELLIBRARY == NEOPIXELBUS
    _strip.Show();
  #elif NEOPIXELLIBRARY == ADAFRUIT
    _strip.show();
  #endif
}


void NeoPixelLightStickClass::color(int pixelCount, byte r, byte g, byte b) {
  for (int i = 0; i < pixelCount; i++) {
    setColor(i, r, g, b);
  }
  show();
}


void NeoPixelLightStickClass::black(int pixelCount) {
  color(pixelCount, 0x00, 0x00, 0x00);
}


void NeoPixelLightStickClass::white(int pixelCount) {
  color(pixelCount, 0xff, 0xff, 0xff);
}


void NeoPixelLightStickClass::showBuffer(uint8_t buffer[], int startIndex, int length) {
  byte r, g, b;
  int maxLen = _maxPixelCount*3;
  for (int i = 0; i < (length<maxLen?length:maxLen); i++) {
   if (i%3 == 0) {
      r = buffer[startIndex + i];
    } else if (i%3 == 1) {
      g = buffer[startIndex + i];
    } else {
      b =  buffer[startIndex + i];
      setColor(i/3, r, g, b);
    }
  }
  delay(0);
  show();
}


NeoPixelLightStickClass NeoPixelLightStick(MAX_PIXEL_COUNT, PIXEL_PIN);


