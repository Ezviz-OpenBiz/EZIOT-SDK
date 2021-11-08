#!/bin/bash
#By zhangyi

PRJ_PWD=$PWD
LOC_PWD=${PRJ_PWD}/src
IOT_COMPONENTS=${PRJ_PWD}/src/components
DYNAMIC_SWITCH=int0
BUILD_PORT=''

function usage()
{
	echo -e "***********************************************************"
	echo -e "USAGE:" 
	echo -e "	./build.sh \${port}"
	echo -e ""
	echo -e "port:" 
	echo -e "            linux"
	echo -e "            esp8266"
    echo -e "            esp32"
	echo -e "EGG:"
	echo -e "	./build.sh linux"
	echo -e "	./build.sh esp8266"
    echo -e "	./build.sh esp32"
	echo -e "***********************************************************"
}

function clean_build() {
    echo "---clean_build now---"
    # rm -rf cmake_build
    echo "---clean_build end---"
}

function build_do() {
    echo "build do now"

	rm -rf ${PRJ_PWD}/bin/${BUILD_PORT}
	mkdir -p ${PRJ_PWD}/bin/${BUILD_PORT}

	if [ "linux" == ${BUILD_PORT} ]; then
		mkdir -p ${PRJ_PWD}/src/port/${BUILD_PORT}/build
    	cd ${PRJ_PWD}/src/port/${BUILD_PORT}/build
    	cmake ..
		make
		
		cp -f ${PRJ_PWD}/src/port/${BUILD_PORT}/build/ut_iotsdk ${PRJ_PWD}/bin/${BUILD_PORT}/
	elif [ "esp8266" == ${BUILD_PORT} ]; then
		cd ${PRJ_PWD}/src/port/${BUILD_PORT}
		make -j5

		cp -f ${PRJ_PWD}/src/port/${BUILD_PORT}/build/ut_iotsdk.bin ${PRJ_PWD}/bin/${BUILD_PORT}/
		cp -f ${PRJ_PWD}/src/port/${BUILD_PORT}/build/ota_data_initial.bin ${PRJ_PWD}/bin/${BUILD_PORT}/
		cp -f ${PRJ_PWD}/src/port/${BUILD_PORT}/build/partitions.bin ${PRJ_PWD}/bin/${BUILD_PORT}/
		cp -f ${PRJ_PWD}/src/port/${BUILD_PORT}/build/bootloader/bootloader.bin ${PRJ_PWD}/bin/${BUILD_PORT}/
	elif [ "esp32" == ${BUILD_PORT} ]; then
		cd ${PRJ_PWD}/src/port/${BUILD_PORT}
		make -j5

		cp -f ${PRJ_PWD}/src/port/${BUILD_PORT}/build/ut_iotsdk.bin ${PRJ_PWD}/bin/${BUILD_PORT}/
		cp -f ${PRJ_PWD}/src/port/${BUILD_PORT}/build/ota_data_initial.bin ${PRJ_PWD}/bin/${BUILD_PORT}/
		cp -f ${PRJ_PWD}/src/port/${BUILD_PORT}/build/partitions.bin ${PRJ_PWD}/bin/${BUILD_PORT}/
		cp -f ${PRJ_PWD}/src/port/${BUILD_PORT}/build/bootloader/bootloader.bin ${PRJ_PWD}/bin/${BUILD_PORT}/
    elif [ "bl602" == ${BUILD_PORT} ]; then
        cd ${PRJ_PWD}/src/port/bl602_app
        ./genromap

        cp -f ${PRJ_PWD}/src/port/bl602_app/build_out/bl602_app.bin ${PRJ_PWD}/bin/${BUILD_PORT}/
	# elif [ "xxxxxxxxx" == ${BUILD_PORT} ]; then
		# add your code
	else
		echo "error occured, port not surport!!!"
		exit 2
	fi

    echo "build do end"
}

if [ $# == 0 ]; then
	echo "Please input the correct parameters!"
	usage
	exit 1
else
	echo "This is release mode!"
	echo "input param 1:$1"

	svn_version=`svn info | cat -n | awk '{if($1==6)print $3}'`
	echo $svn_version

	lib_version=`svn info | cat -n | awk '{if($1==3)print $3}' |  awk -F'/' '{print $10}'`
	echo $lib_version
	
	# 编译动态库版默认开启，以后可以根据'DYNAMIC'控制关闭编译动态库
	BUILD_PORT=$1
    clean_build
    build_do
fi
