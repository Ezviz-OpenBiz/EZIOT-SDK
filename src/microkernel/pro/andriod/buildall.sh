#!/bin/sh
#make andriod microkerne

echo "*************************************************************************************"
echo "*************************make andriod microkernel*******************************"
echo "*************************************************************************************"

platformname="default"
svn_version=`svn info | cat -n | awk '{if($1==6)print $3}'`
echo $svn_version
file="../../../platform/crosscompiler/arm-linux-androideabi"
toolchainfile="../../../platform/crosscompiler/arm-linux-androideabi/ToolChain.cmake"

platformname="${file##*/}"
echo $platformname
rm -rf ToolChain.cmake
cp -rf $toolchainfile ./
rm -rf ../../lib/andriod
rm -rf CMakeCache.txt
cmake -DSVN_VERSION=$svn_version
make clean
make -j 9

cp -rf ../../src/ezdev_sdk_kernel_error.h ../../inc
cp -rf ../../src/ezdev_sdk_kernel_struct.h ../../inc
cp -rf ../../src/ezdev_sdk_kernel.h ../../inc
cp -rf ../../src/base_typedef.h ../../inc
cp -rf ../../src/ezdev_sdk_kernel_ex.h ../../inc
