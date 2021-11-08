[TOC]

---

---

### 第一部分 设备接入配网

#### 1. 接口

```c++
    /**
     *  @fn         AP模块初始化，包括初始化ap以及启动httpserver
     *  @info       httpserver默认ip为192.168.4.1，端口设置为80
     *  @param[in]  dev_info：设备信息
     *  @param[in]  cb：回调函数，用于获取wifi设置状态
     *  @param[in]  timeout：wifi配置超时时间，范围1-60，单位min，默认15min
     *  @param[in]  support_apsta：是否支持ap、sta共存模式
     *  @return     成功：ez_errno_succ，失败：other
     */
    ez_err_e ez_iot_ap_init(ez_iot_ap_dev_info_t *dev_info, wifi_info_cb cb, int timeout, bool support_apsta);
    /**
     *  @fn     AP模块反初始化，包括停掉httpserver和ap模块，一般情况下不需要调用
     *          配网成功或超时之后，会自动释放httpserver相关资源
     */
    ez_err_t ez_iot_ap_finit();
```

```c++
    // 设置wifi信息的回调函数
    typedef void (*wifi_info_cb)(ez_iot_ap_wifi_info_t *wifi_info);
```

结构体数据说明：

```c++
    // 设备信息，用于ap初始化
    typedef struct
    {
        char ap_ssid[32 + 1];       // ap ssid
        char ap_password[64 + 1];   // ap password, 不需要密码则传空
        char auth_mode;             // ap auth mode, 0 for no ap_password 

        char dev_serial[72];        // 设备序列号
        char dev_type[64];          // 设备类型
        char dev_version[64];       // 设备版本号
    } ez_iot_ap_dev_info_t;
    
    // 配网过程获取到的信息
    typedef struct
    {
        char ssid[32+1];        // 设备连接的wifi ssid
        char password[64+1];    // 设备连接的wifi password
        
        char cc[4];             // 国家码cc
        char res[2];
        char token[128];        // app端校验的token
    
        char domain[128];       // 设备注册平台地址;
        char device_id[32];     // 设备uuid，可选配置
        int err_code;           // wifi配置错误码，见ez_iot_errno.h 中 ez_errno_ap_XXXX错误码定义
    } ez_iot_ap_wifi_info_t;
```

#### 2.接入流程

- 首先启动设备ap，示例如下（具体见example_ap.c)：

```c++
void example_ap_init_auth_apsta()
{
    // 设置设备信息
    ez_iot_ap_dev_info_t dev_info = {0};
    strncpy(dev_info.ap_ssid, "EZVIZ_AP_11112", sizeof(dev_info.ap_ssid) - 1);
    strncpy(dev_info.ap_password, "12345678", sizeof(dev_info.ap_password) - 1);
    dev_info.auth_mode = 4;
    strncpy(dev_info.dev_serial, "88888888", sizeof(dev_info.dev_serial) - 1);
    strncpy(dev_info.dev_type, "EZ_001", sizeof(dev_info.dev_type) - 1);
    strncpy(dev_info.dev_version, "V1.0.0 build 210302", sizeof(dev_info.dev_version) - 1);
    
    // wifi初始化
    ez_iot_wifi_init();
    
    // 启动ap
    ez_iot_ap_init(&dev_info, wifi_cb, 5, false);
    return;
}
```

- 将**设备信息、回调函数、以及ap超时时间**通过接口<font color="blue">***ez_iot_ap_init***</font>设置。示例如下：

