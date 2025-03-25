#!/bin/bash
#Version 15
#2025-03-24
# Author:  Will Yonker
# License GPLv3

CURDIR=`pwd`
ISNEW=3

clear
USERNAME=`whoami`
if [ "$USERNAME" != "root" ]
then
    echo "You must be root to run this script.  Exiting."
    exit 1
fi

echo "Checking SELinux status"
	ISENABLED=`/usr/sbin/getenforce 2>&1`
	if [ "$ISENABLED" != "Disabled" ]
	then
		echo "SELinux is enabled.  Disabling..."
		setenforce 0
	else
		echo "SELinux is dsiabled.  Good."
	fi
	ISENABLED=`cat /etc/selinux/config|grep "^SELINUX="|cut -d "=" -f2|tr '[:upper:]' '[:lower:]'`
	if [ "$ISENABLED" != "disabled" ]
	then
		echo "SELinux is not disabled in the configuration file.  Disabling..."
		cd /etc/selinux; sed -i 's/=enforcing/=disabled/g' config; cd $CURDIR
	else
		echo "SELinux is not configured.  Good."
	fi
	sleep 1

OSVER=0
TEMPOSVER=`grep -c "CentOS Linux release 7" /etc/redhat-release`
if [ $TEMPOSVER -gt 0 ]
then
	echo "CentOS 7 found."
	OSVER=1
fi
TEMPOSVER=`grep -c "Red Hat Enterprise Linux release 7" /etc/redhat-release`
if [ $TEMPOSVER -gt 0 ]
then
	echo "RHEL 7 found."
	OSVER=1
fi
TEMPOSVER=`grep -c "Red Hat Enterprise Linux release 8" /etc/redhat-release`
if [ $TEMPOSVER -gt 0 ]
then
	echo "RHEL 8 found."
	OSVER=8
fi
TEMPOSVER=`grep -c "Red Hat Enterprise Linux release 9" /etc/redhat-release`
if [ $TEMPOSVER -gt 0 ]
then
	echo "RHEL 9 found."
	OSVER=9
fi
TEMPOSVER=`grep -c "AlmaLinux release 9" /etc/redhat-release`
if [ $TEMPOSVER -gt 0 ]
then
	echo "AlmaLinux 9 found."
	OSVER=2
fi
sleep 3
if [ $OSVER -lt 1 ]
then
	echo "This script is for CentOS 7, RHEL 7, RHEL 8, , RHEL 9 or AlmaLinux 9.  Exiting..."
	exit 1
fi

echo "Installing any packages that are missing."
if [ $OSVER -eq 1 ]
then
	./install_packages.bash
else
	./install_packages_rhel8.bash
fi
sleep 1
if [ ! -e "/usr/include/libssh.h" ]
then
	echo "/usr/linclude/libssh.h not found where we need it.  Copying..."
	find / -iname "libssh.h" -type f -exec cp {}  /usr/include/ \;
fi

if [ $OSVER -eq 1 ]
then
	echo "Enabling devtoolset-8"
	source scl_source enable devtoolset-8

	echo "Setting devtooset-8 as the default"
	ISTHERE=`grep -c "source scl_source enable devtoolset-8" /root/.bashrc`
	if [ $ISTHERE -lt 1 ]
	then
		echo "source scl_source enable devtoolset-8" >> /root/.bashrc
	else
		echo "devtoolset-8 is already the default."
	fi
	sleep 1
fi

echo "Finding g++"
GPATH="0"
LIST=`find / -name "g++" -type f`
for i in $LIST
do
	THISVERSION=`$i -v 2>&1|tail -1|cut -d " " -f3|cut -d '.' -f1`
	if [ $THISVERSION -ge 8 ]
	then
		GPATH=$i
		echo "Using $i"
	fi
done
if [ "$GPATH" == "0" ]
then
	echo "ERROR:  No g++ found that is version 8 or higher.  Exiting..."
	exit 1
fi

echo "Installing pstreams library."
	git clone git://git.code.sf.net/p/pstreams/code pstreams
	mkdir -p /usr/include/pstreams
	cp pstreams/pstream.h  /usr/include/pstreams/
	sleep 1

