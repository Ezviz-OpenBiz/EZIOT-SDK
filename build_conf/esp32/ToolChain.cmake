SET(CMAKE_SYSTEM_NAME Generic)
SET(CMAKE_C_COMPILER "xtensa-esp32-elf-gcc")
SET(CMAKE_CXX_COMPILER "xtensa-esp32-elf-g++")
SET(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

SET(SDK_BUILD_PORT "esp32")            #芯片型号
SET(SDK_BUILD_OS "freertos")              #操作系统，linux/freertos

SET(SDK_CONPONENT_LINK_SUPPORT ON)      #基础连接
SET(SDK_CONPONENT_AP_SUPPORT ON)        #AP配网
SET(SDK_CONPONENT_TSL_SUPPORT ON)       #物模型
# SET(SDK_CONPONENT_CONNECT_SUPPORT ON)   #局域网互联互通
#SET(SDK_CONPONENT_HUB_SUPPORT ON)       #子设备管理
SET(SDK_CONPONENT_OTA_SUPPORT ON)       #升级
SET(SDK_CONPONENT_TIME_SUPPORT ON)      #校时

SET(SDK_ADD_PRIV_INCLUDEDIRS
    $ENV{IDF_PATH}/components/freertos/include 
    $ENV{IDF_PATH}/components/freertos/include/freertos 
    $ENV{IDF_PATH}/components/freertos/include/freertos/private 

    $ENV{IDF_PATH}/components/esp_common/include 
    $ENV{IDF_PATH}/components/xtensa/include 
    $ENV{IDF_PATH}/components/xtensa/esp32/include/
    $ENV{IDF_PATH}/components/esp_rom/include/
    $ENV{IDF_PATH}/components/soc/esp32/include/
    $ENV{IDF_PATH}/components/newlib/platform_include/

    $ENV{IDF_PATH}/components/lwip/lwip/src/include
    $ENV{IDF_PATH}/components/lwip/lwip/src/include/lwip
    $ENV{IDF_PATH}/components/lwip/port/esp32/include
    $ENV{IDF_PATH}/components/vfs/include
    $ENV{IDF_PATH}/components/freertos/include 
    $ENV{IDF_PATH}/components/esp32/include 

    $ENV{IDF_PATH}/components/lwip/include/apps/sntp
    $ENV{IDF_PATH}/components/heap/include 
    $ENV{IDF_PATH}/components/json/cJSON
    $ENV{IDF_PATH}/components/esp_wifi/include
    $ENV{IDF_PATH}/components/esp_event/include
    $ENV{IDF_PATH}/components/tcpip_adapter/include
    $ENV{IDF_PATH}/components/lwip/include/apps
    $ENV{IDF_PATH}/components/driver/include
    $ENV{IDF_PATH}/components/esp_ringbuf/include
    $ENV{IDF_PATH}/components/soc/include
    #$ENV{IDF_PATH}/components/esp_http_server/include 
    $ENV{IDF_PATH}/components/nghttp/port/include 
    $ENV{IDF_PATH}/components/app_update/include 
    $ENV{IDF_PATH}/components/spi_flash/include
    $ENV{IDF_PATH}/components/bootloader_support/include
    $ENV{IDF_PATH}/components/nvs_flash/include
    #$ENV{IDF_PATH}/components/esp_http_client/include

    $ENV{IDF_PATH}/components/bt/host/bluedroid/api/include/api
    $ENV{IDF_PATH}/components/bt/include
    $ENV{IDF_PATH}/components/bt/common/include
    $ENV{IDF_PATH}/components/bt/common/osi/include
    $ENV{IDF_PATH}/components/bt/common/btc/include
    $ENV{IDF_PATH}/components/bt/host/bluedroid/btc/profile/esp/blufi/include
    $ENV{IDF_PATH}/components/bt/host/bluedroid/stack/include
    $ENV{IDF_PATH}/components/bt/host/bluedroid/common/include

    $ENV{IDF_PATH}/components/log/include
    $ENV{IDF_PATH}/components/newlib/include
)

#设置私有编译宏
SET(SDK_ADD_PRIV_PREMACRO "-DHUOMAN_TRANSLATE_COMPAT")

#设置私有编译参数
SET(SDK_ADD_PRIV_CFLAGS "-Wno-frame-address -ffunction-sections -fdata-sections -fstrict-volatile-bitfields -mlongcalls -nostdlib")
SET(SDK_ADD_PRIV_CXXFLAGS "-Wno-frame-address -ffunction-sections -fdata-sections -fstrict-volatile-bitfields -mlongcalls -nostdlib")
