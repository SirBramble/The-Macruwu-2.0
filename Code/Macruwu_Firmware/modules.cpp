#include "modules.h"

int diodeMatrixToKeyIndex[AMMOUNT_ROW][AMMOUNT_COL] = {KEY_REMAPPING};

_keyboard::_keyboard(String moduleName) : module(moduleName){}

void _keyboard::init(){
  for(int i = 0; i < 6; i++){
    pinMode(this->pins_row[i], INPUT);
  }

  for(int i = 0; i < 17; i++){
    pinMode(this->pins_col[i], INPUT_PULLDOWN);
  }

  for(int i = 0; i < AMMOUNT_KEYS; i++){
    pressed[i] = 0;
    states[i] = IDLE;
  }
}

void _keyboard::update(){
  for(int row = 0; row < AMMOUNT_ROW; row++){
    pinMode(this->pins_row[row], OUTPUT);
    digitalWrite(this->pins_row[row], HIGH);

    for(int col = AMMOUNT_COL - 1; col >= 0; col--){
      if(digitalRead(this->pins_col[col])){
        #ifdef SERIAL_DEBUG
        Serial.print("matrix: "); Serial.print(diodeMatrixToKeyIndex[row][col]);
        #endif
        if(diodeMatrixToKeyIndex[row][col] != 0){
          pressed[diodeMatrixToKeyIndex[row][col] - 1] = true;
        }
        #ifdef SERIAL_DEBUG
        Serial.print(" row: ");Serial.print(row + 1);Serial.print(" col: "); Serial.print(col + 1);Serial.print(" pressed: ");Serial.println(pressed[diodeMatrixToKeyIndex[row][col] - 1]);
        #endif
      }
      else{
        pressed[diodeMatrixToKeyIndex[row][col] - 1] = false;
      }
    }
    digitalWrite(this->pins_row[row], LOW);
    pinMode(this->pins_row[row], INPUT);
    delay(1);
  }
  this->updateSM();
}

/*
bool _keyboard::isPressed(int position){
  return pressed[position];
}
*/

void _keyboard::testKeys(){
  this->update();
  for(int i = 0; i < AMMOUNT_KEYS; i++){
    if(pressed[i] > 0){
      Serial.print("Key Pressed: ");
      Serial.println(i + 1);
    }
  }
}

void _keyboard::updateSM(){
  for(int i = 0; i < AMMOUNT_KEYS; i++){
    switch(states[i]){
      case IDLE:
        if(pressed[i]){
          states[i] = TRANSITION;
        }
        break;
      case TRANSITION:
        if(pressed[i]){
          states[i] = PRESSED;
        }
        else {
          states[i] = IDLE;
        }
        break;
      case PRESSED:
        if(pressed[i] == 0){
          states[i] = RELEASED;
        }
        break;
      case RELEASED:
        if(pressed[i] == 0){
          states[i] = IDLE;
        }
    }
  }
}

bool _keyboard::isPressed(int position){
  if(states[position] == TRANSITION){
    return true;
  }
  else{
    return false;
  }
}

bool _keyboard::isReleased(int position){
  if(states[position] == RELEASED){
    return true;
  }
  else{
    return false;
  }
}

uint8_t *_keyboard::get_led_remap(){
  return this->led_remap;
}

uint8_t _keyboard::get_ammount_keys(){
  return AMMOUNT_KEYS;
}

/*
bool _keyboard::isPressed_single(int position){
  if(states[position] == TRANSITION){
    return true;
  }
  else{
    return false;
  }
}

bool _keyboard::isReleased_single(int position){
  if(states[position] == PRESSED){
    return true;
  }
  else{
    return false;
  }
}
*/

//----------------------------numpad--------------------------
_numpad::_numpad(String moduleName, uint8_t address) : module(moduleName){
  this->_address = address;
}

void _numpad::init(i2cInterface *i2c){
  this->i2c = i2c;
}

uint8_t _numpad::address(){
  return _address;
}

bool _numpad::registered(){
  return i2c->isAddressValid(this->_address);
}

uint16_t *_numpad::get_led_remap(){
  return this->led_remap;
}

uint16_t _numpad::remap_led_key(uint16_t key){
  if(key > this->ammountKeys){
    return 0;
  }
  return this->led_remap[key];
}

