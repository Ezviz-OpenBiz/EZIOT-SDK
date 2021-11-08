# 萤石IoT-SDK芯片适配指导手册

​		根据近期多款芯片的适配工作，总结出当前版本SDK适配芯片的最佳实践经验，希望能帮助其他同学能够快速、准确的评估适工作量以及快速完成适配验证工作。

## 1 移植条件评估

### 1.1  评估编译环境

<!--由芯片/模组供应商来评估-->

1. 支持以工具链形式进行交叉编译

2. 支持ubuntu 18.04 TLS x64编译环境

3. 测试build_demo验证可用，见附件 [build_demo.zip](build_demo)

### 1.2 评估资源

<!--由芯片/模组供应商提供参数，我们评估-->

- 支持多任务：预留给SDK最少4个并发运行任务。
- 单品产品：240K ROM 和 40K RAM
- 网关类产品：260K ROM + 400K可擦写存储 和  440K RAM（按100个子设备估算）

### 1.3 评估接口

#### 1.3.1 是否支持常用的libc接口

<!--由芯片/模组供应商评估，如不支持需提供替代接口-->

printf、memcpy、memset、strcpy、strncpy、sprintf、snprintf、malloc、free、calloc、zalloc、realloc

#### 1.3.2 是否是常用操作系统

<!--由芯片/模组供应商提供参数，我们评估-->

​		如果芯片使用的是Linux、FreeRTOS、RT-Thread其中的一种，不需要适配底层接口。否则芯片/模组厂需要帮适配以下抽象接口：

![hal_list](.\figures\hal_list.png) 

### 1.4 评估时间

| 工作项                           | 是   | 否                |
| -------------------------------- | ---- | ----------------- |
| 支持以工具链形式进行交叉编译     | 0.5  | 5                 |
| 支持ubuntu 18.04 TLS x64编译环境 | 0.5  | 2                 |
| 测试build_demo验证可用           | 1    | 5                 |
| 是否支持常用的libc接口           | 0    | 2                 |
| 是否是常用操作系统               | 0    | 2                 |
| 萤石验证模组基本功能             | 3    | 5（三方厂商验证） |

## 2 适配工作

### 2.1 安装工具链

编译服务器地址：10.1.14.113，没有账号找陈腾飞5开通

工具链安装路径：/opt/toolchain/

芯片SDK安装路径：/opt/

![sdk_path](.\figures\sdk_path.png)

### 2.2 更改shell脚本

- 打开根目录build.sh文件。
- 在ln_ToolChain_all函数中添加工具链路径导出语句，**此处的$BUILD_PORT变量一般用芯片型号命名，后面的步骤中有几处目录会以此命名**。

![toolchain_add](Y:\IOT-SDK\branches\V1.5.0\doc\figures\toolchain_add.png)

### 2.3 新增配置目录

新增一个配置目录：cp -r ./build_conf/template ./build_conf/$BUILD_PORT，同上$BUILD_PORT一般为芯片型号。

配置在Kconfig GUI配置SDK功能：

![Kconfig_conf](.\figures\Kconfig_conf.PNG)

### 2.4 更改编译配置项

<!--当前版本需手动从./build_conf/linux目录下拷贝mcuconfig.h、ToolChain.cmake文件-->

#### 2.4.1 编辑ToolChain.cmake

- 更改CMAKE_SYSTEM_NAME，linux操作系统填“Linux”，否则填“Generic”。例：

  ```cmake
  SET(CMAKE_SYSTEM_NAME Generic)
  ```

- 更改工具链名字CMAKE_C_COMPILER，CMAKE_CXX_COMPILER，这两项改成芯片对应的gcc、g++。例：

  ```cmake
  SET(CMAKE_C_COMPILER "arm-none-eabi-gcc")
  SET(CMAKE_CXX_COMPILER "arm-none-eabi-g++")
  ```

