#!/bin/bash

tar -cvJof /home/$1 /home/dicom /var/www/html --exclude /home/dicom/error --exclude /home/dicom/sent --exclude /home/dicom/outbound --exclude /home/dicom/processing --exclude /home/dicom/inbound --exclude /home/dicom/logs

