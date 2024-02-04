/**
 * if Stuff no work, check this: https://github.com/T-vK/ESP32-BLE-Keyboard/issues/169
 * 
 * for compiling settings see README.md
 */
#include "BleKeyboard.h"
#include <Wire.h>
#include <memory>
#include "esp_bt.h"

//#define BAT_V_PIN 2
//#define PIN_SDA 6
//#define PIN_SCL 7

#define ADDR_I2C 0x69
#define DELAY_FOR_GET_BATTERY_LEVEL 10000

enum
{
  RID_KEYBOARD = 1,
  RID_GAMEPAD,
  RID_CONSUMER_CONTROL, // Media, volume etc ..
};

BleKeyboard bleKeyboard("Macruwu", "FG Labs", 69);

uint8_t delayTime = 60;
uint64_t time_prev = 0;

bool softDisableFlag = 0;

String sendString;
//KeyReport reportBuffer;
std::vector <KeyReport *> reportBuffer;

void Keyboard_print(String text);
void Keyboard_send_buffer();
void clear_send_buffer();
void receiveEvent(int howMany);

void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE work!");
  bleKeyboard.begin();
  bleKeyboard.setBatteryLevel(get_battery_level());
  //Wire.setPins(PIN_SDA, PIN_SCL);
  Wire.begin(ADDR_I2C);
  Wire.onReceive(receiveEvent);
  time_prev = millis();
}



void loop() {
  if(bleKeyboard.isConnected() && reportBuffer.size() > 0) {
    Keyboard_send_buffer();
  }
  else if(reportBuffer.size() > 0){
    for(int i = 0; i < reportBuffer.size(); i++){
      delete reportBuffer.at(i);          //prevent memory leaks (hopefully...)
    }
    reportBuffer.clear();
  }

  if(millis() > (time_prev + DELAY_FOR_GET_BATTERY_LEVEL)){
    time_prev = millis();
    bleKeyboard.setBatteryLevel(get_battery_level());
    Serial.println("Updated  Battery level");
  }

  if(softDisableFlag){
    esp_bt_controller_disable();
    esp_deep_sleep_start();
    //while(softDisableFlag) delay(100);
  }

}

void receiveEvent(int howMany){
  reportBuffer.push_back(new KeyReport);
  KeyReport *tmp = reportBuffer.at(reportBuffer.size()-1);
  uint8_t reportID = 0;
  while(Wire.available()){
    reportID = Wire.read();
    switch(reportID){
      case RID_KEYBOARD:
        tmp->modifiers = Wire.read();
        tmp->keys[0] = Wire.read();
        tmp->keys[1] = Wire.read();
        tmp->keys[2] = Wire.read();
        tmp->keys[3] = Wire.read();
        tmp->keys[4] = Wire.read();
        tmp->keys[5] = Wire.read();
        break;
      case 15:
        softDisableFlag = Wire.read();
        if(softDisableFlag != 0) softDisableFlag = true;
        break;
      default:
        Wire.read();
        break;
    }
    // Read in format ReportID,modifier,keycodes[6]
  }
}

void Keyboard_send_buffer(){
  Serial.println("Sending Buffer");
  for(int i = 0; i < reportBuffer.size(); i++){
    if((i%4)==0 && i > 0){
      delay(delayTime);
    }
    bleKeyboard.sendReport(reportBuffer.at(i));
    //bleKeyboard.releaseAll();
    for(int n = 0; n < 6; n++){
      Serial.print(reportBuffer.at(i)->keys[n]);
    }
    Serial.println(reportBuffer.at(i)->modifiers);
  }

  for(int i = 0; i < reportBuffer.size(); i++){
    delete reportBuffer.at(i);          //prevent memory leaks (hopefully...)
  }
  reportBuffer.clear();
}

uint8_t get_battery_level(){        // Disabled for Macruwu because of error in Board design
  return 69;
  //return (uint8_t)map(analogRead(BAT_V_PIN), 0, 1023, 0, 100);
}



