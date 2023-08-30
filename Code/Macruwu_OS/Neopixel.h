#ifndef NEOPIXEL_H_
#define NEOPIXEL_H_
#include <Adafruit_NeoPixel.h>
#include <Arduino.h>

#define LED_PIN 29
#define LED_COUNT 32

class Lighting{
public:
  Lighting(uint8_t brightness = 50);
  void init();
  void rainbowFade();
  void constColor(String color);
private:
  void rainbow(uint8_t wait);
  uint32_t Wheel(uint8_t WheelPos);
  uint8_t brightness;

  int rainbowSpeed = 30;

  unsigned long pixelPrevious = 0;        // Previous Pixel Millis
  unsigned long patternPrevious = 0;      // Previous Pattern Millis
  int           pixelInterval = 50;       // Pixel Interval (ms)
  int           pixelQueue = 0;           // Pattern Pixel Queue
  int           pixelCycle = 0;           // Pattern Pixel Cycle
  uint16_t      pixelCurrent = 0;         // Pattern Current Pixel Number
  uint16_t      pixelNumber = LED_COUNT;  // Total Number of Pixels
};

#endif/* NEOPIXEL_H_ */