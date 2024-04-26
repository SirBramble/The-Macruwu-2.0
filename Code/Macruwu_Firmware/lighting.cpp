#include "lighting.h"

unsigned long pixelPrevious = 0;        // Previous Pixel Millis
unsigned long patternPrevious = 0;      // Previous Pattern Millis
int           patternCurrent = 0;       // Current Pattern Number
int           patternInterval = 5000;   // Pattern Interval (ms)
int           pixelInterval = 50;       // Pixel Interval (ms)
int           pixelQueue = 0;           // Pattern Pixel Queue
int           pixelCycle = 0;           // Pattern Pixel Cycle
uint16_t      pixelCurrent = 0;         // Pattern Current Pixel Number
uint16_t      pixelNumber = LED_COUNT;  // Total Number of Pixels

//Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

RGB_LIGHTING::RGB_LIGHTING(uint8_t led_pin, uint16_t led_count, uint8_t led_max_brightness){
  this->led_pin = led_pin;
  this->led_count = led_count + 1;
  this->led_max_brightness = led_max_brightness;
  this->strip = new Adafruit_NeoPixel(this->led_count, this->led_pin, NEO_GRB + NEO_KHZ800);
  this->effect_override = new bool[this->led_count];
}

RGB_LIGHTING::~RGB_LIGHTING(){
  delete this->strip;
  delete [] this->effect_override;
  delete [] this->led_remap;
}

void RGB_LIGHTING::setup(){
  for(int i = 0; i < AMMOUNT_EFFECTS; i++){
    this->effect_delays[i] = DEFAULT_EFFECT_SPEED;
  }

  for(uint16_t i = 0; i < this->led_count; i++){
    this->effect_override[i] = 0;
  }

  this->strip->begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  this->strip->show();            // Turn OFF all pixels ASAP
  this->strip->setBrightness(this->led_max_brightness); // Set BRIGHTNESS to about 1/5 (max = 255)
  this->strip->setPixelColor(4, strip->Color(0, 200, 0));
  this->strip->show();
}

void RGB_LIGHTING::set_effect(rgb_wrapper_t wrapper){
  this->effect_set = wrapper.effect;
  this->const_color = wrapper.color;
  this->effect_delays[this->effect_set] = (uint16_t)(100 / wrapper.speed);
}

void RGB_LIGHTING::set_led_remap(uint8_t *led_remap, uint8_t size){
  if(this->led_remap != NULL){
    delete [] this->led_remap;
  }
  this->led_remap = new uint8_t[size];
  for(int i = 0; i < size; i++){
    this->led_remap[i] = led_remap[i];
  }
}

bool RGB_LIGHTING::enable_override(uint16_t position, uint32_t color){
  if(led_remap[position] > this->led_count) return false;

  this->effect_override[led_remap[position]] = true;
  this->strip->setPixelColor(led_remap[position], color);

  return true;
}

bool RGB_LIGHTING::toggle_override(uint16_t position, uint32_t color){
  if(led_remap[position] > this->led_count) return false;

  this->effect_override[led_remap[position]] = !this->effect_override[led_remap[position]]; // toggle
  if(this->effect_override[led_remap[position]]){
    this->strip->setPixelColor(led_remap[position], color);
  }
  

  return true;
}

bool RGB_LIGHTING::disable_override(uint16_t position){
  if(led_remap[position] > this->led_count) return false;

  this->effect_override[led_remap[position]] = false;

  //this->strip->setPixelColor(led_remap[position], 0);

  return true;
}

