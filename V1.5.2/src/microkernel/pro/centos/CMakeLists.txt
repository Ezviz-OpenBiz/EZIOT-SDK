cmake_minimum_required(VERSION 2.8)
PROJECT(microkernel)

ADD_DEFINITIONS(-W -Wall -D_REENTRANT -D_FILE_OFFSET_BITS=64)
SET(CMAKE_C_COMPILER "gcc")
SET(CMAKE_C_FLAGS "-g -std=c99 -O2 -Wall")
SET(LIBRARY_OUTPUT_PATH ../../lib/centos)

AUX_SOURCE_DIRECTORY(../../src src)
AUX_SOURCE_DIRECTORY(../../src/mbedtls src_mbedtls)
AUX_SOURCE_DIRECTORY(../../paho.mqtt.embedded mqtt)
AUX_SOURCE_DIRECTORY(../../paho.mqtt.embedded/src mqtt_src)

INCLUDE_DIRECTORIES(
		../../src
		../../src/mbedtls
		../../paho.mqtt.embedded
		../../paho.mqtt.embedded/src
)


ADD_LIBRARY(microkernel SHARED ${src} ${src_mbedtls} ${mqtt} ${mqtt_src})
TARGET_LINK_LIBRARIES(microkernel)
SET_TARGET_PROPERTIES(microkernel PROPERTIES VERSION 2.0.0.${SVN_VERSION} SOVERSION 1 LINKER_LANGUAGE C)

