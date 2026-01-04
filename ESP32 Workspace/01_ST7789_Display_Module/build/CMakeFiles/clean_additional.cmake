# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "")
  file(REMOVE_RECURSE
  "01_ST7789_Display_Module.bin"
  "01_ST7789_Display_Module.map"
  "bootloader\\bootloader.bin"
  "bootloader\\bootloader.elf"
  "bootloader\\bootloader.map"
  "config\\sdkconfig.cmake"
  "config\\sdkconfig.h"
  "esp-idf\\esptool_py\\flasher_args.json.in"
  "esp-idf\\mbedtls\\x509_crt_bundle"
  "flash_app_args"
  "flash_bootloader_args"
  "flash_project_args"
  "flasher_args.json"
  "hot_wind.bin.S"
  "humidity.bin.S"
  "korea.bin.S"
  "ldgen_libraries"
  "ldgen_libraries.in"
  "project_elf_src_esp32c6.c"
  "rain.bin.S"
  "snow.bin.S"
  "sun.bin.S"
  "temp.bin.S"
  "w_hot.bin.S"
  "w_rain.bin.S"
  "w_snow.bin.S"
  "w_wind.bin.S"
  "x509_crt_bundle.S"
  )
endif()
