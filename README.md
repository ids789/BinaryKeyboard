# Binary Keyboard Device

Firmware for an experimental Bluetooth keyboard which allows you to type using only 2 buttons.

##### Goal
Build a phone or tablet input device for use while flying a plane.  To be used for noting down air traffic control instructions, making navigational notes and doing calculations.  
- Must take up little space so that it can fit in a small cockpit.  
- Must be able to type quickly and comfortably, even during turbulence.  
- Must be able to use it without looking and when under high work load while doing other tasks.  

*This is an alternative to another project for the same purpose [link](https://github.com/ids789/ChordedKeyboardFirmware)

##### Concept
Build small lightweight input device which converts standard Morse code sequences into key presses.  
- Have 2 buttons for input: a left button to represent a dot and a right button to represent a dash.  
- Access extra non-standard codes such as backspace without requiring long sequences by pressing both buttons together.   
- Connect to a phone or computer wirelessly over Bluetooth LE


### Implementation
##### Firmware
- Act as a Bluetooth LE HID keyboard, allowing it to connect to any device that supports Bluetooth LE and act as a keyboard without any additional software.  
- Keys are inputted by quickly pressing the desired sequence, if a key is not pressed for more than 100ms (configurable) it will signal the end of the sequence.  The sequence is then checked in a lookup table for its corresponding key.  
- On the first key press of a sequence if both buttons are pressed simultaniously it will be put into 'alt key' mode and an alternative lookup table is used.  
- The device battery level is sent over a Bluetooth battery service (BAS Service) allowing it to be monitored from a phone's bluetooth settings page.  
- To connect to the keyboard hold a key down while switching the keyboard on, the LED will flash rapidly to indicate that it is in pairing mode.  Holding 1 key will add another device while holding both keys will clear all existing devices.  
- Pressing the power button switches the keyboard off by putting the microcontroller into low power mode.  The keyboard will also sleep after 5 minutes of inactivity, then pressing any key will wake it up.  (it can power up and reconnect to a Blueooth device very quickly)
- The status LED flashes to indicate that it is waiting for a device to connect and is solid ON to indicate that it has connected to a Bluetooth device.  

Key | Alt Code
-----|----
Space | .
Enter | ..
Backspace | -
Left | .--
Right | .---
Shift | .-
Escape | --

##### Hardware:
- Based on the Nordic Semiconductor NRF52840 microcontroller, currently on an Adafruit Feather Express development board.  
- Buttons use Cherry key switches from an old mechanical keyboard.    
- Powered from a 350mAh LiPo battery, which integrates with the charging hardware built into the Adafruit board and lasts for about a week of regular use.    
- An combo power button and LED indicator is used to show the state and put the microcontroller into sleep mode.  

Device | Port
-------|-----
Left Button (DOT) | P0.06 (D11)
Right Button (DASH) | P0.08 (D12)
Power Button | P0.26 (D9) 
Status LED | P0.27 (D10)

<img src="/misc/demo.gif" width="500" />
<p float="left">
  <img src="/misc/photo1.jpg" width="250" />
  <img src="/misc/photo2.jpg" width="250" />
</p>

