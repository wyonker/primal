<?php
	//License GPLv3
	//Version 1.00.04
	//2021-08-11
	session_start();
    header( "Expires: Mon, 20 Dec 1998 01:00:00 GMT" );
    header( "Last-Modified: " . gmdate("D, d M Y H:i:s") . " GMT" );
    header( "Cache-Control: no-cache, must-revalidate" );
	header( "Pragma: no-cache" );

if ($_SESSION['active'] != '1') {
    header("Location: login.php");
    exit();
}

if (isset($_SESSION['obj']) || isset($_SESSION['obj1'])) {
    unset($_SESSION['obj']);
    unset($_SESSION['obj1']);
}

require_once('config.php');
require_once('functions.php');

$ISERROR=0;

if($ISERROR == 0) {
	//We should be passed the PRIMAL ID at a minimum.  Let's get the receveriver from that.
	$intPOS=strpos($_GET['p'], "_");
	if($intPOS !== FALSE) {
		$RECNUM=substr($_GET['p'], 0, $intPOS);
	} else {
		$strERRMSG="Error:  PRIMAL ID passed is either missing or invalid.  Exiting.";
		$ISERROR=1;
	}
}

if(! isset($_SESSION['HAVECONFIG'])) {
	if($ISERROR == 0) {
		$query="select * from patient where puid = '" . $_GET['p'] . "';";
		$result = $conn->query($query);
		while($row = mysqli_fetch_assoc($result)) {
			$_SESSION['strPNAME']=$row['pname'];
			$_SESSION['strPID']=$row['pid'];
			$_SESSION['strPDOB']=$row['pdob'];
		}
		$query="select * from receive where puid = '" . $_GET['p'] . "' limit 1;";
		$result = $conn->query($query);
		while($row = mysqli_fetch_assoc($result)) {
			$_SESSION['strStartRcv']=$row['tstartrec'];
			$_SESSION['strEndRcv']=$row['tendrec'];
			$_SESSION['strNumImg']=$row['rec_images'];
		}
		$query="select * from process where puid = '" . $_GET['p'] . "' limit 1;";
		$result = $conn->query($query);
		while($row = mysqli_fetch_assoc($result)) {
			$_SESSION['strStartProc']=$row['tstartproc'];
			$_SESSION['strEndProc']=$row['tendproc'];
			$_SESSION['strProcError']=$row['perror'];
		}
		$query="select * from study where puid = '" . $_GET['p'] . "' limit 1;";
		$result = $conn->query($query);
		while($row = mysqli_fetch_assoc($result)) {
			$_SESSION['strStudyDesc']=$row['StudyDesc'];
			$_SESSION['strSIUID']=$row['SIUID'];
			$_SESSION['strACCN']=$row['AccessionNum'];
			$_SESSION['strSDATE']=$row['StudyDate'];
		}
	}

	$query="select ilocation from image where puid = '" . $_GET['p'] . "' limit 1;";
	$result = $conn->query($query);
	while($row = mysqli_fetch_assoc($result)) {
		$_SESSION["ilocation"] = $row['ilocation'];
	}
	if ($_SESSION["ilocation"] == NULL || $_SESSION["ilocation"] == "") {
		$strERRMSG="ERROR:  Study DICOM files are no longer stored on this server.<br>";
		$ISERROR=1;
	}

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
							$_SESSION['PRIDESTHIP'][$strDestNum] = $strValue;
						}
					} elseif (stripos($strLINE, "PRIDESTPORT") !== FALSE) {
						$intPOS = stripos($strLINE, "PRIDESTPORT");
						$intPOS2=strpos($strLINE, "=");
						if ($intPOS2 !== FALSE) {
							$strDestNum=substr($strLINE, ($intPOS + 11), ($intPOS2 - ($intPOS + 11)));
							$strValue=substr($strLINE, $intPOS2+1);
							$_SESSION['PRIDESTPORT'][$strDestNum] = $strValue;
						}
					} elseif (stripos($strLINE, "PRIDESTCDCR") !== FALSE) {
						$intPOS = stripos($strLINE, "PRIDESTCDCR");
						$intPOS2=strpos($strLINE, "=");
						if ($intPOS2 !== FALSE) {
							$strDestNum=substr($strLINE, ($intPOS + 11), ($intPOS2 - ($intPOS + 11)));
							$strValue=substr($strLINE, $intPOS2+1);
							$_SESSION['PRIDESTCDCR'][$strDestNum] = $strValue;
						}
					} elseif (stripos($strLINE, "PRIDESTAEC") !== FALSE) {
						$intPOS = stripos($strLINE, "PRIDESTAEC");
						$intPOS2=strpos($strLINE, "=");
						if ($intPOS2 !== FALSE) {
							$strDestNum=substr($strLINE, ($intPOS + 10), ($intPOS2 - ($intPOS + 10)));
							$strValue=substr($strLINE, $intPOS2+1);
							$_SESSION['PRIDESTAEC'][$strDestNum] = $strValue;
						}
					} elseif (stripos($strLINE, "PRIAET") !== FALSE) {
						$intPOS2=strpos($strLINE, "=");
						if ($intPOS2 !== FALSE) {
							$_SESSION['AET']=substr($strLINE, $intPOS2+1);
						}
					} elseif (stripos($strLINE, "PRILOGDIR") !== FALSE) {
						$intPOS2=strpos($strLINE, "=");
						if ($intPOS2 !== FALSE) {
							$_SESSION['PRILOGDIR']=substr($strLINE, $intPOS2+1);
						}
					} elseif (stripos($strLINE, "PRILFOUT") !== FALSE) {
						$intPOS2=strpos($strLINE, "=");
						if ($intPOS2 !== FALSE) {
							$_SESSION['PRILFOUT']=substr($strLINE, $intPOS2+1);
						}
					} elseif (stripos($strLINE, "PRILL") !== FALSE) {
						$intPOS2=strpos($strLINE, "=");
						if ($intPOS2 !== FALSE) {
							$_SESSION['PRILL']=substr($strLINE, $intPOS2+1);
						}
					} elseif (stripos($strLINE, "PRIOUT") !== FALSE) {
						$intPOS2=strpos($strLINE, "=");
						if ($intPOS2 !== FALSE) {
							$_SESSION['PRIOUT']=substr($strLINE, $intPOS2+1);
						}
					}
				}
			}
			fclose($handle);
			$_SESSION['HAVECONFIG']=1;
		}
	}
}

