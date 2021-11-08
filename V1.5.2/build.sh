#!/bin/bash
#By zhangyi

PRJ_PWD=$PWD
LOC_PWD=${PRJ_PWD}
IOT_SDK_DIR=${PRJ_PWD}/src/ez_iot_sdk
IOT_SDK_COMPONENTS=${PRJ_PWD}/src/components
SUBDIRS='src/ez_iot_sdk/src src/microkernel/src'
EXPORT_PRE="eziot-sdk"

function usage()
{
	echo -e "***********************************************************"
	echo -e "USAGE:" 
	echo -e "	./build.sh \${PORT}"
	echo -e ""
	echo -e "PORT:" 
	echo -e "            linux"
    echo -e "            esp8266"
	echo -e "            esp32"
	echo -e "            tr6260"
	echo -e "            bl602"
	echo -e "            l610"
	echo -e "            rtl8710"
	echo -e "            mt7688"
	echo -e "            ln882x"
	echo -e "EGG:"
	echo -e "	./build.sh linux "
	echo -e "	./build.sh esp8266"
    echo -e "	./build.sh esp32"
	echo -e "	./build.sh tr6260"
    echo -e "	./build.sh bl602"
	echo -e "	./build.sh l610"
	echo -e "	./build.sh rtl8710"
	echo -e "	./build.sh Rexense7628"
	echo -e "	./build.sh ln882x"
	echo -e "***********************************************************"
}

function ln_ToolChain_all(){

	if [ "esp8266" == $BUILD_PORT ]; then
		export IDF_PATH=/opt/esp_sdk/ESP8266_RTOS_SDK
	elif [ "esp32" == $BUILD_PORT ]; then
		export IDF_PATH=/opt/esp_sdk/esp-idf
	elif [ "l610" == $BUILD_PORT ]; then
		export PATH=${PATH}:/opt/l610/prebuilts/linux/gcc-arm-none-eabi/bin
	elif [ "tr6260" == $BUILD_PORT ]; then
		export PATH=${PATH}:/opt/toolchain/nds32le-elf-mculib-v3/bin
	elif [ "rtl8710" == $BUILD_PORT ]; then
		export PATH=${PATH}:/opt/toolchain/asdk-6.5.0/linux/newlib/bin
	elif [ "mt7688" == $BUILD_PORT ]; then
		export PATH=${PATH}:/opt/toolchain/openwrt-linux-ramips-mt7688/staging_dir/toolchain-mipsel_24kec+dsp_gcc-4.8-linaro_glibc-2.19/bin
	elif [ "Rexense7628"==$BUILD_PORT];then
	    export PATH=${PATH}:/home/yangweifeng9/toolchain/1806/toolchain-mipsel_24kc_gcc-7.3.0_musl/bin 
	elif [ "ln882x" == $BUILD_PORT ]; then
		export PATH=${PATH}:/opt/toolchain/ln882x/gcc-arm-none-eabi-10-2020-q4-major/bin
	fi

	filelist=${SUBDIRS}
	for file in $filelist
	do
        cd ${LOC_PWD}/${file}/
        rm ToolChain.cmake
        cp ${LOC_PWD}/build_conf/${BUILD_PORT}/ToolChain.cmake ToolChain.cmake
	done
}

function cleanall() {
	filelist=${SUBDIRS}
	for file in $filelist
	do
		echo "---clean ${file} now---"
        cd ${LOC_PWD}/${file}/build
        make clean
        cd ..
        rm -rf build
		echo "---clean ${file} end---"
	done
}

