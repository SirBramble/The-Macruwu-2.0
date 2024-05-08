#include "modules.h"
#include "hidInterface.h"
#include "lighting.h"
#include "i2cinterface.h"

//#define SERIAL_DEBUG


/////////////////////////////////////////////// Choose your base ///////////////////////////////////////////////
#define IS_MACRUWU
//#define IS_KEYBOARD


#define MACRUWU_VERSION 2

#define ADDRESS_NUMPAD1 0x20
#define ADDRESS_Bluetooth 0x69

uint8_t registered_I2C_addresses[] = {ADDRESS_NUMPAD1,ADDRESS_Bluetooth};

#ifdef IS_MACRUWU
_macruwu macruwu("macruwu", MACRUWU_VERSION);
RGB_LIGHTING lighting_macruwu(29, 32, 50);    // RGB_LIGHTING(uint8_t led_pin, uint16_t led_count, uint8_t led_max_brightness)
#endif

#ifdef IS_KEYBOARD
RGB_LIGHTING lighting_keyboard(1, 88, 50);    // RGB_LIGHTING(uint8_t led_pin, uint16_t led_count, uint8_t led_max_brightness)
_keyboard keyboard("keyboard");
_numpad numpad1("numpad1", ADDRESS_NUMPAD1);
#endif

i2cInterface i2c(registered_I2C_addresses, sizeof(registered_I2C_addresses));
hidInterface hid(1000, ADDRESS_Bluetooth);

uint64_t prev;
uint8_t error;


void setup() {
  Serial.begin(115200);
  Serial.println("uwu Setup 1");

  filesystemSetup();
  Serial.println("uwu Setup 2");

  filesystemCreateConfig();
  Serial.println("uwu Setup 3");

  hid.init();

  Serial.println("uwu Setup 4");

  Serial.println("uwu Setup 5");

  i2c.init();     // REMEMBER: if you update the files, you need to disable the setting of the i2C pins. This will cause the controller to crash

  #ifdef IS_KEYBOARD
  lighting_keyboard.setup();
  lighting_keyboard.set_led_remap(keyboard.get_led_remap(), keyboard.get_ammount_keys());
  keyboard.init();
  numpad1.init(&i2c);
  #endif

  #ifdef IS_MACRUWU
  macruwu.init(); // Dependant on I2C!! Call after i2c.init()!!
  macruwu.updateKeymapsFromFile();
  lighting_macruwu.setup();
  lighting_macruwu.set_led_remap(macruwu.get_led_remap(), macruwu.get_ammount_keys());
  
  #endif

  if(hid.bluetooth_mode == 0){
    error = i2c.disableESP(ADDRESS_Bluetooth);
  }

  prev = 0;
}



/////////////////////////////////////////////////////////////// Loop ///////////////////////////////////////////////////////////////

void loop() {

  check_fs_update();

  scanI2C();

  #ifdef IS_KEYBOARD
  lighting_keyboard.update();
  lighting_keyboard.set_effect(keyboard.getLayerLightingEffect());

  handleKeyboard();
  handleNumpad();
  #endif

  #ifdef IS_MACRUWU
  lighting_macruwu.update();
  lighting_macruwu.set_effect(macruwu.getLayerLightingEffect());

  handleMacruwu();
  #endif

  hid.clear_midi_CC_update_available();

  while(hid.readMidi()) hid.readMidi();
  delay(1);
}

