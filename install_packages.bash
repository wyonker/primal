#!/bin/bash

THISPM="yum"

$THISPM install https://dl.fedoraproject.org/pub/epel/epel-release-latest-7.noarch.rpm

$THISPM install -y dsniff.x86_64 iftop.x86_64 strace.x86_64 nmap.x86_64 screen.x86_64
$THISPM install -y libsndfile-devel.x86_64 openssl-devel.x86_64 
#$THISPM install -y libxml.x86_64 libxml-devel.x86_64
$THISPM install -y libxml2.x86_64 libxml2-devel.x86_64 libpng-devel.x86_64 libtiff-devel.x86_64 tcp_wrappers-devel.x86_64 
$THISPM install -y gcc.x86_64 gcc-c++.x86_64 autoconf.noarch make.x86_64
$THISPM install -y mariadb.x86_64 mariadb-server.x86_64 mariadb-devel.x86_64 httpd.x86_64 httpd-tools.x86_64 
$THISPM install -y php-pdo.x86_64 php-pear.noarch php-pecl-jsonc.x86_64 php-process.x86_64 php-xml.x86_64 
$THISPM install -y php.x86_64 php-cli.x86_64 php-common.x86_64 php-gd.x86_64 php-mysqlnd.x86_64 
$THISPM install -y t1lib.x86_64 libwmf-lite ilmbase 
$THISPM install -y mysql++.x86_64 mysql++-devel.x86_64 
$THISPM install -y ImageMagick.x86_64 ImageMagick-devel.x86_64 ImageMagick-doc.x86_64 ImageMagick-c++.x86_64 ImageMagick-c++-devel.x86_64 OpenEXR-libs 
$THISPM install -y java-1.8.0-openjdk.x86_64 java-1.8.0-openjdk-devel.x86_64 java-1.8.0-openjdk-headless.x86_64 
$THISPM install -y vim-common.x86_64 vim-enhanced.x86_64 git.x86_64 awscli telnet
