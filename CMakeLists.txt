cmake_minimum_required(VERSION 3.17)

set(FAMILY "rp2040")
set(LOG 1)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(${CMAKE_CURRENT_SOURCE_DIR}/pico-sdk/pico_sdk_init.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/pico-sdk/lib/tinyusb/hw/bsp/family_support.cmake)

project(picoports C CXX ASM)

family_initialize_project(picoports ${CMAKE_CURRENT_LIST_DIR})

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
)

target_link_libraries(picoports PUBLIC hardware_adc)

option(USE_ADC_GPIOS "Enable the ADC for GP26/GP27/GP28")
if(USE_ADC_GPIOS)
target_compile_definitions(picoports PUBLIC USE_ADC_GPIOS=1)
endif()

option(USE_I2C_GPIOS "Enable I2C on GP16/GP17")
if(USE_I2C_GPIOS)
target_link_libraries(picoports PUBLIC hardware_i2c)
target_compile_definitions(picoports PUBLIC USE_I2C_GPIOS=1)
endif()

family_configure_device_example(picoports noos)
