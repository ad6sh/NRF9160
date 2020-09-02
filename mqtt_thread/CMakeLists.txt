
cmake_minimum_required(VERSION 3.8.2)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(mqtt_thread)

# NORDIC SDK APP START
target_sources(app PRIVATE
src/mqtt_helpers.c
src/main.c
src/mqtt_thread.h
src/coAP_helpers.c
src/coAP_thread.h
src/thread_ipc.h
)

# NORDIC SDK APP END
