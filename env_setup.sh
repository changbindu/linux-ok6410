#!/bin/bash

MAKE=make
JOBS=4
#KERNELDIR="/lib/modules/`uname -r`/build"
export KERNELDIR=`pwd`
export ARCH=arm
export O=~/build/linux
export INSTALL_MOD_PATH=$O/modules
export CROSS_COMPILE=arm-linux-gnueabi-
export BOARD="ok6410"
export defconfig="ok6410_defconfig"

echo "KERNELDIR=$KERNELDIR"
echo "JOBS=$JOBS"
echo "ARCH=$ARCH"
echo "O=$O"
echo "INSTALL_MOD_PATH=$INSTALL_MOD_PATH"
echo "CROSS_COMPILE=$CROSS_COMPILE"
echo "BOARD=$BOARD"
echo "defconfig=$defconfig"
echo ""

if [ ! -d ./drivers -o ! -d ./arch ]
then
	echo "you are not in kernel source root folder"
	return
fi

mkdir -p $O
echo -e "OK!\n"

echo "availabe commands:"
echo "	mk		- excute make with apropriate variables set"
echo "	mm		- build modules in current directory"
echo "	mk_defconfig	- load default kernel config"
echo "	mk_menuconfig	- enter menuconfig"
echo "	mk_uImage	- build kernel to U-Boot image"
echo "	mk_check	- check source"
echo "	mk_clean	- clean files built"
echo "	mk_modules_install - install buit modules to 'INSTALL_MOD_PATH'"

get_kernel_version()
{
	version=""
	awk -F ' = ' '{if(NR==1){printf("%d.",$2)}
		else if(NR==2){printf("%d.",$2)}
		else if(NR==3){printf("%d",$2)}
		else if(NR==4){print $2}
		else {exit}}' "$KERNELDIR/Makefile"
}

mk()
{
	$MAKE -C $KERNELDIR O="$O" -j$JOBS $*
}

mm()
{
	echo "make modules in current folder"
	mk "M=`pwd`" modules
}

mk_defconfig()
{
	echo "load defconfig $defconfig"
	mk $defconfig
}

mk_uImage()
{
	target="$O/ok6410-uImage-linux-v`get_kernel_version`.bin"
	echo "buid to U-Boot wrapped zImage"
	mk uImage
	cp "$O/arch/arm/boot/uImage" "$target"
	echo "your u-image: $target"
}

mk_clean()
{
	echo "clean files"
	mk clean
}

mk_menuconfig()
{
	mk menuconfig
}

mk_check()
{
	echo "checking stack - Generate a list of stack hogs"
	mk checkstack
	echo "checking namespace - Name space analysis on compiled kernel"
	mk namespacecheck
	echo "checking include - Check for duplicate included header files"
	mk includecheck
	echo "checking headers - Sanity check on exported headers"
	mk headers_check
	echo "sparse all c source"
	mk "C=2"
}

mk_modules_install()
{
	mk modules_install
}