- 设置芯片型号。例：

  ```cmake
  SET(SDK_BUILD_PORT "ln882x")
  ```

- 设置芯片操作系统，不属于Linux、FreeRTOS则填写芯片型号。例：

  ```cmake
  SET(SDK_BUILD_OS "ln882x")
  ```

- 编辑主要编译的模块，不需要则注释。例：

  ```cmake
  # SET(SDK_CONPONENT_AP_SUPPORT ON)        #AP配网
  SET(SDK_CONPONENT_LINK_SUPPORT ON)      #基础连接
  SET(SDK_CONPONENT_LOGGER_SUPPORT ON)
  SET(SDK_CONPONENT_TSL_SUPPORT ON)       #物模型
  # SET(SDK_CONPONENT_CONNECT_SUPPORT ON)   #局域网互联互通
  SET(SDK_CONPONENT_HUB_SUPPORT ON)       #子设备管理
  SET(SDK_CONPONENT_OTA_SUPPORT ON)       #升级
  # SET(SDK_CONPONENT_TIME_SUPPORT ON)      #校时
  ```

- 设置芯片SDK依赖头文件目录，不需要则注释。例：

  ```cmake
  SET(SDK_ADD_PRIV_INCLUDEDIRS
      /opt/ln882x-sdk/components/net/lwip-2.0.3/src/include
      /opt/ln882x-sdk/components/net/lwip-2.0.3/src/port
  )
  ```

- 设置预编译宏，不需要则注释。例：

  ```cmake
  # SET(SDK_ADD_PRIV_PREMACRO "-DRT_USING_SAL")
  ```

- 设置私有编译参数，不需要则注释。例：

  ```cmake
  SET(SDK_ADD_PRIV_CFLAGS "-Wno-frame-address -ffunction-sections -fdata-sections -fstrict-volatile-bitfields -nostdlib -march=rv32imfc -mabi=ilp32f")
  SET(SDK_ADD_PRIV_CXXFLAGS "-Wno-frame-address -ffunction-sections -fdata-sections -fstrict-volatile-bitfields -nostdlib -march=rv32imfc -mabi=ilp32f")
  ```

#### 2.4.2 编辑mcuconfig.h

对于一些没有操作系统的芯片，有可能并不支持标准libc函数，这种情况需要通过宏来替换成模组适配的接口，最后在编译可执行文件的时候完成链接。

```c
#define printf    system_printf
#define malloc    os_malloc
#define free      os_free
#define calloc    os_calloc
#define zalloc    os_zalloc
#define realloc   os_realloc
#define strspn    os_strspn
#define strcspn   os_strcspn
#define strdup    os_strdup
#define localtime os_localtime
#define time      os_time
#define sprintf   os_sprintf
#define snprintf  os_snprintf
#define strdup    os_strdup
#define vsnprintf os_vsnprintf
```

### 2.5 适配跨平台接口

如芯片操作系统是Linux、FreeRTOS，不需要适配，否则需要在src\ez_iot_sdk\src\ez_hal目录下完成抽象接口适配。例：

![hal_impl](.\figures\hal_impl.png) 

### 2.6 适配日志库

日志模块是移植的开源组件，当前版本SDK还没有将其配置项纳入Kconfig统一表，需要单独配置。如芯片操作系统是Linux、FreeRTOS，不需要适配，否则需要在src\ez_iot_sdk\src\component\logger\wrappers目录下完成适配。例：

![log_imp](.\figures\log_imp.png) 

### 2.7 编译

在根目录下执行build.sh $BUILD_PORT，$BUILD_PORT为芯片型号。

### 2.8 验证

这里分两种情况，一种是三方接入，验证工作由他们进行。另外一种是我们自研模组，可通过单元测试进行快速验证。

- 适配，在unit_test创建芯片的目录，完成入口函数和文件系统适配：

![utest_al](.\figures\utest_al.png) 

- 运行：

![utest_run](.\figures\utest_run.png) 