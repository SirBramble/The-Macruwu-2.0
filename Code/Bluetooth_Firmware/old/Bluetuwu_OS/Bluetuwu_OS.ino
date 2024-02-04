/**
 * if Stuff no work, check this: https://github.com/T-vK/ESP32-BLE-Keyboard/issues/169
 */
#include "BleKeyboard.h"
#include <Wire.h>
#include <memory>

BleKeyboard bleKeyboard("Macruwu", "FG Labs", 69);

uint8_t delayTime = 60;

String sendString;
bool messageAvailable = 0;
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
  bleKeyboard.setBatteryLevel(69);
  Wire.begin(0x69);
  Wire.onReceive(receiveEvent);
}



void loop() {
  
  if(bleKeyboard.isConnected() && messageAvailable) {
    messageAvailable = 0;
    Keyboard_print(sendString);
    Keyboard_send_buffer();
  }
  else if(sendString.length() > 0){
    sendString.remove(0, sendString.length());
    sendString.trim();
    clear_send_buffer();
  }

}

void Keyboard_print(String text){
    delay(delayTime);
    if(text.length()>15){
      bleKeyboard.print(text.substring(0,15));
      text.remove(0, 15);
      return Keyboard_print(text);
    }
    bleKeyboard.print(text);
  }

void Keyboard_send_buffer(){
  Serial.println("Sending Buffer");
  for(int i = 0; i < reportBuffer.size(); i++){
    if((i%4)==0){
      delay(delayTime);
    }
    bleKeyboard.sendReport(reportBuffer.at(i));
    bleKeyboard.releaseAll();
    for(int n = 0; n < 6; n++){
      Serial.print(reportBuffer.at(i)->keys[n]);
    }
    Serial.println(reportBuffer.at(i)->modifiers);
  }
  clear_send_buffer();
}

void clear_send_buffer(){
  for(int i = 0; i < reportBuffer.size(); i++){
    delete(reportBuffer.at(i));
  }
  reportBuffer.clear();
}

void receiveEvent(int howMany){
  uint8_t itterator = reportBuffer.size();
  Serial.println("recieve Event");
  while(Wire.available()){
    uint8_t check = Wire.read();
    if(check == 0x07){
      while(Wire.available()){
        sendString += (char)Wire.read();
        messageAvailable = 1;
      }
    }
    else if(check == 0x05){
      //clear reportBuffer
      Serial.println("got shit!");
      //for(int i = 0; i < 6; i++){
      //  reportBuffer.keys[i] = 0;
      //}
      //reportBuffer.modifiers = 0;

      //Get 6 bytes as keycodes
      while(Wire.available()){
        reportBuffer.push_back(new KeyReport);
        for(int i = 0; i < 6; i++){
          if(Wire.available()){
            reportBuffer.at(itterator)->keys[i] = Wire.read();
          }
          Serial.print(reportBuffer.at(itterator)->keys[i]);
        }
        //Get modifier
        if(Wire.available()){
          reportBuffer.at(itterator)->modifiers = Wire.read();
        }
        Serial.print(reportBuffer.at(itterator)->modifiers);
        //send
        //if(bleKeyboard.isConnected()){
        //  bleKeyboard.sendReport(reportBuffer.at(itterator));
        //  bleKeyboard.releaseAll();
        //}
        itterator++;
      }
    }
  }
  messageAvailable = 1;
}

