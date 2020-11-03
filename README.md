These are the source files for building the DisplayServer, the program that
is used by DMRHost to control displays

It builds on 32-bit and 64-bit Linux. It can control
various Displays. Currently these are:

- Nextion TFTs (all sizes, both Basic and Enhanced versions)
- OLED 128x64 (SSD1306)
- LCDproc

The Nextion displays can connect to the UART on the Raspberry Pi, or via a USB
to TTL serial converter like the FT-232RL. It may also be connected to the UART
output of the MMDVM modem (Arduino Due, STM32, Teensy)

The OLED display needs an extra library: https://github.com/hallard/ArduiPi_OLED

The LCDproc support enables the use of a multitude of other LCD screens. See
the [supported devices](http://lcdproc.omnipotent.net/hardware.php3) page on
the LCDproc website for more info.

DisplayServer uses CMake as its building system:
```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```

To compile with OLED support:
```
mkdir build
cd build
cmake -DENABLE_OLED=ON -DCMAKE_BUILD_TYPE=Release ..
make
```

If you have questions, feel free to join our [telegram](https://t.me/dmrhost) group.
