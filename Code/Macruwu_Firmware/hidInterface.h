#include <vector>
#ifndef USB_INTERFACE
#define USB_INTERFACE

#include "filesystem.h"
#include "MIDI.h"
#include "Wire.h"

class hidInterface{
public:
  hidInterface(int bluetooth_timeout, uint8_t bluetooth_address);
  void init();
  void press(key * inputKey);       //send single key
  void release(key * inputKey);     //release single key
  void sendMacro(key * inputKey);
  void sendMidi_Analog(key * inputKey, uint8_t value);
  void sendMidi_Digital(key * inputKey, bool state);
  bool readMidi();
  bool get_midi_CC_state(uint8_t channel, uint8_t number);
  bool get_midi_CC_state(key * inputKey);
  bool get_midi_CC_update_available(uint8_t channel, uint8_t number);
  bool get_midi_CC_update_available(key * inputKey);
  void set_midi_CC_update_handled(uint8_t channel, uint8_t number);
  void set_midi_CC_update_handled(key * inputKey);
  void clear_midi_CC_update_available();
  bool bluetooth_mode = 0;
private:
  int bluetooth_timeout;
  uint8_t bluetooth_address;
  uint8_t keycodeBuffer[6];
  uint8_t modifier;
  void send(uint8_t reportID);
  void sendEmpty(uint8_t reportID);
  void clear(uint8_t reportID);
  uint8_t midi_value_storage[16][256];
  std::vector<uint16_t> midi_signals_handled;
};


#endif //USB_INTERFACE