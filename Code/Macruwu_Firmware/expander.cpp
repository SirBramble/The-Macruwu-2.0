#include "Arduino.h"

#include "Expander.h"

///////////////////////////////////////////////////Expander//////////////////////////////////////////////


void Expander::splice(){
  for(int n = 0; n < 2; n++){
    for(int i = 0; i < 8; i++){
      state[i + (n*8)] = !(input_reg[n] & (1 << i));       //REMEMBER: Switches are pulled to GND/logic 0!!!!!!!! Edit: I remembered :)
      }
    }
    
  }

void Expander::setRemapTable(uint8_t *newRemapTable){
  for(int i = 0; i < PINCOUNT;i++){
    remapTable[i] = newRemapTable[i];
  }
}

///////////////////////////////////////////////////PCA9555D//////////////////////////////////////////////

PCA9555D::PCA9555D(uint8_t address){
  Expander::address = address;
  }
void PCA9555D::init(){
  //Nothing here. If something needs to be set, put here.
}
bool PCA9555D::getState(uint8_t pin){
  if(pin >= PINCOUNT){
    return 0;
  }
  pin = remapTable[pin];
  debounce();
  return state_db[pin];
  /*
  if(debounce()){
    if(state_db[pin] == 1) return 1;
    else return 0;
  }
  else if(state_db[pin] == 1) return 1;
  else return 0;
  */
}

bool PCA9555D::getStatePulse_on(uint8_t pin){
  if(pin >= PINCOUNT){
    return 0;
  }
  pin = remapTable[pin];
  if(debounce()){
    if(state_db_pulse_on[pin] == 1){
      state_db_pulse_on[pin] = 0;
      return 1;
    }
    else{
      return 0;  
    }
  }
  else if(state_db_pulse_on[pin] == 1){
    state_db_pulse_on[pin] = 0;
    return 1;
  }
  else{
    return 0;
  }
}

bool PCA9555D::getStatePulse_off(uint8_t pin){
  if(pin >= PINCOUNT){
    return 0;
  }
  pin = remapTable[pin];
  if(debounce()){
    if(state_db_pulse_off[pin] == 1){
      state_db_pulse_off[pin] = 0;
      return 1;
    }
    else return 0;  
  }
  else if(state_db_pulse_off[pin] == 1){
    state_db_pulse_off[pin] = 0;
    return 1;
  }
  else return 0;
}


bool PCA9555D::debounce(){
  if((db_time_prev + DB_TIME) < millis()){
    db_time_prev = millis();
    update();
    for(uint8_t pin = 0; pin < PINCOUNT; pin++){
      switch(sm_state[pin]){
        case 0:
          state_db_pulse_off[pin] = 0;
          if(state[pin] == 1){
            state_db[pin] = 0;
            state_db_pulse_on[pin] = 0;
            sm_state[pin] = 1;
          }
          break;
        case 1:
          if(state[pin] == 1){
            state_db[pin] = 0;
            state_db_pulse_on[pin] = 0;
            sm_state[pin] = 2;
          }
          else{
            sm_state[pin] = 0;
          }
          break;
        case 2:
          if(state[pin] == 1){
            state_db[pin] = 1;
            state_db_pulse_on[pin] = 1;
            sm_state[pin] = 3;
          }
          else{
            sm_state[pin] = 0;
          }
          break;
        case 3:
          if(state[pin] == 1){
            state_db[pin] = 1;
            state_db_pulse_on[pin] = 0;
          }
          else{
            state_db_pulse_off[pin] = 1;
            state_db[pin] = 0;
            state_db_pulse_on[pin] = 0;
            sm_state[pin] = 0;
          }
          break;
        default:
          sm_state[pin] = 0;
          state_db[pin] = 0;
          break;
      }
    }
    return 1;
  }
  else{
    return 0;
    }
  }


