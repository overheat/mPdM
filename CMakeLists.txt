# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13.1)

set(BOARD_ROOT .)

include($ENV{ZEPHYR_BASE}/cmake/app/boilerplate.cmake NO_POLICY_SCOPE)
project(peripheral)

target_sources(app PRIVATE
  src/main.c
  src/ble.c
  src/gatt/hrs.c
  src/gatt/bas.c
  src/gatt/cts.c
)

target_sources(app PRIVATE src/main.c)