void _numpad::update(){
  this->i2c->update(this->_address);
  uint8_t * input = this->i2c->getStates(this->_address);
  if(input == NULL) return;

  for(int i = 0; i < AMMOUNT_KEYS_NUMPAD; i++){
    pressed[i] = input[i];
  }

  for(int i = 0; i < AMMOUNT_KEYS_NUMPAD; i++){
    switch(states[i]){
      case IDLE:
        if(pressed[i]){
          states[i] = TRANSITION;
          //Serial.print("Transition numpad: ");Serial.print(pressed[i]);Serial.print(",");Serial.println(input[i]);
        }
        break;
      case TRANSITION:
        if(pressed[i]){
          states[i] = PRESSED;
        }
        else {
          states[i] = IDLE;
        }
        break;
      case PRESSED:
        if(pressed[i] == 0){
          states[i] = RELEASED;
        }
        break;
      case RELEASED:
        states[i] = IDLE;
        break;
    }
  }
}

bool _numpad::isPressed_hold(int position){
  if(states[position] == TRANSITION){
    return true;
  }
  else{
    return false;
  }
}

bool _numpad::isReleased_hold(int position){
  if(states[position] == RELEASED){
    return true;
  }
  else{
    return false;
  }
}

bool _numpad::isPressed_single(int position){
  if(states[position] == TRANSITION){
    return true;
  }
  else{
    return false;
  }
}

bool _numpad::isReleased_single(int position){
  if(states[position] == PRESSED){
    return true;
  }
  else{
    return false;
  }
}

//----------------------------macruwu--------------------------
_macruwu::_macruwu(String moduleName, uint8_t version): module(moduleName){
  this->version = version;
  if(this->version == 1){
    this->Expander1_PCA = new PCA9555D(ADDone);
    this->Expander2_PCA = new PCA9555D(ADDtwo);
  }
  else if(this->version == 2){
    this->Expander1_PI = new PI4IOE5V6416(ADDone, INT_1);
    this->Expander2_PI = new PI4IOE5V6416(ADDtwo, INT_0);
  }

  
}

_macruwu::~_macruwu(){
  delete this->Expander1_PI;
  delete this->Expander2_PI;
  delete this->Expander1_PCA;
  delete this->Expander2_PCA;
}

void _macruwu::init(){
  if(this->version == 1){
    this->Expander1_PCA->init();
    this->Expander2_PCA->init();
    this->Expander1_PCA->setRemapTable(this->Expander1RemapTable_PCA);
    this->Expander2_PCA->setRemapTable(this->Expander2RemapTable_PCA);
  }
  else if(this->version == 2){
    this->Expander1_PI->init();
    this->Expander1_PI->setRemapTable(this->Expander1RemapTable_PI);
    this->Expander2_PI->init();
    this->Expander2_PI->setRemapTable(this->Expander2RemapTable_PI);
  }
}

void _macruwu::update(){
  if(this->version == 1){
    return;
  }
  else if(this->version == 2){
    this->Expander1_PI->debounce();
    this->Expander2_PI->debounce();
  }
}
bool _macruwu::isPressed(int position){
  if(position >= 32){
    return false;
  }

  if(this->version == 1){
    if(position < 16){
      return this->Expander1_PCA->getStatePulse_on(position);
    }
    if(position >= 16){
      return this->Expander2_PCA->getStatePulse_on(position-16);
    }
  }

  else if(this->version == 2){
    if(position < 16){
      return this->Expander1_PI->getStatePulse_on(position);
    }
    if(position >= 16){
      return this->Expander2_PI->getStatePulse_on(position-16);
    }
  }
  return false;
}
bool _macruwu::isReleased(int position){
  if(position >= 32){
    return false;
  }

  if(this->version == 1){
    if(position < 16){
      return this->Expander1_PCA->getStatePulse_off(position);
    }
    if(position >= 16){
      return this->Expander2_PCA->getStatePulse_off(position-16);
    }
  }

  else if(this->version == 2){
    if(position < 16){
      return this->Expander1_PI->getStatePulse_off(position);
    }
    if(position >= 16){
      return this->Expander2_PI->getStatePulse_off(position-16);
    }
  }
  return false;
}

uint8_t *_macruwu::get_led_remap(){
  return this->led_remap;
}

uint8_t _macruwu::get_ammount_keys(){
  return this->ammount_keys;
}
