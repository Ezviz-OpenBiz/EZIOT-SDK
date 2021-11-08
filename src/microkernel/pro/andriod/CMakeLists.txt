cmake_minimum_required(VERSION 2.8)

SET(CMAKE_TOOLCHAIN_FILE "ToolChain.cmake")

PROJECT(microkernel)
SET(USE_STATIC_LIB_LINKAGE ON)

#添加编译选项
ADD_DEFINITIONS("-Wall")
ADD_DEFINITIONS("-DNETBSD -D_LARGEFILE_SOURCE -D_LARGE_FILES -DFTP_COMBINE_CWDS")

SET(CMAKE_C_FLAGS "-g -Wall")

SET(LIBRARY_OUTPUT_PATH ../../lib/andriod)

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


ADD_LIBRARY(microkernel STATIC ${src} ${src_mbedtls} ${mqtt} ${mqtt_src})
TARGET_LINK_LIBRARIES(microkernel)
SET_TARGET_PROPERTIES(microkernel PROPERTIES VERSION 2.0.1.${SVN_VERSION} SOVERSION 1 LINKER_LANGUAGE C)

