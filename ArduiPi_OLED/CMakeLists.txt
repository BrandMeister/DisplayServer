cmake_minimum_required(VERSION 2.6)

project(ArduiPi_OLED)

set(SOURCES ArduiPi_OLED.cpp
	Adafruit_GFX.cpp
	bcm2835.c
	Wrapper.cpp
	smbus.c)

set(HEADERS Adafruit_GFX.h
	ArduiPi_OLED.h
	ArduiPi_OLED_lib.h
	bcm2835.h
	smbus.h)

add_library(ArduiPi_OLED STATIC ${SOURCES} ${HEADERS})

install(TARGETS ArduiPi_OLED ARCHIVE DESTINATION "${CMAKE_INSTALL_PREFIX}/lib")
install(FILES ${HEADERS} DESTINATION "${CMAKE_INSTALL_PREFIX}/include")
