#include "class/hid/hid.h"
#include "Keyboard.h"



// Report ID
enum
{
  RID_KEYBOARD = 1,
  RID_GAMEPAD,
  RID_CONSUMER_CONTROL, // Media, volume etc ..
};


uint8_t const desc_hid_report[] =
{
  TUD_HID_REPORT_DESC_KEYBOARD( HID_REPORT_ID(RID_KEYBOARD) ),
  TUD_HID_REPORT_DESC_GAMEPAD(HID_REPORT_ID(RID_GAMEPAD)),
  TUD_HID_REPORT_DESC_CONSUMER( HID_REPORT_ID(RID_CONSUMER_CONTROL) )
};
uint8_t const conv_table[128][2] =  { HID_ASCII_TO_KEYCODE_GERMAN };

Adafruit_USBD_HID usb_hid(desc_hid_report, sizeof(desc_hid_report), HID_ITF_PROTOCOL_NONE, 2, false);

hid_gamepad_report_t Gamepad;

///////////////////////////////////////////////////////////////////Gamepad/////////////////////////////////////////////////////////////////////

void Keyboard::gamepad_test_reset(){
  Gamepad.x       = 0;
  Gamepad.y       = 0;
  Gamepad.z       = 0;
  Gamepad.rz      = 0;
  Gamepad.rx      = 0;
  Gamepad.ry      = 0;
  Gamepad.hat     = 0;
  Gamepad.buttons = 0;
  usb_hid.sendReport(0, &Gamepad, sizeof(Gamepad));
}

bool gamepad_state[32] = {0};

void Keyboard::gamepad_test(int test_state, int logic){
  
  switch(gamepad_state[test_state]){
    case 0:
      if(logic){
        Gamepad.buttons |= (1U << test_state);
        gamepad_state[test_state] = 1;
      }
      break;
    case 1:
      if(!logic){
        Gamepad.buttons &= ~(1U << test_state);
        gamepad_state[test_state] = 0;
      }
      break;
  }
}

void Keyboard::gamepad_send_test(){
  while(!usb_hid.ready());
  delay(3);
  usb_hid.sendReport(RID_GAMEPAD, &Gamepad, sizeof(Gamepad));
}

/////////////////////////////////////////////////////////////////////////Keyboard////////////////////////////////////////////////////////////////////////////

Keyboard::Keyboard(String filename, int ammountLayers, int ammountKeys, uint8_t bluetoothAddress){
    this->filename = filename;
    this->ammountLayers = ammountLayers;
    this->ammountKeys = ammountKeys;
}

void Keyboard::init(){          //sets up flash and runs initial read of file
  if(HIDEnabled){
    usb_hid.begin();
    int i = 0;
    while( !TinyUSBDevice.mounted() && (i < 30)){
      delay(100);
      i++;
      Serial.print("Crashing");
    }
    if(i >=30){
      this->bluetoothMode = 1;
      Serial.print("Bluetooth mode started!");
    }
  }
  keymap_ptr = std::unique_ptr<Keymap>(new Keymap(filename, ammountLayers, ammountKeys));
  keymap_ptr->init();
  interpreterBuffer.push_back(new Keysycode(0,0,RID_KEYBOARD));
  flash_setup();
  readFile();
}

void Keyboard::disableHID(){
  HIDEnabled = 0;
}

bool Keyboard::HIDdisabled(){
  return !HIDEnabled;
}

void Keyboard::readFile(){
    keymap_ptr->import();
}

String Keyboard::get_string_from_file(int layer, int button){
    return keymap_ptr->get(layer, button);
}

void Keyboard::set_string_in_file(int layer, int button, String string_to_write){
    keymap_ptr->set(layer, button, string_to_write);
}