if($ISERROR != 1) {
	if($_SERVER['REQUEST_METHOD'] == 'POST') {
		if (isset($_POST['resend'])) {
			$strQuery = 'delete from send where puid = "' . $_GET['p'] . '" limit 10;';
			$result = $conn->query($strQuery);
			$strQuery="select ilocation from image where puid = '" . $_GET['p'] . "' limit 1;";
			$result = $conn->query($strQuery);
			while($row = mysqli_fetch_assoc($result)) {
				$_SESSION["ilocation"] = $row['ilocation'];
			}
			if(file_exists($_SESSION["ilocation"])) {
				$intPos = strrpos($_SESSION["ilocation"], "/");
				$strFullPath = substr($_SESSION["ilocation"], 0, $intPos - 1);
				$strCMD = "mv -f " . $strFullPath . " " . $_SESSION['PRIOUT'] . "/";
				exec($strCMD, $return, $retval);
				header('Location: index.php');
			}
		}
		$intLC1=sizeof($_SESSION['PRIDESTHIP']);
		$_SESSION['PRIDESTAEC'][$intLC1] = $_POST['AEC'];
		$_SESSION['PRIDESTHIP'][$intLC1] = $_POST['HIP'];
		$_SESSION['PRIDESTPORT'][$intLC1] = $_POST['PORT'];
		$_SESSION['PRIDESTCDCR'][$intLC1] = $_POST['CDCR'];
	} elseif(isset($_GET['g'])) {
		if($_GET['g'] == 1) {
			$intLC1=0;
			while ($intLC1 < sizeof($_SESSION['PRIDESTHIP'])) {
				if($_GET['d0'] == "a") {
					$_GET['d' . $intLC1]=1;
				}
				if(isset($_GET['d' . $intLC1])) {
					if($_GET['d'. $intLC1] == 1) {
						if ($_SESSION['PRIDESTCDCR'][$intLC1] == 1) {
							$strEXT = ".j2k";
						} elseif ($_SESSION['PRIDESTCDCR'][$intLC1] == 2) {
							$strEXT = ".ucr";
						} else {
							$strEXT = ".dcm";
						}
						exec("/home/dicom/bin/primalscu -ll " . $_SESSION['PRILL'] . " -aet " . $_SESSION['AET'] . " -xf /home/dicom/bin/storescu.cfg Default -aec " . $_SESSION['PRIDESTAEC'][$intLC1] . " " . $_SESSION['PRIDESTHIP'][$intLC1] . " " . $_SESSION['PRIDESTPORT'][$intLC1] . " " . $_SESSION['ilocation'] . "/*" . $strEXT . " 2>&1", $return, $retval);
						$intLC3=0;
						$handle = fopen($_SESSION['PRILOGDIR'] . "/" . $_SESSION['PRILFOUT'], 'a');
						if($handle !== FALSE) {
							while($intLC3 < sizeof($return)) {
								fwrite($handle, $return[$intLC3] . "\n");
								$intLC3++;
							}
						} else {
							echo "Could not write to file " . $_SESSION['PRILOGDIR'] . "/" . $_SESSION['PRILFOUT'] . "<br>";
						}
						fclose($handle);
						echo "Return Value: " . $retval . "<br>";
					}
				}
				$intLC1++;
			}
			header("Location: index.php?c=9&o=0&p=0");
		}
	}
}

