#include "filesystem.h"

#include "SPI.h"
#include <SdFat.h>
#include "Adafruit_SPIFlash.h"
#include "Adafruit_TinyUSB.h"

#include "flash_config.h"

Adafruit_SPIFlash flash(&flashTransport);

// file system object from SdFat
FatVolume fatfs;

FatFile root;
FatFile file;

// USB Mass Storage object
Adafruit_USBD_MSC usb_msc;

// Check if flash is formatted
bool fs_formatted;

// Set to true when PC write to flash
bool fs_changed;

// Callback invoked when received READ10 command.
// Copy disk's data to buffer (up to bufsize) and 
// return number of copied bytes (must be multiple of block size) 
int32_t msc_read_cb (uint32_t lba, void* buffer, uint32_t bufsize)
{
  // Note: SPIFLash Block API: readBlocks/writeBlocks/syncBlocks
  // already include 4K sector caching internally. We don't need to cache it, yahhhh!!
  return flash.readBlocks(lba, (uint8_t*) buffer, bufsize/512) ? bufsize : -1;
}

// Callback invoked when received WRITE10 command.
// Process data in buffer to disk's storage and 
// return number of written bytes (must be multiple of block size)
int32_t msc_write_cb (uint32_t lba, uint8_t* buffer, uint32_t bufsize)
{
  //digitalWrite(LED_BUILTIN, HIGH);

  // Note: SPIFLash Block API: readBlocks/writeBlocks/syncBlocks
  // already include 4K sector caching internally. We don't need to cache it, yahhhh!!
  return flash.writeBlocks(lba, buffer, bufsize/512) ? bufsize : -1;
}

// Callback invoked when WRITE10 command is completed (status received and accepted by host).
// used to flush any pending cache.
void msc_flush_cb (void)
{
  // sync with flash
  flash.syncBlocks();

  // clear file system's cache to force refresh
  fatfs.cacheClear();

  fs_changed = true;
}


// the setup function runs once when you press reset or power the board
void filesystemSetup()
{

  flash.begin();

  // Set disk vendor id, product id and revision with string up to 8, 16, 4 characters respectively
  usb_msc.setID("Typeuwu", "BIG DICK FLASH", "6.9");

  // Set callback
  usb_msc.setReadWriteCallback(msc_read_cb, msc_write_cb, msc_flush_cb);

  // Set disk size, block size should be 512 regardless of spi flash page size
  usb_msc.setCapacity(flash.size()/512, 512);

  // MSC is ready for read/write
  usb_msc.setUnitReady(true);

  usb_msc.begin();

  // Init file system on the flash
  fs_formatted = fatfs.begin(&flash);

  //Serial.begin(115200);
  //while ( !Serial ) delay(10);   // wait for native usb

  if ( !fs_formatted )
  {
    #ifdef SERIAL_DEBUG
    Serial.println("Failed to init files system, flash may not be formatted");
    #endif
  }
  #ifdef SERIAL_DEBUG
  Serial.print("JEDEC ID: 0x"); Serial.println(flash.getJEDECID(), HEX);
  Serial.print("Flash size: "); Serial.print(flash.size() / 1024); Serial.println(" KB");
  #endif
  fs_changed = true; // to print contents initially
}

bool check_fs_changed(){
  return fs_changed;
}

void set_fs_changed(bool state){
  fs_changed = state;
}

void filesystemClear(){
  if (!flash.eraseChip()) {
    Serial.println("Failed to erase chip!");
  }

  flash.waitUntilReady();
  Serial.println("Successfully erased chip!");
  
  String filename_to_open = "Layout.txt";
  if(!fatfs.exists(filename_to_open)){
    Serial.println("Layout.txt does not exist");
    Serial.println("Trying to create Layout.txt...");
    File32 writeFile = fatfs.open("Layout.txt", FILE_WRITE);
    if (!writeFile) {
      Serial.println("Error, failed to open Layout.txt for writing!");
      while(1) yield();
    }
    Serial.println("Created Layout.txt succesfully");
    writeFile.println("This is a test line, please ignore...");             //perhaps implement default config file...
    writeFile.close();
  }
}

void filesystemCreateConfig(){
  String filename_to_open = "Layout.txt";
  if(!fatfs.exists(filename_to_open)){
    Serial.println("Layout.txt does not exist");
    Serial.println("Trying to create Layout.txt...");
    File32 writeFile = fatfs.open("Layout.txt", FILE_WRITE);
    if (!writeFile) {
      Serial.println("Error, failed to open Layout.txt for writing!");
      while(1) yield();
    }
    Serial.println("Created Layout.txt succesfully");
    writeFile.println("This is a test line, please ignore...");             //perhaps implement default config file...
    writeFile.close();
  }
  return;
}


