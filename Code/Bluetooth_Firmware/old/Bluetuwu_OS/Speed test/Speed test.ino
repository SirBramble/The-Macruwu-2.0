/**
 * if Stuff no work, check this: https://github.com/T-vK/ESP32-BLE-Keyboard/issues/169
 */
#include "BleKeyboard.h"

BleKeyboard bleKeyboard("Macruwu", "FG Labs", 69);

uint8_t delayTime = 60;

int samples = 1000;
int counter = 1000;

void Keyboard_print(String text, uint8_t delayTime){
    delay(delayTime);
    if(text.length()>15){
      bleKeyboard.print(text.substring(0,15));
      text.remove(0, 15);
      return Keyboard_print(text, delayTime);
    }
    bleKeyboard.print(text);
  }

void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE work!");
  bleKeyboard.begin();
  bleKeyboard.setBatteryLevel(69);
}



void loop() {
  String s = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789\n";
  if(bleKeyboard.isConnected() && (delayTime > 0)) {
    //bleKeyboard.write(c, sizeof(c));
    
    if(counter < samples){
      counter++;
      Keyboard_print(s, delayTime);
    }
    else{
      counter = 0;
      delayTime -= 5;
      Keyboard_print("delayTime: ", 100);
      Keyboard_print(String(delayTime), 100);
      Keyboard_print("\n", 100);

    }
    /*
    if(delayTime > 0){
      delayTime--;
    }
    else{
      delayTime = 100;
    }
    */

    //delay(100);

    //Serial.println("Sending Enter key...");
    //bleKeyboard.write(KEY_RETURN);

    //delay(1000);

    //Serial.println("Sending Play/Pause media key...");
    //bleKeyboard.write(KEY_MEDIA_PLAY_PAUSE);

    //delay(1000);

   //
   // Below is an example of pressing multiple keyboard modifiers 
   // which by default is commented out.
    /*
    Serial.println("Sending Ctrl+Alt+Delete...");
    bleKeyboard.press(KEY_LEFT_CTRL);
    bleKeyboard.press(KEY_LEFT_ALT);
    bleKeyboard.press(KEY_DELETE);
    delay(100);
    bleKeyboard.releaseAll();
    */
  }

  //Serial.println("Waiting 5 seconds...");
  //delay(5000);
}
