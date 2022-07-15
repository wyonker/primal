Installation:

This installation script has been updated for RHEL 8.  I tried to maintain CentOS 7 compatability but I'm not all that sure how successful that was.

Hardware Requirements:
4 CPU cores
8GB RAM
30GB hard disk

* Install RHEL 8
* Select web server
* Log in as root
* vim /etc/hostname
  Be sure to set the fully qualified name.
  If you are using multiple nodes, each hostname MUST be unique
  It would be best if every node could resolve and reach every other node
  At a minimum, the node you plan to use the web interface on MUST be 
     able to resolve and reach each node on port 80 and 443
  Reusing hostnames may cause issues.
* Download the latest PRIMAL package to /tmp
* cd /opt
* mkdir primal
* mv primal*.tar.xz primal/
* cd primal
* tar -xvf primal*.tar.xz
* sudo ./install.bash
* sudo vim /etc/primal/primal.conf
     This configuration should start 2 receivers.  Each with very different
     setups.  Configuration options are well documented in the file. 
* cd /home/dicom
* sudo ./startup.bash start ALL

That should be all that is required to recieve, modify and send DICOM files.

The web interface can be accessed from:

http://localhost/primal/index.php
Username:  primal
Password:  primal

Additionally there is a restful API that can be accssed at:

http://localhost:8100/rest.php

This is only accesable from the local host for security reasons. 

Currently supported forms:

* http://localhost:8100/rest.php?PID=<PRIMAL ID>&ACT=resend
This will resend the study referenced by PRIMAL ID to all the original destinations

* http://localhost:8100/rest.php?PID=<PRIMAL ID>&ACT=resend&DEST=0
This will resend the study referenced by PRIMAL ID to the first original dstination.
If DEST is out of bounds, the study will not be resent

* http://localhost:8100/rest.php?PID=<PRIMAL ID>&ACT=send&HIP=<HOST NAME or IP>&PORT=<DESTINATION PORT>&AET=<DESTINATION AET>
This will send the study to any accessable host or IP with a DICOM SCP process listening.
WARNING!!!  This could be a security issue.
