# M5Stack-Thermal-Camera
forked from hkoffer https://github.com/hkoffer/M5Stack-Thermal-Camera-

What differ from the original repo:
- Code refactoring
- Display the min value
- Display FPS
- Autoscaling temperature
- Move the spot value (in float) to the center
- Pin point the min and max pixel (min colored in blue and max in white)
- Auto reboot and reset the i2c port in case of bad connexion
- Add Frozen state

Usage:
- When in running state:
  - Press A to increase minimal temperature
  - Long press A to decrease minimal temperature
  - Press B to freeze the screen
  - Press C to increase maximal temperature
  - Long press C to decrease maximal temperature
- When in frozen state
  - Quickly press B again (within 2seconds after initial push in running state) to autoscale (take min/max) and return to live
  - Press B to unfreeze the screen
  - Press A to activate/deactivate the pin point of MAX temperature
  - Press C to activate/deactivate the pin point of MIN temperature
  
Might do (eventually):
- Save to SD (maybe save the buffer as bmp then display it and let the choice to save when on frozen state)
- Add sensor reading as overlay with low alpha over a real camera feed (need to compensate the angle)
- Web interface (yes, it's useless but I like remote control of crap)
- API REST of the raw values (because why not)

<img style="-webkit-user-select: none;cursor: zoom-in;" src="https://raw.githubusercontent.com/m600x/M5Stack-Thermal-Camera/master/m5stack.jpg" >

This is a library for the Adafruit AMG88xx based thermal cameras:
Adafruit invests time and resources providing this open source code, please support Adafruit and open-source hardware by purchasing products from Adafruit!
Written by Dean Miller for Adafruit Industries. MIT license, all text above must be included in any redistribution
