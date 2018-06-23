# M5Stack Thermal Camera
forked from hkoffer https://github.com/hkoffer/M5Stack-Thermal-Camera-

What differ from the original repo:
- Code refactoring
- UX revamped
- Display the min value
- Display FPS
- Autoscaling temperature
- Move the spot value (in float) to the center
- Pin point the min and max pixel (min colored in blue and max in white)
- Auto reboot and reset the i2c port in case of bad connexion
- Add Frozen state

Usage:
Menu cycle with button A
- MODE:
  - Button B: Autoscaling (take current min/max)
  - Button C: Freeze the screen
- SMIN:
  - Button B: Reduce the minimal temp of the scale
  - Button C: Increase the minimal temp of the scale
- SMAX:
  - Button B: Reduce the maximal temp of the scale
  - Button C: Increase the maximal temp of the scale
- POINT:
  - Button B: Activate/Deactivate the pinpoint of the minimal reading
  - Button C: Activate/Deactivate the pinpoint of the maximal reading

- When in frozen state:
  - Button A: Turn off the camera
  - Button B: Nothing
  - Button C: Unfreeze the camera

<img style="-webkit-user-select: none;cursor: zoom-in;" src="https://raw.githubusercontent.com/m600x/M5Stack-Thermal-Camera/master/m5stack.jpg" >

---

This is a library for the Adafruit AMG88xx based thermal cameras:
Adafruit invests time and resources providing this open source code, please support Adafruit and open-source hardware by purchasing products from Adafruit!
Written by Dean Miller for Adafruit Industries. MIT license, all text above must be included in any redistribution