```c++
static void wifi_cb(ez_iot_ap_wifi_info_t *wifi_info)
{
/** 由上层处理，根据错误码做上册业务处理 */
    switch (wifi_info->err_code)
    {
    case ez_errno_ap_app_connected:
        ez_log_w(TAG_UT_AP, "app connected.");
        break;
    case ez_errno_succ:
        ez_log_w(TAG_UT_AP, "wifi config success.");
        ez_log_i(TAG_UT_AP, "ssid: %s", wifi_info->ssid);
        ez_log_i(TAG_UT_AP, "password: %s", wifi_info->password);
        ez_log_i(TAG_UT_AP, "token: %s", wifi_info->token);
        ez_log_i(TAG_UT_AP, "domain: %s", wifi_info->domain);
        ez_iot_ap_finit();
		break;
    case ez_errno_ap_connecting_route:
        ez_log_w(TAG_UT_AP, "connecting route.");
		break;
    case ez_errno_ap_connect_failed:
        ez_log_w(TAG_UT_AP, "connect failed.");
        ez_iot_ap_finit();
        break;
    case ez_errno_ap_wifi_config_timeout:
        ez_log_w(TAG_UT_AP, "wifi config timeout.");
        ez_iot_ap_finit();
        break;
    default:
        break;
    }
}
```

- 由回调<font color="blue">***wifi_info_cb***</font>返回的wifi相关信息，**token**用于注册平台，**domain**为注册平台地址，应用层需保存，用于接入sdk上平台；**ssid**和**password**为设置的wifi信息，应用层用于连接路由器。

- 设备ap热点启动，信息如下：
  > ap配网服务地址：192.168.4.1
  >
  > ap配网服务端口：80
  >
  > ap配网服务协议：http协议
  
- ap热点启动之后，由app与设备通过http协议进行交互，包括**获取设备信息、获取wifi列表、以及设置需要连接的wifi ssid和password**，具体见第二部分app与设备交互。

- 应用层在获取到ssid和password之后，自行进行wifi连接。

---

---

### 第二部分 app与设备配网交互

#### 1. ap配网交互流程

```sequence
#dev->dev:ap start
app->dev:connect ap
app->dev:req: HTTP Get /AccessDevInfo
dev-->app:rsp: device info
app->dev:req: HTTP Get /PreNetwork/SecurityAndAccessPoint
dev-->app:rsp: wifi lists
app->dev:req: HTTP PUT /PreNetwork/WifiConfig
dev-->app:rsp: 200 OK（recv http put request）
```

#### 2. 交互协议

- 获取设备相关信息

```http
Get /AccessDevInfo
描述：APP端连接之后获取设备相关信息 
入口数据：无 
返回：
    {
         "ap_version":"1.0",//版本，当前ap版本1.0
         "dev_subserial":"123456789",//设备序列号，最大64
         "dev_type":"EZ1",//设备型号，最大64
         "dev_firmwareversion":"V1.0.0 build 190823"//设备固件版本号，最大64
    }
```

- 获取wifi列表

```http
Get /PreNetwork/SecurityAndAccessPoint
描述：AP接入wifi配置操作时，获取相关信息 （最大设置为20个）
入口数据：无 
返回：
	{
      "access_point_list":[{
       "ssid":"",
       /*必填,SSID,string*/
       "signal_strength":-20,
       /*必填,信号强度,"-100-0",int，数值越大信号越强*/
       "security_mode":"",
       /*可选,安全模式: "open,WEP,WPA-personal,WPA2-personal,WPA-WPA2-personal,WPA2-enterprise",string*/
      }]
    ｝
```

- 设置wifi **ssid和password**

```http
PUT /PreNetwork/WifiConfig
描述： AP接入Wifi配置操作（wifi连接可能会耗一定的时间，这里设置wifi连接超时时间为10s） 
入口数据：
	{
      "token":"",//必填，用于认证
      "lbs_domain":"",//必填，设备注册平台地址
      "device_id":"", //可选，设备uuid
      "wifi_info":
      {
       "ssid":"",
       /*必填,wifi的ssid,string*/
       "password":"",
       /*必填,wifi的密码信息,string*/
      },
    }
返回：
	{
         "status_code"："" //状态码， int类型
         "status_string"：""//状态描述， string类型
    }
```

#### 3. 错误码定义

由app设置wifi信息之后，设备返回http响应：

```c++
	"HTTP/1.1 200 OK",
    "HTTP/1.1 400 Bad Request",
    "HTTP/1.1 401 Unauthorized",
    "HTTP/1.1 404 Not Found",
    "HTTP/1.1 405 Method Not Allowed",
    "HTTP/1.1 500 Internal Server Error"
```
