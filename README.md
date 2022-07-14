# RX5808-Div
演示视频见：https://www.bilibili.com/video/BV1yr4y1371b

1.渲染图

![img0](https://user-images.githubusercontent.com/66466560/173740025-2e5a6073-1ca6-43e2-872d-cbe62af1e4f6.jpg)

2.硬件设计采用上下层设计，上层为控制电路，下层为双接收电路
![1](https://user-images.githubusercontent.com/66466560/173743221-a987ee7a-7b8d-42af-af2b-217cb36f312b.jpg)

3.界面设计如下（基于LVGL）

![aaa](https://user-images.githubusercontent.com/66466560/173751076-f095a50c-9c2f-4cbb-949c-72bfd852f218.jpg)

1)主界面长按确定键，可锁定/解锁手动频道,短按进入菜单；
  解锁情况下，按上下左右可调整频率。
  
2）菜单界面分为三块内容：扫描；设置；关于。

   按上下切换菜单选项，确定进入子菜单，左键返回主界面
   
3）扫描菜单有三个子内容：

图扫描显示频率5300-5900MHz信号强度；

表扫描显示将图传频道信号强度按不同颜色显示，扫描结束，频率切换至最当前信号强度最好的频道，并显示在右上角；

RSSI校准用于校准RSSI,未开启图传会校准失败，成功则会保存结果。
                       
4）设置界面可设置显示屏背光强度，开机动画、蜂鸣器是否开启。
                       
5）关于界面显示相关信息




2022.7.14更新：

1.新增ESP32的版本，排针顺序不变，和STM32兼容，硬件如下：

![xxxx](https://user-images.githubusercontent.com/66466560/178969629-03389987-bf69-4e24-b404-36a6a50fd3b9.jpg)

2.新增中文，可以在设置项选择系统语言，中文界面如下：

![未标题-2](https://user-images.githubusercontent.com/66466560/178968786-9ca8ae10-4cd2-4e18-859b-d5b96e02f769.jpg)

3.新增输出信号源设置，选择接收机1或2则变为单接收模式，因此也可以只焊接一片RX5808做单接收机；

4.修复了一些显示上的BUG。