//-----------------------key------------------------------
key::key(){
    keycodes.push_back({0,0,0,0});
}

void key::appendKeysycode(uint8_t keycode, uint8_t modifier, uint8_t reportID, uint8_t immediateSend){
    keycodes.push_back({keycode, modifier, reportID, immediateSend});
    return;
}

void key::clear(){
    keycodes.resize(0);
    keycodes.push_back({0,0,0,0});
    return;
}

void key::clearToZero(){
    keycodes.resize(0);
    return;
}

keysycode key::getKeysycode(uint16_t position){
    if(position < keycodes.size()) return keycodes.at(position);
    else return {0,0,0,0};
}

uint16_t key::getKeycodesSize(){
  return (uint16_t)keycodes.size();
}

void key::setAnalog(uint16_t value){
    this->analogValue = value;
    return;
}

uint16_t key::getAnalog(){
    return this->analogValue;
}

//------------------------keySet--------------------------

keySet::keySet(){
    keys.push_back(key());
}

void keySet::setSize(uint16_t ammountKeys){
    keys.resize(ammountKeys, key());
    return;
}

key * keySet::getKeyPointer(uint16_t position){
    if(position < keys.size()) return &keys.at(position);
    else return nullptr;
}

void keySet::clear(){
    keys.resize(0);
    keys.push_back(key());
    return;
}

//-----------------------module-------------------------

module::module(String moduleName){
    this->moduleName = moduleName;
    layers.push_back(keySet());
}

void module::setSize(uint16_t ammountLayers, uint16_t ammountKeys){
    layers.resize(ammountLayers, keySet());
    for(uint16_t i = 0; i < ammountLayers; i++){
        layers.at(i).setSize(ammountKeys);
    }
    return;
}

void module::clearAll(){
    layers.resize(0);
    layers.push_back(keySet());
    return;
}

key * module::getKeyPointer(uint16_t position){
    if(this->current_layer < layers.size()) return layers.at(this->current_layer).getKeyPointer(position);
    else return nullptr;
}

key * module::getKeyPointerAtLayer(uint16_t position, uint16_t layer){
  if(layer < layers.size()) return layers.at(layer).getKeyPointer(position);
  else return nullptr;
}

void module::setLayer(uint16_t layer){
  this->current_layer = layer;
  return;
}

rgb_wrapper_t module::getLayerLightingEffect(){
  return this->layer_colors.at(this->current_layer);
}

