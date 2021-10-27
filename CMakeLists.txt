# Copyright (c) 2021 ARM Limited. All rights reserved.
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.19.0)

set(MBED_PATH ../../mbed-os CACHE INTERNAL "")
set(MBED_CONFIG_PATH ${CMAKE_CURRENT_BINARY_DIR} CACHE INTERNAL "")
set(APP_TARGET testSPI)

include(${MBED_PATH}/tools/cmake/app.cmake)

project(${APP_TARGET})

message("${CMAKE_CURRENT_BINARY_DIR}")

add_subdirectory(${MBED_PATH} "mbed-os")
add_subdirectory("../../custom_targets" "custom_targets")

add_executable(${APP_TARGET}
    main.cpp
)

target_link_libraries(${APP_TARGET} 
    mbed-os
)

mbed_set_post_build(${APP_TARGET})

option(VERBOSE_BUILD "Have a verbose build process")
if(VERBOSE_BUILD)
    set(CMAKE_VERBOSE_MAKEFILE ON)
endif()