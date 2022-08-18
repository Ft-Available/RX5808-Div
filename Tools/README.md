# RX5808-Div-Tool

RX5808下载固件指南

一、	STM32版本

STM32可用USB，UART，SWD下载，不再赘述。

二、	ESP32版本

1、解压此目录下ESP32_FirmWare.zip文件夹，得到三个xxx.bin文件，如下图：

![image](https://user-images.githubusercontent.com/66466560/183941319-5b98264a-7aaf-42ed-a1c0-3359cdcafc04.png)

2、解压此目录下flash_download_tool_3.9.2_0.zip文件夹，得到以下文件：

![image](https://user-images.githubusercontent.com/66466560/183941369-ae1474e4-ccc6-4826-a105-4ce33f092943.png)
 
3、双击flash_download_tool_3.9.2_0.exe打开下载程序：

![image](https://user-images.githubusercontent.com/66466560/183941402-a557a9e5-d548-456c-b53c-5481e826d153.png)

选择ESP32并点击OK。

4.将解压得到的bin文件按下图顺序打开：

         bootloader.bin     0x1000
         
         partition-table.bin  0x8000
         
         RX5808.bin        0x10000 
         
 ![image](https://user-images.githubusercontent.com/66466560/183941506-98f46ba4-1fad-475d-91d7-f391da223f43.png)

5.SPIFLASH按下图设置：

![image](https://user-images.githubusercontent.com/66466560/183941552-d3622ece-0861-4cdc-a700-2601824ec92c.png)
 
6.选择COM口并下载：（下图是下载完成的样子）
 
![image](https://user-images.githubusercontent.com/66466560/183941582-fadea089-f43e-490e-819a-48b0e2b0c2e5.png)

7.问题
如遇花屏问题用ESP32_FirmWare_Slow固件。

2022.8.10新增模组版固件为ESP32_Model_FirmWare。
 