void module::updateKeymapsFromFile(){
  this->clearAll();
  Serial.println("update Keymaps from file");
  File32 Layout = fatfs.open(CONFIG_FILENAME, FILE_READ);
  if(!Layout){
    Layout.close();
    filesystemCreateConfig();
    Layout = fatfs.open(CONFIG_FILENAME, FILE_READ);
    if(!Layout) while(1){
      Serial.println("failed to open Config!");
      delay(500);
    }
  }

  String fileAll;
  while (Layout.available()) {
    fileAll.concat((char)Layout.read());
  }

  Layout.close();

  MatchState fileAllSearch;
  int fileAllLength = fileAll.length() + 1;      // Safety Margin because of '\0' and all that
  char *buf = new char[fileAllLength];           // may be a waste of space, but we got 16MB... I mean come on...
  for(uint16_t i = 0; i < fileAll.length()-1; i++) buf[i] = '\0';   //clear buffer
  
  char *fileAllBuffer = new char[fileAllLength];
  fileAll.toCharArray(fileAllBuffer, fileAllLength);

  fileAllSearch.Target(fileAllBuffer);

  // search it
  String targetModule = "begin%(" + this->moduleName + "%)(.*)end%(" + this->moduleName + "%)";     //perhaps change (.*) to .* or something
  char result = fileAllSearch.Match (targetModule.c_str(), 0);


  String moduleString;
  // check results

  switch (result)
  {
    case REGEXP_MATCHED:

      Serial.println ("-----");
      Serial.print ("Captured: ");
      
      moduleString += fileAllSearch.GetCapture(buf, 0);
      Serial.println (moduleString.c_str());

      // matching offsets in ms.capture

      Serial.print ("Captures: ");
      Serial.println (fileAllSearch.level);
      break;

    case REGEXP_NOMATCH:
      Serial.println ("No match.");
      break;

    default:
      Serial.print ("Regexp error: ");
      Serial.println (result, DEC);
      break;

  }  // end of switch

  delete[] fileAllBuffer;
  delete[] buf;

  // return;          //worked till here!
  
  String moduleStringSecond = moduleString.substring(0);

  uint16_t layerMax = 1;
  uint16_t positionMax = 1;

  uint16_t layerCurrent;
  uint16_t positionCurrent;
  
  // First run, to get max vals
  while(moduleString.length() > 1){       //perhaps make > 1
    Serial.println("running first while");
    String line = moduleString.substring(0, moduleString.indexOf("\n"));
    Serial.print("line: ");Serial.println(line.c_str());
    moduleString.remove(0, moduleString.indexOf("\n") + 1);     //perhaps add +1 to end index...

    int lineLength = line.length() + 1;     // because of '\0' and all that
    char *lineBuffer = new char[lineLength];
    buf = new char[lineLength];
    line.toCharArray(lineBuffer, lineLength);

    MatchState lineSearch;
    lineSearch.Target(lineBuffer);
    if(lineSearch.Match("^([ \t]*)%%", 0) == REGEXP_MATCHED){}
    else if(lineSearch.Match("Layer (%d+)", 0) == REGEXP_MATCHED){
      layerCurrent = atoi(lineSearch.GetCapture(buf, 0));
      Serial.print("Layer current: ");Serial.println(layerCurrent);
      if(layerCurrent > layerMax) layerMax = layerCurrent;
    }
    else if(lineSearch.Match("Button (%d+)", 0) == REGEXP_MATCHED){
      positionCurrent = atoi(lineSearch.GetCapture(buf, 0));
      Serial.print("Position Current: ");Serial.println(positionCurrent);
      if(positionCurrent > positionMax) positionMax = positionCurrent;
    }
    delete[] buf;
    if(moduleString.indexOf("\n") == -1) break;
  }
  Serial.print("layerMax: ");Serial.println(layerMax);
  Serial.print("positionMax: "); Serial.println(positionMax);
  //return;     //worked till here
  this->setSize(layerMax + (uint16_t)1, positionMax + (uint16_t)1);     // + 1 because start counting at 1 not 0

  layer_colors.resize(layerMax + 1);

  //return;     //worked till here
  //Second Run to get Strings
  while(moduleStringSecond.length() > 0){       //perhaps make > 1
  Serial.println("running second while");
    String stringToInterpret;
    String line = moduleStringSecond.substring(0, moduleStringSecond.indexOf("\n"));
    Serial.print("moduleStringSecond: ");Serial.println(moduleStringSecond.c_str());
    Serial.print("line: ");Serial.println(line.c_str());
    //return;
    moduleStringSecond.remove(0, moduleStringSecond.indexOf("\n") + 1);     //perhaps add +1 to end index...

    int lineLength = line.length() + 1;     // because of '\0' and all that
    char *lineBuffer = new char[lineLength];
    buf = new char[lineLength];
    line.toCharArray(lineBuffer, lineLength);

    std::string buf_effect;
    uint32_t buf_color = 0;
    uint32_t buf_speed = 0;

    MatchState lineSearch;
    lineSearch.Target(lineBuffer);
    
    if(lineSearch.Match("^([ \t]*)%%", 0) == REGEXP_MATCHED){}
    else if(lineSearch.Match("Layer (%d+) (COLOR%{)([%a]+)%}", 0) == REGEXP_MATCHED){
      layerCurrent = atoi(lineSearch.GetCapture(buf, 0));
      Serial.printf("found COLOR MODE on layer: %d\n", layerCurrent);
      buf_effect += lineSearch.GetCapture(buf,2);
      layer_colors.at(layerCurrent) = string_to_color_effect(buf_effect);
    }
    else if(lineSearch.Match("Layer (%d+) (COLOR%{)([%a]+),([%a]+)%}", 0) == REGEXP_MATCHED){
      layerCurrent = atoi(lineSearch.GetCapture(buf, 0));
      Serial.printf("found COLOR MODE + COLOR on layer: %d\n", layerCurrent);
      buf_effect += lineSearch.GetCapture(buf,2);
      buf_color = string_to_color(lineSearch.GetCapture(buf,3));
      layer_colors.at(layerCurrent) = string_to_color_effect(buf_effect , buf_color);
    }
    else if(lineSearch.Match("Layer (%d+) (COLOR%{)([%a]+),([%a]+),([%d]+)%}", 0) == REGEXP_MATCHED){
      layerCurrent = atoi(lineSearch.GetCapture(buf, 0));
      Serial.printf("found COLOR MODE + COLOR on layer: %d\n", layerCurrent);
      buf_effect += lineSearch.GetCapture(buf,2);
      buf_color = string_to_color(lineSearch.GetCapture(buf,3));
      buf_speed = atoi(lineSearch.GetCapture(buf, 4));
      layer_colors.at(layerCurrent) = string_to_color_effect(buf_effect , buf_color , buf_speed);
    }
    else if(lineSearch.Match("Layer (%d+)", 0) == REGEXP_MATCHED){
      layerCurrent = atoi(lineSearch.GetCapture(buf, 0));
      layer_colors.at(layerCurrent) = {effect_rainbow,0,5}; // set to default
    }
    else if(lineSearch.Match("Button (%d+): (.*)", 0) == REGEXP_MATCHED){
      positionCurrent = atoi(lineSearch.GetCapture(buf, 0));
      Serial.print("positionCurrent: ");Serial.println(positionCurrent);
      stringToInterpret += lineSearch.GetCapture(buf, 1);
      Serial.print("first text as ASCII: ");Serial.println((uint8_t)stringToInterpret.charAt(0));
      //this->setLayer(layerCurrent);
      interpret(this->getKeyPointerAtLayer(positionCurrent, layerCurrent), stringToInterpret);       //CRASHED somewere in here!!!!
      stringToInterpret.remove(0);
    }
    delete[] buf;
    if(moduleStringSecond.indexOf("\n") == -1) break;
  }
  return;
}

