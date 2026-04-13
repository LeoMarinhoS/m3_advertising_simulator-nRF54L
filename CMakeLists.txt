# Copyright 2024 Croxel, Inc.
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(m3_advertising_simulator)

file(GLOB app_sources src/*.c)
target_sources(app PRIVATE ${app_sources})

file(GLOB app_headers include/*.h)
target_include_directories(app PRIVATE include)

