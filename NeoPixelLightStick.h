/*****************************************************
 
 *****************************************************/

#ifndef NEO_PIXEL_LIGHT_STICK_H
#define NEO_PIXEL_LIGHT_STICK_H

#define NEOPIXELBUS 0
#define ADAFRUIT 1
#define NEOPIXELLIBRARY ADAFRUIT
#if NEOPIXELLIBRARY == NEOPIXELBUS
  #include <NeoPixelBus.h>
#elif NEOPIXELLIBRARY == ADAFRUIT
  #include <Adafruit_NeoPixel.h>
#endif


class NeoPixelLightStickClass {
  public:
    int _maxPixelCount;
    int _pixelPin;

  public:
    NeoPixelLightStickClass(int maxPixelCount, int pixelPin);

    void setColor(int index, byte r, byte g, byte b);
    void show();

    void color(int pixelCount, byte r, byte g, byte b);
    void black(int pixelCount);
    void white(int pixelCount);

    void showBuffer(uint8_t buffer[], int startIndex, int length);

  private:
    #if NEOPIXELLIBRARY == NEOPIXELBUS
      NeoPixelBus _strip;
    #elif NEOPIXELLIBRARY == ADAFRUIT
      Adafruit_NeoPixel _strip;
    #endif
};

extern NeoPixelLightStickClass NeoPixelLightStick;

#endif // NEO_PIXEL_LIGHT_STICK