void RGB_LIGHTING::update(){
  if(millis() < this->time_prev){   // millis() overflow detection
    this->time_prev = millis();
  }

  if((this->time_prev + this->effect_delays[this->effect_set]) < millis()){
    this->time_prev = millis();
    switch (this->effect_set) {
      case effect_rainbow:
        for(uint16_t i=0; i < this->led_count; i++) {
          if(this->effect_override[i] == 0){
            strip->setPixelColor(i, this->wheel((i + rainbow_pixelCycle) & 255));
          }
        }
        rainbow_pixelCycle++;                             //  Advance current cycle
        if(rainbow_pixelCycle >= 256)
          rainbow_pixelCycle = 0;                         //  Loop the cycle back to the begining
        break;
      case effect_const_color:
        for(uint16_t i=0; i < this->led_count; i++) {
          if(this->effect_override[i] == 0){
            strip->setPixelColor(i, this->const_color);
          }
        }
        break;
    }
  }
  strip->show();
}

//rainbow functions
uint32_t RGB_LIGHTING::wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return this->strip->Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return this->strip->Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return this->strip->Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

//const color functions
void RGB_LIGHTING::set_const_color(uint8_t red, uint8_t green, uint8_t blue){
  this->const_color = this->strip->Color(red,green,blue);
}

void RGB_LIGHTING::set_const_color(uint32_t color){
  this->const_color = color;
}

uint32_t RGB_LIGHTING::color(uint8_t red, uint8_t green, uint8_t blue){
  return this->strip->Color(red,green,blue);
}


/*
void lightingSetup(){
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(LED_BRIGHTNESS); // Set BRIGHTNESS to about 1/5 (max = 255)
  strip.setPixelColor(4, strip.Color(0, 200, 0));
  strip.show();
}

void colorWipe(uint32_t color, int wait) {
  if(pixelInterval != wait)
    pixelInterval = wait;                   //  Update delay time
  strip.setPixelColor(pixelCurrent, color); //  Set pixel's color (in RAM)
  strip.show();                             //  Update strip to match
  pixelCurrent++;                           //  Advance current pixel
  if(pixelCurrent >= pixelNumber)           //  Loop the pattern from the first LED
    pixelCurrent = 0;
}

// Theater-marquee-style chasing lights. Pass in a color (32-bit value,
// a la strip.Color(r,g,b) as mentioned above), and a delay time (in ms)
// between frames.
void theaterChase(uint32_t color, int wait) {
  if(pixelInterval != wait)
    pixelInterval = wait;                   //  Update delay time
  for(int i = 0; i < pixelNumber; i++) {
    strip.setPixelColor(i + pixelQueue, color); //  Set pixel's color (in RAM)
  }
  strip.show();                             //  Update strip to match
  for(int i=0; i < pixelNumber; i+=3) {
    strip.setPixelColor(i + pixelQueue, strip.Color(0, 0, 0)); //  Set pixel's color (in RAM)
  }
  pixelQueue++;                             //  Advance current pixel
  if(pixelQueue >= 3)
    pixelQueue = 0;                         //  Loop the pattern from the first LED
}

// Rainbow cycle along whole strip. Pass delay time (in ms) between frames.
void rainbow(uint8_t wait) {
  if(pixelInterval != wait)
    pixelInterval = wait;                   
  for(uint16_t i=0; i < pixelNumber; i++) {
    strip.setPixelColor(i, Wheel((i + pixelCycle) & 255)); //  Update delay time  
  }
  strip.show();                             //  Update strip to match
  pixelCycle++;                             //  Advance current cycle
  if(pixelCycle >= 256)
    pixelCycle = 0;                         //  Loop the cycle back to the begining
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  if(pixelInterval != wait)
    pixelInterval = wait;                   //  Update delay time  
  for(int i=0; i < pixelNumber; i+=3) {
    strip.setPixelColor(i + pixelQueue, Wheel((i + pixelCycle) % 255)); //  Update delay time  
  }
  strip.show();
  for(int i=0; i < pixelNumber; i+=3) {
    strip.setPixelColor(i + pixelQueue, strip.Color(0, 0, 0)); //  Update delay time  
  }      
  pixelQueue++;                           //  Advance current queue  
  pixelCycle++;                           //  Advance current cycle
  if(pixelQueue >= 3)
    pixelQueue = 0;                       //  Loop
  if(pixelCycle >= 256)
    pixelCycle = 0;                       //  Loop
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

*/