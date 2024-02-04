#ifndef LIGHTING
#define LIGHTING

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#define LED_PIN    1
#define LED_COUNT 88
#define LED_BRIGHTNESS 50

#define AMMOUNT_EFFECTS 2
#define DEFAULT_EFFECT_SPEED 10

typedef enum{
  effect_rainbow = 0,
  effect_const_color
}rgb_effect_t;

typedef struct{
  rgb_effect_t effect;
  uint32_t color;
  uint32_t speed;
}rgb_wrapper_t;

class RGB_LIGHTING{
public:
  RGB_LIGHTING(uint8_t led_pin, uint16_t led_count, uint8_t led_max_brightness);
  ~RGB_LIGHTING();
  void setup();
  void set_effect(rgb_wrapper_t wrapper);
  void set_effect_speed(rgb_effect_t effect, uint16_t delay);
  bool enable_override(uint16_t position, uint32_t color);
  bool disable_override(uint16_t position);
  bool toggle_override(uint16_t position, uint32_t color);
  uint32_t color(uint8_t red, uint8_t green, uint8_t blue);
  void set_const_color(uint8_t red, uint8_t green, uint8_t blue);
  void set_const_color(uint32_t color);
  void set_led_remap(uint8_t *led_remap, uint8_t size);
  void update();
private:
  // Strip options
  Adafruit_NeoPixel *strip = NULL;
  uint8_t led_pin;
  uint16_t led_count;
  uint8_t led_max_brightness;
  uint8_t *led_remap = NULL;

  // effect options
  rgb_effect_t effect_set = effect_rainbow;
  uint16_t effect_delays[AMMOUNT_EFFECTS];

  // effect blocker
  bool *effect_override = NULL;

  // timer
  uint64_t time_prev = 0; 

  //rainbow
  int rainbow_pixelCycle = 0;
  uint32_t wheel(byte WheelPos);

  //const color
  uint32_t const_color = 0;
  
};


void lightingSetup();

void colorWipe(uint32_t color, int wait);
void theaterChase(uint32_t color, int wait);
void rainbow(uint8_t wait);
void theaterChaseRainbow(uint8_t wait);
uint32_t Wheel(byte WheelPos);

#endif //LIGHTING