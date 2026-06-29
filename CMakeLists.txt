# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (c) 2025 sevenlab engineering GmbH
#
cmake_minimum_required(VERSION 3.17)

set(LOG 1)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(PICO_SDK_PATH ${CMAKE_CURRENT_LIST_DIR}/pico-sdk)
set(DEBUGPROBE_PATH ${CMAKE_CURRENT_LIST_DIR}/debugprobe)
set(FREERTOS_KERNEL_PATH ${DEBUGPROBE_PATH}/freertos)

include(${DEBUGPROBE_PATH}/pico_sdk_import.cmake)
include(${DEBUGPROBE_PATH}/FreeRTOS_Kernel_import.cmake)

project(picoports C CXX ASM)

pico_sdk_init()

set(DEBUG_ON_PICO ON CACHE BOOL "Always compiling for Pico" FORCE)

include(${CMAKE_CURRENT_LIST_DIR}/debugprobe.cmake)

add_executable(picoports)

target_sources(picoports PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src/main.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/pp_adc.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/pp_ctrl.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/pp_gpio.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/pp_i2c.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/usb_descriptors.c
)

target_include_directories(picoports PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  # Required for FreeRTOSConfig.h
  ${DEBUGPROBE_PATH}/src/
)

target_link_libraries(picoports PUBLIC
  pico_bootrom
  pico_stdlib
  hardware_adc
  hardware_i2c
  tinyusb_device
  tinyusb_board
  FreeRTOS-Kernel
  FreeRTOS-Kernel-Heap1
  debugprobe
)

option(LOG_ON_GP01 "Enable debug logging on GP0/GP1 (TX/RX resp.)")
if(LOG_ON_GP01)
target_compile_definitions(picoports PUBLIC PP_LOG_ON_GP01=1)
else()
# UART stdio is enabled by default, disabling it
pico_enable_stdio_uart(picoports 0)
endif()

option(BOOTSEL_BUTTON "Pressing the button resets the pico into BOOTSEL mode")
if(BOOTSEL_BUTTON)
target_compile_definitions(picoports PUBLIC PP_BTN_BOOTSEL=1)
endif()

# Create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(picoports)
