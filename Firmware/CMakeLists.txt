cmake_minimum_required(VERSION 3.13)

# Enable compile_commands.json for IDE IntelliSense
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# initialize pico-sdk from GIT
# (note this can come from environment, CMake cache etc)
set(PICO_SDK_FETCH_FROM_GIT on)

# pico_sdk_import.cmake is a single file copied from this SDK
# note: this must happen before project()
include(pico_sdk_import.cmake)

project(my_project)

# Enable USB stdio for serial communication
set(PICO_STDIO_USB 1)
set(PICO_STDIO_UART 0)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# ---------------------------
# USB descriptor overrides
# ---------------------------
#
# Pico SDK's default CDC descriptors use:
#   USBD_MANUFACTURER (default "Raspberry Pi")
#   USBD_PRODUCT      (default "Pico")
#
# These are compile-time macros, so we expose cache variables you can override at
# configure time:
#   cmake -S . -B build -DTCODE_USBD_PRODUCT="Thermocline Controller"
#
set(TCODE_USBD_MANUFACTURER "Team Thermocline" CACHE STRING "USB manufacturer string")
set(TCODE_USBD_PRODUCT "Chamber Controller" CACHE STRING "USB product string")

# -----------------------
# Build info (git version)
# -----------------------

get_filename_component(TCODE_REPO_ROOT "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
set(TCODE_BUILD_INFO_IN "${CMAKE_CURRENT_LIST_DIR}/cmake/tcode_build_info.h.in")
set(TCODE_BUILD_INFO_OUT "${CMAKE_CURRENT_BINARY_DIR}/generated/tcode_build_info.h")

# -----------------
# FreeRTOS (kernel)
# -----------------

set(FREERTOS_KERNEL_PATH ${CMAKE_CURRENT_LIST_DIR}/lib/FreeRTOS-Kernel)

add_library(freertos_kernel STATIC
        ${FREERTOS_KERNEL_PATH}/event_groups.c
        ${FREERTOS_KERNEL_PATH}/list.c
        ${FREERTOS_KERNEL_PATH}/queue.c
        ${FREERTOS_KERNEL_PATH}/stream_buffer.c
        ${FREERTOS_KERNEL_PATH}/tasks.c
        ${FREERTOS_KERNEL_PATH}/timers.c
        ${FREERTOS_KERNEL_PATH}/portable/GCC/ARM_CM0/port.c
        ${FREERTOS_KERNEL_PATH}/portable/GCC/ARM_CM0/portasm.c
        ${FREERTOS_KERNEL_PATH}/portable/MemMang/heap_4.c
)

target_include_directories(freertos_kernel PUBLIC
        ${FREERTOS_KERNEL_PATH}/include
        ${FREERTOS_KERNEL_PATH}/portable/GCC/ARM_CM0
        ${CMAKE_CURRENT_LIST_DIR}/include  # FreeRTOSConfig.h
)

# Everything else
add_library(pico_adg728 STATIC lib/pico_adg728/ADG728.c)
target_include_directories(pico_adg728 PUBLIC ${CMAKE_CURRENT_LIST_DIR}/lib/pico_adg728)
target_link_libraries(pico_adg728 hardware_i2c)

add_executable(thermocline_controller
        main.c
        lib/freertos_support.c
        lib/neopixel_ws2812/neopixel_ws2812.c
        lib/tcode_protocol/tcode_protocol.c
        tasks/sim_thermo_system_task.c
        tasks/serial_task.c
        tasks/status_led_task.c
)

# Override TinyUSB default descriptor strings (pico_stdio_usb default descriptors).
# Must be string literals; CMake will quote/escape as needed.
target_compile_definitions(thermocline_controller PRIVATE
        USBD_MANUFACTURER="${TCODE_USBD_MANUFACTURER}"
        USBD_PRODUCT="${TCODE_USBD_PRODUCT}"
)

# Generate tcode_build_info.h
add_custom_target(tcode_build_info_h
        BYPRODUCTS "${TCODE_BUILD_INFO_OUT}"
        COMMAND "${CMAKE_COMMAND}"
                -DTCODE_REPO_ROOT=${TCODE_REPO_ROOT}
                -DTCODE_BUILD_INFO_IN=${TCODE_BUILD_INFO_IN}
                -DTCODE_BUILD_INFO_OUT=${TCODE_BUILD_INFO_OUT}
                -P "${CMAKE_CURRENT_LIST_DIR}/cmake/gen_tcode_build_info.cmake"
        VERBATIM
)
add_dependencies(thermocline_controller tcode_build_info_h)

# Generate PIO header(s)
pico_generate_pio_header(thermocline_controller ${CMAKE_CURRENT_LIST_DIR}/lib/neopixel_ws2812/ws2812.pio)

# Project headers
target_include_directories(thermocline_controller PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/include
        ${CMAKE_CURRENT_BINARY_DIR}/generated
        ${CMAKE_CURRENT_LIST_DIR}/lib/pico_adg728
        ${CMAKE_CURRENT_LIST_DIR}/lib/neopixel_ws2812
        ${CMAKE_CURRENT_LIST_DIR}/lib/tcode_protocol
        ${CMAKE_CURRENT_LIST_DIR}/tasks
)

# Link pico_stdlib + PIO deps (NeoPixel)
target_link_libraries(thermocline_controller
        pico_stdlib
        hardware_pio
        hardware_clocks
        freertos_kernel
        pico_adg728
)

# create map/bin/hex/uf2 file
pico_add_extra_outputs(thermocline_controller)
