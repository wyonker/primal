#!/bin/bash
clear
USERNAME=`whoami`
if [ "$USERNAME" != "root" ]
then
	echo "You must be root to run this script.  Exiting."
	exit 1
fi

echo; echo; echo
echo "Cleaning up any previous builds..."
sleep 3
make clean


echo; echo; echo
echo "Copying new source files to the correct locations..."
sleep 3
cp -v ../source/*.cc dcmnet/apps/

echo; echo; echo
echo "Running configure"
sleep 3
./configure --prefix=/home/dicom --with-opensslinc=/usr/include/openssl --with-libzlibinc=/usr/include --enable-threads=auto --enable-lfs=auto --with-private-tags --with-openssl --with-zlib --with-libtiff --with-libtiffinc=/usr/lib64 --with-libpng --with-libpnginc=/usr/include --with-libxml --with-libxmlinc=/usr/include/libxml2/libxml --with-libsndfile --with-libsndfileinc=/usr/lib64 --with-libwrap

echo; echo; echo
echo "Modifying Makefile.def"
sleep 3
CXXSTAT=`cat ./config/Makefile.def|grep "^CXXFLAGS"|grep mysql|wc -l`
CXXVAL=`cat ./config/Makefile.def|grep "^CXXFLAGS"`
if [ $CXXSTAT -lt 1 ]
then
	echo "   Modifying CXXFLAGS..."
	CXXNEWVAL=`echo "$CXXVAL -I/usr/include/mysql"`
	sed -i "s!$CXXVAL!$CXXNEWVAL!g" config/Makefile.def
fi

CPPSTAT=`cat ./config/Makefile.def|grep "^CPPFLAGS"|grep mysql|wc -l`
CPPVAL=`cat ./config/Makefile.def|grep "^CPPFLAGS"`
if [ $CPPSTAT -lt 1 ]
then
	echo "   Modifying CPPFLAGS..."
	CPPNEWVAL=`echo "$CPPVAL -I/usr/include/mysql"`
	sed -i "s!$CPPVAL!$CPPNEWVAL!g" config/Makefile.def
fi

LDSTAT=`cat ./config/Makefile.def|grep "^LDFLAGS"|grep mysql|wc -l`
LDVAL=`cat ./config/Makefile.def|grep "^LDFLAGS"`
if [ $LDSTAT -lt 1 ]
then
	echo "   Modifying LDFLAGS..."
	LDNEWVAL=`echo "$LDVAL -L/usr/lib64/mysql"`
	sed -i "s!$LDVAL!$LDNEWVAL!g" config/Makefile.def
fi

LIBSTAT=`cat ./config/Makefile.def|grep "^LIBS ="|grep mysql|wc -l`
LIBVAL=`cat ./config/Makefile.def|grep "^LIBS ="`
if [ $LIBSTAT -lt 1 ]
then
	echo "   Modifying LIBS..."
	LIBNEWVAL=`echo "$LIBVAL -lmysqlpp -lmysqlclient"`
	sed -i "s!$LIBVAL!$LIBNEWVAL!g" config/Makefile.def
fi

echo; echo; echo
echo "Building from source"
sleep 3
make all
exit
