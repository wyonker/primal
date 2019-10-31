<?php
	$strPUID=$_GET['PID'];
	$strAction=$_GET['ACT'];
	$strRecNum=strstr($strPUID, '_', true);

    //Read the configuration file
    if($ISERROR == 0) {
        $handle = fopen("/etc/primal/primal.conf", "r") or die("Unable to open primal.conf file!");
        if ($handle) {
            $intInRec=0;
            while (($buffer = fgets($handle, 4096)) !== false) {
                #Strip off comments
                $intPOS=strpos($buffer, "#");
                if($intPOS !== FALSE && $intPOS != 0) {
                    $strLINE=substr($buffer, 0, ($intPOS - 1));
                } elseif($intPOS !== FALSE && $intPOS == 0) {
                    $strLINE="";
                } else {
                    $strLINE=$buffer;
                }
                $strLINE=trim($strLINE);
                //echo $intPOS . " " . htmlspecialchars($strLINE) . ":::" . htmlspecialchars($buffer) . "<br>";
                //Now that the line is clean, we can work with it.
                //$intPOS=stripos(htmlspecialchars($strLINE), "<scp");
                //echo "intPOS = " . $intPOS . ".<br>";
                if(stripos($strLINE, "<scp" . $RECNUM . ">")  !== FALSE) {
                    $intInRec=1;
                } elseif (stripos($strLINE, "</scp" . $RECNUM . ">") !== FALSE) {
                    $intInRec=0;
                } elseif ($intInRec == 1) {
                    if (stripos($strLINE, "PRIDESTHIP") !== FALSE) {
                        $intPOS = stripos($strLINE, "PRIDESTHIP");
                        $intPOS2=strpos($strLINE, "=");
                        if ($intPOS2 !== FALSE) {
                            $strDestNum=substr($strLINE, ($intPOS + 10), ($intPOS2 - ($intPOS + 10)));
                            $strValue=substr($strLINE, $intPOS2+1);
                            $THISCONFIG['PRIDESTHIP'][$strDestNum] = $strValue;
                        }
                    } elseif (stripos($strLINE, "PRIDESTPORT") !== FALSE) {
                        $intPOS = stripos($strLINE, "PRIDESTPORT");
                        $intPOS2=strpos($strLINE, "=");
                        if ($intPOS2 !== FALSE) {
                            $strDestNum=substr($strLINE, ($intPOS + 11), ($intPOS2 - ($intPOS + 11)));
                            $strValue=substr($strLINE, $intPOS2+1);
                            $THISCONFIG['PRIDESTPORT'][$strDestNum] = $strValue;
                        }
                    } elseif (stripos($strLINE, "PRIDESTCDCR") !== FALSE) {
                        $intPOS = stripos($strLINE, "PRIDESTCDCR");
                        $intPOS2=strpos($strLINE, "=");
                        if ($intPOS2 !== FALSE) {
                            $strDestNum=substr($strLINE, ($intPOS + 11), ($intPOS2 - ($intPOS + 11)));
                            $strValue=substr($strLINE, $intPOS2+1);
                            $THISCONFIG['PRIDESTCDCR'][$strDestNum] = $strValue;
                        }
                    } elseif (stripos($strLINE, "PRIDESTAEC") !== FALSE) {
                        $intPOS = stripos($strLINE, "PRIDESTAEC");
                        $intPOS2=strpos($strLINE, "=");
                        if ($intPOS2 !== FALSE) {
                            $strDestNum=substr($strLINE, ($intPOS + 10), ($intPOS2 - ($intPOS + 10)));
                            $strValue=substr($strLINE, $intPOS2+1);
                            $THISCONFIG['PRIDESTAEC'][$strDestNum] = $strValue;
                        }
                    } elseif (stripos($strLINE, "PRIAET") !== FALSE) {
                        $intPOS2=strpos($strLINE, "=");
                        if ($intPOS2 !== FALSE) {
                            $THISCONFIG['AET']=substr($strLINE, $intPOS2+1);
                        }
                    } elseif (stripos($strLINE, "PRILOGDIR") !== FALSE) {
                        $intPOS2=strpos($strLINE, "=");
                        if ($intPOS2 !== FALSE) {
                            $THISCONFIG['PRILOGDIR']=substr($strLINE, $intPOS2+1);
                        }
                    } elseif (stripos($strLINE, "PRILFOUT") !== FALSE) {
                        $intPOS2=strpos($strLINE, "=");
                        if ($intPOS2 !== FALSE) {
                            $THISCONFIG['PRILFOUT']=substr($strLINE, $intPOS2+1);
                        }
                    } elseif (stripos($strLINE, "PRILL") !== FALSE) {
                        $intPOS2=strpos($strLINE, "=");
                        if ($intPOS2 !== FALSE) {
                            $THISCONFIG['PRILL']=substr($strLINE, $intPOS2+1);
                        }
                    } elseif (stripos($strLINE, "PRISENT") !== FALSE) {
                        $intPOS2=strpos($strLINE, "=");
                        if ($intPOS2 !== FALSE) {
                            $THISCONFIG['PRISENT']=substr($strLINE, $intPOS2+1);
                        }
                    }
                }
            }
            fclose($handle);
            $_SESSION['HAVECONFIG']=1;
        }
    }

	if(strcmp($strAction, "resend") == 0) {
		//This study is going to be resent
		if(!isset($_GET['DEST']) {
			//Resend to all original destinations
			if(file_exists($THISCONFIG['PRISENT'] . "/" . $strPUID) {
				system("/home/dicom/send.bash " . $strRecNum . " " . $THISCONFIG['PRISENT'] . "/" . $strPUID);
			} elseif(file_exists($THISCONFIG['PRIHOLD'] . "/" . $strPUID) {
				system("/home/dicom/send.bash " . $strRecNum . " " . $THISCONFIG['PRIHOLD'] . "/" . $strPUID);
			} elseif(file_exists($THISCONFIG['PRIERROR'] . "/" . $strPUID) {
				system("/home/dicom/send.bash " . $strRecNum . " " . $THISCONFIG['PRIERROR'] . "/" . $strPUID);
			}
		} else {
			if(isset($THISCONFIG['PRIDESTHIP'][$_GET['DEST'])) {
				if(file_exists($THISCONFIG['PRISENT'] . "/" . $strPUID) {
					system("/home/dicom/send2.bash " . $strRecNum . " " . $THISCONFIG['PRISENT'] . "/" . $strPUID . " " . $THISCONFIG['PRIDESTHIP'][$_GET['DEST']);
				} elseif(file_exists($THISCONFIG['PRIHOLD'] . "/" . $strPUID) {
					system("/home/dicom/send2.bash " . $strRecNum . " " . $THISCONFIG['PRIHOLD'] . "/" . $strPUID . " " . $THISCONFIG['PRIDESTHIP'][$_GET['DEST']);
				} elseif(file_exists($THISCONFIG['PRIERROR'] . "/" . $strPUID) {
					system("/home/dicom/send2.bash " . $strRecNum . " " . $THISCONFIG['PRIERROR'] . "/" . $strPUID . " " . $THISCONFIG['PRIDESTHIP'][$_GET['DEST']);
				}
		}
	} elseif(strcmp($strAction, "send") == 0) {
		if(file_exists($THISCONFIG['PRISENT'] . "/" . $strPUID) {
			system("/home/dicom/bin/primalscu -ll " . $THISCONFIG['PRILL'] . " -aet " . $THISCONFIG['PRIAET'] . 
				" -xf /home/dicom/bin/storescu.cfg Default -aec " . $_GET['AET'] . " " . $_GET['HIP'] . " " . $_GET['PORT'] . " " . 
				$THISCONFIG['PRISENT'] . "/" . $strPUID . "/*.dcm");
		} elseif(file_exists($THISCONFIG['PRIHOLD'] . "/" . $strPUID) {
			system("/home/dicom/bin/primalscu -ll " . $THISCONFIG['PRILL'] . " -aet " . $THISCONFIG['PRIAET'] . 
				" -xf /home/dicom/bin/storescu.cfg Default -aec " . $_GET['AET'] . " " . $_GET['HIP'] . " " . $_GET['PORT'] . " " . 
				$THISCONFIG['PRIHOLD'] . "/" . $strPUID . "/*.dcm");
		} elseif(file_exists($THISCONFIG['PRIERROR'] . "/" . $strPUID) {
			system("/home/dicom/bin/primalscu -ll " . $THISCONFIG['PRILL'] . " -aet " . $THISCONFIG['PRIAET'] . 
				" -xf /home/dicom/bin/storescu.cfg Default -aec " . $_GET['AET'] . " " . $_GET['HIP'] . " " . $_GET['PORT'] . " " . 
				$THISCONFIG['PRIERROR'] . "/" . $strPUID . "/*.dcm");
		}
	}
?>
