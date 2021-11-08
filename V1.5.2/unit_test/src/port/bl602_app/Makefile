#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := bl602_app

PROJECT_PATH := $(abspath .)
PROJECT_BOARD := evb
export PROJECT_PATH PROJECT_BOARD
#CONFIG_TOOLPREFIX :=

CFLAGS = -I$(PROJECT_PATH)/../../../../src/components/FlashDB/inc -I$(PROJECT_PATH)/../../../../src/components/FlashDB/port/fal/inc -I$(PROJECT_PATH)/../../../../src/components/utilities/utest -I$(PROJECT_PATH)/../../../../inc -I$(PROJECT_PATH)/../../../../inc/ez_utils/cjson -D_RTOS_ -D_BL_SDK_ -DNETBSD -D_LARGEFILE_SOURCE -D_LARGE_FILES -DFTP_COMBINE_CWDS -DUTEST_TC_USE_CONSTRUCTOR #-std=gnu99

-include ./proj_config.mk

COMPONENTS_NETWORK := sntp dns_server
COMPONENTS_BLSYS   := bltime blfdt blmtd blota bloop loopadc looprt loopset
COMPONENTS_VFS     := romfs

INCLUDE_COMPONENTS += utest FlashDB
# INCLUDE_COMPONENTS += freertos_riscv_ram bl602 bl602_std hal_drv vfs yloop utils cli blog blog_testc
INCLUDE_COMPONENTS += freertos_riscv_ram bl602 bl602_std bl602_wifi bl602_wifidrv hal_drv lwip lwip_dhcpd mbedtls vfs yloop utils cli aws-iot httpc netutils blog blog_testc
INCLUDE_COMPONENTS += easyflash4
INCLUDE_COMPONENTS += $(COMPONENTS_NETWORK)
INCLUDE_COMPONENTS += $(COMPONENTS_BLSYS)
INCLUDE_COMPONENTS += $(COMPONENTS_VFS)
INCLUDE_COMPONENTS += $(PROJECT_NAME)

EXTRA_COMPONENT_DIRS := $(PROJECT_PATH)/../../../../src/components/FlashDB $(PROJECT_PATH)/../../../../src/components/utilities/utest

LDFLAGS += -march=rv32imfc -mabi=ilp32f -Wl,--cref -nostartfiles -Wl,--gc-sections -Wl,-static -Wl,--start-group  \
-L$(PROJECT_PATH)/build_out/bl602  -L /opt/bl_sdk/bl_iot_sdk/components/bl602/bl602/evb/ld -T flash_rom.ld -lbl602 \
-L$(PROJECT_PATH)/build_out/sntp  -lsntp  \
-L$(PROJECT_PATH)/build_out/FlashDB  -lFlashDB \
-L$(PROJECT_PATH)/build_out/dns_server  -ldns_server  \
-L$(PROJECT_PATH)/build_out/bl602_std  -lbl602_std  \
-L$(PROJECT_PATH)/build_out/blfdt  -lblfdt  \
-L$(PROJECT_PATH)/build_out/blmtd  -lblmtd  \
-L$(PROJECT_PATH)/build_out/blota  -lblota  \
-L$(PROJECT_PATH)/build_out/blog  -lblog  \
-L$(PROJECT_PATH)/build_out/blog_testc  -lblog_testc  \
-L$(PROJECT_PATH)/build_out/bloop  -lbloop  \
-L$(PROJECT_PATH)/build_out/bltime  -lbltime  \
-L$(PROJECT_PATH)/build_out/cli  -lcli  \
-L$(PROJECT_PATH)/build_out/freertos_riscv_ram -lfreertos_riscv_ram  \
-L$(PROJECT_PATH)/build_out/lwip  -llwip  \
-L$(PROJECT_PATH)/build_out/mbedtls  -lmbedtls  \
-L$(PROJECT_PATH)/build_out/lwip_dhcpd  -llwip_dhcpd \
-L$(PROJECT_PATH)/build_out/looprt  -llooprt  \
-L$(PROJECT_PATH)/build_out/loopset  -lloopset  \
-L$(PROJECT_PATH)/build_out/romfs  -lromfs  \
-L$(PROJECT_PATH)/build_out/bl602_app  -lbl602_app  \
-L$(PROJECT_PATH)/build_out/utils  -lutils  \
-L$(PROJECT_PATH)/build_out/vfs  -lvfs  \
-L$(PROJECT_PATH)/build_out/yloop  -lyloop \
-L$(PROJECT_PATH)/build_out/httpc  -lhttpc \
-L$(PROJECT_PATH)/build_out/hal_drv  -lhal_drv  \
-L$(PROJECT_PATH)/build_out/netutils  -lnetutils \
-L$(PROJECT_PATH)/build_out/easyflash4  -leasyflash4 \
-L$(PROJECT_PATH)/build_out/utest  -lutest \
-L$(PROJECT_PATH)/build_out/aws-iot  -laws-iot \
-L$(BL60X_SDK_PATH)/components/bl602/bl602_wifi/lib  -lbl602_wifi \
-L$(PROJECT_PATH)/build_out/bl602_wifidrv  -lbl602_wifidrv \
-L$(PROJECT_PATH)/../../../../lib/bl602 -lez_iot -lez_linkcore \
-Wl,--end-group -Wl,-EL -lm

# LDFLAGS += -L$(PROJECT_PATH)/../../../../lib/riscv64-unknown-elf/libez_iot.a -L$(PROJECT_PATH)/../../../../lib/riscv64-unknown-elf/libez_linkcore.a
# LDFLAGS += -march=rv32imfc -mabi=ilp32f -L$(PROJECT_PATH)/../../../../lib/riscv64-unknown-elf/libez_iot.a -L$(PROJECT_PATH)/../../../../lib/riscv64-unknown-elf/libez_linkcore.a

include $(BL60X_SDK_PATH)/make_scripts_riscv/project.mk