cmake_minimum_required(VERSION 2.8)
SET(CMAKE_TOOLCHAIN_FILE "ToolChain.cmake")

#项目名称
PROJECT(ez_iot)
SET(USE_STATIC_LIB_LINKAGE ON)

ADD_DEFINITIONS("-Wall")
ADD_DEFINITIONS("-DEZ_IOT_SDK")

if(DEFINED SDK_ADD_PRIV_PREMACRO)
	ADD_DEFINITIONS("${SDK_ADD_PRIV_PREMACRO}")
endif()

if(DEFINED SDK_ADD_PRIV_CFLAGS)
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${SDK_ADD_PRIV_CFLAGS} -Wno-pointer-sign")
else()
	SET(CMAKE_C_FLAGS "-O2 -g  -Wall -std=gnu99 -Wno-pointer-sign")
endif()	

if(DEFINED SDK_ADD_PRIV_CXXFLAGS)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${SDK_ADD_PRIV_CXXFLAGS} -Wno-pointer-sign")
else()
	SET(CMAKE_CXX_FLAGS "-O2 -g  -Wall -std=gnu99 -Wno-pointer-sign")
endif()	

if ("freertos" MATCHES ${SDK_BUILD_OS})
    if("esp32" MATCHES ${SDK_BUILD_PORT})
	    ADD_DEFINITIONS("-D_FREE_RTOS_32_")
    else("esp8266" MATCHES ${SDK_BUILD_PORT})
	    ADD_DEFINITIONS("-D_FREE_RTOS_")
	endif()
else()
    #linux
endif()

if(DEFINED SDK_CONPONENT_LINK_SUPPORT)
	MESSAGE(STATUS "SDK_CONPONENT_LINK_SUPPORT ENABLE ")
	AUX_SOURCE_DIRECTORY(./ ez_iot_sdk )
	AUX_SOURCE_DIRECTORY(./ez_utils/ ezutils)
	AUX_SOURCE_DIRECTORY(./ez_utils/ezlist ezlist)
	AUX_SOURCE_DIRECTORY(./ez_utils/ezkv ezkv)
	AUX_SOURCE_DIRECTORY(./ez_utils/base64 base64)
	AUX_SOURCE_DIRECTORY(./ez_utils/uuid uuid)
	AUX_SOURCE_DIRECTORY(./ez_utils/timer timer)
	AUX_SOURCE_DIRECTORY(./ez_hal/${SDK_BUILD_OS}/ ez_iot_hal)
endif()

if(DEFINED SDK_CONPONENT_LOGGER_SUPPORT)
	MESSAGE(STATUS "SDK_CONPONENT_LOGGER_SUPPORT ENABLE ")
    AUX_SOURCE_DIRECTORY(./component/logger/ ez_iot_log)
	AUX_SOURCE_DIRECTORY(./component/logger/src ez_iot_log_src)
	AUX_SOURCE_DIRECTORY(./component/logger/wrappers/${SDK_BUILD_OS} ez_iot_log_wrapper)
endif()

if(DEFINED SDK_CONPONENT_AP_SUPPORT)
	MESSAGE(STATUS "SDK_CONPONENT_AP_SUPPORT ENABLE ")
	AUX_SOURCE_DIRECTORY(./component/http_server/src http_server)
	AUX_SOURCE_DIRECTORY(./component/http_server/depends/http_parser http_parser)
	AUX_SOURCE_DIRECTORY(./component/ap ez_iot_ap)
	AUX_SOURCE_DIRECTORY(./component/ap/wrappers/${SDK_BUILD_OS} ez_iot_ap_wrappers)
endif()

if(DEFINED SDK_CONPONENT_TSL_SUPPORT)
	MESSAGE(STATUS "SDK_CONPONENT_TSL_SUPPORT ENABLE ")
	AUX_SOURCE_DIRECTORY(./component/tsl ez_iot_tsl)
	# AUX_SOURCE_DIRECTORY(./component/connect ez_iot_conn)
	AUX_SOURCE_DIRECTORY(./component/shadow ez_iot_shadow)
	AUX_SOURCE_DIRECTORY(./component/http http)