echo <<<EOT
<HTML>
<!DOCTYPE !DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN"
            "http://www.w3.org/TR/html4/loose.dtd">
<HEAD>
	<!-- Written by Will Yonker-->
	<TITLE>PRIMAL Web Interface</TITLE>
	<link rel="stylesheet" href="default.css">
EOT;

echo '</HEAD>';
echo '<BODY>';
Display_Header2();
echo '<br>';
echo '<table>';
echo '<tr><td>Patient Name</td><td align="left">' . $_SESSION['strPNAME'] . "</td></tr>";
echo '<tr><td>Patient ID</td><td align="left">' . $_SESSION['strPID'] . "</td></tr>";
echo '<tr><td>Date of Birth</td><td align="left">' . $_SESSION['strPDOB'] . "</td></tr>";
echo '<tr><td>Accession Number</td><td align="left">' . $_SESSION['strACCN'] . "</td></tr>";
echo '<tr><td>Study Date/Time</td><td align="left">' . $_SESSION['strSDATE'] . "</td></tr>";
echo '<tr><td>Study Instance UID</td><td align="left">' . $_SESSION['strSIUID'] . "</td></tr>";
echo '<tr><td>Start Receive</td><td align="left">' . $_SESSION['strStartRcv'] . "</td></tr>";
echo '<tr><td>End Receive</td><td align="left">' . $_SESSION['strEndRcv'] . "</td></tr>";
echo '<tr><td>Start Processing</td><td align="left">' . $_SESSION['strStartProc'] . "</td></tr>";
echo '<tr><td>End Processing</td><td align="left">' . $_SESSION['strEndProc'] . "</td></tr>";
echo '<tr><td># Images</td><td align="left">' . $_SESSION['strNumImg'] . "</td></tr>";
echo '<tr><td>Study Description</td><td align="left">' . $_SESSION['strStudyDesc'] . "</td></tr>";
if(file_exists($_SESSION["ilocation"] . "/payload.json")) {
	echo '<tr><td>JSON File</td><td align="left">';
	echo '<a href="/primal/resend.php?p=' . $_GET["p"];
	echo '&j=' . $_GET["p"] . '">View</a></td></tr>';
}
if(file_exists($_SESSION["ilocation"] . "/package.conf")) {
	echo '<tr><td>PKG File</td><td align="left">';
	echo '<a href="/primal/resend.php?p=' . $_GET["p"];
	echo '&k=' . $_GET["p"] . '">View</a></td></tr>';
}
echo '</table><br>';

