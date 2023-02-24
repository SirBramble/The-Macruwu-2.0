# The Macruwu
The macro pad for all! (well, at least, if you can understand the Clusterfuck posted here)
The Firmware is based on the Arduino Pico Core by earlephilhower https://github.com/earlephilhower/arduino-pico. 

**This is only a brief overview** The full documentation and instructions can be found in the "Documentation"-folder.

## Components used

I went with the RP2040 based board from seeed studio. It has a fast chip, offers integrated USB support and is only about 6€. However, it has limited GPIO pins. So if you want to build your own, I would recommend using I/O Expanders or something similar. Another option would be to use a raspberry Pi with a RP2040. This Board beaks out most of the RP2040 Pins and has Options for Wi-Fi.

In order to try out different Key switches and keep the Macruwu modular, I decided to use hot swap sockets. They aren’t to expensive, if you order them for AliExpress (about 1\$/10psc). But beware, when soldering. If your board manufacturer decides to give you the shit coating on your Pads, prepare for heavy flux use.

For the I/O Expanders I used the PCA9555 from NXP. They conveniently don’t need a lot of setup and are automatically set to INPUT_PULLUP. The detailed Addressing scheme and co can be found in the Datasheet. But, as per usual nowadays, the I/O expanders are sold out. So, you might want to look for them somewhere else, or find a suitable replacement. But if you can find them, I can recommend the NXP Expanders. They need minimal to no Software Setup and I haven't had a dead one so far.

The level shifter (TXU0104PWR) is a cheap option I got from Mouser. It has 4 channels it can shift and is enough for the two 5V I²C Lines and the neopixel line.

The keyswitches were purchased for AliExpress and some smaller Keyboard parts suppliers. I like clicky switches, so I went for the Gateron greens. However, I cannot recommend them. They sound quite hollow and scratchy. Could maybe fixed with some lube *gigidy*, but it’s not really worth the trouble. 
If you want some really clicky Keyswitches, that will any the hell out of anybody in a 30km Radius, you could go for the Kailh Low profile Chock whites. My, extremely pissed of, fellow student can confirm. However, be aware that they can not be used with relegendable keycaps.
## How to compile

### With the Arduino IDE 2.0
#### Macruwu
First download the Code either from the repo or via the release tab. Put it in a folder and open the ```Macruwu_OS.ino``` file.

In order to compile the Code, you need to install the Arduino Pico core. There is a Tutorial on how to do this on earlephilhowers GitHub. You will also need the Adafruit SPIFlash Library. It can be downloaded via the Library Manager. The rest should be included in the Pico Core.
If you get Library conflicts, I'm afraid you're on you’re own. I somehow got it to compile and haven’t touched my setup since. No Idea why it suddenly worked, but the only thing I can recommend, is to back up your libraries and start fresh.

EDIT: If you get a Library conflict, you may want to look for other Bugs. Sometimes the Arduino IDE shows this message, just to inform you, that it has used a certain Library. However, your Code may not compile due to another error.

Also remember to change the USB Stack to the Adafruit TinyUsb Stack via Tools->USB Stack.
If you want to use the inbuilt Flash as a drive, you also have to allocate Memory under Tools->Flash Size. The first time you allocate Memory, you will have to format it. Just doing it in Windows should be fine.
P.s. The flash drive code is more or less copied from Adafruit. sorry, I was lazy...

#### Bluetuwu
First download the Code either from the repo or via the release tab. Put it in a folder and open the ```Bluetuwu_OS.ino``` file.

To Compile the Code you need to install the ESP32 Arduino Core from Espressif. After you installed this, you can just upload with the standard Settings.

### With PlatformIO
Well... I have no Idea, but let me know, if you find out.

## How to use (isch...)

The Macruwu OS is divided in to several Files for maximum ```'someFuckingShit' is not defined did you mean 'goFuckYourself'?```

### With macroLayout.txt

NOTE: The first time you start the Macruwu, you might have problem, with it crashing. This may be caused by the File not being present in the Flash directory. You may have to disable the ```readFile```-function in the ```Keyboard::init```-function in ```Keyboard.cpp``` and upload the File first. Then reenable the function and it should run.

The Layout can be set with several Commands. ```Layer x: ``` sets the Layer under which the Buttons will be accessible. ```Button x: {string}``` sets the Button and is followed by the string, that will be printed by the keyboard. Remember to leave a space between ```:``` and ```{string}```. This is due to the way that the file gets read. The Character straight after the ```:``` will be ignored.

So far, most characters will be interpreted correctly, however ```<``` and ```>``` have a special Meaning. They press the left and right arrow keys respectively. Verry useful for Macro pads that print code with a lot of Brackets. Other functions have not yet been implemented, but feel free to add them in the ```Keyboard.cpp``` under ```Keyboard::interpet```.

In order to read the file from the Flash in to the Data structure you want to run the ```readFile```-function at least once. I recommend running it once in ```setup()``` and then running it each time the file is changed. this can be done with:
```
if(Macruwu.fsChanged()){
    Macruwu.readFile();
  }
```

In order to run the commands that have been set in the Data structure you will have to run the ```interpret(layer, button)```-function.

### With Hardcoded Mapping
The Keymapping can be set in the ```Mapping.cpp```.

In order to write something over USB, you can use ```Keyboard1.write("something");``` But this only works for most normal characters like letters and sometimes other specialty symbols. You'll have to try it out. Things like Brackets or things that use function keys, will have to be set up manually. This can either be done, by sending the commands via the Tiny USB library directly, or by adding a new function to the Keyboard class. But look through the Keyboard class, as I may add more functions in the future.

**Word of Warning:** If you do not have a US Keyboard, be prepared for some HID Code treasure hunting. I haven’t found a way to change the layout, so you either have to find a Layout and redefine all the keys in the library, or just try and hope you find what you need. If you have a German Keyboard layout set, you can reference the code for some mappings.

## Bugs
Yes

## Further documentation
I recommend reading the documentation by earlephilhower https://github.com/earlephilhower/arduino-pico. It´s a far better than this clusterfuck, but feel free to look through the code.

And in case you didn't read the first 3 lines, you can find more documentation in the "Documentation"-folder.
