cmake_minimum_required(VERSION 3.20.0)

if(BOARD MATCHES "qemu_.*")
  list(APPEND QEMU_EXTRA_FLAGS -serial file:channel0_0)
endif()

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(rt_zephyr)

target_sources(app PRIVATE src/rt_zephyr.c)