$query="select * from image where puid = '" . $_GET['p'] . "';";
$result = $conn->query($query);
if(! isset($SERIUID)) {
	$SERIUID = [];
}
$intPOS=0;
while($row = mysqli_fetch_assoc($result)) {
	//if(in_array_r($row['SERIUID'], $SERIUID) === FALSE || !isset($SERIUID)) {
	if(in_array_r($row['SERIUID'], $SERIUID) === FALSE) {
		$intPOS=sizeof($SERIUID);
		$SERIUID[$intPOS]['SERIUID']=$row['SERIUID'];
		$SERIUID[$intPOS]['numimg']=1;
		$query="select * from series where puid = '" . $_GET['p'] . "' and SERIUID = '" . $row['SERIUID'] . "';";
		$result2 = $conn->query($query);
		while($row = mysqli_fetch_assoc($result2)) {
			$SERIUID[$intPOS]['SeriesDesc']=$row['SeriesDesc'];
		}
	} else {
		$intLC1=0;
		while($intLC1 < sizeof($SERIUID)) {
			if($SERIUID[$intLC1]['SERIUID'] == $row['SERIUID']) {
				$SERIUID[$intPOS]['numimg']++;
			}
			$intLC1++;
		}
	}
	//echo '<tr><td><a href="http://' . $_SERVER['HTTP_HOST'] . '/resend.php?p=' . $_GET["p"];
	//echo '&v=' . $row['SOPIUID'] . '">' . $row['ifilename'] . '</a></td></tr>';
}
echo '<div style="width: 100%;overflow:auto;">';
//echo '<div style="float:left; width: 50%" class="leftdiv">';
echo '<div style="float:left; class="leftdiv">';
echo '<table border="1">';
echo '<tr><th>Series Instance UID</th><th>Series Description</th><th># Images</th></tr>';
$intLC1=0;
if(! isset($_GET['e']))
	$_GET['e']="";