///////////////////////////////////////////////////////////////////////////////////////////Interpreter//////////////////////////////////////////////////////////////////////////////////////////////////
//____________________________________________________________________________________________________________________________________________________________________________________________________//
uint8_t Keyboard::interpret(int layer, int button){
  if(HIDEnabled == 0){
    return 0;
  }
  Serial.println("uwu");
  String stringToInterpret = keymap_ptr->get(layer,button);
  char charToInterpret;
  bool inBuffer = 0;
  String bufferString;
  for(uint32_t i = 0; i<stringToInterpret.length(); i++){
    charToInterpret = stringToInterpret.charAt(i);
    #ifdef SERIAL_DEBUG
    Serial.print(this->plainFlag);
    #endif
/////////////////////////////////////////////////////////////////Commands/////////////////////////////////////////////////////////////////
    if(this->plainFlag == 0){
      if(charToInterpret == 92){                  //checks for '\'
        if(stringToInterpret.charAt(i+1) == 'n'){
          interpreterBuffer.push_back(new Keysycode(HID_KEY_ENTER,0,RID_KEYBOARD));
          i++;
          continue;
        }
        if(stringToInterpret.charAt(i+1) == 't'){
          interpreterBuffer.push_back(new Keysycode(HID_KEY_TAB,0,RID_KEYBOARD));
          i++;
          continue;
        }
        if(stringToInterpret.indexOf("plain{", i+1) == i+1){
          i += 6;
          this->plainFlag = 1;
          #ifdef SERIAL_DEBUG
          Serial.print("plain flag set");
          #endif
          continue;
        }
        if(stringToInterpret.indexOf("STRG", i+1) == i+1){
          i += 4;
          interpreterBuffer.push_back(new Keysycode(HID_KEY_CONTROL_LEFT,0,RID_KEYBOARD));
          continue;
        }
        if(stringToInterpret.indexOf("SHIFT", i+1) == i+1){
          i += 5;
          interpreterBuffer.push_back(new Keysycode(HID_KEY_SHIFT_LEFT,0,RID_KEYBOARD));
          continue;
        }
        if(stringToInterpret.indexOf("ALTGR", i+1) == i+1){
          i += 5;
          interpreterBuffer.push_back(new Keysycode(HID_KEY_ALT_RIGHT,0,RID_KEYBOARD));
          continue;
        }
        if(stringToInterpret.indexOf("ALT", i+1) == i+1){
          i += 3;
          interpreterBuffer.push_back(new Keysycode(HID_KEY_ALT_LEFT,0,RID_KEYBOARD));
          continue;
        }
        if(stringToInterpret.indexOf("TAB", i+1) == i+1){
          i += 3;
          interpreterBuffer.push_back(new Keysycode(HID_KEY_TAB,0,RID_KEYBOARD));
          continue;
        }
        if(stringToInterpret.indexOf("SEND", i+1) == i+1){
          i += 4;
          interpreterBuffer.push_back(new Keysycode(0,0,RID_KEYBOARD,1));
          continue;
        }
        if(stringToInterpret.indexOf("SWITCH{", i+1) == i+1){
          i += 7;
          uint8_t brc = stringToInterpret.indexOf("}", i);
          uint8_t layerIndex = 0;
          if(brc != -1 && (brc-i) <= 3){
            String num = stringToInterpret.substring(i+1, brc);
            for(int i = 0; i < num.length(); i++){
              layerIndex *= 10;
              layerIndex += num.charAt(i) - 48;
            }
            if((layerIndex <= 0)&&(layerIndex > this->ammountLayers)){
            layerIndex = this->currentLayer;
            }
            this->currentLayer = layerIndex;
            i = brc;            
          }
          continue;
        }
      }
    }
/////////////////////////////////////////////////////////////plain text/////////////////////////////////////////////////////////////////

    if((charToInterpret >= 97)&&(charToInterpret <= 122)){                                          //Small letters
      interpreterBuffer.push_back(new Keysycode(conv_table[charToInterpret][1],0,RID_KEYBOARD));
      sendString += charToInterpret;
    }
    if((charToInterpret >= 65)&&(charToInterpret <= 90)){                                           //Big letters
      interpreterBuffer.push_back(new Keysycode(conv_table[charToInterpret][1],KEYBOARD_MODIFIER_LEFTSHIFT,RID_KEYBOARD));
      sendString += charToInterpret;
    }
    if((charToInterpret >= 48)&&(charToInterpret <= 57)){                                           //Numbers
      interpreterBuffer.push_back(new Keysycode(conv_table[charToInterpret][1],0,RID_KEYBOARD));
      sendString += charToInterpret;
    }
    if((charToInterpret == '!')||(charToInterpret == 34)||(charToInterpret == '$')||(charToInterpret == 37)||(charToInterpret == '&')||(charToInterpret == '/')||(charToInterpret == '(')||(charToInterpret == ')')||(charToInterpret == '=')||(charToInterpret == '?')){
      interpreterBuffer.push_back(new Keysycode(conv_table[charToInterpret][1],KEYBOARD_MODIFIER_LEFTSHIFT,RID_KEYBOARD));
      sendString += charToInterpret;
    }
    if((charToInterpret == ' ')||(charToInterpret == '.')){                                         //special Chars
      interpreterBuffer.push_back(new Keysycode(conv_table[charToInterpret][1],0,RID_KEYBOARD));
      sendString += charToInterpret;
    }
    if(charToInterpret == '<'){                 //checks for '<'and triggers the left arrow key
        if(this->plainFlag == 1){               //If the plain flag is set with the '\plain' command, '<' is printed as a character
          interpreterBuffer.push_back(new Keysycode(0x64,0,RID_KEYBOARD));
          sendString += charToInterpret;
          continue;
        }
        interpreterBuffer.push_back(new Keysycode(HID_KEY_ARROW_LEFT,0,RID_KEYBOARD));
      }
    if(charToInterpret == '>'){                 //checks for '>' and triggers the right arrow key
      if(this->plainFlag == 1){                 //If the plain flag is set with the '\plain' command, '>' is printed as a character
        interpreterBuffer.push_back(new Keysycode(0x64,KEYBOARD_MODIFIER_LEFTSHIFT,RID_KEYBOARD));
        continue;
      }
      interpreterBuffer.push_back(new Keysycode(HID_KEY_ARROW_RIGHT,0,RID_KEYBOARD));
    }
    if(charToInterpret == '|'){                 //checks for '|'
      interpreterBuffer.push_back(new Keysycode(0x64,KEYBOARD_MODIFIER_RIGHTALT,RID_KEYBOARD));
      continue;
    }
    if(charToInterpret == '_'){                 //checks for '_'
      interpreterBuffer.push_back(new Keysycode(56,KEYBOARD_MODIFIER_LEFTSHIFT,RID_KEYBOARD));
      continue;
    }
    if(charToInterpret == '{'){                 //checks for '{'
      if(stringToInterpret.charAt(i+1) == 125){ //checks next character for '}' and if found, sends both brackets at once (to save time)
        interpreterBuffer.push_back(new Keysycode(HID_KEY_7,KEYBOARD_MODIFIER_RIGHTALT,RID_KEYBOARD));
        interpreterBuffer.push_back(new Keysycode(HID_KEY_0,KEYBOARD_MODIFIER_RIGHTALT,RID_KEYBOARD));
        i++;
      }
      else{
        interpreterBuffer.push_back(new Keysycode(HID_KEY_7,KEYBOARD_MODIFIER_RIGHTALT,RID_KEYBOARD));
      }
      continue;
    }
    if(charToInterpret == '}'){                 //checks for '}'
      if(this->plainFlag == 1){
        this->plainFlag = 0;
        //i++;
        continue;
      }
      interpreterBuffer.push_back(new Keysycode(HID_KEY_0,KEYBOARD_MODIFIER_RIGHTALT,RID_KEYBOARD));
      continue;
    }
    if(charToInterpret == '['){                 //checks for '['
        if(stringToInterpret.charAt(i+1) == ']'){   //checks next character for ']' and if found, sends both square brackets at once (to save time)
            interpreterBuffer.push_back(new Keysycode(HID_KEY_8,KEYBOARD_MODIFIER_RIGHTALT,RID_KEYBOARD));
            interpreterBuffer.push_back(new Keysycode(HID_KEY_9,KEYBOARD_MODIFIER_RIGHTALT,RID_KEYBOARD));
            i++;
        }
        else{
            interpreterBuffer.push_back(new Keysycode(HID_KEY_8,KEYBOARD_MODIFIER_RIGHTALT,RID_KEYBOARD));
        }
        continue;
    }
    if(charToInterpret == ']'){                 //checks for ']'
        interpreterBuffer.push_back(new Keysycode(HID_KEY_9,KEYBOARD_MODIFIER_RIGHTALT,RID_KEYBOARD));
        continue;
    }
    if(charToInterpret == '^'){                 //checks for '^'
      interpreterBuffer.push_back(new Keysycode(HID_KEY_GRAVE,0,RID_KEYBOARD));
      //cout<<"send funny to the power of symbol thingymabob"<<endl;
        continue;
    }
    if(charToInterpret == '-'){
      interpreterBuffer.push_back(new Keysycode(0x38, 0, RID_KEYBOARD));
      continue;
    }
    if(charToInterpret == 92){
      interpreterBuffer.push_back(new Keysycode(HID_KEY_MINUS,KEYBOARD_MODIFIER_RIGHTALT,RID_KEYBOARD));
      continue;
    }
  }
  return 1;
}
/////////////////////////////////////////////////////////////end////////////////////////////////////////////////////////////////////////