echo "Restarting MariaDB"
	systemctl enable mariadb.service
	systemctl restart mariadb.service
	sleep 1

echo "Checking for PRIMAL directory in /etc"
if [ ! -d "/etc/primal" ]
then
    echo "Creating /etc/primal directory"
    mkdir /etc/primal
    sleep 1
    if [ ! -d "/etc/primal" ]
    then
        echo "ERROR:  Could not create the /etc/primal directory.  Exiting."
        exit 1
    fi
else
    echo "Good, directory exists."
	sleep 1
fi

if [ ! -e "/etc/primal/primal.conf" ]
then
    echo "Creating sample primal.conf file"
    cp etc/primal/primal.conf /etc/primal/
	ISNEW=2
    if [ ! -e "/etc/primal/primal.conf" ]
    then
        echo "ERROR:  Could not create /etc/primal/primal.conf file.  Exiting."
        exit 1
    fi
else
    echo "Good, file exists too."
	ISNEW=1
	sleep 1
fi

if [ ! -e "/etc/primal/primal.db" ]
then
    echo "Creating sample primal.db file"
    cp etc/primal/primal.db /etc/primal/
	ISNEW=2
    if [ ! -e "/etc/primal/primal.db" ]
    then
        echo "ERROR:  Could not create /etc/primal/primal.db file.  Exiting."
        exit 1
    fi
else
    echo "Good, file exists too."
	ISNEW=1
	sleep 1
fi

if [ -e "/etc/primal/primal.version" ]
then
	rm -f /etc/primal/primal.version
fi
cp etc/primal/primal.version /etc/primal/
echo "Changing ownership and permissions on /etc/primal to apache (for web edits)"
	chown apache.apache -R /etc/primal
	chmod 755 -R /etc/primal

if [ $OSVER -ge 8]
then
	systemctl enable rc-local.service
	systemctl start rc-local.service
fi

if [ ! -e "/etc/rc.d/rc.local" ]
then
	echo "rc.local file does not exist.  Creating it."
	touch /etc/rc.d/rc.local
	echo "#!/bin/bash" >> /etc/rc.d/rc.local
	sleep 1
	if [ ! -e "/etc/rc.d/rc.local" ]
	then
		echo "rc.local files STILL does not exist.  Exiting..."
		exit 1
	fi
else
	echo "Good, rc.local file exists."
	sleep 1
fi
chmod 755 /etc/rc.d/rc.local
systemctl start rc-local
systemctl enable rc-local

ISINSTARTUP=`cat /etc/rc.d/rc.local|grep "/home/dicom/startup.bash start"|wc -l`
if [ $ISINSTARTUP -lt 1 ]
then
	echo "Inserting PRIMAL startup script..."
	echo "ulimit -q 3276800" >> /etc/rc.d/rc.local
	echo "sleep 60" >> /etc/rc.d/rc.local
	echo "/home/dicom/startup.bash start ALL" >> /etc/rc.d/rc.local
	ISINSTARTUP=`cat /etc/rc.d/rc.local|grep "/home/dicom/startup.bash start ALL"|wc -l`
	if [ $ISINSTARTUP -gt 0 ]
	then
		echo "Added to rc.local file."
		sleep 1
	else
		echo "Error:  Could not add to the rc.local file."
		exit 1
	fi
fi

if [ ! -e "/var/spool/cron/root" ]
then
	echo "Root's crontab file does not exist.  Creating it..."
	touch /var/spool/cron/root
	sleep 1
	if [ ! -e "/var/spool/cron/root" ]
	then
		echo "Error:  Root's crontab STILL doesn't exist.  Exiting..."
		exit 1
	fi
else
	echo "Good, Root's crontab exists."
	sleep 1
fi

