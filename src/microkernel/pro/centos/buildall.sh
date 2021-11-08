#!/bin/sh
#make all platform microkerne

svn_version=`svn info | cat -n | awk '{if($1==6)print $3}'`
echo $svn_version

rm -rf ../../lib/centos
rm -rf CMakeCache.txt
cmake -DSVN_VERSION=$svn_version
make clean
make -j 9

