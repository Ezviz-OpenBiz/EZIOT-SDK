#配置测试的领域模块
UNITTEST_KV_ENABLE := ON
# UNITTEST_ONLINE_ENABLE := ON
# UNITTEST_OTA_ENABLE := ON
# UNITTEST_GATWAY_ENABLE := ON
# UNITTEST_TSL_ENABLE := ON

#配置编译选项,"Debug" || "Release"
CMAKE_BUILD_TYPE := Debug
CMAKE_BUILD_PORT := esp8266

PROJECT_INC_DIR := ../../../../../inc
PROJECT_LIB_DIR := $(COMPONENT_PATH)/../../../../../lib/esp8266
PROJECT_LIBS := -lez_iot -lez_linkcore -lmain
PROJECT_CFLAGS := -DNETBSD -D_LARGEFILE_SOURCE -D_LARGE_FILES -DFTP_COMBINE_CWDS -DUTEST_TC_USE_CONSTRUCTOR -DUTEST_CASE_CONSTRUCTOR_EXPORT=102 -DUTEST_CASE_CONSTRUCTOR_INIT=103 -Wno-pointer-sign

##############################################根据开关定义编译的文件和链接的库#############################################

src_ut_utest := ../../../../../inc/components/utilities/utest
src_ut_port := ..
src_ut_all := ../../..

ifdef UNITTEST_KV_ENABLE
    $(warning UNITTEST_KV_ENABLE)
    src_ut_flashdb := ${PROJECT_INC_DIR}/components/FlashDB/src ${PROJECT_INC_DIR}/components/FlashDB/port/fal/src
    PROJECT_CFLAGS += -DUNITTEST_KV_ENABLE
else
    src_ut_kvdb_exclude := ut_kvdb.c
endif

ifdef UNITTEST_ONLINE_ENABLE
    $(warning UNITTEST_ONLINE_ENABLE)
    PROJECT_CFLAGS += -DUNITTEST_ONLINE_ENABLE
else
    src_ut_online_exclude := ut_online.c
endif

ifdef UNITTEST_TSL_ENABLE
    $(warning UNITTEST_TSL_ENABLE)
    PROJECT_CFLAGS += -UNITTEST_TSL_ENABLE
else
    src_ut_tsl_exclude := ut_tsl.c
endif

ifdef UNITTEST_GATEWAY_ENABLE
    $(warning DUNITTEST_GATWAY_ENABLE)
    PROJECT_CFLAGS += -DUNITTEST_GATEWAY_ENABLE
else
    src_ut_getway_exclude := ut_gateway.c
endif

ifdef UNITTEST_OTA_ENABLE
    $(warning UNITTEST_OTA_ENABLE)
    PROJECT_CFLAGS += -UNITTEST_OTA_ENABLE
else
    src_ut_ota_exclude := ut_ota.c
endif

##############################################根据开关定义编译的文件和链接的库#############################################

#原文件搜索路径

COMPONENT_SRCDIRS += ${src_ut_port}
COMPONENT_SRCDIRS += ${src_ut_all}
COMPONENT_SRCDIRS += ${src_ut_flashdb}
COMPONENT_SRCDIRS += ${src_ut_utest}

# COMPONENT_SRCEXCLUDE += ${src_ut_ota_exclude}
# COMPONENT_SRCEXCLUDE += ${src_ut_getway_exclude}
# COMPONENT_SRCEXCLUDE += ${src_ut_tsl_exclude}
# COMPONENT_SRCEXCLUDE += ${src_ut_online_exclude}
# COMPONENT_SRCEXCLUDE += ${src_ut_kvdb_exclude}

#头文件搜索路径
COMPONENT_PRIV_INCLUDEDIRS += ${PROJECT_INC_DIR}
COMPONENT_PRIV_INCLUDEDIRS += ${PROJECT_INC_DIR}/components/utilities/utest
COMPONENT_PRIV_INCLUDEDIRS += ${PROJECT_INC_DIR}/components/FlashDB/inc
COMPONENT_PRIV_INCLUDEDIRS += ${PROJECT_INC_DIR}/components/FlashDB/port/fal/inc
COMPONENT_PRIV_INCLUDEDIRS += ../
COMPONENT_PRIV_INCLUDEDIRS += ../../../
COMPONENT_PRIV_INCLUDEDIRS += ../../../com

#添加依赖库目录
COMPONENT_ADD_LDFLAGS = -Wl,--whole-archive,--start-group -L ${PROJECT_LIB_DIR} ${PROJECT_LIBS} -mlongcalls -Wl,--end-group,-no-whole-archive
CFLAGS += ${PROJECT_CFLAGS}

$(warning 00000000000000000000 ${PROJECT_LIB_DIR})
$(warning 11111111111111111111 ${COMPONENT_SRCDIRS})
$(warning 22222222222222222222 ${COMPONENT_SRCEXCLUDE})
# $(warning 33333333333333333333 ${COMPONENT_PRIV_INCLUDEDIRS})
$(warning 44444444444444444444 ${COMPONENT_ADD_LDFLAGS})
$(warning 55555555555555555555 ${LDFLAGS})