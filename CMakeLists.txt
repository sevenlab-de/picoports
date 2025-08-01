cmake_minimum_required(VERSION 3.17)

set(FAMILY "rp2040")
set(LOG 1)

include(${CMAKE_CURRENT_SOURCE_DIR}/pico-sdk/pico_sdk_init.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/pico-sdk/lib/tinyusb/hw/bsp/family_support.cmake)

project(picoports C CXX ASM)

family_initialize_project(picoports ${CMAKE_CURRENT_LIST_DIR})

add_executable(picoports)

target_sources(picoports PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src/main.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/pp_ctrl.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/pp_gpio.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/usb_descriptors.c
)

target_include_directories(picoports PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

family_configure_device_example(picoports noos)
