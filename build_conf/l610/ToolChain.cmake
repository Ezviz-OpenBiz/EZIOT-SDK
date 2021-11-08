SET(CMAKE_SYSTEM_NAME Generic)
SET(CMAKE_C_COMPILER "arm-none-eabi-gcc")
SET(CMAKE_CXX_COMPILER "arm-none-eabi-g++")

SET(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

SET(SDK_BUILD_PORT "l610")            #芯片型号
SET(SDK_BUILD_OS "l610")           #操作系统，linux/freertos

SET(SDK_CONPONENT_LINK_SUPPORT ON)      #基础连接
# SET(SDK_CONPONENT_AP_SUPPORT ON)        #AP配网
SET(SDK_CONPONENT_TSL_SUPPORT ON)       #物模型
# SET(SDK_CONPONENT_CONNECT_SUPPORT ON)   #局域网互联互通
SET(SDK_CONPONENT_HUB_SUPPORT ON)       #子设备管理
SET(SDK_CONPONENT_OTA_SUPPORT ON)       #升级
# SET(SDK_CONPONENT_TIME_SUPPORT ON)      #校时

SET(SDK_ADD_PRIV_INCLUDEDIRS 
	$ENV{L610_SDK_PATH}/components/include
)

#设置私有编译宏
# SET(SDK_ADD_PRIV_PREMACRO "-DRT_USING_SAL")

#设置私有编译参数
# SET(SDK_ADD_PRIV_CFLAGS "")
# SET(SDK_ADD_PRIV_CXXFLAGS "-Wno-frame-address -ffunction-sections -fdata-sections -fstrict-volatile-bitfields -nostdlib -march=rv32imfc -mabi=ilp32f")