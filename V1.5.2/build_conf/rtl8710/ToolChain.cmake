SET(CMAKE_SYSTEM_NAME Generic)
SET(CMAKE_C_COMPILER "arm-none-eabi-gcc")
SET(CMAKE_CXX_COMPILER "arm-none-eabi-g++")

SET(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

SET(SDK_BUILD_PORT "rtl8710")            #芯片型号
SET(SDK_BUILD_OS "rtl8710")           #操作系统，linux/freertos

SET(SDK_CONPONENT_LINK_SUPPORT ON)      #基础连接
#SET(SDK_CONPONENT_AP_SUPPORT ON)        #AP配网
SET(SDK_CONPONENT_TSL_SUPPORT ON)       #物模型
# SET(SDK_CONPONENT_CONNECT_SUPPORT ON)   #局域网互联互通
SET(SDK_CONPONENT_HUB_SUPPORT ON)       #子设备管理
SET(SDK_CONPONENT_OTA_SUPPORT ON)       #升级
SET(SDK_CONPONENT_TIME_SUPPORT ON)      #校时

SET(SDK_ADD_PRIV_INCLUDEDIRS 
    /opt/sdk-ameba-v7.1d/component/common/network/lwip/lwip_v2.0.2/src/include/posix
    /opt/sdk-ameba-v7.1d/component/common/network/lwip/lwip_v2.0.2/src/include  
    /opt/sdk-ameba-v7.1d/component/common/api/network/include
    /opt/sdk-ameba-v7.1d/component/common/api
    /opt/sdk-ameba-v7.1d/component/soc/realtek/8710c/cmsis/rtl8710c/include
    /opt/sdk-ameba-v7.1d/project/realtek_amebaz2_v0_example/inc
    /opt/sdk-ameba-v7.1d/component/common/network/lwip/lwip_v2.0.2/port/realtek
    /opt/sdk-ameba-v7.1d/component/os/freertos/freertos_v10.0.1/Source/include
    /opt/sdk-ameba-v7.1d/component/soc/realtek/8710c/app/rtl_printf/include
    /opt/sdk-ameba-v7.1d/component/soc/realtek/8710c/cmsis/cmsis-core/include
    /opt/sdk-ameba-v7.1d/component/soc/realtek/8710c/app/stdio_port
    /opt/sdk-ameba-v7.1d/component/soc/realtek/8710c/misc/utilities/include
    /opt/sdk-ameba-v7.1d/component/os/freertos/freertos_v10.0.1/Source/portable/GCC/ARM_RTL8710C
)

#设置私有编译宏
SET(SDK_ADD_PRIV_PREMACRO "-DCONFIG_PLATFORM_8710C -D_POSIX_THREADS -D_POSIX_MONOTONIC_CLOCK -DSNTP_SERVER_DNS")

#设置私有编译参数
SET(SDK_ADD_PRIV_CFLAGS "-Wno-frame-address -ffunction-sections -fdata-sections -fstrict-volatile-bitfields -nostdlib -march=armv8-m.main+dsp -mthumb -mcmse -mfloat-abi=soft -D__thumb2__ -g -gdwarf-3 -D__ARM_ARCH_8M_MAIN__=1 -gdwarf-3 -fstack-usage -fdiagnostics-color=always -Wall -Wundef -Wno-write-strings -Wno-maybe-uninitialized --save-temps -c -MMD -DCONFIG_BUILD_RAM=1 -DV8M_STKOVF -DCONFIG_SYSTEM_TIME64=0 -DSUPPORT_MP_MODE=0")
SET(SDK_ADD_PRIV_CXXFLAGS "-Wno-frame-address -ffunction-sections -fdata-sections -fstrict-volatile-bitfields -nostdlib -march=armv8-m.main+dsp -mthumb -mcmse -mfloat-abi=soft -D__thumb2__ -g -gdwarf-3 -D__ARM_ARCH_8M_MAIN__=1 -gdwarf-3 -fstack-usage -fdiagnostics-color=always -Wall -Wundef -Wno-write-strings -Wno-maybe-uninitialized --save-temps -c -MMD -DCONFIG_BUILD_RAM=1 -DV8M_STKOVF -DCONFIG_SYSTEM_TIME64=0 -DSUPPORT_MP_MODE=0")