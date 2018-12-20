Installation:

This installation script has been tested on Fedora 24 and CentOS 7.

* sudo ./install.bash
* sudo vim /etc/primal/primal.conf
     This configuration should start 2 receivers.  Each with very different
     setups.  
* cd /home/dicom
* ./startup.bash start ALL

That should be all that is required to recieve, modify and send DICOM files.
