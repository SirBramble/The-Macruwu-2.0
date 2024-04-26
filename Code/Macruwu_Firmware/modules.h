#include <stdint.h>
#ifndef MODULES
#define MODULES

#include "filesystem.h"
#include "expander.h"
#include "i2cinterface.h"

#define AMMOUNT_KEYS 88
#define AMMOUNT_KEYS_NUMPAD 27      // actually 23, but for testing adding buffer


#define KEY_REMAPPING \
{73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 0 },\
{ 1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 63, 64, 65},\
{15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27,  0, 66, 67, 68},\
{29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 28,  0,  0,  0},\
{42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54,  0,  0, 69,  0},\
{55, 56, 57,  0,  0,  0, 58,  0,  0, 59, 60, 61, 62,  0, 70, 71, 72}\

#define KEYBOARD_KEY_TO_LED \
  71,70,69,68,67,66,65,64,63,62,61,60,59,58,\
  54,53,52,51,50,49,48,47,46,45,44,43,42,25,\
  38,37,36,35,34,33,32,31,30,29,28,27,26,\
  24,23,22,21,20,19,18,17,16,15,14,13,12,\
  10,9,8,7,6,5,4,3,\
  57,56,55,41,40,39,\
  11,2,1,0,\
  87,86,85,84,83,82,81,80,79,78,77,76,75,74,73,72\

// HELP FOR NUMPAD_KEY_TO_LED
// 01 = 16
// 02 = 15
// 03 = 14
// 04 = 13
// 05 = 12
// 06 = 11
// 07 = 10
// 08 = 06
// 09 = 09
// 10 = 08
// 11 = 07
// 12 = 05
// 13 = 04
// 14 = 03
// 15 = 00
// 16 = 02
// 17 = 01
// 18 = 18
// 19 = 19
// 20 = 17

#define NUMPAD_KEY_TO_LED \
  16,17,14,13,12,11,10,06,9,8,7,5,4,3,0,2,1,18,19,17,\
  0,0,0,0,0,0,0      // zeros due to testing

#define AMMOUNT_ROW 6
#define ROW1 6
#define ROW2 7
#define ROW3 8
#define ROW4 9
#define ROW5 14
#define ROW6 13

#define AMMOUNT_COL 17
#define COL1 10
#define COL2 11
#define COL3 16
#define COL4 19
#define COL5 20
#define COL6 21
#define COL7 22
#define COL8 23
#define COL9 24
#define COL10 25
#define COL11 26
#define COL12 27
#define COL13 28
#define COL14 29
#define COL15 15
#define COL16 17
#define COL17 12

typedef enum{
  IDLE = 0,
  TRANSITION,
  PRESSED,
  RELEASED
} _state;

#define MACRUWU_KEY_TO_LED \
  0,1,2,3,4,5,6,7,\
  8,9,10,11,12,13,14,15,\
  16,17,18,19,20,21,22,23,\
  24,25,26,27,28,29,30,31

class _keyboard : public module{
public:
  _keyboard(String moduleName);
  void init();
  void testKeys();
  //bool isPressed(int position);
  void update();
  bool isPressed(int position);
  bool isReleased(int position);      //true if released
  uint8_t *get_led_remap();
  uint8_t get_ammount_keys();
  //bool isPressed_single(int position);
  //bool isReleased_single(int position);    //true if released
private:
  int pins_row[AMMOUNT_ROW] = {ROW1, ROW2, ROW3, ROW4, ROW5, ROW6};
  int pins_col[AMMOUNT_COL] = {COL1, COL2, COL3, COL4, COL5, COL6, COL7, COL8, COL9, COL10, COL11, COL12, COL13, COL14, COL15, COL16, COL17};
  bool pressed[AMMOUNT_KEYS];     //remember -1 for start count at 0
  uint8_t led_remap[AMMOUNT_KEYS] = {KEYBOARD_KEY_TO_LED};
  //SM
  _state states[AMMOUNT_KEYS];
  void updateSM();
};

class _numpad : public module{
public:
  _numpad(String moduleName, uint8_t address);
  void init(i2cInterface *i2c);
  uint8_t address();
  bool registered();
  void update();
  bool isPressed_hold(int position);
  bool isReleased_hold(int position);      //true if released
  bool isPressed_single(int position);
  bool isReleased_single(int position);    //true if released
  uint16_t *get_led_remap();
  uint16_t remap_led_key(uint16_t key);
  uint8_t ammountKeys = AMMOUNT_KEYS_NUMPAD;
private:
  uint8_t _address;
  uint8_t pressed[AMMOUNT_KEYS_NUMPAD];     //remember -1 for start count at 0
  uint16_t led_remap[AMMOUNT_KEYS] = {NUMPAD_KEY_TO_LED};
  //SM
  _state states[AMMOUNT_KEYS_NUMPAD];
  i2cInterface *i2c = NULL;
};

class _macruwu : public module{
public:
  _macruwu(String moduleName, uint8_t version = 2);
  ~_macruwu();
  void init();
  void update();
  bool isPressed(int position);
  bool isReleased(int position);      //true if released
  uint8_t *get_led_remap();
  uint8_t get_ammount_keys();
private:
  uint8_t ADDone = 0b0100000;
  uint8_t ADDtwo = 0b0100001;
  uint8_t INT_0 = 2u;   // D8
  uint8_t INT_1 = 4u;   // D9
  PI4IOE5V6416 *Expander1_PI = nullptr;
  PI4IOE5V6416 *Expander2_PI = nullptr;
  uint8_t Expander1RemapTable_PI[16] = {15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0};
  uint8_t Expander2RemapTable_PI[16] = {15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0};
  PCA9555D *Expander1_PCA = nullptr;
  PCA9555D *Expander2_PCA = nullptr;
  uint8_t Expander1RemapTable_PCA[16] = {7,6,5,4,3,2,1,0,15,14,13,12,11,10,9,8};
  uint8_t Expander2RemapTable_PCA[16] = {7,6,5,4,3,2,1,0,15,14,13,12,11,10,9,8};
  uint8_t led_remap[AMMOUNT_KEYS] = {MACRUWU_KEY_TO_LED};
  uint8_t ammount_keys = 32;
  uint8_t version;
};

#endif // MODULES