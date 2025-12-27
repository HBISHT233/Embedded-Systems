# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "E:/Espressif/frameworks/esp-idf-v5.3.1/components/bootloader/subproject"
  "E:/Embedded-Systems/ESP32 Workspace/01_ST7789_Display_Module/build/bootloader"
  "E:/Embedded-Systems/ESP32 Workspace/01_ST7789_Display_Module/build/bootloader-prefix"
  "E:/Embedded-Systems/ESP32 Workspace/01_ST7789_Display_Module/build/bootloader-prefix/tmp"
  "E:/Embedded-Systems/ESP32 Workspace/01_ST7789_Display_Module/build/bootloader-prefix/src/bootloader-stamp"
  "E:/Embedded-Systems/ESP32 Workspace/01_ST7789_Display_Module/build/bootloader-prefix/src"
  "E:/Embedded-Systems/ESP32 Workspace/01_ST7789_Display_Module/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "E:/Embedded-Systems/ESP32 Workspace/01_ST7789_Display_Module/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "E:/Embedded-Systems/ESP32 Workspace/01_ST7789_Display_Module/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
