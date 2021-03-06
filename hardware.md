# 硬件说明

本飞控的硬件平台为TM4C1294XL LaunchPad

![输入图片说明](https://images.gitee.com/uploads/images/2021/0306/112542_58c179a3_673614.png "屏幕截图.png")

做了一块转接板放置IMU以及整理各信号引脚以方便接线。

![输入图片说明](https://images.gitee.com/uploads/images/2021/0306/114332_66c739dd_673614.png "屏幕截图.png")

使用的传感器：

| 型号      | 说明    |
|---------|-------|
| LSM9DS0 | 九轴IMU |
| MPU9250 | 九轴IMU |
| MS5611  | 气压计  |

LSM9DS0和MPU9250的方位：x正方向-->飞机头部，y正方向-->飞机右侧。

## boosterpack引脚

主要使用两对boosterpack引脚连接外设。

![输入图片说明](https://images.gitee.com/uploads/images/2021/0306/115140_3c0cf331_673614.png "屏幕截图.png")

下面的引脚列表中的序号，与上图中的引脚序号一致.
- 实际功能：是指选择的GPIO复用功能。
- 业务功能：是指对应的ardupilot功能。

 **boosterpack1** 

序号 | 管脚名 | BoosterPack功能 | 实际功能 | 业务功能 | 备注
---|---|---|---|---|---
01 | 3.3V | 3.3V |  |  
02 | PE4 | Analog In |  |  
03 | PC4 | UART RX | U7Rx | serial4, uartE, normally 2nd GPS
04 | PC5 | UART TX | U7Tx | serial4, uartE, normally 2nd GPS
05 | PC6 | GPIO | GPIO | rc_in(ppm)
06 | PE5 | Analog In |  |  
07 | PD3 | SPI CLK | SSI2Clk | SPI_SENS_CLK
08 | PC7 | GPIO | GPIO | 性能测试，目前用于显示fast_loop频率
09 | PB2 | I2C SCL | I2C0SCL | I2C SCL | 用1K电阻连3.3V
10 | PB3 | I2C SDA | I2C0SDA | I2C SDA | 用1K电阻连3.3V
11 | PP2 | GPIO | GPIO |  SPI_SENS_CS_LSM9DS0_AM
12 | PN3 | GPIO | GPIO |  SPI_SENS_CS_LSM9DS0_G
13 | PN2 | GPIO | GPIO |  SPI_SENS_CS_MPU9250
14 | PD0 | SPI MISO | SSI2XDAT1 |  SPI_SENS_MISO
15 | PD1 | SPI MOSI | SSI2XDAT0 |  SPI_SENS_MOSI
16 | RESET | RST |  |  
17 | PH3 | GPIO | GPIO |  SPI_SENS_CS_MS5611
18 | PH2 | GPIO |  |  
19 | PM3 | GPIO |  |  
20 | GND | GND |  |  
21 | 5V | 5V |  |  
22 | GND | GND |  |  
23 | PE0 | Analog In | GPIO | 
24 | PE1 | Analog In | GPIO | 
25 | PE2 | Analog In |  |  
26 | PE3 | Analog In |  |  
27 | PD7 | Analog In |  |  
28 | PA6 | Analog In |  |  
29 | PM4 | Analog Out |  |  
30 | PM5 | Analog Out |  |  
31 | PL3 | GPIO | GPIO | 
32 | PL2 | GPIO |  |  
33 | PL1 | GPIO |  |  
34 | PL0 | GPIO |  |  
35 | PL5 | Timer Capture |  |  
36 | PL4 | Timer Capture |  |  
37 | PG0 | PWM out | M0PWM4 | motor4 
38 | PF3 | PWM out | M0PWM3 | motor3 
39 | PF2 | PWM out | M0PWM2 | motor2 
40 | PF1 | PWM out | M0PWM1 | motor1 

 **boosterpack2** 

序号 | 管脚名 | BoosterPack功能 | 实际功能 | 业务功能 | 备注
---|---|---|---|---|---
01 | 3.3V | 3.3V |  |  
02 | PD2 | Analog In |  |  
03 | PP0 | UART RX | U6Rx | serial2, uartD, normally telem2
04 | PP1 | UART TX | U6Tx | serial2, uartD, normally telem2
05 | PD4 | GPIO | U2Rx | serial1, uartC, normally telem1
06 | PD5 | Analog In | U2Tx | serial1, uartC, normally telem1
07 | PQ0 | SPI CLK | SSI3Clk | 
08 | PP4 | GPIO | GPIO | 
09 | PN5 | I2C SCL |  | 
10 | PN4 | I2C SDA |  | 
11 | PM6 | GPIO |  |  
12 | PQ1 | GPIO |  |  
13 | PP3 | GPIO | GPIO | 
14 | PQ3 | SPI MISO | SSI3XDAT1 | 
15 | PQ2 | SPI MOSI | SSI3XDAT0 | 
16 | RESET | RST |  |  
17 | PA7 | GPIO | GPIO | 
18 | PP5 | GPIO | GPIO |  
19 | PM7 | GPIO | GPIO |  
20 | GND | GND |  |  
21 | 5V  | 5V  |  |  
22 | GND | GND |  |  
23 | PB4 | Analog In |  | 
24 | PB5 | Analog In |  | 
25 | PK0 | Analog In | U4Rx | serial3, uartB, normally 1st GPS 
26 | PK1 | Analog In | U4Tx | serial3, uartB, normally 1st GPS
27 | PK2 | Analog In |  |  
28 | PK3 | Analog In |  |  
29 | PA4 | Analog Out | U3Rx | serial5, uartF 
30 | PA5 | Analog Out | U3Tx | serial5, uartF 
31 | PK7 | GPIO |  | 
32 | PK6 | GPIO |  |  
33 | PH1 | GPIO |  |  
34 | PH0 | GPIO |  |  
35 | PM2 | Timer Capture |  |  
36 | PM1 | Timer Capture | T2CCP1 | BEEP 
37 | PM0 | PWM out |  | 
38 | PK5 | PWM out | M0PWM7 | 
39 | PK4 | PWM out | M0PWM6 | 
40 | PG1 | PWM out | M0PWM5 | 

## 其他引脚

- 安全开关[safty switch](https://ardupilot.org/copter/docs/common-safety-switch-pixhawk.html?highlight=safety%20switch)，用的是开发板上的LED和按钮来实现。
- 调试日志口

序号 | 管脚名 | 功能 | 业务功能 
---|---|---|---
41 | PN1 | LED1 | SaftySwitch_LED
42 | PN0 | LED2 |
43 | PF4 | LED3 |
44 | PF0 | LED4 | 系统异常报警（如由看门狗触发重启），触发后无法解锁，需要人工重启（复位或重新上电）
45 | PJ0 | SW1 | SaftySwitch_KEY
46 | PJ1 | SW2 |
47 | PA0 | UART0_RX | uartA - the console
48 | PA1 | UART0_TX | uartA - the console