//------------------------------interpreter-------------------------------------
void interpreter::interpret(key * inputKey, String inputString){
  if(inputKey == nullptr){
    Serial.println("Nullus Mullus!!");
    return;
  }
  inputKey->clearToZero();
  int itterations = 0;

  char buff[50];
  uint16_t tmp_counter = 0;

  while(inputString.length() > 0 && itterations < MAX_WHILE_ITTERATIONS){
    int inputLength = inputString.length() + 1;
    char * inputBuffer = new char[inputLength];
    inputString.toCharArray(inputBuffer, inputLength);
    char * buf = new char[inputLength];
    String keycodeBufferString;
    Serial.printf("input Buffer: ");
    for(int i = 0; i < inputLength; i++){
      Serial.printf("%c", inputBuffer[i]);
    }
    Serial.printf("\n");
    
    MatchState inputSearch;
    inputSearch.Target(inputBuffer);
    if(inputSearch.Match("^([%a%d]+)") == REGEXP_MATCHED){
      Serial.printf("found letters: %s\n", keycodeBufferString.c_str());
      keycodeBufferString += inputSearch.GetCapture(buf, 0);
      stringToKeycodes(inputKey, keycodeBufferString);
      inputString.remove(0, keycodeBufferString.indexOf('\0'));
      //keycodeBufferString.remove(0);
      Serial.printf("buffer String Index of \\0: %d\n", keycodeBufferString.indexOf('\0'));
    }
    else if(inputSearch.Match("^([=!_%[%]%+%-%.%^%$#,<{}| ]+)") == REGEXP_MATCHED){
      Serial.println("found other shit");
      keycodeBufferString += inputSearch.GetCapture(buf, 0);
      stringToKeycodes(inputKey, keycodeBufferString);
      inputString.remove(0, keycodeBufferString.length());
    }
    else if(inputSearch.Match("^(\\ae)") == REGEXP_MATCHED){
      Serial.println("found ä");
      inputKey->appendKeysycode(0x34, 0, RID_KEYBOARD, 1);
      inputString.remove(0, 3);
      Serial.print("inputString after ä: ");Serial.println(inputString.c_str());
      Serial.print("size after ä: "); Serial.println(inputString.length());
    }
    else if(inputSearch.Match("^(\\AE)") == REGEXP_MATCHED){
      Serial.println("found Ä");
      inputKey->appendKeysycode(0x34, KEY_MOD_LSHIFT, RID_KEYBOARD, 1);
      inputString.remove(0, 3);
      Serial.print("inputString after Ä: ");Serial.println(inputString.c_str());
      Serial.print("size after Ä: "); Serial.println(inputString.length());
    }
    else if(inputSearch.Match("^(\\oe)") == REGEXP_MATCHED){
      Serial.println("found ö");
      inputKey->appendKeysycode(0x33, 0, RID_KEYBOARD, 1);
      inputString.remove(0, 3);
    }
    else if(inputSearch.Match("^(\\OE)") == REGEXP_MATCHED){
      Serial.println("found Ö");
      inputKey->appendKeysycode(0x33, KEY_MOD_LSHIFT, RID_KEYBOARD, 1);
      inputString.remove(0, 3);
    }
    else if(inputSearch.Match("^(\\ue)") == REGEXP_MATCHED){
      Serial.println("found ü");
      inputKey->appendKeysycode(0x2F, 0, RID_KEYBOARD, 1);
      inputString.remove(0, 3);
    }
    else if(inputSearch.Match("^(\\UE)") == REGEXP_MATCHED){
      Serial.println("found Ü");
      inputKey->appendKeysycode(0x2F, KEY_MOD_LSHIFT, RID_KEYBOARD, 1);
      inputString.remove(0, 3);
    }
    else if(inputSearch.Match("^(\\SZ_KEY)") == REGEXP_MATCHED){
      Serial.println("found SZ");
      inputKey->appendKeysycode(0x2D, 0, RID_KEYBOARD, 1);
      inputString.remove(0, 7);
    }
    else if(inputSearch.Match("^(\\GRAVE_KEY)") == REGEXP_MATCHED){
      Serial.println("found GRAVE");
      inputKey->appendKeysycode(0x2E, 0, RID_KEYBOARD, 1);
      inputString.remove(0, 10);
    }
    else if(inputSearch.Match("^(\\LSHIFT_KEY)") == REGEXP_MATCHED){
      Serial.println("found LSHIFT");
      inputKey->isModifier = true;
      inputKey->appendKeysycode(0, KEY_MOD_LSHIFT, RID_KEYBOARD, 1);
      inputString.remove(0, 11);
    }
    else if(inputSearch.Match("^(\\LCTRL_KEY)") == REGEXP_MATCHED){
      Serial.println("found LCRTL");
      inputKey->isModifier = true;
      inputKey->appendKeysycode(0, KEY_MOD_LCTRL, RID_KEYBOARD, 1);
      inputString.remove(0, 10);
    }
    else if(inputSearch.Match("^(\\STRG%{)([%a])%}") == REGEXP_MATCHED){
      Serial.println("found STRG");
      inputSearch.GetCapture(buff,1);
      Serial.printf("buff[0]: %c\n", buff[0]);
      //inputKey->appendKeysycode(0, KEY_MOD_LCTRL, RID_KEYBOARD, 1);
      inputKey->appendKeysycode(ASCII_conv_table_german[buff[0]][1], KEY_MOD_LCTRL, RID_KEYBOARD, 1);
      //inputKey->appendKeysycode(0, 0, RID_KEYBOARD, 1);
      inputString.remove(0, inputString.indexOf("}")+1);
    }
    else if(inputSearch.Match("^(\\LALT_KEY)") == REGEXP_MATCHED){
      Serial.println("found LALT");
      inputKey->isModifier = true;
      inputKey->appendKeysycode(0, KEY_MOD_LALT, RID_KEYBOARD, 1);
      inputString.remove(0, 9);
    }
    else if(inputSearch.Match("^(\\WIN_KEY)") == REGEXP_MATCHED){
      Serial.println("found WIN");
      inputKey->isModifier = true;
      inputKey->appendKeysycode(0, KEY_MOD_LMETA, RID_KEYBOARD, 1);
      inputString.remove(0, 8);
    }
    else if(inputSearch.Match("^(\\APP_KEY)") == REGEXP_MATCHED){
      Serial.println("found APP");
      inputKey->appendKeysycode(0x65, 0, RID_KEYBOARD, 1);
      inputString.remove(0, 8);
    }
    else if(inputSearch.Match("^(\\RSHIFT_KEY)") == REGEXP_MATCHED){
      Serial.println("found RSHIFT");
      inputKey->isModifier = true;
      inputKey->appendKeysycode(0, KEY_MOD_RSHIFT, RID_KEYBOARD, 1);
      inputString.remove(0, 11);
    }
    else if(inputSearch.Match("^(\\RCTRL_KEY)") == REGEXP_MATCHED){
      Serial.println("found RCRTL");
      inputKey->isModifier = true;
      inputKey->appendKeysycode(0, KEY_MOD_RCTRL, RID_KEYBOARD, 1);
      inputString.remove(0, 10);
    }
    else if(inputSearch.Match("^(\\RALT_KEY)") == REGEXP_MATCHED){
      Serial.println("found RALT");
      inputKey->isModifier = true;
      inputKey->appendKeysycode(0, KEY_MOD_RALT, RID_KEYBOARD, 1);
      inputString.remove(0, 9);
    }
    else if(inputSearch.Match("^(\\RMETA_KEY)") == REGEXP_MATCHED){
      Serial.println("found RMETA");
      inputKey->isModifier = true;
      inputKey->appendKeysycode(0, KEY_MOD_RMETA, RID_KEYBOARD, 1);
      inputString.remove(0, 10);
    }
    else if(inputSearch.Match("^(\\CAPS_LOCK_KEY)") == REGEXP_MATCHED){
      Serial.println("found CAPS LOCK");
      inputKey->appendKeysycode(0x39, 0, RID_KEYBOARD, 1);
      inputString.remove(0, 14);
    }
    else if(inputSearch.Match("^(\\SPACE_KEY)") == REGEXP_MATCHED){
      Serial.println("found SPACE");
      inputKey->appendKeysycode(0x2C, 0, RID_KEYBOARD, 1);
      inputString.remove(0, 10);
    }
    else if(inputSearch.Match("^(\\BACKSPACE_KEY)") == REGEXP_MATCHED){
      Serial.println("found BACKSPACE");
      inputKey->appendKeysycode(0x2A, 0, RID_KEYBOARD, 1);
      inputString.remove(0, 14);
    }
    else if(inputSearch.Match("^(\\DEL_KEY)") == REGEXP_MATCHED){
      Serial.println("found DEL");
      inputKey->appendKeysycode(0x4C, 0, RID_KEYBOARD, 1);
      inputString.remove(0, 8);
    }
    else if(inputSearch.Match("^(\\ENTER_KEY)") == REGEXP_MATCHED){
      Serial.println("found ENTER");
      inputKey->appendKeysycode(0x28, 0, RID_KEYBOARD, 1);
      inputString.remove(0, 10);
    }
    else if(inputSearch.Match("^(\\ESC_KEY)") == REGEXP_MATCHED){
      Serial.println("found ESC");
      inputKey->appendKeysycode(0x29, 0, RID_KEYBOARD, 1);
      inputString.remove(0, 8);
    }
    else if(inputSearch.Match("^(\\TAB_KEY)") == REGEXP_MATCHED){
      Serial.println("found TAB");
      inputKey->appendKeysycode(0x2B, 0, RID_KEYBOARD, 1);
      inputString.remove(0, 8);
    }
    else if(inputSearch.Match("^(\\F1_KEY)") == REGEXP_MATCHED){
      Serial.println("found F1");
      inputKey->appendKeysycode(0x3A, 0, RID_KEYBOARD, 1);
      inputString.remove(0, 7);
    }
    else if(inputSearch.Match("^(\\F2_KEY)") == REGEXP_MATCHED){
      Serial.println("found F2");
      inputKey->appendKeysycode(0x3B, 0, RID_KEYBOARD, 1);
      inputString.remove(0, 7);
    }
    else if(inputSearch.Match("^(\\F3_KEY)") == REGEXP_MATCHED){
      Serial.println("found F3");
      inputKey->appendKeysycode(0x3C, 0, RID_KEYBOARD, 1);
      inputString.remove(0, 7);
    }
    else if(inputSearch.Match("^(\\F4_KEY)") == REGEXP_MATCHED){
      Serial.println("found F4");
      inputKey->appendKeysycode(0x3D, 0, RID_KEYBOARD, 1);
      inputString.remove(0, 7);
    }
    else if(inputSearch.Match("^(\\F5_KEY)") == REGEXP_MATCHED){
      Serial.println("found F5");
      inputKey->appendKeysycode(0x3E, 0, RID_KEYBOARD, 1);
      inputString.remove(0, 7);
    }
    else if(inputSearch.Match("^(\\F6_KEY)") == REGEXP_MATCHED){
      Serial.println("found F6");
      inputKey->appendKeysycode(0x3F, 0, RID_KEYBOARD, 1);
      inputString.remove(0, 7);
    }
    else if(inputSearch.Match("^(\\F7_KEY)") == REGEXP_MATCHED){
      Serial.println("found F7");
      inputKey->appendKeysycode(0x40, 0, RID_KEYBOARD, 1);
      inputString.remove(0, 7);
    }
    else if(inputSearch.Match("^(\\F8_KEY)") == REGEXP_MATCHED){
      Serial.println("found F8");
      inputKey->appendKeysycode(0x41, 0, RID_KEYBOARD, 1);
      inputString.remove(0, 7);
    }
    else if(inputSearch.Match("^(\\F9_KEY)") == REGEXP_MATCHED){
      Serial.println("found F9");
      inputKey->appendKeysycode(0x42, 0, RID_KEYBOARD, 1);
      inputString.remove(0, 7);
    }
    else if(inputSearch.Match("^(\\F10_KEY)") == REGEXP_MATCHED){
      Serial.println("found F10");
      inputKey->appendKeysycode(0x43, 0, RID_KEYBOARD, 1);
      inputString.remove(0, 8);
    }
    else if(inputSearch.Match("^(\\F11_KEY)") == REGEXP_MATCHED){
      Serial.println("found F11");
      inputKey->appendKeysycode(0x44, 0, RID_KEYBOARD, 1);
      inputString.remove(0, 8);
    }
    else if(inputSearch.Match("^(\\F12_KEY)") == REGEXP_MATCHED){
      Serial.println("found F12");
      inputKey->appendKeysycode(0x45, 0, RID_KEYBOARD, 1);
      inputString.remove(0, 8);
    }
    else if(inputSearch.Match("^(\\UP_KEY)") == REGEXP_MATCHED){
      Serial.println("found UP");
      inputKey->appendKeysycode(0x52, 0, RID_KEYBOARD, 1);
      inputString.remove(0, 7);
    }
    else if(inputSearch.Match("^(\\DOWN_KEY)") == REGEXP_MATCHED){
      Serial.println("found DOWN");
      inputKey->appendKeysycode(0x51, 0, RID_KEYBOARD, 1);
      inputString.remove(0, 9);
    }
    else if(inputSearch.Match("^(\\LEFT_KEY%{)([%d]+)%}") == REGEXP_MATCHED){
      Serial.println("found LEFT COUNT");
      tmp_counter = atoi(inputSearch.GetCapture(buff,1));
      for(int i = 0; i < tmp_counter; i++){
        inputKey->appendKeysycode(0x50, 0, RID_KEYBOARD, 1);
      }
      inputString.remove(0, inputString.indexOf("}")+1);
    }
    else if(inputSearch.Match("^(\\LEFT_KEY)") == REGEXP_MATCHED){
      Serial.println("found LEFT");
      inputKey->appendKeysycode(0x50, 0, RID_KEYBOARD, 1);
      inputString.remove(0, 9);
    }
    else if(inputSearch.Match("^(\\RIGHT_KEY%{)([%d]+)%}") == REGEXP_MATCHED){
      Serial.println("found RIGHT COUNT");
      tmp_counter = atoi(inputSearch.GetCapture(buff,1));
      for(int i = 0; i < tmp_counter; i++){
        inputKey->appendKeysycode(0x4F, 0, RID_KEYBOARD, 1);
      }
      inputString.remove(0, inputString.indexOf("}")+1);
    }
    else if(inputSearch.Match("^(\\RIGHT_KEY)") == REGEXP_MATCHED){
      Serial.println("found RIGHT");
      inputKey->appendKeysycode(0x4F, 0, RID_KEYBOARD, 1);
      inputString.remove(0, 10);
    }
    else if(inputSearch.Match("^(\\MIDI_CC%{)([%d]+),([%d]+)%}") == REGEXP_MATCHED){  // \MIDI_CC{CONTROL_NUM,CHANNEL}   //Get value from Poti
      Serial.println("found MIDI_CC");
      inputKey->isAnalog = 1;
      inputKey->isMIDI = 1;
      inputKey->MIDI_mode = MIDI_CC;
      inputKey->MIDI_data1 = atoi(inputSearch.GetCapture(buff,1));
      inputKey->MIDI_channel = atoi(inputSearch.GetCapture(buff, 2));
      inputString.remove(0, inputString.indexOf("}")+1);
    }
    else if(inputSearch.Match("^(\\MIDI_CC_KEY%{)([%d]+),([%d]+)%}") == REGEXP_MATCHED){  // \MIDI_CC_KEY{CONTROL_NUM,CHANNEL}   //Get key state
      Serial.println("found MIDI_CC_KEY");
      inputKey->isMIDI = 1;
      inputKey->MIDI_mode = MIDI_CC;
      inputKey->MIDI_data1 = atoi(inputSearch.GetCapture(buff,1));
      inputKey->MIDI_channel = atoi(inputSearch.GetCapture(buff, 2));
      inputString.remove(0, inputString.indexOf("}")+1);
    }
    else if(inputSearch.Match("^(\\COLOR%{)([%a]+),([%a]+)%}") == REGEXP_MATCHED){  // \COLOR{MODE,COLOR}
      Serial.println("found COLOR");
      inputKey->color_mode = string_to_color_mode(inputSearch.GetCapture(buff,1));
      inputKey->color = string_to_color(inputSearch.GetCapture(buff,2));
      inputString.remove(0, inputString.indexOf("}")+1);
    }
    else if(inputSearch.Match("^(\\COLOR%{)([%a]+),([%d]+),([%d]+),([%d]+)%}") == REGEXP_MATCHED){  // \COLOR{MODE,R,G,B}
      Serial.println("found COLOR_RGB");
      inputKey->color_mode = string_to_color_mode(inputSearch.GetCapture(buff,1));
      uint8_t r = atoi(inputSearch.GetCapture(buff, 2));
      uint8_t g = atoi(inputSearch.GetCapture(buff, 3));
      uint8_t b = atoi(inputSearch.GetCapture(buff, 4));
      inputKey->color = ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
      inputString.remove(0, inputString.indexOf("}")+1);
    }
    else if(inputSearch.Match("^(\\LAYER%{)([%d]+)%}") == REGEXP_MATCHED){  // \LAYER{LAYER_NUM}   //Get value from Poti
      Serial.println("found Layer change");
      inputKey->hasLayerChange = true;
      inputKey->changeToLayer = atoi(inputSearch.GetCapture(buff,1));
      inputString.remove(0, inputString.indexOf("}")+1);
    }
    else if(inputSearch.Match("^([\\]+)") == REGEXP_MATCHED){
      Serial.println("found backslash");
      keycodeBufferString += inputSearch.GetCapture(buf, 0);
      inputKey->appendKeysycode(0x2D, KEY_MOD_RALT, RID_KEYBOARD, 1);
      inputString.remove(0, keycodeBufferString.length());
    }
    else{
      Serial.println("broke");
      keycodeBufferString.remove(0);
      break;
    }
    delete[] inputBuffer;
    delete[] buf;
    itterations++;
  }
  Serial.print("KeycodesSize: "); Serial.println(inputKey->getKeycodesSize());
  if(inputKey->getKeycodesSize() == (uint16_t)1){
    Serial.println("is single set");
    inputKey->isSingleKey = true;
  }
  if(inputKey->getKeycodesSize() == (uint16_t)0){
    Serial.println("key was empty after interpret");
    inputKey->appendKeysycode(0,0,0,0);
  }
  Serial.print("is single read: ");Serial.println(inputKey->isSingleKey);
}

