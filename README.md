|# esp8266_NeoPixelLightStick
An ESP8266 based light wand for light painting.

It uses the Arduino IDE (https://www.arduino.cc/) with the staging version of https://github.com/esp8266/Arduino to support the ESP8266 to build the software.
The light wand may be controlled by an Android device with the companion application LightStickSender found under https://github.com/alj56/LightStickSender.git.

Build parameters:
* Generic ESP8266 Modul
* Flash Mode: DIO
* Flash Frequency: 40 MHz, 80 MHz works too
* CPU Frequency: 80 MHz, 160 MHz works too
* Flash Size: at least 1M (512 SPIFFS) to work with uploaded bitmaps

Commands:

Type | Command                                   | Explanation
---- | ----------------------------------------- | -----------
TCP  | LS<0><1>L< bytes>                         | load bitmap to internal storage
TCP  | LS<0><1>S< delay LSB><delay MSB>          | show loaded bitmap file with <delay> ms between lines
TCP  | LS<0><1>U                                 | switch mode to udp
TCP  | LS<0><1>D< byte>                          | set debug mode
UDP  | LS<0><1>T                                 | switch mode to tcp
UDP  | LS<0><1>L< length LSB><length MSB><bytes> | show data for a line

Where:

Item          | Explanation
------------- | -----------
<0>           | binary 0
<1>           | binary 1
< bytes>      | binary data
< byte>       | a binary byte
< length MSB> | binary data, most significatnt byte of length parameter
< length LSB> | binary data, least significatnt byte of length parameter

Bitmap data format:
LS<0><1>B<width LSB><width MSB><height LSB><height MSB><bytes>
<bytes>: length * width * 3 bytes

Bitmap storage in a raw flash sector:
Address RawFlashDriver._startAddress + RawFlashDriver._flashSecSize:
  LS<0><1>B<width LSB><width MSB><height LSB><height MSB>
Startaddress + linenumber * BUFFER_SIZE
  <bytes> for a line
