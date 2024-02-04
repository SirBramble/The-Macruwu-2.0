#include "hidInterface.h"

uint8_t const desc_hid_report[] =
{
  TUD_HID_REPORT_DESC_KEYBOARD( HID_REPORT_ID(RID_KEYBOARD) ),
  TUD_HID_REPORT_DESC_GAMEPAD(HID_REPORT_ID(RID_GAMEPAD)),
  TUD_HID_REPORT_DESC_CONSUMER( HID_REPORT_ID(RID_CONSUMER_CONTROL) )
};

Adafruit_USBD_HID usb_hid(desc_hid_report, sizeof(desc_hid_report), HID_ITF_PROTOCOL_NONE, 2, false);
Adafruit_USBD_MIDI usb_midi;
MIDI_CREATE_INSTANCE(Adafruit_USBD_MIDI, usb_midi, MIDI);

void handleNoteOn(byte channel, byte pitch, byte velocity);
void handleNoteOff(byte channel, byte pitch, byte velocity);
void handleControlChange(byte channel, byte number, byte value);
void handleProgramChange(byte channel, byte number);

bool midi_CC_state_storage[16][256];
bool midi_CC_update_available[16][256];

//////////////////////////////USB_INTERFACE//////////////////////////

hidInterface::hidInterface(int bluetooth_timeout, uint8_t bluetooth_address){
  this->modifier = 0;
  this->bluetooth_timeout = bluetooth_timeout;
  this->bluetooth_address = bluetooth_address;
}

void hidInterface::init(){
  // USB HID
  usb_hid.begin();

  // MIDI
  usb_midi.setStringDescriptor("Typeuwu MIDI");
  MIDI.begin(MIDI_CHANNEL_OMNI);
  MIDI.turnThruOff();
  // Handlers
  MIDI.setHandleNoteOn(handleNoteOn);
  MIDI.setHandleNoteOff(handleNoteOff);
  MIDI.setHandleControlChange(handleControlChange);
  MIDI.setHandleProgramChange(handleProgramChange);

  while(!TinyUSBDevice.mounted() && this->bluetooth_timeout > 0){
    delay(100);
    this->bluetooth_timeout -= 100;
    Serial.println("crashing...");
  }

  if(bluetooth_timeout <= 0){
    this->bluetooth_mode = 1;
  }

  for(int i = 0; i < 6; i++){
    this->keycodeBuffer[i] = 0;
  }
  //for(int i = 0; i < 16; i++){
  //  for(int n = 0; n < 256; n++){
  //    midi_value_storage[i][n] = 0;
  //  }
  //}

  for(int i = 0; i < 16; i++){
    for(int n = 0; n < 256; n++){
      midi_CC_state_storage[i][n] = false;
    }
  }
}

void hidInterface::press(key * inputKey){
  if(inputKey == nullptr) return;
  Serial.print("isSingle: "); Serial.println(inputKey->isSingleKey);
  if(inputKey->isSingleKey == 0){
    this->sendMacro(inputKey);
    return;
  }
  if(inputKey->isAnalog == 1) return;

  keysycode inputKeycode = inputKey->getKeysycode(0);

  if(inputKey->isModifier && ((this->modifier & inputKeycode.modifier) == 0)){
    this->modifier += inputKeycode.modifier;
  }
  Serial.print("KeycodeBuffer: ");
  for(uint8_t i = 0; i < 6; i++){
    Serial.print(keycodeBuffer[i]);
    if(this->keycodeBuffer[i] == 0){
      keycodeBuffer[i] = inputKeycode.keycode;
      break;
    }
  }
  Serial.println();
  //this->sendEmpty(inputKeycode.reportID);
  this->send(inputKeycode.reportID);

  return;
}

void hidInterface::release(key * inputKey){
  if(inputKey == nullptr) return;
  Serial.print("input keysycode: "); Serial.println(inputKey->getKeysycode(0).keycode);
  Serial.print("input keysycode 1: "); Serial.println(inputKey->getKeysycode(1).keycode);
  if(inputKey->isSingleKey == 0) return;
  if(inputKey->isAnalog == 1) return;

  keysycode inputKeycode = inputKey->getKeysycode(0);
  if(inputKey->isModifier && ((this->modifier & inputKeycode.modifier) > 0)){
    this->modifier -= inputKeycode.modifier;
  }

  for(uint8_t i = 0; i < 6; i++){
    if(this->keycodeBuffer[i] == inputKeycode.keycode){
      keycodeBuffer[i] = 0;
    }
  }

  this->send(inputKeycode.reportID);
  
  return;
}

void hidInterface::sendMacro(key * inputKey){
  if(inputKey == nullptr) return;
  
  int keycodeBufferIndex = 0;

  this->clear(inputKey->getKeysycode(0).reportID);

  for(int i = 0; i < inputKey->getKeycodesSize(); i++){
    for(uint8_t n = 0; n < keycodeBufferIndex; n++){
      if(this->keycodeBuffer[n] == inputKey->getKeysycode(i).keycode){
        this->send(inputKey->getKeysycode(i).reportID);
        this->clear(inputKey->getKeysycode(i).reportID);
        keycodeBufferIndex = 0;
        break;
      }
    }
    this->keycodeBuffer[keycodeBufferIndex] = inputKey->getKeysycode(i).keycode;
    this->modifier = inputKey->getKeysycode(i).modifier;
    if(inputKey->getKeysycode(i).immediateSend || keycodeBufferIndex >= 5){
      this->send(inputKey->getKeysycode(i).reportID);
      this->clear(inputKey->getKeysycode(i).reportID);
      keycodeBufferIndex = 0;
    }
    else keycodeBufferIndex++;
  }

  if(keycodeBufferIndex > 0){
    this->send(inputKey->getKeysycode(inputKey->getKeycodesSize()-1).reportID);
    this->clear(inputKey->getKeysycode(inputKey->getKeycodesSize()-1).reportID);
  }
  return;
}

