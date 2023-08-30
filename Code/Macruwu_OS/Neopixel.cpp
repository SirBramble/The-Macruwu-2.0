#include "Neopixel.h"


Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);





Lighting::Lighting(uint8_t brightness){
  this->brightness = brightness;
}

void Lighting::init(){
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(this->brightness); // Set BRIGHTNESS to about 1/5 (max = 255)
}

void Lighting::rainbowFade(){
  unsigned long currentMillis = millis();                     //  Update current time
  if((currentMillis - patternPrevious) >= rainbowSpeed) {  //  Check for expired time
    patternPrevious = currentMillis;
     rainbow(150);
  }
}

void Lighting::constColor(String color){
  uint32_t colorcode = 0;
  Serial.println(color.c_str());
  if(color.indexOf("white") != -1) colorcode = strip.Color(255,255,255);
  else if(color.indexOf("red") != -1) colorcode = strip.Color(255,0,0);
  else if(color.indexOf("green") != -1) colorcode = strip.Color(0,255,0);
  else if(color.indexOf("blue") != -1) colorcode = strip.Color(0,0,255);
  else if(color.indexOf("purple") != -1) colorcode = strip.Color(255,0,255);
  else if(color.indexOf("pink") != -1) colorcode = strip.Color(255,155,55);
  else if(color.indexOf("uwu") != -1) colorcode = strip.Color(0,255,0);
  else colorcode = strip.Color(255,255,255);
  for(int i = 0; i < LED_COUNT; i++){
    strip.setPixelColor(i, colorcode);
  }
  strip.show();

}


// Rainbow cycle along whole strip. Pass delay time (in ms) between frames.
void Lighting::rainbow(uint8_t wait) {
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

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Lighting::Wheel(uint8_t WheelPos) {
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