#!/bin/sh
#make all platform microkerne

echo "*************************************************************************************"
echo "*************************make all platform microkernel*******************************"
echo "*************************************************************************************"

platformname="default"

svn_version=`svn info | cat -n | awk '{if($1==5)print $3}'`
echo $svn_version

lib_version=`svn info | cat -n | awk '{if($1==2)print $3}' |  awk -F'/v' '{print $2}' | awk -F'/' '{print $1}'`
echo $lib_version


for file in ../../../platform/toolchain/linux-store/*
do
	if test -d $file
	then
		echo $file
		for toolchainfile in $file/*
		do
			if test -f $toolchainfile
			then
				platformname="${file##*/}"
				echo $platformname
				rm -rf ToolChain.cmake
				cp -rf $toolchainfile ./
				rm -rf ../../lib/linux-stone/$platformname
				rm -rf CMakeCache.txt
				cmake -DSVN_VERSION=$svn_version -DLIB_VERSION=$lib_version -DPLATFORM_NAME=$platformname
				make -j 9
			fi
		done
	fi
done

cp -rf ../../src/ezdev_sdk_kernel_error.h ../../inc
cp -rf ../../src/ezdev_sdk_kernel_struct.h ../../inc
cp -rf ../../src/ezdev_sdk_kernel.h ../../inc
cp -rf ../../src/base_typedef.h ../../inc
cp -rf ../../src/ezdev_sdk_kernel_ex.h ../../inc