/////////////////////////////////////////////////////////////// Macruwu Handler ///////////////////////////////////////////////////////////////
#ifdef IS_MACRUWU
inline void handleMacruwu(){
  macruwu.update();

  for(int i = 0; i < 32; i++){
    key *currentKey = macruwu.getKeyPointer(i + 1);

    if(currentKey->color_mode == midi_bound && hid.get_midi_CC_update_available(currentKey)){
      if(hid.get_midi_CC_state(currentKey) ){
        lighting_macruwu.enable_override(i, currentKey->color);
        hid.set_midi_CC_update_handled(currentKey);
      }
      else {
        lighting_macruwu.disable_override(i);
        hid.set_midi_CC_update_handled(currentKey);
      }
    }

    if(macruwu.isPressed(i)){
      if(currentKey->hasLayerChange) macruwu.setLayer(currentKey->changeToLayer);
      
      if(currentKey->isMIDI) hid.sendMidi_Analog(macruwu.getKeyPointer(i + 1), 255);
      
      else Serial.printf("pressing index: %d\n", i); hid.press(currentKey);
      
      switch(currentKey->color_mode){
        case no_override: break;
        case midi_bound: break;
        case pressed: lighting_macruwu.enable_override(i, currentKey->color); break;
        case not_pressed: lighting_macruwu.disable_override(i); break;
        case toggle: lighting_macruwu.toggle_override(i, currentKey->color); break;
        case disabled: lighting_macruwu.disable_override(i); break;
      }
    }
    else if(macruwu.isReleased(i)){
      if(currentKey->isMIDI) hid.sendMidi_Analog(macruwu.getKeyPointer(i + 1), 0);
      
      else Serial.println("releasing"); hid.release(macruwu.getKeyPointer(i + 1));
      

      switch(currentKey->color_mode){
        case no_override: break;
        case midi_bound: break;
        case pressed: lighting_macruwu.disable_override(i); break;
        case not_pressed: lighting_macruwu.enable_override(i, currentKey->color); break;
        case toggle: break;
        case disabled: lighting_macruwu.disable_override(i); break;
      }
    }
  }
}
#endif

/////////////////////////////////////////////////////////////// Keyboard Handler ///////////////////////////////////////////////////////////////
#ifdef IS_KEYBOARD
inline void handleKeyboard(){
  keyboard.update();

  for(int i = 0; i < AMMOUNT_KEYS; i++){
    key *currentKey = keyboard.getKeyPointer(i + 1);

    if(currentKey->color_mode == midi_bound && hid.get_midi_CC_update_available(currentKey)){
      if(hid.get_midi_CC_state(currentKey)){
        lighting_keyboard.enable_override(i, currentKey->color);
        hid.set_midi_CC_update_handled(currentKey);
      }
      else {
        hid.set_midi_CC_update_handled(currentKey);
        lighting_keyboard.disable_override(i);
      }
    }

    if(currentKey->color_mode == const_color) lighting_keyboard.enable_override(i, currentKey->color);

    if(currentKey->color_mode == disabled) lighting_keyboard.enable_override(i, 0);

    if(keyboard.isPressed(i)){
      if(currentKey->hasLayerChange) keyboard.setLayer(currentKey->changeToLayer);

      if(currentKey->isMIDI) hid.sendMidi_Analog(keyboard.getKeyPointer(i + 1), 255);
      
      else Serial.println("pressing"); hid.press(currentKey);
      
      switch(currentKey->color_mode){
        case no_override: break;
        case midi_bound: break;
        case pressed: lighting_keyboard.enable_override(i, currentKey->color); break;
        case not_pressed: lighting_keyboard.disable_override(i); break;
        case toggle: lighting_keyboard.toggle_override(i, currentKey->color); break;
        case disabled: lighting_keyboard.disable_override(i); break;
      }
    }
    else if(keyboard.isReleased(i)){
      if(currentKey->isMIDI) hid.sendMidi_Analog(keyboard.getKeyPointer(i + 1), 0);
      
      else Serial.println("releasing"); hid.release(keyboard.getKeyPointer(i + 1));

      switch(currentKey->color_mode){
        case no_override: break;
        case midi_bound: break;
        case pressed: lighting_keyboard.disable_override(i); break;
        case not_pressed: lighting_keyboard.enable_override(i, currentKey->color); break;
        case toggle: break;
        case disabled: lighting_keyboard.disable_override(i);
      }
    }
  }
}
#endif

/////////////////////////////////////////////////////////////// Numpad Handler ///////////////////////////////////////////////////////////////

