#cmake版本要求
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(CMAKE_C_COMPILER, "gcc")
SET(CMAKE_CXX_COMPILER, "g++")

#项目名称
PROJECT(DEV_DEMO_OTA)
SET(USE_STATIC_LIB_LINKAGE ON)
SET(CMAKE_BUILD_PORT "linux")

#添加编译选项
ADD_DEFINITIONS("-Wall")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -Wno-unused-variable")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wno-unused-variable  -std=c++11")
else()
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-variable")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wno-unused-variable")
endif(CMAKE_BUILD_TYPE STREQUAL "Debug")

SET(PROJECT_INC_DIR ${PROJECT_SOURCE_DIR}/../../../inc)
SET(PROJECT_SRC_DIR ${PROJECT_SOURCE_DIR})
SET(PROJECT_LIB_DIR ${PROJECT_SOURCE_DIR}/../../../lib/${CMAKE_BUILD_PORT})

##############################################根据开关定义编译的文件和链接的库#############################################

AUX_SOURCE_DIRECTORY(${PROJECT_SRC_DIR} src_ota)
AUX_SOURCE_DIRECTORY(${PROJECT_SRC_DIR}/storage src_kv)
AUX_SOURCE_DIRECTORY(${PROJECT_INC_DIR}/components/FlashDB/src src_flashdb)
message(STATUS ${PROJECT_INC_DIR})
##############################################根据开关定义编译的文件和链接的库#############################################

#########################################################添加依赖库######################################################
SET(lib_rt -lpthread -lm -lrt)
SET(lib_sdk -lez_iot -lez_linkcore)
#########################################################添加依赖库######################################################

SET(src_files
    ${src_flashdb}
    ${src_ota}
    ${src_kv}
    )

#头文件搜索路径
INCLUDE_DIRECTORIES(
                    ${PROJECT_INC_DIR}
                    ${PROJECT_INC_DIR}/components/FlashDB/inc
                    ${PROJECT_SRC_DIR}/storage
                    )

#添加依赖库目录
LINK_DIRECTORIES(
                ${PROJECT_LIB_DIR}
                )

#添加需要编译的源文件
ADD_EXECUTABLE(demo_ota ${src_files})

#添加链接库
target_link_libraries(demo_ota
                        ${lib_sdk}
                        ${lib_rt}
                        )