//DISCLAIMER:
//My spelling is shit, so please ignore the spelling mistakes in the comments.
//Thx uwu

#include <Wire.h>
#include "Expander.h"
#include "Keyboard.h"
#include "Neopixel.h"

#define DEBUG false
#define ADDone 0b0100000        
#define ADDtwo 0b0100001
#define ADDBLE 0x69
#define INT_0 D8
#define INT_1 D9
//#define MACRUWUV1
#define MACRUWUV2

#ifdef MACRUWUV1
uint8_t Expander1RemapTable[PINCOUNT] = {7,6,5,4,3,2,1,0,15,14,13,12,11,10,9,8};
uint8_t Expander2RemapTable[PINCOUNT] = {7,6,5,4,3,2,1,0,15,14,13,12,11,10,9,8};
PCA9555D Expander1(ADDone);
PCA9555D Expander2(ADDtwo);
#endif
#ifdef MACRUWUV2
uint8_t Expander1RemapTable[PINCOUNT] = {15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0};
uint8_t Expander2RemapTable[PINCOUNT] = {15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0};
PI4IOE5V6416 Expander1(ADDone, INT_1);
PI4IOE5V6416 Expander2(ADDtwo, INT_0);
#endif

Lighting lighting(50);

//default Constructor. Initialises the Keyboard with: layout file: "macroLayout.txt", Layers: 5, Buttons: 32
Keyboard Macruwu;

// To change default parameters, call the constructor with arguments: 
// Keyboard Keyboard_Name("FILENAME", AMMOUNT_LAYERS, AMMOUNT_KEYS);
// For Example:
// Keyboard Macruwu("macroLayout.txt", 5, 32);           


void setup() {
  Serial.begin(115200);       //Open Serial Port at 115200 baud
  Wire.begin();               //start I²C for I/O expanders
  flash_setup();              //Setup flash storage as mounted drive
  //flash_test();             //try running if flash storage won´t work. Will try to create File and run some Tests.
                              //concider running it on call in loop(), due to a bug, where Serial prints are not sent from setup()
  //flash_clear();            //If your filesystem or mapping file is corrupted, run this to clear the storage
  //Macruwu.disableHID();
  Macruwu.init();             //Setup code for file mapped Keyboard
  Expander1.init();
  Expander2.init();
  Expander1.setRemapTable(Expander1RemapTable);
  Expander2.setRemapTable(Expander2RemapTable);
  lighting.init();            //Setup code for Neopixels with optional parameter: brightness (default is 50, max is 255)
  //lighting.constColor("white");
  Serial.println("done innit?");
  //Macruwu.gamepad_test_reset();
}

void loop() {
  //debug_serial();           //Meant for testing the switches via the Serial Monitor
  flash_loop();             //Try running this if you are having problems with the flash storage
  
  if(Macruwu.fsChanged()){
    Macruwu.readFile();
  }
  switch(Macruwu.currentLayer){
    case 1:
      lighting.rainbowFade();
      break;
    case 2:
      lighting.constColor("green");
      break;
    default: 
      lighting.rainbowFade();
      break;
  }
  //test_I2C();

  run();
  //test_bluetooth();
  //test_gamepad();

  delay(1);
}

void test_bluetooth(){
  String test = "I am test uwu!\n";
  #ifdef MACRUWUV2
  Expander1.debounce();
  #endif
  if(Expander1.getStatePulse_on(0)){
    Wire.beginTransmission(ADDBLE);
    Wire.write(0x05);
    //for(uint8_t i = 0; i < test.length(); i++){
    //  Wire.write(test.charAt(i));
    //  Serial.print(test.charAt(i));
    //}
    for(int i = 0; i < 6; i++){
      Wire.write(0x1E);
    }
    Wire.write(0x02);
    Wire.endTransmission();
  }
}

void debug_serial(){              //For testing the Inputs
  #ifdef MACRUWUV2
  Expander1.debounce();
  #endif
  for(int i = 0; i < 16; i++){
    if(Expander1.getStatePulse_on(i)){
      Serial.print("Button ");
      Serial.println(i);
      }  
  }
  #ifdef MACRUWUV2
  Expander2.debounce();
  #endif
  for(int i = 0; i < 16; i++){
    if(Expander2.getStatePulse_on(i)){
      Serial.print("Button ");
      Serial.println(i+16);
      }  
  }
}

void test_gamepad(){
  for(int i = 0; i < 16; i++){
    Macruwu.gamepad_test(i, Expander1.getState(i));
  }
  for(int i = 0; i < 16; i++){
    Macruwu.gamepad_test(i+16, Expander2.getState(i));
  }
  Macruwu.gamepad_send_test();
}

bool triggered = 0;

void test_I2C(){

  if(Expander1.getState(1) && (triggered == 0)){
    triggered = 1;
    Wire.beginTransmission(8);
    Wire.write(123);
    int error = Wire.endTransmission();
    Serial.println("Sent I2C 123!");
    Serial.print("Error: "); Serial.println(error);
  }
  if((Expander1.getState(1) == 0) && (triggered == 1)){
    triggered = 0;
  }
}

void run(){
  #ifdef MACRUWUV2
  Expander1.debounce();
  #endif
  for(int i = 0; i < 16; i++){
    if(Expander1.getStatePulse_on(i)){
      long before = millis();
      Macruwu.interpret(Macruwu.currentLayer, i);
      long interpretTime = millis()-before;
      Serial.print("Time taken interpret: "); Serial.println(interpretTime);
      Macruwu.send_buffer();
      Serial.print("Time taken buffer: "); Serial.println(millis()-interpretTime-before);
    }
  }
  #ifdef MACRUWUV2
  Expander2.debounce();
  #endif
  for(int i = 0; i < 16; i++){
    if(Expander2.getStatePulse_on(i)){
      long before = millis();
      Macruwu.interpret(Macruwu.currentLayer, i+16);
      long interpretTime = millis()-before;
      Serial.print("Time taken: "); Serial.println(interpretTime);
      Macruwu.send_buffer();
      Serial.print("Time taken buffer: "); Serial.println(millis()-interpretTime-before);
    }
  }

  
}