#!/bin/bash
#version 2
#2022-07-06

VERSION=`cat etc/primal/primal.version|head -1|cut -d '=' -f2`
CORES=`cat /proc/cpuinfo |grep "^processor"|wc -l`
CORES=`echo "$CORES-1"|bc -l`

if [ -e "./primal-$VERSION.tar" ]
then
	rm -f ./primal-$VERSION.tar
fi

if [ -e "./primal-$VERSION.tar.xz" ]
then
	rm -f ./primal-$VERSION.tar.xz
fi

echo "Creating tarball"
tar -cf primal-$VERSION.tar install.bash install_packages.bash install_packages_rhel8.bash README.txt etc home var dcmtk-3.6.5.tar.gz
echo "Compressing tarball"
xz --verbose -T $CORES primal-$VERSION.tar
echo "Done"