bool Keyboard::fsChanged(){
  if(check_fs_changed()){
    set_fs_changed(0);
    return 1;
  }
  return 0;
}

//////////////////////////////////////////////////////////////////////////////////////Interpreter Send/////////////////////////////////////////////////////////////////////////////

void Keyboard::send_buffer(){
  uint8_t keycodeBuffer[6] = {0};
  uint8_t bufferIndex = 0;
  
  if(HIDEnabled == 0) return;

  if(!(this->bluetoothMode)){
    //#ifdef SERIAL_DEBUG
    Serial.println("---------Send Buffer---------");
    Serial.print("Buffer Size: "); Serial.println(interpreterBuffer.size());
    Serial.println("keycode\t modifier\t reportID\t buffer");
    //#endif

    interpreterBuffer.push_back(new Keysycode(0,0,0));

    for(int i = 0; i < (interpreterBuffer.size()-1); i++){
      //#ifdef SERIAL_DEBUG
      Serial.print(interpreterBuffer.at(i)->keycode);
      Serial.print("\t\t");
      Serial.print(interpreterBuffer.at(i)->modifier);
      Serial.print("\t\t");
      Serial.print(interpreterBuffer.at(i)->reportID);
      Serial.println("\t");
      //#endif

      if(interpreterBuffer.at(i)->immediateSend == 1){      //If immediateSend flag is set, ignore Keycodes in this Keysycode and send what has been set in the Buffer. Previous modifier will be used
        uint8_t modifier = 0;
        if(i > 0){
          modifier = interpreterBuffer.at(i-1)->modifier;
        }
        while(!usb_hid.ready());
        usb_hid.keyboardReport(RID_KEYBOARD, modifier, keycodeBuffer);
        while(!usb_hid.ready());
        usb_hid.keyboardRelease(RID_KEYBOARD);
        for(int i = 0; i < 6; i++){
          keycodeBuffer[i] = 0;
        }
        i++;
      }
      if( ( bufferIndex < 6 ) && ( interpreterBuffer.at(i+1)->modifier == interpreterBuffer.at(i)->modifier ) && (interpreterBuffer.at(i+1)->keycode != interpreterBuffer.at(i)->keycode)){
        keycodeBuffer[bufferIndex] = interpreterBuffer.at(i)->keycode;
        bufferIndex++;
      }
      else{
        //send
        if(bufferIndex < 6){
          keycodeBuffer[bufferIndex] = interpreterBuffer.at(i)->keycode;
        }
        while(!usb_hid.ready());
        usb_hid.keyboardReport(RID_KEYBOARD, interpreterBuffer.at(i)->modifier, keycodeBuffer);
        while(!usb_hid.ready());
        usb_hid.keyboardRelease(RID_KEYBOARD);

        for(int i = 0; i < 6; i++){
          keycodeBuffer[i] = 0;
        }
        if(bufferIndex >= 6){
          keycodeBuffer[0] = interpreterBuffer.at(i)->keycode;
          bufferIndex = 1;
          if(interpreterBuffer.at(i+1)->modifier != interpreterBuffer.at(i)->modifier){
            while(!usb_hid.ready());
            usb_hid.keyboardReport(RID_KEYBOARD, interpreterBuffer.at(i)->modifier, keycodeBuffer);
            while(!usb_hid.ready());
            usb_hid.keyboardRelease(RID_KEYBOARD);
            bufferIndex = 0;
          }
        }
        else{
          bufferIndex = 0;
        }
      }
    }
    //send
    while(!usb_hid.ready());
    usb_hid.keyboardReport(RID_KEYBOARD, interpreterBuffer.at((interpreterBuffer.size()-1))->modifier, keycodeBuffer);
    while(!usb_hid.ready());
    usb_hid.keyboardRelease(RID_KEYBOARD);
    bufferIndex = 0;
    for(int i = 0; i < 6; i++){
      keycodeBuffer[i] = 0;
    }

    //delete interpreterBuffer
    for(int i = 0; i < interpreterBuffer.size(); i++){
      delete(interpreterBuffer.at(i));
    }
    interpreterBuffer.clear();
    interpreterBuffer.push_back(new Keysycode(0,0,0));
  }
  //Bluetooth Mode
  else{
    interpreterBuffer.push_back(new Keysycode(0,0,0));
    for(int i = 0; i < (interpreterBuffer.size()-1); i++){
      if(interpreterBuffer.at(i)->immediateSend == 1){      //If immediateSend flag is set, ignore Keycodes in this Keysycode and send what has been set in the Buffer. Previous modifier will be used
        uint8_t modifier = 0;
        if(i > 0){
          modifier = interpreterBuffer.at(i-1)->modifier;
        }
        KeycodeSendBuffer.push_back(new KeysycodeSendBuffer(keycodeBuffer, modifier));
        for(int i = 0; i < 6; i++){
          keycodeBuffer[i] = 0;
        }
        i++;
      }
      if( ( bufferIndex < 6 ) && ( interpreterBuffer.at(i+1)->modifier == interpreterBuffer.at(i)->modifier ) && (interpreterBuffer.at(i+1)->keycode != interpreterBuffer.at(i)->keycode)){
          keycodeBuffer[bufferIndex] = interpreterBuffer.at(i)->keycode;
          bufferIndex++;
        }
      else{
        //send
        if(bufferIndex < 6){
          keycodeBuffer[bufferIndex] = interpreterBuffer.at(i)->keycode;
        }
        //while(!usb_hid.ready());
        //usb_hid.keyboardReport(RID_KEYBOARD, interpreterBuffer.at(i)->modifier, keycodeBuffer);
        //while(!usb_hid.ready());
        //usb_hid.keyboardRelease(RID_KEYBOARD);
        KeycodeSendBuffer.push_back(new KeysycodeSendBuffer(keycodeBuffer, interpreterBuffer.at(i)->modifier));

        for(int i = 0; i < 6; i++){
          keycodeBuffer[i] = 0;
        }
        if(bufferIndex >= 6){
          keycodeBuffer[0] = interpreterBuffer.at(i)->keycode;
          bufferIndex = 1;
          if(interpreterBuffer.at(i+1)->modifier != interpreterBuffer.at(i)->modifier){
            //while(!usb_hid.ready());
            //usb_hid.keyboardReport(RID_KEYBOARD, interpreterBuffer.at(i)->modifier, keycodeBuffer);
            //while(!usb_hid.ready());
            //usb_hid.keyboardRelease(RID_KEYBOARD);

            KeycodeSendBuffer.push_back(new KeysycodeSendBuffer(keycodeBuffer, interpreterBuffer.at(i)->modifier));
            bufferIndex = 0;
          }
        }
        else{
          bufferIndex = 0;
        }
      }
    }
    //send
    //while(!usb_hid.ready());
    //usb_hid.keyboardReport(RID_KEYBOARD, interpreterBuffer.at((interpreterBuffer.size()-1))->modifier, keycodeBuffer);
    //while(!usb_hid.ready());
    //usb_hid.keyboardRelease(RID_KEYBOARD);

    KeycodeSendBuffer.push_back(new KeysycodeSendBuffer(keycodeBuffer, interpreterBuffer.at(interpreterBuffer.size()-1)->modifier));
    bufferIndex = 0;
    for(int i = 0; i < 6; i++){
      keycodeBuffer[i] = 0;
    }

    send_keycode_buffer();

    //delete KeycodeSendBuffer
    for(int i = 0; i < KeycodeSendBuffer.size(); i++){
      delete(KeycodeSendBuffer.at(i));
    }
    KeycodeSendBuffer.clear();

    //delete interpreterBuffer
    for(int i = 0; i < interpreterBuffer.size(); i++){
      delete(interpreterBuffer.at(i));
    }
    interpreterBuffer.clear();
    interpreterBuffer.push_back(new Keysycode(0,0,0));
  }
}