void PCA9555D::update(){
  int i = 0;
  Wire.beginTransmission(address);
  Wire.write(0);                            //Send Register Byte
  Wire.endTransmission();
  Wire.requestFrom(address, REGISTERCOUNT, true);       //According to Datasheet, I/O Expander will send both Input registers, when one is Requested and the Master does not break the connection
  while(Wire.available()){
    input_reg[i] = Wire.read();
    i++;
    }
  splice();
  i = 0;
  }



  //////////////////////////////////////////////////////////////////////////////////PI4IOE5V6416///////////////////////////////////////////////////////////////

  PI4IOE5V6416::PI4IOE5V6416(uint8_t address, uint8_t interrupt_pin){
    Expander::address = address;
    this->interrupt_pin = interrupt_pin;
  }

void PI4IOE5V6416::init(){
  //Pull up enable
  //possible multi byte write. TO BE TESTED! I WILL KNOW IF YOU DONT!!! DON´T FUCKING FORGET!!!!
  Wire.beginTransmission(address);
  Wire.write(0b01000110);
  Wire.write(0b11111111);
  Wire.endTransmission();
  Wire.beginTransmission(address);
  Wire.write(0b01000111);
  Wire.write(0b11111111);
  Wire.endTransmission();

  //Interrupt enable
  //possible multi byte write. TO BE TESTED! I WILL KNOW IF YOU DONT!!! DON´T FUCKING FORGET!!!!
  Wire.beginTransmission(address);
  Wire.write(0b01001010);
  Wire.write(0b00000000);
  Wire.endTransmission();
  Wire.beginTransmission(address);
  Wire.write(0b01001011);
  Wire.write(0b00000000);
  Wire.endTransmission();
  
  //Latch Register enable
  //possible multi byte write. TO BE TESTED! I WILL KNOW IF YOU DONT!!! DON´T FUCKING FORGET!!!!
  Wire.beginTransmission(address);
  Wire.write(0b01000100);
  Wire.write(0b11111111);
  Wire.endTransmission();
  Wire.beginTransmission(address);
  Wire.write(0b01000101);
  Wire.write(0b11111111);
  Wire.endTransmission();
  
}

void PI4IOE5V6416::update(){
  uint8_t reg = 0b00000000;
  for(int i = 0; i<REGISTERCOUNT; i++){
    Wire.beginTransmission(address);
    Wire.write(reg);                            //Send Register Byte
    Wire.endTransmission();
    reg++;
    Wire.requestFrom(address, 1, true);
    while(Wire.available()){
      input_reg[i] = Wire.read();
      }
  }
  splice();
}


void PI4IOE5V6416::debounce(){
  update();
  for(uint8_t pin = 0; pin < PINCOUNT; pin++){
    switch(sm_state[pin]){
      case 0:
        if(state[pin] == 1){
          state_db[pin] = 1;
          sm_state[pin] = 1;
        }
        break;
      case 1:
        if(state[pin] == 1){
          state_db_pulse_on[pin] = 1;
          sm_state[pin] = 2;
        }
        else{
          sm_state[pin] = 0;
        }
        break;
      case 2:
        if(state[pin] == 0){
          state_db[pin] = 0;
          state_db_pulse_off[pin] = 1;
          sm_state[pin] = 0;
        }
        break;
      default:
        sm_state[pin] = 0;
        break;
    }
  }
  }

bool PI4IOE5V6416::getState(uint8_t pin){
  if(pin >= PINCOUNT){
    return 0;
  }
  //debounce();
  return state_db[remapTable[pin]];


  
}

bool PI4IOE5V6416::getStatePulse_on(uint8_t pin){
  bool ret = 0;
  if(pin >= PINCOUNT){
    return 0;
  }
  //debounce();
  ret = state_db_pulse_on[remapTable[pin]];
  state_db_pulse_on[remapTable[pin]] = 0;
  return ret;
}

bool PI4IOE5V6416::getStatePulse_off(uint8_t pin){
  bool ret = 0;
  if(pin >= PINCOUNT){
    return 0;
  }
  //debounce();
  ret = state_db_pulse_off[remapTable[pin]];
  state_db_pulse_off[remapTable[pin]] = 0;
  return ret;
}
  