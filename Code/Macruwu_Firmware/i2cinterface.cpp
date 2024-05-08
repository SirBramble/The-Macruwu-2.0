#include "i2cinterface.h"

bool sda_set = 0;
bool scl_set = 0;

i2cInterface::i2cInterface(uint8_t *_registered_addresses, uint8_t _size){
  if(_registered_addresses != NULL){
    this->registered_addresses = _registered_addresses;
    this->size_registered_addresses = _size;
  }
  else{
    this->registered_addresses = new uint8_t[1];
    registered_addresses[0] = 0x19;   // "Random" address to prevent 0x00 (global) call
    this->size_registered_addresses = 1;
  }
}

void i2cInterface::init(){
  #ifdef IS_KEYBOARD
  sda_set = Wire.setSDA(PIN_SDA);
  scl_set = Wire.setSCL(PIN_SCL);
  #endif
  Wire.begin();
  for(int i = 0; i < I2C_BUFFER_SIZE; i++){
    button_states[i] = 0;
  }
}

void i2cInterface::probe(){
  uint8_t error;
  address_valid.clear();
  for(uint8_t i = 0; i < size_registered_addresses; i++){
    Wire.beginTransmission(registered_addresses[i]);
    error = Wire.endTransmission();
    if(error == 0){
      address_valid.push_back(registered_addresses[i]);
      #ifdef SERIAL_DEBUG
      Serial.print("Found Address: ");Serial.println(registered_addresses[i]);
      #endif
    }
    if(registered_addresses[i] == 0x20){
      #ifdef SERIAL_DEBUG
      Serial.print("error: ");Serial.println(error);
      Serial.print("SDA set: ");Serial.println(sda_set);
      Serial.print("SCL set: ");Serial.println(scl_set);
      #endif
    }
  }

  if(address_valid.size() == 0){
    current_address = 0;
  }
}

void i2cInterface::update(uint8_t address){
  uint8_t error;
  bool addressIsValid = 0;
  for(int i = 0; i < I2C_BUFFER_SIZE; i++){
      button_states[i] = 0;
  }
  for(uint8_t i = 0; i < address_valid.size(); i++){
    if(address == address_valid.at(i)){
      addressIsValid = 1;
    }
  }
  if(!addressIsValid){
    //Serial.println("address not found valid!");
    return;
  }
  if(current_command != GET_STATES || current_address != address){
    current_address = address;
    current_command = GET_STATES;
    Wire.beginTransmission(address);
    Wire.write(GET_STATES);
    error = Wire.endTransmission();
    delay(1);
  }
  if(error == 0){
    Wire.requestFrom(address, I2C_BUFFER_SIZE, true);
    uint8_t i = 0;
    
    while(Wire.available() && i < I2C_BUFFER_SIZE){
      button_states[i] = Wire.read();
      for(int i = 0; i < I2C_BUFFER_SIZE; i++){
        if(button_states[i] > 0){
          //Serial.print(pressed[i]);
          //Serial.print("button pressed I2C: ");Serial.println(i);
        }
      }
      i++;
    }
    //while(i < I2C_BUFFER_SIZE){
    //  button_states[i] = 0;
    //  i++;
    //}
  }
}

uint8_t i2cInterface::getVal(uint8_t button, uint8_t address){
  if(button < I2C_BUFFER_SIZE){
    if(current_address != address) this->update(address);
    return button_states[button];
  }
  else return 0;
}

uint8_t i2cInterface::getVal_db(uint8_t button, uint8_t address){
  if(button < I2C_BUFFER_SIZE){
    if(current_address != address) this->update(address);
    return button_states[button];
  }
  else return 0;
}

uint8_t * i2cInterface::getStates(uint8_t address){
  if(current_address != address) this->update(address);
  return button_states;
}

void i2cInterface::set_led_override(uint8_t address, uint16_t key, uint32_t color){
  bool addressIsValid = 0;
  for(uint8_t i = 0; i < address_valid.size(); i++){
    if(address == address_valid.at(i)){
      addressIsValid = 1;
    }
  }
  if(!addressIsValid){
    //Serial.println("address not found valid!");
    return;
  }

  Wire.beginTransmission(address);
  Wire.write(SET_LED_OVERRIDE);
  Wire.write((uint8_t)(key>>8));
  Wire.write((uint8_t)(key>>0));
  Wire.write((uint8_t)(color>>24));
  Wire.write((uint8_t)(color>>16));
  Wire.write((uint8_t)(color>>8));
  Wire.write((uint8_t)(color>>0));
  Wire.endTransmission();
}
void i2cInterface::send_led_override_update(uint8_t address, uint16_t key, bool state){
  bool addressIsValid = 0;
  for(uint8_t i = 0; i < address_valid.size(); i++){
    if(address == address_valid.at(i)){
      addressIsValid = 1;
    }
  }
  if(!addressIsValid){
    //Serial.println("address not found valid!");
    return;
  }

  Wire.beginTransmission(address);
  Wire.write(SET_LED_OVERRIDE);
  Wire.write((uint8_t)(key>>8));
  Wire.write((uint8_t)(key>>0));
  Wire.write((uint8_t)(state));
  Wire.endTransmission();
}

bool i2cInterface::isAddressValid(uint8_t address){
  bool addressIsValid = 0;
  for(uint8_t i = 0; i < address_valid.size(); i++){
    if(address == address_valid.at(i)){
      addressIsValid = 1;
    }
  }
  return addressIsValid;
}

uint8_t i2cInterface::disableESP(uint8_t address){
  Wire.beginTransmission(address);
  Wire.write(15);
  Wire.write(1);
  uint8_t error = Wire.endTransmission();
  return error;
}
