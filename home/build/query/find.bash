#!/bin/bash

export DCMDICTPATH="/home/dicom/share/dcmtk/dicom.dic"
#dump2dcm -ll debug input.txt input.dcm
#findscu -aet radprimal01 -aec RADAGC2IDC -ll trace -P -k 0008,0052="PATIENT" -k 0020,000D 10.88.226.99 11112 input.dcm
#echo "Patient Level:"
#findscu -aet radprimal01 -aec RADAGC2IDC -P -xi -k 0008,0052="PATIENT" -k 0010,0020 -k 0020,1200 -k 0020,000D 10.88.226.99 11112 input.dcm
#findscu -aet radprimal01 -aec RADAGC2IDC -P -xi -k 0008,0052="PATIENT" -k "(0010,0010)=WOOLF^MONICA*" -k "(0010,0020)=" -k "(0020,1200)" -k 0020,000D 10.88.226.99 11112
echo
#echo "Study Level:"
#findscu -aet radprimal01 -aec RADAGC2IDC -v -S -xi -k 0008,0052="STUDY" -k 0020,000D 10.88.226.99 11112 input.dcm
#findscu -aet radprimal01 -aec RADAGC2IDC -v -S -xi -k 0008,0052="STUDY" -k "(0010,0010)=WOOLF^MONICA^M^" -k "(0010,0020)=MD000218875" -k 0020,000D 10.88.226.99 11112

findscu -aet radprimal01 -aec RADAGC2IDC -P -xi -k 0008,0052="PATIENT" -k "(0010,0010)=WOOLF^MONICA*" -k "(0010,0020)=" -k "(0020,1200)" -k 0020,000D 10.88.230.74 11112

echo "Study Level direct to SAS:"
findscu -aet radprimal01 -aec RADAGC2IDC -P -xi -k 0008,0052="IMAGE" -k "(0010,0010)" -k "(0010,0020)=MD000218875" -k 0020,000D 10.88.230.74 11112

findscu -aet radprimal01 -aec PACSSTOREC2 -v -S -xi -k 0008,0052="STUDY" -k "(0010,0020)=MD000218875" -k 0020,000D 10.88.226.98 104