void interpreter::stringToKeycodes(key * inputKey, String inputString){
  // clear keysycode vector with key.clearToZero()

  //inputKey->clearToZero();

  uint8_t sendimediate = 0;

  if(inputKey->getKeycodesSize() > 0){
    sendimediate = 1;
  }

  for(uint16_t i = 0; i < inputString.length(); i++){           //may need +1
    if(inputString.charAt(i+1) != -1){
      if(ASCII_conv_table_german[inputString.charAt(i)][0] != ASCII_conv_table_german[inputString.charAt(i + 1)][0]){
        sendimediate = 1;
      }
    }
    Serial.printf("running stringToKeycode with char: %c\n", inputString.charAt(i));
    if(inputString.length() - i == 1){
      sendimediate = 1;
    }
    inputKey->appendKeysycode(ASCII_conv_table_german[inputString.charAt(i)][1], ASCII_conv_table_german[inputString.charAt(i)][0], RID_KEYBOARD, sendimediate);
    sendimediate = 0;
  }
  return;
}

uint32_t interpreter::string_to_color(char *input){
  std::string buffer;
  uint8_t r,g,b;

  buffer.append(input);

  if(buffer.find("RED", 0) != -1 || buffer.find("red", 0) != -1){
    r = 255;
    g = 0;
    b = 0;
  }
  else if(buffer.find("YELLOW", 0) != -1 || buffer.find("yellow", 0) != -1){
    r = 255;
    g = 255;
    b = 0;
  }
  else if(buffer.find("GREEN", 0) != -1 || buffer.find("green", 0) != -1){
    r = 0;
    g = 255;
    b = 0;
  }
  else if(buffer.find("CYAN", 0) != -1 || buffer.find("cyan", 0) != -1){
    r = 0;
    g = 255;
    b = 255;
  }
  else if(buffer.find("BLUE", 0) != -1 || buffer.find("blue", 0) != -1){
    r = 0;
    g = 0;
    b = 255;
  }
  else if(buffer.find("MAGENTA", 0) != -1 || buffer.find("magenta", 0) != -1){
    r = 255;
    g = 0;
    b = 255;
  }
  else if(buffer.find("WHITE", 0) != -1 || buffer.find("white", 0) != -1){
    r = 255;
    g = 255;
    b = 255;
  }
  else{
    r = 0;
    g = 0;
    b = 0;
  }

  return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;

}