endif()

if(DEFINED SDK_CONPONENT_HUB_SUPPORT)
	MESSAGE(STATUS "SDK_CONPONENT_HUB_SUPPORT ENABLE ")
	AUX_SOURCE_DIRECTORY(./component/hub ez_iot_hub)
endif()

if(DEFINED SDK_CONPONENT_OTA_SUPPORT)
	MESSAGE(STATUS "SDK_CONPONENT_OTA_SUPPORT ENABLE ")
	AUX_SOURCE_DIRECTORY(./component/ota ez_iot_ota)
	AUX_SOURCE_DIRECTORY(./component/http http)
endif()

if(DEFINED SDK_CONPONENT_TIME_SUPPORT)
	MESSAGE(STATUS "SDK_CONPONENT_TIME_SUPPORT ENABLE ")
	AUX_SOURCE_DIRECTORY(./component/time/${SDK_BUILD_OS} ez_iot_time_wrappers)
endif()

if(DEFINED SDK_CONPONENT_CONNECT_SUPPORT)
	MESSAGE(STATUS "SDK_CONPONENT_CONNECT_SUPPORT ENABLE ")
	AUX_SOURCE_DIRECTORY(./component/connect ez_iot_conn)
endif()

INCLUDE_DIRECTORIES(
	${PROJECT_SOURCE_DIR}/../../../build_conf/${SDK_BUILD_PORT}
	${PROJECT_SOURCE_DIR}
	${PROJECT_SOURCE_DIR}/../inc
	${PROJECT_SOURCE_DIR}/../inc/ez_hal
	${PROJECT_SOURCE_DIR}/../inc/hal_drv
	${PROJECT_SOURCE_DIR}/../../microkernel/inc
	${PROJECT_SOURCE_DIR}/../../microkernel/src
	${PROJECT_SOURCE_DIR}/component/ap
	${PROJECT_SOURCE_DIR}/component/time
	${PROJECT_SOURCE_DIR}/component/logger/inc
	${PROJECT_SOURCE_DIR}/component/logger/wrappers/${SDK_BUILD_OS}
	${PROJECT_SOURCE_DIR}/component/shadow
	${PROJECT_SOURCE_DIR}/component/tsl
	${PROJECT_SOURCE_DIR}/../inc/ez_utils/http_server

	${PROJECT_SOURCE_DIR}/component/connect
	${PROJECT_SOURCE_DIR}/../inc/ez_utils/http
	${PROJECT_SOURCE_DIR}/ez_utils
	${PROJECT_SOURCE_DIR}/../inc/ez_utils
	${PROJECT_SOURCE_DIR}/../../
	${SDK_ADD_PRIV_INCLUDEDIRS}
)

MESSAGE(STATUS "c compile option "${CMAKE_C_FLAGS})
MESSAGE(STATUS "c++ compile option "${CMAKE_CXX_FLAGS})

#同时生成静态库和动态库，target的名字要唯一，但是生成的库名可以重定义
ADD_LIBRARY(ez_iot_sdk_STATIC STATIC 
			${ez_iot_sdk} 
			${ez_iot_hal} 
			${ez_iot_utils} 
			${ezutils} 
			${ezlist}
			${ezkv}
			${base64} 
			${uuid}
            ${timer}
			${ez_iot_log} 
			${ez_iot_log_src} 
			${ez_iot_log_wrapper} 
			${ez_iot_ap} 
			${ez_iot_ap_wrappers} 
			${ez_iot_time_wrappers} 
			${ez_iot_shadow} 
			${ez_iot_hub} 
			${ez_iot_tsl}
			${ez_iot_conn}
			${http} 
            ${http_server}
            ${http_parser}
			${ez_iot_ota})

SET_TARGET_PROPERTIES(ez_iot_sdk_STATIC PROPERTIES OUTPUT_NAME ez_iot)

SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/../lib/${SDK_BUILD_PORT})
