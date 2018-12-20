#!/bin/bash


mkdir -p /home/dicom/bin /home/dicom/sent /home/dicom/error /home/dicom/logs /home/dicom/inbound /home/dicom/processing /home/dicom/share/dcmtk
find /home/dcmtk-3.6.0/dcmdata/apps -name "*" -type f -executable -exec cp {} /home/dicom/bin/ \;
find /home/dcmtk-3.6.0/dcmnet/apps -name "*" -type f -executable -exec cp {} /home/dicom/bin/ \;
find /home/dcmtk-3.6.0/dcmqrdb/apps -name "*" -type f -executable -exec cp {} /home/dicom/bin/ \;
find /home/dcmtk-3.6.0/ -name "dicom.dic" -type f -exec cp {} /home/dicom/share/dcmtk/ \;

echo "export PATH=/home/dicom/bin:\$PATH" >> ~/.bashrc

