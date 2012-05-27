#!/bin/bash

MAKE=make
JOBS=4
#KERNELDIR="/lib/modules/`uname -r`/build"
export KERNELDIR=`pwd`
export ARCH=arm
export O=~/build
export CROSS_COMPILE=arm-linux-gnueabi-
export BOARD="ok6410"
export defconfig="ok6410_defconfig"

echo "KERNELDIR=$KERNELDIR"
echo "JOBS=$JOBS"
echo "ARCH=$ARCH"
echo "O=$O"
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

echo "availabe commands:"
echo "	mk"
echo "	mk_defconfig"
echo "	mk_menuconfig"
echo "	mk_uImage"
echo "	mk_check"
echo "	mk_clean"

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
	echo "buid to U-Boot wrapped zImage"
	mk uImage
	cp "$O/arch/arm/boot/uImage" "$O"
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