while($intLC1 < sizeof($SERIUID)) {
	if($_GET['e'] == $SERIUID[$intLC1]['SERIUID']) {
		$e_var = "collapse";
		echo '<tr style="background-color:#886A08;"><td><a href="/primal/resend.php?p=' . $_GET["p"];
	} else { 
		$e_var = $SERIUID[$intLC1]['SERIUID'];
		echo '<tr><td><a href="/primal/resend.php?p=' . $_GET["p"];
	}
	echo '&e=' . $e_var . '">' . $SERIUID[$intLC1]['SERIUID'] . '</a></td>';
	echo '<td>' . $SERIUID[$intLC1]['SeriesDesc'] . '</td>';
	echo '<td>' . $SERIUID[$intLC1]['numimg'] . '</td></tr>';
	if(isset($_GET['e'])) {
		if($_GET['e'] == $SERIUID[$intLC1]['SERIUID']) {
			$query="select * from image where puid = '" . $_GET['p'] . "' and SERIUID = '" . $_GET['e'] . "';";
			$result3 = $conn->query($query);
			$intLC2=1;
			while($row = mysqli_fetch_assoc($result3)) {
				if($_GET['v'] == $row['SOPIUID'] || $_GET['z'] == $row['SOPIUID']) {
					echo '<tr style="background-color:#886A08;">';
					$strPreviousSOPIUID=$strTEMPPreviousSOPIUID;
					$intSel=$intLC1;
					$intNumInSer=$intLC2;
					$intSetNext=1;
				} else {
					echo '<tr>';
					if($intSetNext==1) {
						$strNextSOPIUID=$row['SOPIUID'];
						unset($intSetNext);
					}
				}
				echo '<td colspan="2"><a href="/primal/resend.php?p=' . $_GET["p"];
				echo '&e=' . $SERIUID[$intLC1]['SERIUID'] . '&v=' . $row['SOPIUID'];
				echo '">' . $row['ifilename'] . '</a></td>';
				echo '<td><a href="/primal/resend.php?p=' . $_GET["p"];
				echo '&e=' . $SERIUID[$intLC1]['SERIUID'] . '&z=' . $row['SOPIUID'];
				echo '">View</a></td>';
				echo '</tr>';
				$strTEMPPreviousSOPIUID=$row['SOPIUID'];
				$intLC2++;
			}
		}
	}
	$intLC1++;
}
echo "</table><br>";
if($ISERROR == 1) {
	echo "<h3>" . $strERRMSG . "</h3><br>";
} elseif(isset($_GET['j'])) {
	$strOutput='<div style="float:left; margin-left: 10px; border-style: solid; border-width: 3px;" id="divright">';
	$strOutput.='<table style="table-layout: fixed; width: 1000px"><tr><th>JSON Payload</th></tr>';
	$strOutput.='<tr><td align="left"><pre>';
	$strJsonFileName = $_SESSION["ilocation"] . "/payload.json";
	$handle2 = fopen($strJsonFileName, "r");
	/*
	while (!feof($handle2)) {
		$strOutput.= fread($handle2, 160);
		$strOutput.= "<br>";
	}
	*/
	$strTemp=fread($handle2, filesize($strJsonFileName));
	$strOutput.=pretty_print($strTemp);
	//$strTemp=str_replace(",", ",\n", $strTemp);
	//$strOutput.=json_encode($strTemp, JSON_PRETTY_PRINT);
	//$strOutput.=$strTemp;
	fclose($handle2);
	$strOutput.='</pre></td></tr></table>';
	$strOutput.='</div id="divright">';
} elseif(isset($_GET['k'])) {
	$strOutput='<div style="float:left; margin-left: 10px; border-style: solid; border-width: 3px;" id="divright">';
	$strOutput.='<table style="table-layout: fixed; width: 1000px"><tr><th>PKG File</th></tr>';
	$strOutput.='<tr><td align="left"><pre>';
	$strJsonFileName = $_SESSION["ilocation"] . "/package.conf";
	$handle2 = fopen($strJsonFileName, "r");
	while (!feof($handle2)) {
		$strOutput.= fread($handle2, 160);
		$strOutput.= "<br>";
	}
	$strTemp=str_replace(",", ",\n", $strTemp);
	$strOutput.=json_encode($strTemp, JSON_PRETTY_PRINT);
	$strOutput.=$strTemp;
	fclose($handle2);
	$strOutput.='</pre></td></tr></table>';
	$strOutput.='</div id="divright">';
} elseif(isset($_GET['v'])) {
	$strOutput='<div style="float:left; margin-left: 10px; border-style: solid; border-width: 3px;" id="divright">';
	$query="select * from image where puid = '" . $_GET['p'] . "' and SOPIUID = '" . $_GET['v'] . "' limit 1;";
	$result = $conn->query($query);
	while($row = mysqli_fetch_assoc($result)) {
		exec("/home/dicom/bin/dcmdump " . $row['ilocation'] . "/" . $row['ifilename'] . " 2>&1", $return, $retval);
		$intLC3=0;
		$strOutput.='<table style="width: 100%"><tr><th>Series Description</th><th>Image number</th></tr>';
		$strOutput.='<tr><td>' . $SERIUID[$intSel]['SeriesDesc'] . "</td><td>" . '(' . $intNumInSer . ' of ' . $SERIUID[$intSel]['numimg'] . ')' . "</td></tr></table>";
		$strOutput.="<pre>";
		while($intLC3 < sizeof($return)) {
			$strOutput.=$return[$intLC3] . "<br>";
			$intLC3++;
		}
		$strOutput.="</pre><br>";
		$strOutput.='<a href="/primal/resend.php?p=' . $_GET["p"] . '">Back</a>';
		$strOutput.="<br><br>";
	}
	$strOutput.='</div id="divright">';
} elseif(isset($_GET['z'])) {
	$strOutput='<div style="float:left; margin-left: 10px;" id="divright">';
	$query="select * from image where puid = '" . $_GET['p'] . "' and SOPIUID = '" . $_GET['z'] . "' limit 1;";
	$result = $conn->query($query);
	$strOutput.='<table style="width: 100%"><tr><th>Series Description</th><th>Image number</th></tr>';
	$strOutput.='<tr><td>' . $SERIUID[$intSel]['SeriesDesc'] . "</td><td>";
	if($intNumInSer > 1) {
		$strOutput.='<a href="/primal/resend.php?p=' . $_GET["p"];
		$strOutput.='&e=' . $_GET["e"] . '&z=' . $strPreviousSOPIUID . '"><</a> ';
	}
	$strOutput.='(' . $intNumInSer . ' of ' . $SERIUID[$intSel]['numimg'] . ')';
	if($intNumInSer < $SERIUID[$intSel]['numimg']) {
		$strOutput.=' <a href="/primal/resend.php?p=' . $_GET["p"];
		$strOutput.='&e=' . $_GET["e"] . '&z=' . $strNextSOPIUID . '">></a> ';
	}
	$strOutput.="</td></tr></table>";
	while($row = mysqli_fetch_assoc($result)) {
		exec("/usr/local/bin/dcmj2pnm +oj " . $row['ilocation'] . "/" . $row['ifilename'] . " /var/www/html/primal/tmp/" . $row['ifilename'] . ".jpg 2>&1", $return, $retval);
		$strOutput.='<img src="/primal/tmp/' . $row['ifilename'] . '.jpg" alt="Image" width="600">';
		$strOutput.="<br>";
	}
	$strOutput.='</div id="divright">';
}
	echo '<form action="resend.php?p=' . $_GET['p'] . '" method="post">';
	echo '<table border="1">';
	echo "<tr><th>Destination AET</th><th>Destination</th><th>Port</th><th>Compress</th><th>Action</th></tr>";
	$strQuery = "SELECT * FROM conf_send WHERE active = 1;";
	$result = $conn->query($query);
	while($row = mysqli_fetch_assoc($result)) {
		echo "<tr>";
		echo "<td>" . $row['send_aet'] . "</td>";
		echo "<td>" . $row['send_hip'] . "</td>";
		echo "<td>" . $row['send_port'] . "</td>";
		echo "<td>" . $row['send_comp_level'] . "</td>";
		echo '<td><a href="/primal/resend.php?p=' . $_GET["p"] . "&d=" . $row['conf_send_id'] . 'class="button" >Send</a></td>';
	}
	/*
	while (isset($_SESSION['PRIDESTHIP'][$intLC1])) {
		echo "<tr>";
		eco "<td>" . $_SESSION['PRIDESTAEC'][$intLC1] . "</td>";
		echo "<td>" . $_SESSION['PRIDESTHIP'][$intLC1] . "</td>";
		echo "<td>" . $_SESSION['PRIDESTPORT'][$intLC1] . "</td>";
		echo "<td>" . $_SESSION['PRIDESTCDCR'][$intLC1] . "</td>";
		echo "<td>" . '<a href="/primal/resend.php?p=' . $_GET["p"] . "&d=" . $intLC1 . 'class="button" >Requeue</a></td>';
		if(! isset($_GET['d0'])) {
			//Nothing to do here.
		} elseif($_GET['d0'] == "a") {
			$_GET['d' . $intLC1]=1;
		}
		$intLC2=0;
		while (isset($_SESSION['PRIDESTHIP'][$intLC2])) {
			if($intLC2 == $intLC1) {
				if(! isset($_GET['d0'])) {
					$strURL.="&d" . $intLC2 . "=0";
				} elseif($_GET['d'. $intLC1] == 0) {
					$strURL.="&d" . $intLC2 . "=1";
				} else {
					$strURL.="&d" . $intLC2 . "=0";
				}
			} elseif(isset($_GET['d' . $intLC2])) {
				$strURL.="&d" . $intLC2 . "=" . $_GET['d' . $intLC2];
			} else {
				$strURL.="&d" . $intLC2 . "=0";
			}
			$intLC2++;
		}
		echo '<td><button type="submit" name="Requeue">Requeue</button></td></tr>';

	echo $strURL . "</tr>";
		$intLC1++;
	}

	echo '<tr><td><input type="text" name="AEC" />' . '</td>';
	echo '<td><input type="text" name="HIP" />' . '</td>';
	echo '<td><input type="text" name="PORT" />' . '</td>';
	echo '<td><input type="text" name="CDCR" />' . '</td>';
	echo '<td><button type="submit" name="Send">Send</button></td></tr>';
	*/
	echo "</table><br>";
	echo '</form>';
