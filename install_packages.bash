#!/bin/bash

THISPM="yum"

$THISPM install https://dl.fedoraproject.org/pub/epel/epel-release-latest-7.noarch.rpm
$THISPM install -y 
$THISPM install -y install 
$THISPM install -y dsniff.x86_64 
$THISPM install -y iftop.x86_64 
$THISPM install -y libsndfile-devel.x86_64 
$THISPM install -y libxml.x86_64 libxml-devel.x86_64 libxml2-devel.x86_64 
$THISPM install -y libpng-devel.x86_64 
$THISPM install -y libtiff-devel.x86_64 
$THISPM install -y openssl-devel.x86_64 
$THISPM install -y gcc.x86_64 $THISPM install -y gcc-c++.x86_64 
$THISPM install -y strace.x86_64 
$THISPM install -y tcp_wrappers-devel.x86_64 
$THISPM install -y mariadb.x86_64 mariadb-server.x86_64 mariadb-devel.x86_64 
$THISPM install -y php.x86_64 php-cli.x86_64 php-common.x86_64 php-gd.x86_64 
$THISPM install -y php-mysqlnd.x86_64 
$THISPM install -y httpd.x86_64 httpd-tools.x86_64 
$THISPM install -y php-pdo.x86_64 
$THISPM install -y php-pear.noarch 
$THISPM install -y php-pecl-jsonc.x86_64 
$THISPM install -y php-process.x86_64 
$THISPM install -y t1lib.x86_64 
$THISPM install -y php-xml.x86_64 
$THISPM install -y mysql++.x86_64 mysql++-devel.x86_64 
$THISPM install -y screen.x86_64 
$THISPM install -y ImageMagick ImageMagick-libs 
$THISPM install -y OpenEXR-libs 
$THISPM install -y ilmbase 
$THISPM install -y libwmf-lite 
$THISPM install -y java-1.8.0-openjdk.x86_64 java-1.8.0-openjdk-devel.x86_64 java-1.8.0-openjdk-headless.x86_64 
$THISPM install -y vim-common.x86_64 vim-enhanced.x86_64 
$THISPM install -y git.x86_64
