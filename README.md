# RX5808-Div
## 1.界面设计基于LVGL

演示视频见：https://www.bilibili.com/video/BV1yr4y1371b

![ui](https://user-images.githubusercontent.com/66466560/218503938-571cd1fa-2c89-4279-a6aa-281c7fcf8234.jpeg)


1)主界面长按确定键，可锁定/解锁手动频道,短按进入菜单；  解锁情况下，按上下左右可调整频率。

2)菜单界面分为三块内容：扫描；设置；关于。

 按上下切换菜单选项，确定进入子菜单，左键返回主界面

3）扫描菜单有三个子内容：

图扫描显示频率5300-5900MHz信号强度；

表扫描显示将图传频道信号强度按不同颜色显示，扫描结束，频率切换至最当前信号强度最好的频道，并显示在右上角。

RSSI校准用于校准RSSI,未开启图传会校准失败，成功则会保存结果。

4）设置界面可设置显示屏背光强度，风扇转速、开机动画、蜂鸣器等是否开启，OSD制式需要保存后重新在主页面开启才能生效。

5）关于界面显示相关信息。

 

## 2.OSD支持

OSD功能由林面包（B站ID）添加，非叠加模式，主界面下解锁即可开启。

演示视频见：[https://www.bilibili.com/video/BV1ya411g78U ](https://www.bilibili.com/video/BV1ya411g78U) 与接收机UI完全同步。

![osd](https://user-images.githubusercontent.com/66466560/218504602-102e7fe0-b935-48ca-be9e-f459200034c8.jpg)


## 3.硬件设计

硬件开源地址：https://oshwhub.com/ftps/rx5808-div
