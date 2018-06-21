# M5Stack-Thermal-Camera
forked from hkoffer https://github.com/hkoffer/M5Stack-Thermal-Camera-

What differ from the original repo:
- Code refactoring
- Display the min value
- Display FPS
- Autoscaling temperature with B button
- Move the spot value (in float) to the center
- Pin point the min and max pixel (min colored in blue and max in white)
- Auto reboot and reset the i2c port in case of bad connexion
- (ON HOLD) Add Frozen state (press B button)

Usage:
- Press A to increase minimal temperature
- Long press A to decrease minimal temperature
- Press B to Autoscale the temperature (take current min/max)
- Press C to increase maximal temperature
- Long press C to decrease maximal temperature

<img style="-webkit-user-select: none;cursor: zoom-in;" src="https://raw.githubusercontent.com/m600x/M5Stack-Thermal-Camera/master/m5stack.jpg" >

This is a library for the Adafruit AMG88xx based thermal cameras:
Adafruit invests time and resources providing this open source code, please support Adafruit and open-source hardware by purchasing products from Adafruit!
Written by Dean Miller for Adafruit Industries. MIT license, all text above must be included in any redistribution
