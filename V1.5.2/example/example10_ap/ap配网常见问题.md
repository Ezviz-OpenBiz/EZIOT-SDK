**Q**：ap和station共存和不共存有什么区别？

**A**：ap和station模式<font color = blue>***共存***</font>的情况下，设备ap热点可以在与app不断开连接的情况下，去尝试连接app设置的wifi路由器，并得到连接错误码。此时返回给app，则app可通过错误码进行下一步处理。

​      ap和station模式<font color=red>***不共存***</font>的情况下，设备在收到app下发的wifi信息之后，就必须关闭ap热点、开启station模式，然后去连接app设置的wifi路由器。若密码错误或者其他连接失败的情况，则设备最终会添加失败。

​		两种情况，只在配网交互的最后一步——设备收到app设置的wifi信息之后的处理不同。



**Q**：wifi驱动实现接口和示例？

**A**：wifi驱动接口见<font color=blue>hal_wifi_drv.h</font>;

​		wifi驱动实现示例见<font color=blue>example_drv_wifi.c</font>，驱动示例基于乐鑫esp32实现，仅供参考;

​        

**Q**：ap配网实现了哪些内容？

**A**：ap配网实现的内容有下列：

- 设备http server。ap配网使用http协议，iot_sdk里面实现了http server，启动ap模式的时候，自动开启server；
- 设备与app的http协议交互。iot_sdk封装了http协议，实现了与app的交互过程，接入方只需要关注设备上层业务以及wifi驱动实现，即可接入；



​     ap配网未实现的内容：

- wifi驱动。接入方根据wifi驱动接口说明实现，<font color=blue>hal_wifi_drv.h</font>；

​        