void hidInterface::send(uint8_t reportID){
  if(!this->bluetooth_mode){
    while(!usb_hid.ready());
    usb_hid.keyboardReport(reportID, this->modifier, this->keycodeBuffer);
    Serial.printf("sent Report with: Buffer: %02X,%02X,%02X,%02X,%02X,%02X  Modifier: %02X\n", this->keycodeBuffer[0],this->keycodeBuffer[1],this->keycodeBuffer[2],this->keycodeBuffer[3],this->keycodeBuffer[4],this->keycodeBuffer[5], this->modifier);
  }
  else{
    Wire.beginTransmission(this->bluetooth_address);
    Wire.write(reportID);
    Wire.write(this->modifier);
    for(int i = 0; i < 6; i++){
      Wire.write(keycodeBuffer[i]);
    }
    Wire.endTransmission();
  }
  return;
}

void hidInterface::sendEmpty(uint8_t reportID){
  uint8_t emptyBuffer[6] = {0,0,0,0,0,0};
  if(!this->bluetooth_mode){
    while(!usb_hid.ready());
    usb_hid.keyboardReport(reportID, 0, emptyBuffer);
    Serial.printf("sent empty Report with: Buffer: %02X,%02X,%02X,%02X,%02X,%02X  Modifier: %02X\n",emptyBuffer[0],emptyBuffer[1],emptyBuffer[2],emptyBuffer[3],emptyBuffer[4],emptyBuffer[5], 0);
  }
  else{
    Wire.beginTransmission(this->bluetooth_address);
    Wire.write(reportID);
    Wire.write(0);
    for(int i = 0; i < 6; i++){
      Wire.write(0);
    }
    Wire.endTransmission();
  }
  return;
}

void hidInterface::clear(uint8_t reportID){
  for(uint8_t i = 0; i < 6; i++){
    this->keycodeBuffer[i] = 0;
  }
  this->modifier = 0;
  this->send(reportID);
  //while(!usb_hid.ready());
  //usb_hid.keyboardReport(reportID, this->modifier, this->keycodeBuffer);

  return;
}

//////////////////////////////MIDI_INTERFACE//////////////////////////

void hidInterface::sendMidi_Analog(key * inputKey, uint8_t value){
  if(!inputKey->isMIDI) {
    Serial.printf("nullptr\n");
    return;
  }
  if(value != midi_value_storage[inputKey->MIDI_channel][inputKey->MIDI_data1]){
    midi_value_storage[inputKey->MIDI_channel][inputKey->MIDI_data1] = value;
    switch (inputKey->MIDI_mode) {
      case MIDI_CC:
        MIDI.sendControlChange(inputKey->MIDI_data1, value, inputKey->MIDI_channel);
        Serial.printf("Sending MIDI CC value: %d on channel %d\n", value, inputKey->MIDI_channel);
        break;
      default:
        break;
    }
  }
}

bool hidInterface::readMidi(){
  return MIDI.read();
}

bool hidInterface::get_midi_CC_state(uint8_t channel, uint8_t number){
  return midi_CC_state_storage[channel][number];
}

bool hidInterface::get_midi_CC_update_available(uint8_t channel, uint8_t number){
  return midi_CC_update_available[channel][number];
}

void hidInterface::set_midi_CC_update_handled(uint8_t channel, uint8_t number){
  this->midi_signals_handled.push_back(((uint16_t)(channel))<<8 + (uint16_t)(number));
}

void hidInterface::clear_midi_CC_update_available(){
  int itteration = 0;
  while(!this->midi_signals_handled.empty() && itteration < MAX_WHILE_ITTERATIONS){
    uint16_t tmp = midi_signals_handled.back();
    midi_CC_update_available[(uint8_t)(tmp>>8)][(uint8_t)(tmp)] = false;
    this->midi_signals_handled.pop_back();
    itteration++;
  }
}

void handleNoteOn(byte channel, byte pitch, byte velocity)
{
  // Log when a note is pressed.
  Serial.print("Note on: channel = ");
  Serial.print(channel);

  Serial.print(" pitch = ");
  Serial.print(pitch);

  Serial.print(" velocity = ");
  Serial.println(velocity);
}

void handleNoteOff(byte channel, byte pitch, byte velocity)
{
  // Log when a note is released.
  Serial.print("Note off: channel = ");
  Serial.print(channel);

  Serial.print(" pitch = ");
  Serial.print(pitch);

  Serial.print(" velocity = ");
  Serial.println(velocity);
}

void handleControlChange(byte channel, byte number, byte value){
  // Log when a note is released.
  Serial.print("Control Change: channel: ");
  Serial.print(channel);

  Serial.print(" number: ");
  Serial.print(number);

  Serial.print(" value: ");
  Serial.println(value);

  midi_CC_state_storage[channel][number] = (value > 126)?true:false;
  midi_CC_update_available[channel][number] = true;

  Serial.printf("midi_CC_state_storage[channel][number] = %d\n",midi_CC_state_storage[channel][number]);
}
void handleProgramChange(byte channel, byte number){
  Serial.print("Program Change: channel = ");
  Serial.print(channel);

  Serial.print(" number = ");
  Serial.print(number);
}

