#!/bin/bash

CURDIR=`pwd`
ISNEW=3

clear
USERNAME=`whoami`
if [ "$USERNAME" != "root" ]
then
    echo "You must be root to run this script.  Exiting."
    exit 1
fi

echo "SELinux must die!!!"
	setenforce 0
	cd /etc/selinux; sed -i 's/=enforcing/=disabled/g' config; cd $CURDIR
	sleep 1

echo "Installing any packages that are missing."
	./install_packages.bash
	dnf update -y
	sleep 1

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

if [ -e "/etc/primal/primal.version" ]
then
	rm -f /etc/primal/primal.version
fi
cp etc/primal/primal.version /etc/primal/
echo "Changing ownership and permissions on /etc/primal to apache (for web edits)"
	chown apache.apache -R /etc/primal
	chmod 755 -R /etc/primal

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

ISINSTARTUP=`cat /etc/rc.d/rc.local|grep "/home/dicom/startup.bash start"|wc -l`
if [ $ISINSTARTUP -lt 1 ]
then
	echo "Inserting PRIMAL startup script..."
	echo "sleep 60" >> /etc/rc.d/rc.local
	echo "/home/dicom/startup.bash start" >> /etc/rc.d/rc.local
	ISINSTARTUP=`cat /etc/rc.d/rc.local|grep "/home/dicom/startup.bash start"|wc -l`
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
	rm -f /home/dicom/share/*
	cp -pr home/dicom/share/* /home/dicom/share/
	chown apache.apache -R /home/dicom
	chmod 777 -R /home/dicom

echo "Copying dcm4che to /home"
	if [ ! -e "/home/dcm4che" ]
	then
		mkdir /home/dcm4che
	fi
	rm -fr /home/dcm4che/*
	cp -pr home/dcm4che /home/

echo "Installing web componet"
	rm -f /var/www/html/*
	cp -p var/www/html/* /var/www/html/
	if [ ! -e "/var/www/html/tmp" ]
	then
		mkdir /var/www/html/tmp
	fi
	chown apache.apache -R /var/www/html/*
	chmod 777 -R /var/www/html/*

echo "Compiling executables for this platform."
cd home/build; ./build.bash
cd $CURDIR

echo "Please check if there are errors at the end for storescu.o or storescp.o. Type 'yes' to continue"
	read USER_INPUT
	USER_INPUT=`echo "$USER_INPUT"|tr '[:upper:]' '[:lower:]'`
	while [ "$USER_INPUT" != "yes" ]
	do
		USER_INPUT=`echo "$USER_INPUT"|tr '[:upper:]' '[:lower:]'`
	done

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
	else
		echo "Dropping database..."
		echo "drop database primal;"|mysql -u root
		echo "Importing database design..."
		mysql -u root < home/dicom/install/install.sql
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
if [ -e "home/build/dcmnet/apps/storescp" ]
then
	cp home/build/dcmnet/apps/storescp /home/dicom/bin/
else
	echo "Error:  storescp is not found.  Exiting..."
	exit 1
fi

if [ -e "/home/dicom/bin/primalscu" ]
then
	rm -f /home/dicom/bin/primalscu
fi

if [ -e "home/build/dcmnet/apps/storescu" ]
then
	cp home/build/dcmnet/apps/storescu /home/dicom/bin/primalscu
else
	echo "Error:  storescu is not found.  Exiting..."
	exit 1
fi

rm /home/dicom/bin/*.cfg
cp home/build/dcmnet/etc/*.cfg /home/dicom/bin/

if [ -e "/home/dicom/bin/rec_check" ]
then
	rm /home/dicom/bin/rec_check
fi
echo "Building the receiver check app"
cd home/source; ./compile.bash
cd $CURDIR

if [ -e "home/source/rec_check" ]
then
	mv home/source/rec_check /home/dicom/bin/
else
	echo "Error:  rec_check is not found.  Exiting..."
	exit 1
fi

ISINPATH=`cat /root/.bashrc|grep /home/dicom/bin|wc -l`
if [ $ISINPATH -lt 1 ]
then
	echo "Adding /home/dicom/bin to Root's search path"
	echo 'export PATH=/home/dicom/bin:$PATH' >> /root/.bashrc
	export PATH=/home/dicom/bin:$PATH
fi

echo "Modifying Apache config files"
	echo '<Directory "/home/dicom">' >> /etc/httpd/conf/httpd.conf
	echo '    AllowOverride None' >> /etc/httpd/conf/httpd.conf
	echo '    Options None' >> /etc/httpd/conf/httpd.conf
	echo '    Require all granted' >> /etc/httpd/conf/httpd.conf
	echo '</Directory>' >> /etc/httpd/conf/httpd.conf
	echo '<Directory "/etc/primal">' >> /etc/httpd/conf/httpd.conf
	echo '    Require all granted' >> /etc/httpd/conf/httpd.conf
	echo '</Directory>' >> /etc/httpd/conf/httpd.conf

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
	systemctl disable firewalld.service rolekit
	systemctl stop firewalld.service

echo "Restarting Apache"
	systemctl enable httpd.service
	systemctl restart httpd.service

if [ $ISNEW -eq 2 ]
then
	echo "PRIMAL installation is complete.  This appears to be a new installation.  To run the software you will need to edit /etc/primal/primal.conf."
	echo "In addition, you will need to create any working directories that are specified in that file.  You can log into the web interface at: "
	echo "http://`hostname`/index.php using the username and password of primal/primal"
else
	echo "PRIMAL upgrade complete.  Please restart your receivers if they were not restarted already."
fi

