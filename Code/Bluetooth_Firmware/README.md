# README Bluetooth Firmware

## Compilation instructions

First you will need the esp32 Core by Espressif.

**IMPORTANT:** You need to install Version *2.0.11*. The newer Versions feature a different String type in the *begin* function. This will cause compilation errors.

### Upload settings

In order of appearance:

- **Board:** "XIAO_ESP32C3"
- **USB CDC On Boot:** "Enabled"
- **CPU Frequency** "160MHz (WiFi)"
- **Core Debug Level:** "None"
- **Erase All Flash Before Sketch Upload:** "Disabled"
- **Flash Frequency:** "80MHz"
- **Flash Mode:** "QIO"
- **Flash Size:** "4MB (32Mb)"
- **Partition Scheme:** "Default 4MB with spiffs (1.2MB APP/1.5MB SPIFFS)"
- **Upload Speed:** "921600"

## Config Options

 In ```BleKeyboard bleKeyboard("Macruwu", "FG Labs", 69);``` the *Device Name*, *Manufacturer* and *Initial Battery Level* can be set.

In the ```receiveEvent``` function new/custom I²C commands can be added.

```c
while(Wire.available()){
reportID = Wire.read();
switch(reportID){
  case RID_KEYBOARD:
    tmp->modifiers = Wire.read();
    tmp->keys[0] = Wire.read();
    tmp->keys[1] = Wire.read();
    tmp->keys[2] = Wire.read();
    tmp->keys[3] = Wire.read();
    tmp->keys[4] = Wire.read();
    tmp->keys[5] = Wire.read();
    break;
  case 15:
    softDisableFlag = Wire.read();
    if(softDisableFlag != 0) softDisableFlag = true;
    break;
  default:
    Wire.read();
    break;
}
```

As of creation of the README, 2 functions have been implemented.

- Function 1 (ReportID = 1)
  - Reads a Key report
  - Order:
    - modifier: 1 byte
    - keys: 6 bytes
- Function 2 (ReportID = 15)
  - Sends ESP in to deep sleep
  - This is used to save power, when the Keyboard is connected and registered via USB HID.
- default (ReportID = *none of the above*)
  - voids the I²C buffer

To add a function

- assign a ReportID
- add a new case to the switch

## Troubleshooting

- Macruwu is connected, but doesn't send Keycodes
  1. Unpair Keyboard from Host
  2. power cycle the Keyboard
  3. Re-pair the Keyboard
- Macruwu can't be found
  1. Toggle the power (quick ON/OFF flipping of the power switch)
- ESP stuck in boot loop
  1. Sorry to hear that :(
  2. Make sure the battery board is powered on correctly
     - If the Voltage converter is buzzing, you have a power problem
       - The Battery could be flat (or close to)
       - The board is not turned on properly (use the power toggle switch)
  3. Check the upload parameters
     - Try changing the *Flash Frequency* to *40MHz*
