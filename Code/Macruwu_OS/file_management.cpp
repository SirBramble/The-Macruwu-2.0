#include "file_management.h"

#include "SPI.h"
#include <SdFat.h>
#include "Adafruit_SPIFlash.h"
#include "Adafruit_TinyUSB.h"

#include "flash_config.h"

// for flashTransport definition

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
  digitalWrite(LED_BUILTIN, HIGH);

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

  digitalWrite(LED_BUILTIN, LOW);
}


// the setup function runs once when you press reset or power the board
void flash_setup()
{
  pinMode(LED_BUILTIN, OUTPUT);

  flash.begin();

  // Set disk vendor id, product id and revision with string up to 8, 16, 4 characters respectively
  usb_msc.setID("Macruwu", "BIG DICK FLASH", "6.9");

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



void flash_loop()
{
  if ( fs_changed )
  {
    fs_changed = false;

    // check if host formatted disk
    if (!fs_formatted)
    {
      fs_formatted = fatfs.begin(&flash);
    }

    // skip if still not formatted
    if (!fs_formatted) return;

    Serial.println("Opening root");

    if ( !root.open("/") )
    {
      Serial.println("open root failed");
      return;
    }

    Serial.println("Flash contents:");

    // Open next file in root.
    // Warning, openNext starts at the current directory position
    // so a rewind of the directory may be required.

    while ( file.openNext(&root, O_RDONLY) )
    {
      file.printFileSize(&Serial);
      Serial.write(' ');
      file.printName(&Serial);
      if ( file.isDir() )
      {
        // Indicate a directory.
        Serial.write('/');
      }
      Serial.println();
      file.close();
    }

    root.close();
    //flash_test();

    Serial.println();
    delay(1000); // refresh every 1 second
  }
}

bool check_fs_changed(){
  return fs_changed;
}

void set_fs_changed(bool fs_changed_in){
  fs_changed = fs_changed_in;
}

void flash_clear(){
  if (!flash.eraseChip()) {
    Serial.println("Failed to erase chip!");
  }

  flash.waitUntilReady();
  Serial.println("Successfully erased chip!");
  
  String filename_to_open = "macroLayout.txt";
  if(!fatfs.exists(filename_to_open)){
    Serial.println("macroLayout.txt does not exist");
    Serial.println("Trying to create macroLayout.txt...");
    File32 writeFile = fatfs.open("macroLayout.txt", FILE_WRITE);
    if (!writeFile) {
      Serial.println("Error, failed to open macroLayout.txt for writing!");
      while(1) yield();
    }
    Serial.println("Created macroLayout.txt succesfully");
    writeFile.println("This is a test line, please ignore...");
    writeFile.close();
  }
}

void flash_test(){
  String filename_to_open = "macroLayout.txt";
  if(!fatfs.exists(filename_to_open)){
    Serial.println("macroLayout.txt does not exist");
    Serial.println("Trying to create macroLayout.txt...");
    File32 writeFile = fatfs.open("macroLayout.txt", FILE_WRITE);
    if (!writeFile) {
      Serial.println("Error, failed to open macroLayout.txt for writing!");
      while(1) yield();
    }
    Serial.println("Created macroLayout.txt succesfully");
    writeFile.println("This is a test line, please ignore...");
    writeFile.close();
  }
  File32 readFile = fatfs.open(filename_to_open, FILE_READ);
  if (!readFile) {
    Serial.println("Error, failed to open macroLayout.txt for reading!");
    while(1) yield();
  }

  String file;
  while (readFile.available()) {
    file.concat((char)readFile.read());
  }
  Serial.println("file at 3:");
  Serial.println(file.charAt(3));
  if(file.indexOf('\n') != -1){
    Serial.println("found some shit");
  }

  if (!readFile.seek(0)) {
    Serial.println("Error, failed to seek back to start of file!");
    while(1) yield();
  }

  Serial.println("Entire contents of file:");
  while (readFile.available()) {
    char c = (char)readFile.read();
    Serial.print(c);
  }
  String sub;
  Serial.println("Entire contents of file from string:");
  Serial.println(file);
  Serial.println("linewise:");
    if (!readFile.seek(0)) {
      Serial.println("Error, failed to seek back to start of file!");
      while(1) yield();
    }
    while(readFile.available()){
      sub = readFile.readStringUntil('\n');
      Serial.println(sub);
    }
    readFile.close();
  delay(1000);
}



const char* digits = "0123456789";




/////////
//Layer//
/////////
Layer::Layer(int ammountKeys){
    for(int i = 0; i < ammountKeys; i++){
        Button.push_back("");
    }
}
String Layer::get(int indexButton){
    return Button.at(indexButton);
}
void Layer::set(int indexButton, String funktionString){
  if(indexButton != -1){
      Button.at(indexButton) = funktionString;
  }

}

//////////
//Keymap//
//////////
Keymap::Keymap(String filename, int ammountLayers, int ammountKeys){
    this->filename = filename;
    this->ammountLayers = ammountLayers;
    this->ammountKeys = ammountKeys;
}

void Keymap::init(){
  Layers.reserve(ammountLayers);
  for(int i = 0; i < ammountLayers; i++){
    Layers.push_back(Layer(ammountKeys));
  }
}

String Keymap::get(int indexLayer, int indexButton){
    indexLayer--;
    if((indexLayer>=ammountLayers)||(indexLayer < 0)){return "";}
    if((indexButton>=this->ammountKeys)||(indexButton < 0)){return "";}
    return (&Layers.at(indexLayer))->get(indexButton);
}

void Keymap::set(int indexLayer, int indexButton, String funktionString){
  #ifdef SERIAL_DEBUG
  Serial.println("Keymap Set Start:");
  Serial.print("indeyLayer: "); Serial.println(indexLayer);
  Serial.print("indeyButton: "); Serial.println(indexButton);
  Serial.print("funktionString: "); Serial.println(funktionString);
  #endif

  if(ammountKeys <= indexButton){
    #ifdef SERIAL_DEBUG
    Serial.println("Big Shit hit Fan.");
    #endif

    indexButton = -1;
  }
  
  if(ammountLayers < indexLayer){
    #ifdef SERIAL_DEBUG
    Serial.println("Big Shit hit Fan. You no make Layers Vektor big enough in Keymap.");
    #endif
  }
  else{
    (&Layers.at(indexLayer))->set(indexButton, funktionString);
  }
  
  #ifdef SERIAL_DEBUG
  Serial.println("Keymap::set end:");
  #endif
}

void Keymap::import(){
  #ifdef SERIAL_DEBUG
  Serial.println("Keymap Import:");
  #endif
    for(int i = 0; i < ammountLayers; i++){                   //Clear Mapping Structure
      for(int n = 0; n < AMMOUNT_KEYS; n++){
        this->set(i, n, "");
      }
    }
    String tmp;
    File32 macroLayout = fatfs.open(filename, FILE_READ);

    if (!macroLayout) {
      #ifdef SERIAL_DEBUG
      Serial.println("Error, failed to open file for reading!");
      Serial.println("Creating blank config file");
      #endif
      macroLayout.close();
      File32 blankLayout = fatfs.open(filename, FILE_WRITE);
      if(!blankLayout){
        while(1){
          #ifdef SERIAL_DEBUG
            Serial.println("If you can read this, the filesystem is fucked!");
          #endif
          delay(10);
        }
      }
      blankLayout.close();
    }

    if (!macroLayout.seek(0)) {
      #ifdef SERIAL_DEBUG
      Serial.println("Error, failed to seek back to start of file!");
      #endif
      while(1) yield();
    }
    
    while (macroLayout.available()) {
      tmp = macroLayout.readStringUntil('\n');
      if(tmp.indexOf("Layer") != -1){
        currentLayer = getNum(tmp) - 1;
        if(currentLayer == -1){
          continue;
        }
      }
      if(tmp.indexOf("Button") != -1){
        int sendNum = getNum(tmp);
        if(sendNum == -1){
          continue;
        }
        set(currentLayer, sendNum - 1 , getString(tmp));
      }
    }
  macroLayout.close();
}

int Keymap::getNum(String tmp){
    //Tipp: NEVER FUCKING USE ARDUINO STRINGS!!!! AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH!
    /*
    int tmpPos_first = 0;
    int tmpPos_first_tmp = 0;
    bool found_first_first = 0;
    int tmpPos_last = 0;
    int tmpPos_last_tmp = 0;
    int returnNumber = 0;
    for(int i = 0; i < 10; i++){
      tmpPos_first_tmp = tmp.indexOf(digits[i]);

      if((tmpPos_first_tmp != -1)&&(found_first_first == 0)){
        tmpPos_first = tmpPos_first_tmp;
        found_first_first = 1;
      }

      if((tmpPos_first_tmp < tmpPos_first) && (tmpPos_first_tmp > 0)&&(found_first_first == 1)){
        tmpPos_first = tmpPos_first_tmp;
      }

      tmpPos_last_tmp = tmp.lastIndexOf(digits[i]);

      if(tmpPos_last_tmp > tmpPos_last){
        tmpPos_last = tmpPos_last_tmp;
      }
    }
    */

    int tmpPos_first = -1;
    int tmpPos_last = 0;
    int indexOfDigits;
    int indexDoubleDot = tmp.indexOf(":");
    int returnNumber = 0;
    if(indexDoubleDot == -1){                                     //if no doubleDot was found, something went wrong. return -1
      return -1;
    }
    String searchString = tmp.substring(0, indexDoubleDot);

    for(int i = 0; i < 10; i++){                                  //find lowest index of any number in String
      indexOfDigits = searchString.indexOf(digits[i]);
      if(indexOfDigits == -1){                                    //if no number found in String, skip this loop itteration
        continue;
      }
      if(tmpPos_first == -1){
        tmpPos_first = indexOfDigits;
      }
      else if((indexOfDigits < tmpPos_first)&&(indexOfDigits != -1)){
        tmpPos_first = indexOfDigits;
      }
    }

    if(tmpPos_first == -1){                                       //if no number was found in String, return -1
      return -1;
    }
    tmpPos_last = tmpPos_first;                                   //try and prevent crashes

    for(int i = 0; i < 10; i++){
      indexOfDigits = searchString.lastIndexOf(digits[i]);        //returns last number found after the first number in the string
      if(indexOfDigits == -1){                                    //if no number found in String, skip this loop itteration
        continue;
      }
      if(indexOfDigits > indexDoubleDot){                         //if the number found comes after the ':', skip this loop itteration
        continue;
      }
      if(indexOfDigits > tmpPos_last){                            //if a number is found between the first found number and the ':', save this number as the last position
        tmpPos_last = indexOfDigits;
      }
    }

    if((tmpPos_last-tmpPos_first) > 2){                           //if the distance between the 2 points is to large, something went wrong. return -1
      return -1;
    }
    
    #ifdef SERIAL_DEBUG
    Serial.print("tmpPos_first: ");Serial.println(tmpPos_first);
    Serial.print("tmpPos_last: ");Serial.println(tmpPos_last);
    #endif

    String returnString = tmp.substring(tmpPos_first, (tmpPos_last + 1));
    for(int i = 0; i < returnString.length(); i++){
        returnNumber *= 10;
        returnNumber += returnString.charAt(i) - 48;
    }
    if((returnNumber <= 0)&&(returnNumber > 999)){
      return -1;
    }

    #ifdef SERIAL_DEBUG
    Serial.print("return Number in getNum(): ");Serial.println(returnNumber);
    #endif
    
    return returnNumber;
}

String Keymap::getString(String tmp){
    return tmp.substring(tmp.lastIndexOf(":") + 2);
}