color_mode_t interpreter::string_to_color_mode(char *input){
  std::string buffer;

  buffer.append(input);
  if(buffer.find("CONST",0) != -1 || buffer.find("const",0) != -1){
    return const_color;
  }
  else if(buffer.find("PRESSED",0) != -1 || buffer.find("pressed",0) != -1){
    return pressed;
  }
  else if(buffer.find("NOT PRESSED",0) != -1 || buffer.find("not pressed",0) != -1){
    return not_pressed;
  }
  else if(buffer.find("TOGGLE",0) != -1 || buffer.find("toggle",0) != -1){
    return toggle;
  }
  else if(buffer.find("MIDI",0) != -1 || buffer.find("midi",0) != -1){
    return midi_bound;
  }
  else if(buffer.find("DISABLED",0) != -1 || buffer.find("disabled",0) != -1){
    return disabled;
  }
  else{
    Serial.printf("no string_to_color found\n");
    return no_override;
  }
}
rgb_wrapper_t interpreter::string_to_color_effect(std::string effect, uint32_t color, uint32_t speed){
  
  if(effect.find("RAINBOW",0) != -1 || effect.find("rainbow",0) != -1){
    return {effect_rainbow, color, speed};
  }
  else if(effect.find("CONST_COLOR",0) != -1 || effect.find("const_color",0) != -1 || effect.find("CONST",0) != -1 || effect.find("const",0) != -1){
    return {effect_const_color, color, speed};
  }
  else{
    Serial.printf("no effect found. Parameters: Input: %s, input_size: %d, color: %d, speed: %d\n", effect.c_str(), effect.size(), color, speed);
    return {effect_rainbow, 0, 5};    //default  
  }
  
}
// make seperate Funktion to check for immediate Send!!!! Will make Life a lot easier