ISINCRONTAB=`cat /var/spool/cron/root|grep age.bash|wc -l`
if [ $ISINCRONTAB -lt 1 ]
then
	echo "Inserting age.bash into Root's crontab file."
	echo '1 * * * * /home/dicom/age.bash >> /var/log/age_dicom.log 2>&1' >> /var/spool/cron/root
	ISINCRONTAB=`cat /var/spool/cron/root|grep age.bash|wc -l`
	if [ $ISINCRONTAB -lt 1 ]
	then
		echo "Error:  Could not insert age.bash into Root's crontab file.  Exiting..."
		exit 1
	else
		echo "Good, age.bash is in Root's crontab file."
		sleep 1
	fi
else
	echo "Good, age.bash is in Root's crontab file."
	sleep 1
fi

ISINCRONTAB=`cat /var/spool/cron/root|grep rec_check|wc -l`
if [ $ISINCRONTAB -lt 1 ]
then
	echo "Inserting rec_check into Root's crontab file."
	echo '* * * * *  /home/dicom/bin/rec_check >> /dev/null 2>&1' >> /var/spool/cron/root
	ISINCRONTAB=`cat /var/spool/cron/root|grep rec_check|wc -l`
	if [ $ISINCRONTAB -lt 1 ]
	then
		echo "Error:  Could not insert rec_check into Root's crontab file.  Exiting..."
		exit 1
	else
		echo "Good, rec_check is in Root's crontab file."
		sleep 1
	fi
else
	echo "Good, rec_check is in Root's crontab file."
	sleep 1
fi