#ifdef IS_KEYBOARD
inline void handleNumpad(){
  
  numpad1.update();

  if(numpad1.registered()){
    for(int i = 0; i < I2C_BUFFER_SIZE; i++){
      key *currentKey = numpad1.getKeyPointer(i + 1);
      if(currentKey == nullptr) continue;

      // Key color handler
      if(currentKey->color_mode == midi_bound && hid.get_midi_CC_update_available(currentKey)){
        if (hid.get_midi_CC_state(currentKey)) {
          i2c.send_led_override_update(numpad1.address(), numpad1.remap_led_key(i), true);
          hid.set_midi_CC_update_handled(currentKey);
        }
        else {
          i2c.send_led_override_update(numpad1.address(), numpad1.remap_led_key(i), false);
          hid.set_midi_CC_update_handled(currentKey);
        }
      }

      // Update Fader values
      if(currentKey->isMIDI && currentKey->isAnalog){         //don't forget to compare val with previous val when setting. Yes mum...
        hid.sendMidi_Analog(numpad1.getKeyPointer(i + 1), map(i2c.getVal(i + 1, numpad1.address()), 0, 255, 0, 127));
      }

      else if(numpad1.isPressed_hold(i)){
        if(currentKey->hasLayerChange){
          numpad1.setLayer(currentKey->changeToLayer);
        }
        if(currentKey->isMIDI){
          hid.sendMidi_Digital(numpad1.getKeyPointer(i + 1), true);
        }
        else{
          Serial.println("pressing");
          hid.press(numpad1.getKeyPointer(i + 1));
        }
        switch(currentKey->color_mode){
          case no_override: break;
          case midi_bound: break;
          case pressed: i2c.send_led_override_update(numpad1.address(), numpad1.remap_led_key(i), true); break;
          case not_pressed: i2c.send_led_override_update(numpad1.address(), numpad1.remap_led_key(i), false); break;
          case toggle: i2c.send_led_override_update(numpad1.address(), numpad1.remap_led_key(i), false); break;
          case disabled: i2c.send_led_override_update(numpad1.address(), numpad1.remap_led_key(i), false); break;
        }
      }
      else if(numpad1.isReleased_hold(i)){
        if(currentKey->isMIDI) hid.sendMidi_Digital(numpad1.getKeyPointer(i + 1), false);

        else Serial.println("releasing"); hid.release(numpad1.getKeyPointer(i + 1));

        switch(currentKey->color_mode){
          case no_override: break;
          case midi_bound: break;
          case pressed: i2c.send_led_override_update(numpad1.address(), numpad1.remap_led_key(i), false); break;
          case not_pressed: i2c.send_led_override_update(numpad1.address(), numpad1.remap_led_key(i), true); break;
          case toggle: break;
          case disabled: i2c.send_led_override_update(numpad1.address(), numpad1.remap_led_key(i), false); break;
        }
      }
      
    }
  }
}
#endif

/////////////////////////////////////////////////////////////// I2C scanner ///////////////////////////////////////////////////////////////

inline void scanI2C(){
  if(millis() > prev + 1000){
    prev = millis();
    #ifdef IS_KEYBOARD
    i2c.probe();
    #endif
    #ifdef SERIAL_DEBUG
      Serial.println("uwu");
      if(error == 0) Serial.println("ESP disabled successfully");
      else Serial.print("ESP has given error on disable: ");Serial.println(error);
    #endif
  }
}


/////////////////////////////////////////////////////////////// Filesystem Update Handler ///////////////////////////////////////////////////////////////

inline void check_fs_update(){
  if(check_fs_changed()){
    #ifdef IS_KEYBOARD
    keyboard.updateKeymapsFromFile();
    numpad1.updateKeymapsFromFile();
    for(int i = 0; i < numpad1.ammountKeys; i++){
      key *currentKey = numpad1.getKeyPointer(i);
      if(currentKey->color_mode != no_override){
        i2c.set_led_override(numpad1.address(), numpad1.remap_led_key(i), currentKey->color);
      }
    }
    #endif

    #ifdef IS_MACRUWU
    macruwu.updateKeymapsFromFile();
    Serial.printf("current Layer: %d\n", macruwu.current_layer);
    #endif

    
    
    set_fs_changed(false);
  }
}
