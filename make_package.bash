#!/bin/bash

VERSION=`cat etc/primal/primal.version`
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
tar -cf primal-$VERSION.tar install.bash install_packages.bash README.txt etc home var
echo "Compressing tarball"
xz --verbose -T $CORES primal-$VERSION.tar
echo "Done"