function buildone() {
	echo "build ${LOC_Name} now"
    if [ -e ${LOC_PWD}/${LOC_Name}/build ];then
        echo "---------"
    else
        mkdir ${LOC_PWD}/${LOC_Name}/build
        cd ${LOC_PWD}/${LOC_Name}/build
        cmake ..
    fi

    cd ${LOC_PWD}/${LOC_Name}/build
        make
        
    echo "---------"
    echo "build ${LOC_Name} end"
	
	# 更新头文件和库文件
	cp ${LOC_PWD}/${LOC_Name}/../lib/$BUILD_PORT/*.a ${LOC_PWD}/lib/$BUILD_PORT

	if [ 0 != "$?" ];then
		exit 1
	fi
}

function buildall() {
	rm -rf ${LOC_PWD}/inc
	rm -rf ${LOC_PWD}/lib/$BUILD_PORT
	mkdir -p ${LOC_PWD}/inc
	mkdir -p ${LOC_PWD}/lib/$BUILD_PORT

	# cd ${LOC_PWD}/build_conf/tools
	# python menuconfig.py ../$BUILD_PORT

	filelist=${SUBDIRS}
	echo $filelist
	for file in $filelist
	do
		LOC_Name=$file
		buildone
	done

	export_iot_sdk
}

function export_iot_sdk()
{
    cp ${IOT_SDK_DIR}/inc/*.h ${LOC_PWD}/inc
    cp -r ${IOT_SDK_DIR}/inc/ez_utils/ ${LOC_PWD}/inc
	cp -r ${IOT_SDK_DIR}/inc/ez_hal/ ${LOC_PWD}/inc
	cp -r ${IOT_SDK_COMPONENTS} ${LOC_PWD}/inc

	# 删除旧目录，创建新目录
	rm -rf ${LOC_PWD}/export/${EXPORT_PRE}_${BUILD_PORT}_${lib_version}_revision_${svn_version}
	mkdir -p ${LOC_PWD}/export/${EXPORT_PRE}_${BUILD_PORT}_${lib_version}_revision_${svn_version}
	cp -rf ${LOC_PWD}/inc/. ${LOC_PWD}/export/${EXPORT_PRE}_${BUILD_PORT}_${lib_version}_revision_${svn_version}/inc
	cp -rf ${LOC_PWD}/doc/export/. ${LOC_PWD}/export/${EXPORT_PRE}_${BUILD_PORT}_${lib_version}_revision_${svn_version}/doc
	cp -rf ${LOC_PWD}/unit_test/. ${LOC_PWD}/export/${EXPORT_PRE}_${BUILD_PORT}_${lib_version}_revision_${svn_version}/unit_test
    rm -rf ${LOC_PWD}/export/${EXPORT_PRE}_${BUILD_PORT}_${lib_version}_revision_${svn_version}/unit_test/bin/*
	cp -rf ${LOC_PWD}/unit_test/bin/$BUILD_PORT ${LOC_PWD}/export/${EXPORT_PRE}_${BUILD_PORT}_${lib_version}_revision_${svn_version}/unit_test/bin
	rm -rf ${LOC_PWD}/export/${EXPORT_PRE}_${BUILD_PORT}_${lib_version}_revision_${svn_version}/unit_test/src/port/*
	cp -rf ${LOC_PWD}/unit_test/src/port/$BUILD_PORT ${LOC_PWD}/export/${EXPORT_PRE}_${BUILD_PORT}_${lib_version}_revision_${svn_version}/unit_test/src/port
	cp -rf ${LOC_PWD}/lib/$BUILD_PORT/. ${LOC_PWD}/export/${EXPORT_PRE}_${BUILD_PORT}_${lib_version}_revision_${svn_version}/lib
}

function build_sdk()
{
	echo "Start building ..."
	rm CMakeCache.txt cmake_install.cmake Makefile CMakeFiles -rf
	cmake $1
	if [ -e 'Makefile' ]; then
		make clean
		make
	else
		echo "The makefile build script wasn't been generated successfully!"
		exit 1
	fi
}

function build_unit_test()
{
	rm -rf ./unit_test/bin
	rm -rf ./unit_test/cache

	cd ${LOC_PWD}/unit_test
	./build.sh ${BUILD_PORT}

	rm -rf ./unit_test/src/${BUILD_PORT}/build
}

if [ $# == 0 ]; then
	echo "Please input the correct parameters!"
	usage
	exit 1
else
	echo "input param 1:$1"
	
	svn_version=`svn info | cat -n | awk '{if($1==7)print $3}'`
	echo $svn_version

	lib_version=`svn info | cat -n | awk '{if($1==3)print $3}' |  awk -F'/' '{print $7}'`
	echo $lib_version
	
	# 编译动态库版默认开启，以后可以根据'DYNAMIC'控制关闭编译动态库
	BUILD_PORT=$1
    ln_ToolChain_all
    cleanall
    buildall
	if [ "linux" == $BUILD_PORT ]; then
		build_unit_test
	fi	
	export_iot_sdk
fi
