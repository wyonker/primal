#!/bin/bash

THISPM="dnf"

$THISPM install -y https://dl.fedoraproject.org/pub/epel/epel-release-latest-8.noarch.rpm

subscription-manager repos --enable=codeready-builder-for-rhel-8-x86_64-rpms

$THISPM install -y gcc-toolset-11-gcc.x86_64 gcc-toolset-11-binutils.x86_64 gcc-toolset-11-binutils-devel.x86_64 gcc-toolset-11-build.x86_64 gcc-toolset-11-gcc-c++.x86_64 gcc-toolset-11-gcc-gdb-plugin.x86_64 gcc-toolset-11-gdb.x86_64 gcc-toolset-11-gdb-gdbserver.x86_64 gcc-toolset-11-libstdc++-devel.x86_64 gcc-toolset-11-make.x86_64 gcc-toolset-11-strace.x86_64

LIST="cmake.x86_64 dsniff.x86_64 iftop.x86_64 strace.x86_64 nmap.x86_64 screen.x86_64 libsndfile-devel.x86_64 openssl-devel.x86_64 libxml.x86_64 libxml-devel.x86_64 libxml2.x86_64 libxml2-devel.x86_64 libpng-devel.x86_64 libtiff-devel.x86_64 tcp_wrappers-devel.x86_64 autoconf.noarch make.x86_64 mariadb.x86_64 mariadb-server.x86_64 mariadb-devel.x86_64 httpd.x86_64 httpd-tools.x86_64 php php-pdo.x86_64 php-pear.noarch php-pecl-jsonc.x86_64 php-process.x86_64 php-xml.x86_64 php.x86_64 php-cli.x86_64 php-common.x86_64 php-gd.x86_64 php-mysqlnd.x86_64 t1lib.x86_64 libwmf-lite ilmbase mysql++.x86_64 mysql++-devel.x86_64 ImageMagick.x86_64 ImageMagick-devel.x86_64 ImageMagick-doc.x86_64 ImageMagick-c++.x86_64 ImageMagick-c++-devel.x86_64 OpenEXR-libs java-1.8.0-openjdk.x86_64 java-1.8.0-openjdk-devel.x86_64 java-1.8.0-openjdk-headless.x86_64 vim-common.x86_64 vim-enhanced.x86_64 git awscli telnet libcurl-devel libssh-devel pigz.x86_64 bind-utils openjpeg2.x86_64 openjpeg2-devel.x86_64 libjpeg-turbo-devel.x86_64 mariadb.x86_64 mariadb-common.x86_64 mariadb-server.x86_64 mariadb-server-utils.x86_64 mariadb-connector-c.x86_64 mariadb-connector-c-devel.x86_6" 
for i in $LIST
do
	$THISPM list installed $i > /dev/null 2>&1
	if [ $? -eq 0 ]
	then
		echo "Already On system:  $i"
	else
		$THISPM install -y $i >/dev/null 2>&1
		if [ $? -ne 0 ]
		then
			echo "Not Installed:  $i"
		else
			echo "Installed:  $i"
		fi
	fi
done

exit 0