sleep 3
echo "Copying PRIMAL software to /home"
	if [ ! -e "/home" ]
	then
		mkdir -p /home/dicom/bin
	fi
	if [ ! -e "/home/dicom" ]
	then
		mkdir -p /home/dicom/bin
	fi
	if [ ! -e "/home/dicom/bin" ]
	then
		mkdir /home/dicom/bin
	fi
	if [ ! -e "/home/dicom/share" ]
	then
		mkdir /home/dicom/share
	fi
	rm -f /home/dicom/*.bash
	cp -p home/dicom/*.bash /home/dicom/
	rm -f /home/dicom/bin/*
	cp -p home/dicom/bin/* /home/dicom/bin/
	#rm -fr /home/dicom/share/dcmtk
	#cp -pr home/build/share/* /home/dicom/share/
	chown apache.apache -R /home/dicom
	chmod 777 -R /home/dicom
	if [ ! -e "/home/dicom/logs" ]
	then
		mkdir /home/dicom/logs
		chown apache.apache -R /home/dicom/logs
	fi
	if [ ! -e "/home/dicom/inbound" ]
	then
		mkdir /home/dicom/inbound
		chown apache.apache -R /home/dicom/inbound
	fi
	if [ ! -e "/home/dicom/processing" ]
	then
		mkdir /home/dicom/processing
		chown apache.apache -R /home/dicom/processing
	fi
	if [ ! -e "/home/dicom/outbound" ]
	then
		mkdir /home/dicom/outbound
		chown apache.apache -R /home/dicom/outbound
	fi
	if [ ! -e "/home/dicom/sent" ]
	then
		mkdir /home/dicom/sent
		chown apache.apache -R /home/dicom/sent
	fi
	if [ ! -e "/home/dicom/hold" ]
	then
		mkdir /home/dicom/hold
		chown apache.apache -R /home/dicom/hold
	fi
	if [ ! -e "/home/dicom/error" ]
	then
		mkdir /home/dicom/error
		chown apache.apache -R /home/dicom/error
	fi
	mkdir -p /home/dicom/share/dcmtk
	cp -pvr home/build/share/dcmtk/* /home/dicom/share/dcmtk/
	mkdir -p /usr/local/share/dcmtk
	cp -pvr /home/dicom/share/dcmtk/dicom.dic /usr/local/share/dcmtk/dicom.dic

echo "Copying dcm4che to /home"
	if [ ! -e "/home/dcm4che" ]
	then
		mkdir /home/dcm4che
	fi
	rm -fr /home/dcm4che/*
	cp -pr home/dcm4che /home/

echo "Installing web componet"
	if [ ! -e "/var/www/html/primal" ]
	then
		mkdir /var/www/html/primal
		chown apache.apache /var/www/html/primal
	fi
	rm -fr /var/www/html/primal/*
	cp -p var/www/html/primal/* /var/www/html/primal/
	if [ ! -e "/var/www/html/primal/tmp" ]
	then
		mkdir /var/www/html/primal/tmp
	fi
	chown apache.apache -R /var/www/html/*
	chmod 777 -R /var/www/html/*

rm -f home/build/dcmnet/apps/storescp
rm -f home/build/dcmnet/apps/storescu

echo "Searching for the DCMTK package."
NUMDCMTK=`ls -1 dcmtk*.tar.gz 2>/dev/null|wc -l`
if [ $NUMDCMTK -lt 1 ]
then
	echo "DCMTK package not found.  Please download it and place it in the primal install directory.  Exiting..."
	exit 1
elif [ $NUMDCMTK -gt 1 ]
then
	echo "Multiple DCMTK packages found.  Please remove all but the most current one.  Exiting..."
	exit 1
fi

DCMTK=`ls -1 dcmtk*.tar.gz 2>/dev/null|tail -1`
echo "$DCMTK found.  Do you wish to use this version?."
read USER_INPUT
USER_INPUT=`echo "$USER_INPUT"|tr '[:upper:]' '[:lower:]'`
if [ "$USER_INPUT" == "yes" ]
then
	DCMTK2=`echo $DCMTK|sed 's/.tar.gz//g'`
	tar -xf $DCMTK
	if [ -e "$DCMTK2-build" ]
	then
		rm -fr "$DCMTK2-build"
	fi
	mkdir "$DCMTK2-build"
	cd "$DCMTK2-build"
	#cmake -DCMTK_CXX11_FLAGS:STRING=-std=c++17 -DCMTK_ENABLE_STL:STRING=ON -DCMTK_ENABLE_CXX11:STRING=INFERRED -DCMTK_ENABLE_PRIVATE_TAGS:STRING=ON ../dcmtk-3.6.5
	cmake -D CMAKE_CXX_STANDARD=20 -D DCMTK_ENABLE_BUILTIN_OFICONV_DATA:BOOL=ON -D DCMTK_ENABLE_PRIVATE_TAGS=ON -D DCMTK_ENABLE_STL=ON -D DCMTK_WITH_PNG=ON -D DCMTK_WITH_TIFF=ON -D DCMTK_WITH_XML=ON -D BUILD_SHARED_LIBS:BOOL=ON
	make -j4 install
	cd $CURDIR
else
	echo "Please download the DCMTK and make sure it's in the path.  Press any key to continue..."
	read USER_INPUT
fi

echo "Compiling PRIMAL services for this platform"
	cd home/source
	if [ -e prim_receive_server ]
	then
		rm -f prim_receive_server
	fi
	if [ -e prim_send_server ]
	then
		rm -f prim_send_server
	fi
	if [ -e prim_qr_server ]
	then
		rm -f prim_qr_server
	fi
	if [ -e prim_process_server ]
	then
		rm -f prim_process_server
	fi
	if [ -e prim_store_server ]
	then
		rm -f prim_store_server
	fi
	./build_primal_processes.bash "$GPATH"
	systemctl stop prim_receive_server
	systemctl stop prim_send_server
	systemctl stop prim_store_server
	systemctl stop prim_process_server
	systemctl stop prim_qr_server
	mv -f prim_receive_server /usr/local/bin/
	mv -f prim_send_server /usr/local/bin/
	mv -f prim_send_worker /usr/local/bin/
	mv -f prim_process_server /usr/local/bin/
	mv -f prim_qr_server /usr/local/bin/
	mv -f prim_store_server /usr/local/bin/
	cd $CURDIR
	if [ -e "/etc/systemd/system/prim_qr_server.service" ]
	then
		rm -f /etc/systemd/system/prim_qr_server.service
	fi
	echo "[Unit]" > /etc/systemd/system/prim_qr_server.service
	echo "Description = PRIMAL Query/Retrieve service" >> /etc/systemd/system/prim_qr_server.service
	echo "After = network.target" >> /etc/systemd/system/prim_qr_server.service
	echo "" >> /etc/systemd/system/prim_qr_server.service
	echo "[Service]" >> /etc/systemd/system/prim_qr_server.service
	echo "ExecStart = /usr/local/bin/prim_qr_server" >> /etc/systemd/system/prim_qr_server.service
	echo "Restart=always" >> /etc/systemd/system/prim_qr_server.service
	echo "" >> /etc/systemd/system/prim_qr_server.service
	echo "[Install]" >> /etc/systemd/system/prim_qr_server.service
	echo "WantedBy = multi-user.target" >> /etc/systemd/system/prim_qr_server.service
    systemctl daemon-reload
	systemctl start prim_qr_server.service
	systemctl enable prim_qr_server.service

	if [ -e "/etc/systemd/system/prim_receive_server.service" ]
	then
		rm -f /etc/systemd/system/prim_receive_server.service
	fi
	echo "[Unit]" > /etc/systemd/system/prim_receive_server.service
	echo "Description = PRIMAL receive service" >> /etc/systemd/system/prim_receive_server.service
	echo "After = network.target" >> /etc/systemd/system/prim_receive_server.service
	echo "" >> /etc/systemd/system/prim_receive_server.service
	echo "[Service]" >> /etc/systemd/system/prim_receive_server.service
	echo "ExecStart = /usr/local/bin/prim_receive_server" >> /etc/systemd/system/prim_receive_server.service
	echo "Restart=always" >> /etc/systemd/system/prim_receive_server.service
	echo "" >> /etc/systemd/system/prim_receive_server.service
	echo "[Install]" >> /etc/systemd/system/prim_receive_server.service
	echo "WantedBy = multi-user.target" >> /etc/systemd/system/prim_receive_server.service
    systemctl daemon-reload
	systemctl start prim_receive_server.service
	systemctl enable prim_receive_server.service

	if [ -e "/etc/systemd/system/prim_send_server.service" ]
	then
		rm -f /etc/systemd/system/prim_send_server.service
	fi
	echo "[Unit]" > /etc/systemd/system/prim_send_server.service
	echo "Description = PRIMAL send service" >> /etc/systemd/system/prim_send_server.service
	echo "After = network.target" >> /etc/systemd/system/prim_send_server.service
	echo "" >> /etc/systemd/system/prim_send_server.service
	echo "[Service]" >> /etc/systemd/system/prim_send_server.service
	echo "ExecStart = /usr/local/bin/prim_send_server" >> /etc/systemd/system/prim_send_server.service
	echo "Restart=always" >> /etc/systemd/system/prim_send_server.service
	echo "" >> /etc/systemd/system/prim_send_server.service
	echo "[Install]" >> /etc/systemd/system/prim_send_server.service
	echo "WantedBy = multi-user.target" >> /etc/systemd/system/prim_send_server.service
    systemctl daemon-reload
	systemctl start prim_send_server.service
	systemctl enable prim_send_server.service

	if [ -e "/etc/systemd/system/prim_process_server.service" ]
	then
		rm -f /etc/systemd/system/prim_process_server.service
	fi
	echo "[Unit]" > /etc/systemd/system/prim_process_server.service
	echo "Description = PRIMAL process service" >> /etc/systemd/system/prim_process_server.service
	echo "After = network.target" >> /etc/systemd/system/prim_process_server.service
	echo "" >> /etc/systemd/system/prim_process_server.service
	echo "[Service]" >> /etc/systemd/system/prim_process_server.service
	echo "ExecStart = /usr/local/bin/prim_process_server" >> /etc/systemd/system/prim_process_server.service
	echo "Restart=always" >> /etc/systemd/system/prim_process_server.service
	echo "" >> /etc/systemd/system/prim_process_server.service
	echo "[Install]" >> /etc/systemd/system/prim_process_server.service
	echo "WantedBy = multi-user.target" >> /etc/systemd/system/prim_process_server.service
    systemctl daemon-reload
	systemctl start prim_process_server.service
	systemctl enable prim_process_server.service

    systemctl daemon-reload

#echo "Please check if there are errors at the end for storescu.o or storescp.o. Type 'yes' to continue"
#	read USER_INPUT
#	USER_INPUT=`echo "$USER_INPUT"|tr '[:upper:]' '[:lower:]'`
#	while [ "$USER_INPUT" != "yes" ]
#	do
#		USER_INPUT=`echo "$USER_INPUT"|tr '[:upper:]' '[:lower:]'`
#	done

#if [ ! -e home/build/dcmnet/apps/storescp ]
#then
#	echo "Error:  storescp did not build properly.  Please run home/build/build.bash manually for more information."
#	exit 1
#fi

#if [ ! -e home/build/dcmnet/apps/storescu ]
#then
#	echo "Error:  storescu did not build properly.  Please run home/build/build.bash manually for more information."
#	exit 1
#fi

ISRUNNING=`ps -ef|grep scp.bash|wc -l`
if [ $ISRUNNING -gt 0 ]
then
	if [ ! -e "/home/dicom/startup.bash" ]
	then
		echo "Error:  PRIMAL startup script does not exist.  Exiting..."
		exit 1
	fi
else
	cd /home/dicom; startup.bash stop
	ISRUNNING=`ps -ef|grep scp.bash|wc -l`
	if [ $ISRUNNING -gt 0 ]
	then
		echo "Error:  scp.bash is STILL running.  Exiting..."
		exit 1
	fi
fi

echo "Checking to see if HL7 software is installed"
HASHL7=`rpm -qa|grep mirthconnect|wc -l`
if [ $HASHL7 -gt 0 ]
then
	echo "Good.  HL7 software is installed."
	sleep 1
else
	echo "HL7 software not found.  Do you want to install? {yes/no}"
	read USER_INPUT
	USER_INPUT=`echo "$USER_INPUT"|tr '[:upper:]' '[:lower:]'`
	if [ "$USER_INPUT" == "yes" ]
	then
		rpm -Uvh home/hl7/packages/*.rpm
		HASHL7=`rpm -qa|grep mirthconnect|wc -l`
		if [ $HASHL7 -lt 1 ]
		then
			echo "Error:  HL7 software could not be installed.  Exiting..."
			exit 1
		fi
		cp home/hl7/packages/mirth.properties /opt/mirthconnect/conf/
		echo "Checking if HL7 database exists."
		HL7DBEXISTS=`echo "show databases"|mysql -u root|grep mirthdb|wc -l`
		if [ $HL7DBEXISTS -gt 0 ]
		then
			echo "HL7 database exists."
			sleep 1
			echo "Checking if PRIMAL has access to the database."
			HASDBACCESS=`echo "SELECT user, Grant_priv FROM mysql.db WHERE Db = 'primalhl7';"|mysql -u root|grep "primal"|tr "\t" " "|cut -d " " -f2`
			if [ "$HASDBACCESS" == "Y" ]
			then
				echo "PRIMAL has access to the database."
				sleep 1
			else
				echo "Granting PRIMAL user access to primalhl7 database"
				echo "grant all privileges on primalhl7.* to 'primal'@'localhost' identified by 'primal' with grant option;"|mysql -u root
				HASDBACCESS=`echo "SELECT user, Grant_priv FROM mysql.db WHERE Db = 'primalhl7';"|mysql -u root|grep "primal"|tr "\t" " "|cut -d " " -f2`
				if [ "$HASDBACCESS" != "Y" ]
				then
					echo "Could not grant access to the primalhl7 database for PRIMAL user.  Exiting..."
					exit 1
				fi
			fi
		fi
	else
		echo "Not installing HL7 software..."
	fi
fi


echo "Checking for existing database..."
HASDB=`echo "show databases"|mysql -u root|grep primal|wc -l`
if [ $HASDB -gt 0 ]
then
	echo "Existing PRIMAL database found.  Backing it up..."
	mysqldump -cael -u primal -pprimal primal > ./primal_backup.sql
	if [ ! -e "./primal_backup.sql" ]
	then
		echo "PRIMAL backup file not found.  Exiting..."
		exit 1
	fi
	echo "Creating backup without schema..."
	mysqldump --no-create-info --skip-triggers --no-create-db --compact -u primal -pprimal primal > ./primal_backup_noschema.sql
	if [ ! -e "./primal_backup_noschema.sql" ]
	then
		echo "PRIMAL no schema backup file not found.  Exiting..."
		exit 1
	else
		echo "Dropping database..."
		echo "drop database primal;"|mysql -u root
		echo "Importing database design..."
		mysql -u root < home/dicom/install/install.sql
		echo "Restoring backup..."
		mysql -u root primal < ./primal_backup_noschema.sql
	fi
else
	echo "Importing database design..."
	mysql -u root < home/dicom/install/install.sql
fi
sleep 5

if [ -e "/home/dicom/bin/storescp" ]
then
	rm -f /home/dicom/bin/storescp
fi
if [ -e "dcmtk-3.6.9-build/bin/storescp" ]
then
	cp dcmtk-3.6.9-build/bin/storescp /home/dicom/bin/
	cp dcmtk-3.6.9/dcmnet/etc/storescp.cfg /home/dicom/bin/
fi

STORESCP=`which storescp 2>/dev/null`
if [ "$STORESCP" == "" ]
then
	echo "Error:  storescp is not found but was detected.  Exiting..."
	exit 1
fi

if [ -e "/home/dicom/bin/dcmdump" ]
then
	rm -f /home/dicom/bin/dcmdump
fi
DCMDUMP=`which dcmdump 2>/dev/null`
if [ "$DCMDUMP" != "" ]
then
	cp $DCMDUMP /home/dicom/bin/
fi

if [ -e "dcmtk-3.6.9-build/bin/dcmdump" ]
then
	cp dcmtk-3.6.9-build/bin/dcmdump /home/dicom/bin/
fi

STORESCU=`which storescu 2>/dev/null`
if [ "$STORESCU" != "" ]
then
	cp $STORESCU /home/dicom/bin/
fi

if [ -e "dcmtk-3.6.9-build/bin/storescu" ]
then
	cp dcmtk-3.6.9-build/bin/storescu /home/dicom/bin/primalscu
	cp dcmtk-3.6.9-build/bin/storescu /home/dicom/bin/
	cp dcmtk-3.6.9/dcmnet/etc/storescu.cfg /home/dicom/bin/
fi

rm /home/dicom/bin/*.cfg
cp dcmtk-3.6.9/dcmnet/etc/*.cfg /home/dicom/bin/

ISINPATH=`cat /root/.bashrc|grep /home/dicom/bin|wc -l`
if [ $ISINPATH -lt 1 ]
then
	echo "Adding /home/dicom/bin to Root's search path"
	echo 'export PATH=/home/dicom/bin:$PATH' >> /root/.bashrc
	export PATH=/home/dicom/bin:$PATH
fi

echo "Check if Apache config files are modified"
	ISMODIFIED=`grep -c "/home/dicom" /etc/httpd/conf/httpd.conf`
	if [ $ISMODIFIED -lt 1 ]
	then
		echo "Adding /home/dicom to Apache configuration file"
		echo '<Directory "/home/dicom">' >> /etc/httpd/conf/httpd.conf
		echo '    AllowOverride None' >> /etc/httpd/conf/httpd.conf
		echo '    Options None' >> /etc/httpd/conf/httpd.conf
		echo '    Require all granted' >> /etc/httpd/conf/httpd.conf
		echo '</Directory>' >> /etc/httpd/conf/httpd.conf
		echo '<Directory "/etc/primal">' >> /etc/httpd/conf/httpd.conf
		echo '    Require all granted' >> /etc/httpd/conf/httpd.conf
		echo '</Directory>' >> /etc/httpd/conf/httpd.conf
	else
		echo "Good, /home/dicom is in the Apache configuration file."
	fi

echo "Checking if apache is in the sudoers file"
	ISINSUDOERS=`cat /etc/sudoers|grep -e apache -e startup.bash|wc -l`
	if [ $ISINSUDOERS -lt 1 ]
	then
		echo "Adding apache to sudoers file"
		chmod 755 /etc/sudoers
		echo "apache ALL=(ALL) NOPASSWD: /home/dicom/startup.bash" >> /etc/sudoers
		chmod 400 /etc/sudoers
	else
		echo "Good apache is in the sudoers file already"
	fi

echo "Modifying Firewalld"
	firewall-cmd --zone=internal --add-service=http
	firewall-cmd --zone=internal --add-port=104/tcp
	firewall-cmd --zone=internal --add-port=2002/tcp
	firewall-cmd --permanent --zone=internal --add-service=http
	firewall-cmd --permanent --zone=internal --add-port=104/tcp
	firewall-cmd --permanent --zone=internal --add-port=2002/tcp

echo "Yea well, firewalld must die anyway..."
	systemctl disable firewalld
	systemctl stop firewalld

echo "Restarting Apache"
	systemctl enable httpd.service
	systemctl restart httpd.service

echo "Checking if mq is installed"
	ISINSTALLED=`which mq 2>/dev/null|wc -l`
	if [ $ISINSTALLED -lt 1 ]
	then
		echo "Addind POSIX command line queue manager mq"
		THISPATH=`pwd`
		cd home/source
		git clone https://github.com/goeb/mq.git >/dev/null 2>&1
		cd mq
		./bootstrap >/dev/null 2>&1
		./configure >/dev/null 2>&1
		make all >/dev/null 2>&1
		make install >/dev/null 2>&1
		cd $THISPATH
	else 
		echo "Good, mq is installed."
	fi

echo "Setting up DB access for background process"
	ISCLEAN=0
	LC=0
	while [ $ISCLEAN -lt 1 ] && [ $LC -lt 3 ]
	do
		clear
		echo "Please enter the username for DB access"
		read USER_INPUT
		ISCLEAN=`echo "$USER_INPUT"|grep -c -E "^[a-zA-Z0-9]+$"`
		if [ $ISCLEAN -lt 1 ]
		then
			echo "Error:  Username can only be alpha or numeric characters."
			sleep 3
		fi
		LC=$((LC+1))
	done

	ISCLEAN=0
	LC=0
	while [ $ISCLEAN -lt 1 ] && [ $LC -lt 3 ]
	do
		clear
		echo "Please enter the password for DB access"
		read USER_INPUT2
		ISCLEAN=`echo "$USER_INPUT2"|grep -c -E "^[a-zA-Z0-9]+$"`
		if [ $ISCLEAN -lt 1 ]
		then
			echo "Error:  Password can only be alpha or numeric characters."
			sleep 3
		fi
		LC=$((LC+1))
	done

	echo "grant all privileges on primal.* to '$USER_INPUT'@'localhost' identified by '$USER_INPUT2' with grant option;"|mysql -u root
	sed -i "s/DBUSER.*/DBUSER=$USER_INPUT/g" /etc/primal/primal.conf
	sed -i "s/DBPASS.*/DBPASS=$USER_INPUT2/g" /etc/primal/primal.conf

if [ $ISNEW -eq 2 ]
then
	echo "PRIMAL installation is complete.  This appears to be a new installation.  To run the software you will need to edit /etc/primal/primal.conf."
	echo "In addition, you will need to create any working directories that are specified in that file.  You can log into the web interface at: "
	echo "http://`hostname`/index.php using the username and password of primal/primal"
else
	echo "PRIMAL upgrade complete.  Please restart your receivers if they were not restarted already."
fi

echo "fs.file-max = 100000" >> /etc/sysctl.conf
sed -i 's/# End of file//g' /etc/security/limits.conf 
sed -i '/* soft nproc/d' /etc/security/limits.conf 
sed -i '/* hard nproc/d' /etc/security/limits.conf 
sed -i '/* soft nofile/d' /etc/security/limits.conf 
sed -i '/* hard nofile/d' /etc/security/limits.conf 

echo " * soft nproc 65535
 * hard nproc 65535
 * soft nofile 65535
 * hard nofile 65535
# End of file" >> /etc/security/limits.conf

echo "/home/dicom/logs/*log {
    missingok
    notifempty
	compress
	copytruncate
	maxsize 1G
	rotate 5
	weekly
    su apache apache
}" > /etc/logrotate.d/primal
