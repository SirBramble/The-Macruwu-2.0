#ifndef NEOPIXEL_H_
#define NEOPIXEL_H_
#include <Adafruit_NeoPixel.h>
#include <Arduino.h>

#define LED_PIN 29
#define LED_COUNT 32


void testNeopixel();
void neopixelSetup();

void rainbow(uint8_t wait);
uint32_t Wheel(uint8_t WheelPos);
#endif/* NEOPIXEL_H_ */