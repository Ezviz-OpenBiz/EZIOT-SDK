cmake_minimum_required(VERSION 2.8)

SET(CMAKE_TOOLCHAIN_FILE "ToolChain.cmake")

PROJECT(microkernel)
SET(USE_STATIC_LIB_LINKAGE ON)

#添加编译选项
ADD_DEFINITIONS("-Wall")
if(DEFINED SDK_ADD_PRIV_PREMACRO)
	ADD_DEFINITIONS("${SDK_ADD_PRIV_PREMACRO}")
endif()

if(DEFINED SDK_ADD_PRIV_CFLAGS)
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${SDK_ADD_PRIV_CFLAGS} -Wno-pointer-sign")
else()
	SET(CMAKE_C_FLAGS "-O2 -g  -Wall -std=gnu99 -Wno-pointer-sign")
endif()	

if ("freertos" MATCHES ${SDK_BUILD_OS})
    ADD_DEFINITIONS("-D_FREE_RTOS_")
elseif ("linux" MATCHES ${SDK_BUILD_OS})
	#linux
else()
	ADD_DEFINITIONS("-D_GENERIC_OS_")
endif()

AUX_SOURCE_DIRECTORY(./ src)
AUX_SOURCE_DIRECTORY(./mbedtls src_mbedtls)
AUX_SOURCE_DIRECTORY(../paho.mqtt.embedded mqtt)
AUX_SOURCE_DIRECTORY(../paho.mqtt.embedded/src mqtt_src)


INCLUDE_DIRECTORIES(
		./
		../inc
		./mbedtls
		../paho.mqtt.embedded
		../paho.mqtt.embedded/src
		${PROJECT_SOURCE_DIR}/../../../build_conf/${SDK_BUILD_PORT}
)

MESSAGE(STATUS "c compile option: ${CMAKE_C_FLAGS}")
MESSAGE(STATUS "c++ compile option: ${CMAKE_CXX_FLAGS}")
MESSAGE(STATUS "c toolchain "${TOOCHAIN_C_COMPILER})
MESSAGE(STATUS "c++ toolchain "${TOOCHAIN_CXX_COMPILER})
ADD_LIBRARY(microkernel_STATIC STATIC ${src} ${src_mbedtls} ${mqtt} ${mqtt_src})
SET_TARGET_PROPERTIES(microkernel_STATIC PROPERTIES OUTPUT_NAME ez_linkcore)

IF (${DYNAMIC})
	ADD_LIBRARY(microkernel_SHARED SHARED ${src} ${src_mbedtls} ${mqtt} ${mqtt_src})
	SET_TARGET_PROPERTIES(microkernel_SHARED PROPERTIES OUTPUT_NAME ez_linkcore)
ENDIF (${DYNAMIC})

SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/../lib/${SDK_BUILD_PORT})
