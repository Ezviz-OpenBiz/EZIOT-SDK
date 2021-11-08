cmake_minimum_required(VERSION 2.8)

SET(CMAKE_TOOLCHAIN_FILE "ToolChain.cmake")

PROJECT(microkernel)
SET(USE_STATIC_LIB_LINKAGE ON)

#添加编译选项
ADD_DEFINITIONS("-Wall")
ADD_DEFINITIONS("-DNETBSD -D_LARGEFILE_SOURCE -D_LARGE_FILES -DFTP_COMBINE_CWDS")

if(DEFINED PLATFORM_C_FLAGS)
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${PLATFORM_C_FLAGS}")
else()
	SET(CMAKE_C_FLAGS "-g -Wall")
endif()	

if(DEFINED PLATFORM_C_FLAGS)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${PLATFORM_CXX_FLAGS}")
else()
endif()

SET(LIBRARY_OUTPUT_PATH ../../lib/linux-store/${PLATFORM_NAME})

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

MESSAGE(STATUS "c compile option "${CMAKE_C_FLAGS})
MESSAGE(STATUS "c++ compile option "${CMAKE_CXX_FLAGS})

 MESSAGE(STATUS "output path: "${LIBRARY_OUTPUT_PATH})

ADD_LIBRARY(microkernel SHARED ${src} ${src_mbedtls} ${mqtt} ${mqtt_src})
TARGET_LINK_LIBRARIES(microkernel)
SET_TARGET_PROPERTIES(microkernel PROPERTIES VERSION ${LIB_VERSION}.${SVN_VERSION} SOVERSION 1 LINKER_LANGUAGE C)

