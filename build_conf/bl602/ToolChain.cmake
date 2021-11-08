SET(CMAKE_SYSTEM_NAME Generic)
SET(CMAKE_C_COMPILER "riscv64-unknown-elf-gcc")
SET(CMAKE_CXX_COMPILER "riscv64-unknown-elf-g++")

SET(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

SET(SDK_BUILD_PORT "bl602")            #芯片型号
SET(SDK_BUILD_OS "bl602")           #操作系统，linux/freertos

SET(SDK_CONPONENT_LINK_SUPPORT ON)      #基础连接
SET(SDK_CONPONENT_AP_SUPPORT ON)        #AP配网
SET(SDK_CONPONENT_TSL_SUPPORT ON)       #物模型
# SET(SDK_CONPONENT_CONNECT_SUPPORT ON)   #局域网互联互通
SET(SDK_CONPONENT_HUB_SUPPORT ON)       #子设备管理
SET(SDK_CONPONENT_OTA_SUPPORT ON)       #升级
# SET(SDK_CONPONENT_TIME_SUPPORT ON)      #校时

SET(SDK_ADD_PRIV_INCLUDEDIRS
    $ENV{BL60X_SDK_PATH}/components/network/lwip/src/include/compat/posix
	$ENV{BL60X_SDK_PATH}/components/network/lwip/src/include
	$ENV{BL60X_SDK_PATH}/components/network/lwip/lwip-port/config
	$ENV{BL60X_SDK_PATH}/components/network/lwip/lwip-port
	$ENV{BL60X_SDK_PATH}/components/network/lwip/src/include/lwip/
	$ENV{BL60X_SDK_PATH}/components/fs/vfs/posix/include
	$ENV{BL60X_SDK_PATH}/toolchain/riscv/Linux/riscv64-unknown-elf/include
	$ENV{BL60X_SDK_PATH}/components/bl602/freertos_riscv/config
	$ENV{BL60X_SDK_PATH}/components/bl602/freertos_riscv/portable/GCC/RISC-V/
	$ENV{BL60X_SDK_PATH}/components/bl602/bl602_wifidrv/bl60x_wifi_driver/include
    $ENV{BL60X_SDK_PATH}/components/stage/yloop/include/aos
    $ENV{BL60X_SDK_PATH}/components/stage/yloop/include
)

#设置私有编译宏
SET(SDK_ADD_PRIV_PREMACRO "-D_BL_SDK_ -D_POSIX_THREADS -DSSIZE_MAX -D_POSIX_MONOTONIC_CLOCK")

#设置私有编译参数
SET(SDK_ADD_PRIV_CFLAGS "-Wno-frame-address -ffunction-sections -fdata-sections -fstrict-volatile-bitfields -nostdlib -march=rv32imfc -mabi=ilp32f")
SET(SDK_ADD_PRIV_CXXFLAGS "-Wno-frame-address -ffunction-sections -fdata-sections -fstrict-volatile-bitfields -nostdlib -march=rv32imfc -mabi=ilp32f")