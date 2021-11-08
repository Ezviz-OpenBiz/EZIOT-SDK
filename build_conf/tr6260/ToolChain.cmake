SET(CMAKE_SYSTEM_NAME Generic)
SET(CMAKE_C_COMPILER "nds32le-elf-gcc")
SET(CMAKE_CXX_COMPILER "nds32le-elf-g++")
SET(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

SET(SDK_BUILD_PORT "tr6260")            #芯片型号
SET(SDK_BUILD_OS "tr6260")              #操作系统，linux/freertos

SET(SDK_CONPONENT_LINK_SUPPORT ON)      #基础连接
# SET(SDK_CONPONENT_AP_SUPPORT ON)        #AP配网
SET(SDK_CONPONENT_TSL_SUPPORT ON)       #物模型
# SET(SDK_CONPONENT_CONNECT_SUPPORT ON)   #局域网互联互通
# SET(SDK_CONPONENT_HUB_SUPPORT ON)       #子设备管理
SET(SDK_CONPONENT_OTA_SUPPORT ON)       #升级
# SET(SDK_CONPONENT_TIME_SUPPORT ON)      #校时

SET(SDK_ADD_PRIV_INCLUDEDIRS
    /opt/tr6260/platform/lwip/lwip-2.1.0/src/include
    /opt/tr6260/platform/lwip/contrib/port
    /opt/tr6260/include
    /opt/tr6260/include/FreeRTOS
    /opt/tr6260/include/drivers
    /opt/tr6260/include/wifi
    /opt/tr6260/include/drivers/bsp
    /opt/tr6260/nv/inc
)

#设置私有编译宏
SET(SDK_ADD_PRIV_PREMACRO "-DLWIP_SO_SNDRCVTIMEO_NONSTANDARD")

#设置私有编译参数
SET(SDK_ADD_PRIV_CFLAGS "-x none -Wall -Wno-unused-value -Wno-unused-variable -Wno-unused-but-set-variable -Wno-misleading-indentation \
-Wno-memset-elt-size -Wno-maybe-uninitialized -Wno-unused-function -Wno-format-overflow -Wno-format-truncation -Wno-bool-compare \
-fno-omit-frame-pointer -fno-common -fno-exceptions -gdwarf-2 -ffunction-sections -fdata-sections \
-Os -mno-ex9 -mext-zol -mext-dsp -DCONFIG_HWZOL -D__TARGET_IFC_EXT -D__TARGET_ZOL_EXT \
-mcmodel=large --specs=nosys.specs -std=gnu99")

SET(SDK_ADD_PRIV_CXXFLAGS "-x none -Wall -Wno-unused-value -Wno-unused-variable -Wno-unused-but-set-variable -Wno-misleading-indentation \
-Wno-memset-elt-size -Wno-maybe-uninitialized -Wno-unused-function -Wno-format-overflow -Wno-format-truncation -Wno-bool-compare \
-fno-omit-frame-pointer -fno-common -fno-exceptions -gdwarf-2 -ffunction-sections -fdata-sections \
-Os -mno-ex9 -mext-zol -mext-dsp -DCONFIG_HWZOL -D__TARGET_IFC_EXT -D__TARGET_ZOL_EXT \
-mcmodel=large --specs=nosys.specs -std=gnu99")