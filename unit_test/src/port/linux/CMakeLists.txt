#cmake版本要求
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(CMAKE_TOOLCHAIN_FILE "ToolChain.cmake")
SET(CMAKE_C_COMPILER, TOOCHAIN_C_COMPILER)
SET(CMAKE_CXX_COMPILER, TOOCHAIN_CXX_COMPILER)

#项目名称
PROJECT(DEV_IOTSDK_UNITTEST)
SET(USE_STATIC_LIB_LINKAGE ON)

ADD_DEFINITIONS("-D_PORT_=linux")

#添加编译选项
ADD_DEFINITIONS("-Wall")
ADD_DEFINITIONS("-DNETBSD -D_LARGEFILE_SOURCE -D_LARGE_FILES -DFTP_COMBINE_CWDS -DUTEST_TC_USE_CONSTRUCTOR")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -Wno-unused-variable -Wno-pointer-sign")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wno-unused-variable  -std=c++11 -Wno-pointer-sign")
else()
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-variable -Wno-pointer-sign")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wno-unused-variable -Wno-pointer-sign")
endif(CMAKE_BUILD_TYPE STREQUAL "Debug")

SET(PROJECT_INC_DIR ${PROJECT_SOURCE_DIR}/../../../../inc)
SET(PROJECT_SRC_DIR ${PROJECT_SOURCE_DIR}/../..)
SET(PROJECT_LIB_DIR ${PROJECT_SOURCE_DIR}/../../../../lib/${CMAKE_BUILD_PORT})

SET(PROJECT_HTTP_SRC_DIR ${PROJECT_SOURCE_DIR}/../../../../inc/ez_utils/http)

##############################################根据开关定义编译的文件和链接的库#############################################

SET(src_ut_entry  ${PROJECT_SRC_DIR}/port/${CMAKE_BUILD_PORT}/ut_entry.c)

AUX_SOURCE_DIRECTORY(${PROJECT_INC_DIR}/components/utilities/utest src_ut_utest)
AUX_SOURCE_DIRECTORY(${PROJECT_SRC_DIR}/port/${CMAKE_BUILD_PORT} src_ut_port)

if(UNITTEST_KV_ENABLE)
    message(STATUS "UNITTEST_KV_ENABLE")
    AUX_SOURCE_DIRECTORY(${PROJECT_INC_DIR}/components/FlashDB/src src_ut_flashdb)
    AUX_SOURCE_DIRECTORY(${PROJECT_SRC_DIR}/ut_kvdb src_ut_kvdb)
    ADD_DEFINITIONS("-DUNITTEST_KV_ENABLE")
endif()

if(UNITTEST_ONLINE_ENABLE)
    message(STATUS "UNITTEST_ONLINE_ENABLE")
    AUX_SOURCE_DIRECTORY(${PROJECT_SRC_DIR}/ut_online src_ut_online)
    SET(lib_sdk -lez_iot -lez_linkcore)
    ADD_DEFINITIONS("-DUNITTEST_ONLINE_ENABLE")
endif()

if(UNITTEST_TSL_ENABLE)
    message(STATUS "UNITTEST_TSL_ENABLE")
    AUX_SOURCE_DIRECTORY(${PROJECT_SRC_DIR}/ut_tsl src_ut_tsl)
    SET(lib_sdk -lez_iot -lez_linkcore)
    ADD_DEFINITIONS("-UNITTEST_TSL_ENABLE")
endif()

if(UNITTEST_GATEWAY_ENABLE)
    message(STATUS "UNITTEST_GATEWAY_ENABLE")
    AUX_SOURCE_DIRECTORY(${PROJECT_SRC_DIR}/ut_gateway src_ut_gateway)
    SET(lib_sdk -lez_iot -lez_linkcore)
    ADD_DEFINITIONS("-DUNITTEST_GATEWAY_ENABLE")
endif()

if(UNITTEST_OTA_ENABLE)
    message(STATUS "UNITTEST_OTA_ENABLE")
    AUX_SOURCE_DIRECTORY(${PROJECT_SRC_DIR}/ut_ota src_ut_ota)
    SET(lib_sdk -lez_iot -lez_linkcore)
    ADD_DEFINITIONS("-DUNITTEST_OTA_ENABLE")
endif()

if(UNITTEST_LOGGER_ENABLE)
    message(STATUS "UNITTEST_LOGGER_ENABLE")
    AUX_SOURCE_DIRECTORY(${PROJECT_SRC_DIR}/ut_logger src_ut_logger)
    SET(lib_sdk -lez_iot -lez_linkcore)
    ADD_DEFINITIONS("-DUNITTEST_LOGGER_ENABLE")
endif()

if(UNITTEST_BIND_ENABLE)
    message(STATUS "UNITTEST_BIND_ENABLE")
    AUX_SOURCE_DIRECTORY(${PROJECT_SRC_DIR}/ut_bind src_ut_bind)
    SET(lib_sdk -lez_iot -lez_linkcore)
    ADD_DEFINITIONS("-DUNITTEST_BIND_ENABLE")
endif()

##############################################根据开关定义编译的文件和链接的库#############################################

#########################################################添加依赖库######################################################
SET(lib_rt -lpthread -lm -lrt)
#########################################################添加依赖库######################################################

SET(src_files
    ${src_ut_port}
    ${src_ut_kvdb}
    ${src_ut_online}
    ${src_ut_ota}
    ${src_ut_tsl}
    ${src_ut_gateway}
    ${src_ut_flashdb}
    ${src_ut_utest}
    ${src_ut_entry}
    ${src_ut_logger}
    ${src_ut_bind}
    )

#头文件搜索路径
INCLUDE_DIRECTORIES(
                    ${PROJECT_INC_DIR}
                    ${PROJECT_INC_DIR}/components/utilities/utest
                    ${PROJECT_INC_DIR}/components/FlashDB/inc
                    ${PROJECT_SRC_DIR}
                    ${PROJECT_SRC_DIR}/com
                    ${PROJECT_SRC_DIR}/port/${CMAKE_BUILD_PORT}
                    ${PROJECT_HTTP_SRC_DIR}
                    )

#添加依赖库目录
LINK_DIRECTORIES(
                ${PROJECT_LIB_DIR}
                )

#添加需要编译的源文件
ADD_EXECUTABLE(ut_iotsdk ${src_files})

#添加链接库
target_link_libraries(ut_iotsdk
                        ${lib_sdk}
                        ${lib_rt}
                        )
