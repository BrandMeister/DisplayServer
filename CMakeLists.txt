cmake_minimum_required(VERSION 3.0)

set(APP_NAME "DisplayServer")
project(${APP_NAME} CXX)

if(GIT_VERSION)
  set(GIT_HASH ${GIT_VERSION})
else()
  execute_process(
    COMMAND
      git log -1 --pretty=format:%h --abbrev=10 HEAD ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_HASH
  )
  if (NOT GIT_HASH)
    set(GIT_HASH "0000000000")
  endif()
endif()
message(STATUS "Git revision: " ${GIT_HASH})

file(WRITE ${CMAKE_SOURCE_DIR}/GitVersion.h "const char *gitversion = \"${GIT_HASH}\";")

file(GLOB SOURCES "*.cpp")
file(GLOB HEADERS "*.h")

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Os -Wall -std=c++0x -pthread $ENV{CXXFLAGS}")
set(DEPLIBS "pthread")
set(INCLUDE_DIRS "")

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  # use our local copy of ArduiPi_OLED and libi2c (smbus.*)
  add_subdirectory(ArduiPi_OLED)
  list(APPEND DEPLIBS ArduiPi_OLED)
  list(APPEND INCLUDE_DIRS ArduiPi_OLED)
  add_definitions(-DOLED)
endif()

add_executable(${APP_NAME} ${SOURCES} ${HEADERS})
target_include_directories(${APP_NAME} PRIVATE ${INCLUDE_DIRS})
target_link_libraries(${APP_NAME} ${DEPLIBS})

include(GNUInstallDirs)
install (TARGETS ${APP_NAME} RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
install (FILES ${PROJECT_SOURCE_DIR}/DisplayServer.ini DESTINATION "${CMAKE_INSTALL_SYSCONFDIR}")
