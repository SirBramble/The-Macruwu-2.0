#ifndef EXPANDER_H_
#define EXPANDER_H_

#include <Arduino.h>
#include <Wire.h>

#define PINCOUNT 16
#define REGISTERCOUNT 2
#define DB_TIME 5              //in ms


class Expander{
  protected:
    uint8_t address;
    uint8_t state[PINCOUNT];
    uint8_t sm_state[PINCOUNT] = {0};
    uint8_t state_db[PINCOUNT] = {0};
    uint8_t state_db_pulse_on[PINCOUNT] = {0};
    uint8_t state_db_pulse_off[PINCOUNT] = {0};
    uint8_t input_reg[REGISTERCOUNT];
    uint8_t remapTable[PINCOUNT] = {0};
    uint64_t db_time_prev = millis();
    void splice();
  public:
    void setRemapTable(uint8_t *newRemapTable);
};

class PCA9555D: public Expander{
  public:
    PCA9555D(uint8_t address);
    void init();
    bool getState(uint8_t pin);
    bool getStatePulse_on(uint8_t pin);
    bool getStatePulse_off(uint8_t pin);
    void update();
  private:
    bool debounce();
};

class PI4IOE5V6416: public Expander{
  public:
    PI4IOE5V6416(uint8_t address, uint8_t interrupt_pin);
    void init();
    bool getState(uint8_t pin);
    bool getStatePulse_on(uint8_t pin);
    bool getStatePulse_off(uint8_t pin);
    void debounce();
  private:
    uint8_t interrupt_pin;
    void update();

};

#endif /* EXPANDER_H_ */