SET(CMAKE_SYSTEM_NAME Generic)
SET(CMAKE_C_COMPILER "xtensa-lx106-elf-gcc")
SET(CMAKE_CXX_COMPILER "xtensa-lx106-elf-g++")
SET(PLATFORM_C_FLAGS "-Wno-frame-address -ffunction-sections -fdata-sections -fstrict-volatile-bitfields -mlongcalls -nostdlib")
SET(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

SET(SDK_BUILD_PORT "esp8266")            #芯片型号
SET(SDK_BUILD_OS "freertos")              #操作系统，linux/freertos

SET(SDK_CONPONENT_LINK_SUPPORT ON)      #基础连接
SET(SDK_CONPONENT_AP_SUPPORT ON)        #AP配网
SET(SDK_CONPONENT_TSL_SUPPORT ON)       #物模型
# SET(SDK_CONPONENT_CONNECT_SUPPORT ON)   #局域网互联互通
# SET(SDK_CONPONENT_HUB_SUPPORT ON)       #子设备管理
SET(SDK_CONPONENT_OTA_SUPPORT ON)       #升级
SET(SDK_CONPONENT_TIME_SUPPORT ON)      #校时

SET(SDK_ADD_PRIV_INCLUDEDIRS
    $ENV{IDF_PATH}/components/freertos/include 
    $ENV{IDF_PATH}/components/freertos/include/freertos 
    $ENV{IDF_PATH}/components/freertos/include/freertos/private 
    $ENV{IDF_PATH}/components/freertos/port/esp8266/include 
    $ENV{IDF_PATH}/components/freertos/port/esp8266/include/freertos 
    $ENV{IDF_PATH}/components/esp8266/include 
    $ENV{IDF_PATH}/components/heap/include 
    $ENV{IDF_PATH}/components/heap/port/esp8266/include 
    $ENV{IDF_PATH}/components/nvs_flash/test_nvs_host 
    $ENV{IDF_PATH}/components/lwip/include/lwip
    $ENV{IDF_PATH}/components/lwip/include/lwip/apps
    $ENV{IDF_PATH}/components/lwip/lwip/src/include
    $ENV{IDF_PATH}/components/lwip/lwip/src/include/posix
    $ENV{IDF_PATH}/components/lwip/port/esp8266/include
    $ENV{IDF_PATH}/components/lwip/port/esp8266/include/port
        
    # ap模块
    #$ENV{IDF_PATH}/components/esp_http_server/include 
    $ENV{IDF_PATH}/components/freertos/include 
    $ENV{IDF_PATH}/components/freertos/port/esp8266/include/freertos 
    $ENV{IDF_PATH}/components/freertos/port/esp8266/include 
    $ENV{IDF_PATH}/components/esp8266/include 
    $ENV{IDF_PATH}/components/heap/include 
    $ENV{IDF_PATH}/components/heap/port/esp8266/include
    $ENV{IDF_PATH}/components/freertos/include/freertos/private
    $ENV{IDF_PATH}/components/http_parser/include 
    $ENV{IDF_PATH}/components/cjson/cJSON
    $ENV{IDF_PATH}/components/lwip/lwip/src/include
    $ENV{IDF_PATH}/components/lwip/port/esp8266/include 
    $ENV{IDF_PATH}/components/lwip/include/lwip/apps 
    $ENV{IDF_PATH}/components/tcpip_adapter/include

    #校时模块
    $ENV{IDF_PATH}/components/log/include

    #升级模块
    $ENV{IDF_PATH}/components/app_update/include
    $ENV{IDF_PATH}/components/nvs_flash/include
    $ENV{IDF_PATH}/components/spi_flash/include
    #$ENV{IDF_PATH}/components/esp_http_client/include
    $ENV{IDF_PATH}/components/vfs/include
)

#设置私有编译宏
SET(SDK_ADD_PRIV_PREMACRO "-D__ESP_FILE__=__FILE__")

#设置私有编译参数
SET(SDK_ADD_PRIV_CFLAGS "-Wno-frame-address -ffunction-sections -fdata-sections -fstrict-volatile-bitfields -mlongcalls -nostdlib")
SET(SDK_ADD_PRIV_CXXFLAGS "-Wno-frame-address -ffunction-sections -fdata-sections -fstrict-volatile-bitfields -mlongcalls -nostdlib")