void Keyboard::send_keycode_buffer(){
  Wire.beginTransmission(0x69);
  Wire.write(0x05);
  for(uint8_t i = 0; i < KeycodeSendBuffer.size(); i++){
    if((i%12 == 0)&&(i!=0)){
      Wire.endTransmission();
      Wire.beginTransmission(0x69);
      Wire.write(0x05);
    }
    for(uint8_t n = 0; n < 6; n++){
      Wire.write(KeycodeSendBuffer.at(i)->keycodes[n]);
      Serial.print(KeycodeSendBuffer.at(i)->keycodes[n]);
    }
    Wire.write(KeycodeSendBuffer.at(i)->modifier);
    Serial.println(KeycodeSendBuffer.at(i)->modifier);
  }
  Wire.endTransmission();
}


//////////////////////////////////////////////////////Keysycode////////////////////////////////////////////////////////////////////////////////

Keysycode::Keysycode(uint8_t keycode, uint8_t modifier, uint8_t reportID, uint8_t immediateSend){
  this->keycode = keycode;
  this->modifier = modifier;
  this->reportID = reportID;
  this->immediateSend = immediateSend;
}

////////////////////////////////////////////////KeysycodeSendBuffer///////////////////////////////////////////////////////////////////////////

KeysycodeSendBuffer::KeysycodeSendBuffer(uint8_t *keycodes, uint8_t modifier){
  for(int i = 0; i < 6; i++){
    this->keycodes[i] = keycodes[i];
  }
  this->modifier = modifier;
}

