# M5Stack Thermal Camera
forked from hkoffer https://github.com/hkoffer/M5Stack-Thermal-Camera-

Latest revision: **20180626**

* [What differ from the original repo](#what-differ-from-the-original-repo)
* [Onboarding](#onboarding)
  + [Required library](#required-library)
  + [Settings](#settings)
* [Usage](#usage)
* [Might do (eventually)](#might-do-eventually)
* [Changelog](#changelog)

## What differ from the original repo
- Code refactoring (pretty much everything)
- UX revamped
- Display the min value
- Display FPS
- Autoscaling temperature
- Move the spot value (in float) to the center
- Pin point the min and max pixel (min colored in blue and max in white)
- Auto reboot and reset the i2c port in case of bad connexion
- Pause state
- Autosleep function

## Onboarding
### Required library
  - [M5Stack](https://github.com/m5stack/M5Stack)
  - [Adafruit AMG88xx](https://github.com/adafruit/Adafruit_AMG88xx)

### Settings
Please change the settings in the file M5Stack/M5Stack.ino if something is wrong (especially the orientation).

| Setting | Default | |
|:-:|:-:|---|
|`ORIENTATION`|1|Set the orientation of the screen<br>Some batch of M5Stack need it `1` while some other need `0`|
|`BRIGHTNESS`|255|Brightness level from 0 to 255|
|`SLEEP`|5|Time in minutes before auto turn off|
|`DEFAULT_MIN`|22|Minimal temperature scale at boot|
|`DEFAULT_MAX`|32|Maximal temperature scale at boot|

## Usage
If no buttons is used after the set period (5mins by default) the unit will power off automatically.

| Mode | Button | Function |
|---|:-:|---|
|MODE|<kbd>  B  </kbd>|Autoscaling (take current min/max)|
||<kbd>C</kbd>|Pause the screen|
|SMIN|<kbd>B</kbd>|Reduce the minimal temp of the scale|
||<kbd>C</kbd>|Increase the minimal temp of the scale|
|SMAX|<kbd>B</kbd>|Reduce the maximal temp of the scale|
||<kbd>C</kbd>|Increase the maximal temp of the scale|
|POINT|<kbd>B</kbd>|Activate/Deactivate the pinpoint of the minimal reading|
||<kbd>C</kbd>|Activate/Deactivate the pinpoint of the maximal reading|

When in frozen state:

|Button|Function|
|:-:|---|
|<kbd>A</kbd>|Turn OFF the camera (Also turn back ON)|
|<kbd>B</kbd>|Nothing|
|<kbd>C</kbd>|Unfreeze the camera|

## Might do (eventually)
- Save to SD (maybe save the buffer as bmp then display it and let the choice to save when on frozen state)
- Add sensor reading as overlay with low alpha over a real camera feed (need to compensate the angle)
- Web interface (yes, it's useless but I like remote control of crap)
- API REST of the raw values (because why not)

## Changelog
20180626 : Init.

---
(outdated picture but you get an idea of what it does)
<img style="-webkit-user-select: none;cursor: zoom-in;" src="https://raw.githubusercontent.com/m600x/M5Stack-Thermal-Camera/master/m5stack.jpg" >

---

This is a library for the Adafruit AMG88xx based thermal cameras:
Adafruit invests time and resources providing this open source code, please support Adafruit and open-source hardware by purchasing products from Adafruit!
Written by Dean Miller for Adafruit Industries. MIT license, all text above must be included in any redistribution
