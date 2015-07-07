`This kernel may be not working now since I am no longer working on this. Someone who intrests in this can have a test and fix the issues.`

Linux kernel for OK6410 development board (http://www.witech.com.cn/product/OK6410.html)

![github](http://www.witech.com.cn/product/up_pic/jkjsy.jpg "ok6410") 
#Introduction
This is a Linux kernel supports for OK6410 ARM development board. This project aims to keep ok6410 board updating with newest version kernel from kernel.org. 
This project is created for having an embedded platform environment that could verify some changes to latest kernel. It will be very nice if you can join in to help this. 

#Download and build source
##Download kernel source
You can download the whole source from https://github.com/changbindu/linux-ok6410.git via git

    $ git clone https://github.com/changbindu/linux-ok6410.git

Also you can download the latest revision as a tarball [here](https://github.com/changbindu/linux-ok6410/archive/master.tar.gz).

##How to build kernel for ok6410
Build linux-ok6410 with follow steps:

First, install cross-compiler and U-Boot tools. <br>
For Ubuntu:

    $ sudo apt-get install gcc-arm-linux-gnueabi
    $ sudo apt-get install u-boot-tools

For Archlinux:

    $ sudo yaourt -S gcc-arm-none-eabi uboot-mkimage 

Before building, you may change boar-id code in file "arch/arm/tools/mach-types" according to your uboot configuartion.
```txt
machine_is_xxx        CONFIG_xxxx             MACH_TYPE_xxx           number
...
smdk6410                MACH_SMDK6410           SMDK6410                1626
ok6410                  MACH_OK6410             OK6410                  1628
u300                    MACH_U300               U300                    1627
```
Then, there are two methods to compile kernel:

Use script "env_setup.sh"(recommended):
load default configuration

    $ cd linux-ok6410
    $ source env_setup.sh
    $ mk_defconfig
    $ mk_menuconfig

build code

    $ mk_uImage

Also you can compile it manually:
load default configuration

    $ cd linux-ok6410
    $ mkdir build
    $ make ok6410_defconfig  ARCH=arm  O=build

build code

    $ make uImage ARCH=arm CROSS_COMPILE=arm-linux-gnueabi-  O=build

#Flash and test kernel
##Nand partition configuration
```c
static struct mtd_partition ok6410_nand_part[] = {
        [0] = {
                .name           = "uboot",
                .size           = SZ_1M,
                .offset         = 0,
                .mask_flags     = MTD_CAP_NANDFLASH,
        },
        [1] = {
                .name           = "kernel",
                .size           = 5 * SZ_1M,
                .offset         = MTDPART_OFS_APPEND,
                .mask_flags     = MTD_CAP_NANDFLASH,
        },
        [2] = {
                .name           = "rootfs",
                .size           = MTDPART_SIZ_FULL,
                .offset         = MTDPART_OFS_APPEND,
        },
};
```
##Flash binary image to your board
Image downloading on Linux is recommended, it's stable and easy. You can download it at: 
http://code.google.com/p/dnw-linux 

Make U-Boot enter download mode:

    $ dnw 50008000 && nand erase 100000 500000 && nand write.e 50008000 100000 500000
    
you can download image to your board now:

    $ dnw ~/build/linux/uImage-linux-xxx.bin
#Test the kernel
You may need uboot [here](http://linux-ok6410.googlecode.com/files/u-boot_network_ok6410.bin).<br>
http://code.google.com/p/linux-ok6410<br> <br>
On download page has provided two root filesystem images of different formats for testing: <br>
[`ubifs image`](http://code.google.com/p/linux-ok6410/downloads/detail?name=qtopia-rootfs.ubifs&can=2&q=)
[`cramfs(readonly)`](http://code.google.com/p/linux-ok6410/downloads/detail?name=qtopia-rootfs.cramfs&can=2&q=)  <br><br>
Please note this kernel doesn't support yaffs/yaffs2 filesystem. Instead, to reduce maintenance efforts I use ubi filesystem which has been in community Linux kernel.<br>

For how to flash ubifs image, please refer to:<br>
https://raw.github.com/changbindu/dnw-linux/master/README

If you want to customize rootfs image, please refer to:<br>
https://github.com/changbindu/ok6410-stuff

##Flash UBI rootfs image
Make U-Boot enter download mode(change size accordingly):

    $ dnw 50008000 && nand erase 600000 && nand write.e 50008000 600000 8000000
    
you can download image to your board now:

    $ dnw qtopia-rootfs.ubifs
    
##Set kernel parameters
To boot using ubi image, please set boot arguments as:

    $ setenv bootargs console=ttySAC0,115200 ubi.mtd=rootfs root=ubi0:rootfs rootfstype=ubifs rw init=/linuxrc debug
    
If you use cramfs image(compressed and read only filesystem), please set boot arguments as(asuming you have downloaded to mtd partition 2):

    $ setenv bootargs console=ttySAC0,115200 root=/dev/mtdblock2 rootfstype=cramfs init=/linuxrc debug
    
Do not forget to save your change:

    $ save
    $ reset
Notice:

All enabled features are only tested on my board of which configuration is 256M ram and 2G nand.If you have a different board and failed booting kernel, you could submit your problem on issues page.
<br>
Best Regards

Contact: changbin.du@gmail.com
Links: [kernel.org](http://www.kernel.org/) [dnw-linux](http://code.google.com/p/dnw-linux)