if(isset($_GET['d0'])) {
	$intLC1=0;
	$strURL='<a href="/primal/resend.php?p=' . $_GET["p"];
	while (isset($_SESSION['PRIDESTHIP'][$intLC1])) {
		$strURL=$strURL . "&d" . $intLC1 . "=" . $_GET['d' . $intLC1];
		$intLC1++;
	}
	$strURL=$strURL . '&g=1">Execute</a>';
	echo $strURL . "<br>";
}
Display_Footer();
echo '</div class="leftdiv">';
if(isset($strOutput)) {
	echo $strOutput;
	unlink("/var/www/html/tmp/" . $row['ifilename'] . ".jpg");
}
echo '</div>';
echo '</BODY>';
echo '</HTML>';

function pretty_print($json_data) {
	//Initialize variable for adding space
	$space = 0;
	$flag = false;
	//loop for iterating the full json data
	for($counter=0; $counter<strlen($json_data); $counter++) {
		//Checking ending second and third brackets
		if ( $json_data[$counter] == '}' || $json_data[$counter] == ']' ) {
			$space--;
			$strReturn.="\n";
			$strReturn.=str_repeat(' ', ($space*2));
		}
 		//Checking for double quote(“) and comma (,)
		if ( $json_data[$counter] == '"' && ($json_data[$counter-1] == ',' || $json_data[$counter-2] == ',') ) {
			$strReturn.="\n";
			$strReturn.=str_repeat(' ', ($space*2));
		}
		if ( $json_data[$counter] == '"' && !$flag ) {
			if ( $json_data[$counter-1] == ':' || $json_data[$counter-2] == ':' )
				//Add formatting for question and answer
				$strReturn.='<span style="color:blue;font-weight:bold">';
			else
				//Add formatting for answer options
				$strReturn.='<span style="color:red;">';
		}
		$strReturn.=$json_data[$counter];
		//Checking conditions for adding closing span tag
		if ( $json_data[$counter] == '"' && $flag )
			$strReturn.='</span>';
		if ( $json_data[$counter] == '"' )
			$flag = !$flag;
		//Checking starting second and third brackets
		if ( $json_data[$counter] == '{' || $json_data[$counter] == '[' ) {
			$space++;
			$strReturn.="\n";
			$strReturn.=str_repeat(' ', ($space*2));
		}
	}
	$strReturn.="</pre>";

	return $strReturn;
}
?>
