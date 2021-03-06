# Ardupilot for TM4C1294XL LaunchPad 

![输入图片说明](https://images.gitee.com/uploads/images/2021/0306/120309_c1d850e8_673614.png "屏幕截图.png")

## 简介

本项目由[Ardupilot-3.5.7](https://github.com/ArduPilot/ardupilot/tree/Copter-3.5.7)移植而来。烧录固件后，使用方法与官方一致。需要使用Mission Planner进行参数配置校准，详见[First Time Setup](https://ardupilot.org/copter/docs/initial-setup.html)。

这里列出与官方不一样的地方：
- 本项目未做BootLoader，编译后的固件，直接烧进单片机即可。也不支持通过Mission Planner更新固件。
- 本项目未做日志系统，无法使用Mission Planner查看飞行日志。

## 目录及重要文件说明

 **根目录** 

记为/

| 名称         | 说明                               |
|------------|----------------------------------|
| ardupilot  | 飞控代码，即移植的官方代码                    |
| bsp        | rtthread的版级支持包，只保留了一个，即本项目对应的bsp |
| 其他目录 | 都是rtthread的                      |


 **本硬件的HAL目录** 

移植时，主要修改的是ardupilot\libraries\AP_HAL_PX4中的代码。作者偷懒了，使用偷梁换柱的方法，把PX4改成了TM4C1294XL LaunchPad。

 **本硬件的bsp目录** 

/bsp/tm4c129x

| 名称         | 说明                               |
|------------|----------------------------------|
|  application | rtthread应用程序入口，就是本飞控的入口啦 |
| ardupilot_app | ardupilot的ccs工程 |
| ardupilot_driver | ardupilot底层驱动（即TM4C1294XL LaunchPad底层驱动）的ccs工程 |

补充说明：
- ccs就是Code Composer Stduio
- ardupilot_app工程包含了ardupilot_driver工程，如果是要编译完整固件的话，仅打开ardupilot_app工程即可。作者之所以别开了一个ardupilot_driver工程，是为了便于开发驱动。虽然ccs是增量编译，不过整个ardupilot编译出的固件太大了，烧录就要1分钟。。。。。。
- ardupilot_app和ardupilot_driver仅仅是IDE工程目录，里面不包含代码的。它们引用的是同一份代码。

## 编译方法

本项目是2年前做的，用的是CCS8， Version: 8.3.0.00009 。更高版本的IDE应该没问题，因为CCS8自带的编译器也编译不过，主要是在编译RT-Thread的POXIS接口时有问题。研究无果，最终简单粗暴，使用RT-Thread官方编译器来编译。下面讲解换编译器的方法。

安装RT-Thread env环境，参考[Env 用户手册](https://www.rt-thread.org/document/site/programming-manual/env/env/)。

将env中的编译器（mingw）复制到ccs目录中。比如，笔者是将C:\rtthread\env\tools\gnu_gcc\arm_gcc复制到C:\ti\ccsv8\tools\compiler，最终如图所示：

![输入图片说明](https://images.gitee.com/uploads/images/2021/0306/110610_d88a76e5_673614.png "屏幕截图.png")

选择菜单Windows->Preferences，定位到Code Composer Studio->Build->Compilers，点击Refresh，Discovered tools将列出刚才复制的编译器

![输入图片说明](https://images.gitee.com/uploads/images/2021/0306/110945_653f98ac_673614.png "屏幕截图.png")

在项目浏览器的工程上右击菜单->Properties，定位到General，选择Compiler version为GNU v5.4.1，这个就是刚才复制的RT-Thread编译器。

![输入图片说明](https://images.gitee.com/uploads/images/2021/0306/111118_bea4d9c4_673614.png "屏幕截图.png")

下面就可以进行数分钟愉快的编译了：

```
Building target: "ardupilot_app.out"
Invoking: GNU Linker
'Flags: -fno-exceptions -DPART_TM4C1294NCPDT -D"APM_BUILD_DIRECTORY=APM_BUILD_ArduCopter" -D"__PX4_NUTTX" -D"UAVCAN_NO_ASSERTIONS=1" -D"HAVE_CCONFIG_H" -D"__STDC_FORMAT_MACROS" -D"gcc" -D"UAVCAN_NULLPTR=nullptr" -D'"'"'SKETCH="ArduCopter"'"'"' -D"CONFIG_WCHAR_BUILTIN" -D'"'"'SKETCHNAME="ArduCopter"'"'"' -D"HAVE_OCLOEXEC=0" -D"__DF_NUTTX" -D"PART_TM4C129XNCZAD" -D"RT_USING_NEWLIB" -D"UAVCAN_CPP_VERSION=UAVCAN_CPP03" -D"FRAME_CONFIG=MULTICOPTER_FRAME" -D"CONFIG_ARCH_BOARD_PX4FMU_V2" -D'"'"'SKETCHBOOK="E:\ardupilot\src\ardupilot-rtthread\ardupilot"'"'"' -D"HAVE_STD_NULLPTR_T=0" -D"CONFIG_HAL_BOARD=HAL_BOARD_PX4" -Og -ffunction-sections -fdata-sections -g -gdwarf-3 -gstrict-dwarf -Wall -specs="nosys.specs" -march=armv7e-m -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Wl,-Map,"ardupilot_app.map" -nostartfiles'
"C:/ti/ccsv8/tools/compiler/mingw/bin/arm-none-eabi-gcc.exe" @"ccsLinker.opt" -o"ardupilot_app.out"
Finished building target: "ardupilot_app.out"
 
arm-none-eabi-size.exe ardupilot_app.out
   text	   data	    bss	    dec	    hex	filename
1000440	   3104	  31876	1035420	  fcc9c	ardupilot_app.out
```


## 硬件接线

详见[hardware.md](hardware.md)






