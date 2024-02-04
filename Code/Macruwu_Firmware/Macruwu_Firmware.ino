#include "modules.h"
#include "hidInterface.h"
#include "lighting.h"
#include "i2cinterface.h"

//#define SERIAL_DEBUG
#define ADDRESS_MODULE1 0x20
#define ADDRESS_Bluetooth 0x69

// Uncomment to enable
#define IS_MACRUWU

uint8_t registered_I2C_addresses[] = {ADDRESS_MODULE1,ADDRESS_Bluetooth};

_macruwu macruwu("macruwu",2);

i2cInterface i2c(registered_I2C_addresses, sizeof(registered_I2C_addresses));
hidInterface hid(1000, ADDRESS_Bluetooth);
RGB_LIGHTING light(29, 32, 50);    // RGB_LIGHTING(uint8_t led_pin, uint16_t led_count, uint8_t led_max_brightness)


uint64_t prev;
uint8_t error;

bool miditrigger = 0;
int midibrokencount = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("uwu Setup 1");

  filesystemSetup();
  Serial.println("uwu Setup 2");

  filesystemCreateConfig();
  Serial.println("uwu Setup 3");

  hid.init();
  light.setup();
  light.set_led_remap(macruwu.get_led_remap(), macruwu.get_ammount_keys());
  Serial.println("uwu Setup 4");

  Serial.println("uwu Setup 5");

  macruwu.updateKeymapsFromFile();

  i2c.init();     // REMEMBER: if you update the files, you need to disable the setting of the i2C pins. This will cause the controller to crash
  macruwu.init(); // Dependant on I2C!! Call after i2c.init()!!

  if(hid.bluetooth_mode == 0){
    error = i2c.disableESP(ADDRESS_Bluetooth);
  }

  prev = 0;
}





void loop() {

  light.update();
  light.set_effect(macruwu.getLayerLightingEffect());

  if(check_fs_changed()){
    macruwu.updateKeymapsFromFile();
    Serial.printf("curren Layer: %d\n", macruwu.current_layer);
    set_fs_changed(false);
  }

  if(millis() > prev + 1000){
    prev = millis();
    //i2c.probe();
    #ifdef SERIAL_DEBUG
      Serial.println("uwu");
      if(error == 0) Serial.println("ESP disabled successfully");
      else Serial.print("ESP has given error on disable: ");Serial.println(error);
    #endif
  }

  macruwu.update();

  for(int i = 0; i < 32; i++){
    key *currentKey = macruwu.getKeyPointer(i + 1);

    if(currentKey->color_mode == midi_bound){
      if(hid.get_midi_CC_state(currentKey->MIDI_channel, currentKey->MIDI_data1) && hid.get_midi_CC_update_available(currentKey->MIDI_channel, currentKey->MIDI_data1)){
        hid.set_midi_CC_update_handled(currentKey->MIDI_channel, currentKey->MIDI_data1);
        light.enable_override(i, currentKey->color);
      }
      else if(hid.get_midi_CC_update_available(currentKey->MIDI_channel, currentKey->MIDI_data1)){
        hid.set_midi_CC_update_handled(currentKey->MIDI_channel, currentKey->MIDI_data1);
        light.disable_override(i);
      }
    }

    if(macruwu.isPressed(i)){
      if(currentKey->hasLayerChange){
        macruwu.setLayer(currentKey->changeToLayer);
      }
      if(currentKey->isMIDI){
        hid.sendMidi_Analog(macruwu.getKeyPointer(i + 1), 255);
      }
      else{
        Serial.printf("pressing index: %d\n", i);
        hid.press(currentKey);
      }
      switch(currentKey->color_mode){
        case no_override:
          break;
        case midi_bound:
          break;
        case pressed:
          light.enable_override(i, currentKey->color);
          break;
        case not_pressed:
          light.disable_override(i);
          break;
        case toggle:
          light.toggle_override(i, currentKey->color);
          break;
        case disabled:
          light.disable_override(i);
      }
    }
    else if(macruwu.isReleased(i)){
      if(currentKey->isMIDI){
        hid.sendMidi_Analog(macruwu.getKeyPointer(i + 1), 0);
      }
      else{
        Serial.println("releasing");
        hid.release(macruwu.getKeyPointer(i + 1));
      }

      switch(currentKey->color_mode){
        case no_override:
          break;
        case midi_bound:
          break;
        case pressed:
          light.disable_override(i);
          break;
        case not_pressed:
          light.enable_override(i, currentKey->color);
          break;
        case toggle:
          break;
        case disabled:
          light.disable_override(i);
      }
    }
  }

  hid.clear_midi_CC_update_available();

  //rainbow(10);
  while(hid.readMidi()) hid.readMidi();
  delay(1);
}