<?php
    session_start();
    header( "Expires: Mon, 20 Dec 1998 01:00:00 GMT" );
    header( "Last-Modified: " . gmdate("D, d M Y H:i:s") . " GMT" );
    header( "Cache-Control: no-cache, must-revalidate" );
    header( "Pragma: no-cache" );
    require_once('config.php');
    require_once('functions.php');
    date_default_timezone_set('America/New_York');

if ($_SESSION['active'] != '1') {
    header("Location: login.php");
    exit();
}
if (isset($_SESSION['obj']) || isset($_SESSION['obj1'])) {
    unset($_SESSION['obj']);
    unset($_SESSION['obj1']);
}

if (substr($_SESSION['login_sec_bit'], 0, 1) != 1) {
	header("Location:  http://" . $_SERVER['HTTP_HOST'] . '/index.php');
}

if( substr($_SESSION['login_sec_bit'], 1, 4) == "0000") {
	header("Location: http://" . $_SERVER['HTTP_HOST'] . '/migration/cleanup.php');
}

//Need to detect if the user has tried to use a browser navigation button
if(isset($_SESSION['is_started']) && $_SESSION['is_started'] != "results.php") {
    header("Location: index.php");
    exit();
} else {
    $_SESSION['is_started'] = "retrieve.php";
}

echo <<<EOT
<HTML>
<!DOCTYPE !DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN"
            "http://www.w3.org/TR/html4/loose.dtd">
<HEAD>
    <!-- Written by Will Yonker-->
    <TITLE>PRIMAL Web Interface</TITLE>
    <link rel="stylesheet" href="default.css">
</HEAD>
<BODY>
EOT;

header( "Refresh: 1800; URL=http://" . $_SERVER['HTTP_HOST'] . "/migration/logout.php");

unset($_SESSION['is_error']);
unset($_SESSION['add_error']);

Display_Header3();

//Unset saved search criteria
unset($_SESSION['pname2']);
unset($_SESSION['pid2']);
unset($_SESSION['pdob2']);
unset($_SESSION['saccn2']);
unset($_SESSION['sdate2']);
unset($_SESSION['emod2']);
unset($_SESSION['edesc2']);
unset($_SESSION['psource2']);

$query="select * from image where siuid = '" . $_GET['r'] . "';";
$result = mysql_query($query);
if (!$result) {
	echo $query . "<br>";
    die('Invalid query: ' . mysql_error());
}
//echo $query . "<br>";
$num_rows1 = mysql_num_rows($result);
$row = mysql_fetch_assoc($result);
$ilocation=$row["ilocation"];
$puid=$row["puid"];
$siuid=$_GET['r'];

$query="select * from patient where puid = '" . $puid . "';";
$result = mysql_query($query);
if (!$result) {
	echo $query . "<br>";
    die('Invalid query: ' . mysql_error());
}
$num_rows = mysql_num_rows($result);
if($num_rows > 1) {
    echo "More than one Patient with the unique (ish) id of " . $puid . " found...<br>";
	exit;
}
$row2 = mysql_fetch_assoc($result);
$pname=$row2["pname"];
$pid=$row2["pid"];
$pdob=$row2["pdob"];
$psex=$row2["psex"];
$psource=$row2["psource"];

$query="select * from study where siuid = '" . $_GET['r'] . "';";
$result = mysql_query($query);
if (!$result) {
	echo $query . "<br>";
    die('Invalid query: ' . mysql_error());
}
$num_rows = mysql_num_rows($result);
if($num_rows > 1) {
    echo "More than one study with the unique (ish) id of " . $_GET["r"] . " found...<br>";
	exit;
}
$row3 = mysql_fetch_assoc($result);
$saccn=$row3["saccn"];
$simg=$row3["simg"];
$sdate=$row3["sdate"];
$stime=$row3["stime"];

echo 'Retrieving patient: <b>' . $pname . '</b> MRN: <b>' . $pid . '</b> DOB: <b>' . $pdob . '</b><br>';
echo 'Accesssion number: <b>' . $saccn . '</b> Study date: <b>' . $sdate . '</b> Study time: <b>' . $stime . '</b><br>';
echo '<br>';

$query="select * from image where siuid = '" . $_GET['r'] . "';";
$result = mysql_query($query);
if (!$result) {
	echo $query . "<br>";
	die('Invalid query: ' . mysql_error());
}

//Need to fetch eatch file associated with a SIUID.  
echo "<br><br>";
echo "<table>";
echo "<th>Action</th><th>Result</th>";
unset($retval);
chdir("/var/www/html/migration/tmp/");

$is_error=0;
$is_warn=0;
//Creating a directory to put all the files we pulled from ARC1
$dirname = "/var/www/html/migration/tmp/" . $_GET['r'] . "/";
$dirnameshort = $_GET['r'];
//Let's see if that directory exists already.
$DONE=0;
$LC2=0;
while ($DONE == 0) {
	if($LC2 > 5) {
		$DONE = 1;
	} else {
		if(file_exists($dirname)) {
			$dirnameshort = $_GET['r'] . rand(0, 1000);
			$dirname = "/var/www/html/migration/tmp/" . $dirnameshort . "/";
		}
	}
	$LC2++;
}
if(!mkdir($dirname)) {
	echo ("<tr><td>Unable to create directory " . $dirname . '.  Please start over and try again.  If this issue persists, please contact your administrator...</b></td><td><img src="red_x.gif" alt="Success" height="30" width="30"></td></tr>');
	$is_error=1;
} else {
	chdir($dirname);
	//If this doesn't get unset then we will clean up the directory before going back to index.
	$_SESSION['cleanupdir'] = $dirname;
	$got_one=0;
	$ilocation = array("empty");
	while($row = mysql_fetch_array($result, MYSQL_BOTH)) {
		if($psource == "si") {
			$strcmd="ncftpget -t 10 -u agfastk -p agfastore cc-radpres02.cc.ad.cchs.net . " . $row["ilocation"];
		} else {
			$strcmd="ncftpget -t 10 -u agfastk -p agfastore arc1.ccf.org . " . $row["ilocation"];
		}
		//echo $strcmd . "<br>";
		session_write_close();
		exec($strcmd, $retval, $return);
		if($return == -1) {
			//Couldn't execute the ncftpget command for some reason.  Let's sleep 2 seconds and try again.
			$done1=0;
			$LC5=0;
			while($done1 != 1 && $LC5 <= 5) {
				sleep(2);
				unset($retval);
				session_write_close();
				exec($strcmd, $retval, $return);
				if($return != -1) {
					$done1=1;
				}
				$LC5++;
			}
		}
		if($return == 1 || $return == 2) {
			echo '<tr><td>Unable to connect to ARC1.  Please contact HVI (57070)!</td><td><img src="red_x.gif" alt="Failed" height="30" width="30"></td></tr>';
			$is_error=1;
		} elseif($return == 3) {
			echo "<tr><td>File (" . $row["ilocation"] . ") not found on archive (" . $return . ')!  Please try again after a few minutes and hope that it appears.</td><td><img src="red_x.gif" alt="Failed" height="30" width="30"></td></tr>';
			$is_error=1;
		} elseif($return == 4) {
			echo '<tr><td>Connection to ARC1 timed out.  Please contact HVI (57070)!</td><td><img src="red_x.gif" alt="Failed" height="30" width="30"></td></tr>';
			$is_error=1;
		} elseif($return == 0) {
			echo "<tr><td>Copy " . $row["ilocation"] . " from STK " . '</td><td><img src="green_Check2.gif" alt="Success" height="30" width="30"></td></tr>';
			//$ilocation = array($got_one => $row["ilocation"]);
			$ilocation[$got_one] = $row["ilocation"];
			//echo "<tr><td>" . $got_one . " - ilocation = " . $ilocation[$got_one] . "</td></tr>";
			//echo "<tr><td>ilocation = " . $ilocation[0] . "</td></tr>";
			$got_one++;
		} else {
			echo '<tr><td>There was an unknown error trying to retrieve ' . $row["ilocation"] . '.  Please contact your system administrator.</td><td><img src="red_x.gif" alt="Failed" height="30" width="30"></td></tr>';
			$is_error=1;
		}
	}

	$skip_cleanup=0;
	//Need to loop through all the tars we pulled and untar them.
	$LC6=0;
	while($got_one > $LC6 && $is_error != 1) {
		//echo "<tr><td>ilocation = " . $ilocation[$LC6] . "</td></tr>";
		$pos = strrpos($ilocation[$LC6], "/");
		$filename=substr($ilocation[$LC6], $pos + 1);
		$newfilename = $dirname . $filename;
		if (file_exists($filename)) {
			rename($filename, $newfilename);
			unset($retval);
			session_write_close();
			//Need to see if the same filename is in the tar
			exec("/usr/bin/tar -tf " . $filename, $retval, $return);
			foreach($retval as $tarfile) {
				$samename=0;
				if(strpos($tarfile, $filename) !== FALSE) {
					$samename=1;
				}
			}
			unset($retval);
			session_write_close();
			exec("/usr/bin/tar -xvof " . $filename, $retval, $return);
			if($return == 0) {
				echo "<tr><td>Untar " . $filename . '</td><td><img src="green_Check2.gif" alt="Success" height="30" width="30"></td></tr></td></tr>';
				unset($retval);
				if($samename != 1) {
					unlink($filename);
				}
			}
		} else {
			$is_error=1;
			echo "<tr><td>The file " . $filename . ' does not exist</td><td><img src="red_x.gif" alt="Failed" height="30" width="30"></td></tr>';
		}
		$LC6++;
	}

	//Now we need to find any .tar or .tar.Z files that were stored in the archive
	$directory  = $dirname . '/';
	$all_files  = new RecursiveIteratorIterator(new RecursiveDirectoryIterator($directory));
	//Create an array with a list of all .tar.Z files
	$zarc_files = new RegexIterator($all_files, '/\.tar.Z$/');

	//Loop through each file and try to untar and uncompress
	foreach($zarc_files as $file) {
		$zfilename=basename($file);
		rename($file, $directory . "/" . $zfilename);
		session_write_close();
		unset($retval);
		exec("/usr/bin/tar -xvzof " . $directory . "/" . $zfilename, $retval, $return);
		if($return == 0) {
			echo "<tr><td>Untar and uncompress of tar " . basename($zfilename) . ' </td><td><img src="green_Check2.gif" alt="Success" height="30" width="30"></td></tr>';
			unlink($directory . "/" . $zfilename);
		} else {
			echo "<tr><td>There was an error untarring " . $zfilename . '!</td><td><img src="red_x.gif" alt="Failed" height="30" width="30"></td></tr>';
		}
	}

	//Create a list of all .tar files
	$arc_files = new RegexIterator($all_files, '/\.tar$/');

	//Loop through each .tar and try to untar them
	foreach($arc_files as $file) {
		$zfilename=basename($file);
		rename($file, $directory . "/" . $zfilename);
		session_write_close();
		unset($retval);
		exec("/usr/bin/tar -xvof " . $directory . "/" . $zfilename, $retval, $return);
		if($return == 0) {
			echo "<tr><td>Untar and uncompress of tar " . $directory . "/" . $zfilename . '</td><td><img src="green_Check2.gif" alt="Success" height="30" width="30"></td></tr>';
			unlink($directory . "/" . $zfilename);
		} else {
			echo "<tr><td>There was an error untarring " . $zfilename . '!</td><td><img src="red_x.gif" alt="Failed" height="30" width="30"></td></tr>';
			$is_error=1;
		}
	}

	//Each tar could have contained misc files that we don't need.  Looping through and removing any we know about.
	$fdbo_files = new RegexIterator($all_files, '/\.fdbo$/');
	foreach($fdbo_files as $file) {
		unlink($file);
	}

	$toc_files = new RegexIterator($all_files, '/\.toc$/');
	foreach($toc_files as $file) {
		unlink($file);
	}

	//Theer are files in the top of this tree that I don't know the name or what to do with.  Let's just delete them for now
	if ($handle = opendir('.')) {
		while (($entry = readdir($handle)) !== FALSE) {
			if($entry != "." && $entry != ".." && is_dir($entry) !== TRUE) {
				unlink($entry);
			}
		}
	} else {
		echo '<tr><td>Could not open the current directory.</td><td><img src="red_x.gif" alt="Failed" height="30" width="30"></td></tr>';
	}


	//I don't remember why I had the following two lines.  Just going to comment them out for now.
	//$pos=strrpos($filename, ".");
	//$shortfilename=substr($filename, 0, $pos);

	//Looping through all files that are remaining.  These should all be images of some type or directories.
	$objects = new RecursiveIteratorIterator(new RecursiveDirectoryIterator("."), RecursiveIteratorIterator::SELF_FIRST);
	$LC1=0;
	$is_converted=0;
	$is_corrupted=0;
	$is_file=0;
	$err_msg = "";
	$shown_816=0;
	$shown_add_mr=0;
	foreach($objects as $name => $object) {
		$is_missmatch=0;
		if(file_exists($name) === TRUE && is_dir($name) === FALSE && $name != "." && $name != "..") {
			$this_converted=0;
			$foundpn=0;
			$done=0;
			$is_valid=0;
			$is_file++;
			while($done <= 1) {
				unset($retval);
				session_write_close();
				exec("/home/dicom/bin/dcmdump " . $name . " 2>&1", $retval, $return);
				if($return == 0) {
					foreach ($retval as $dcmtag) {
						if(strpos($dcmtag, "(0010,0010) PN") !== FALSE) {
							$done=1;
							$foundpn++;
							$posOB = strpos($dcmtag, "[");
							$posCB = strpos($dcmtag, "]");
							$fpname = substr($dcmtag, $posOB + 1, $posCB-$posOB-1);
							if($foundpn == 1) {
								$fpname2 = $fpname;
							} else {
								if($fpname != $fpname2) {
									$is_error=1;
									$err_msg .= "<tr><td>Error:  Patient name changes within the study!  " . $fpname . " changes to " . $fpname2 . '!!!</td><td><img src="red_x.gif" alt="Failed" height="30" width="30"></td></tr>';
								}
							}
							if($fpname != $pname) {
								if($is_missmatch != 0) {
									$is_warn = 1;
									$warn_msg .= "<tr><td>Patient name " . $fpname . " does not match " . $pname . ' which is in the study.</td><td><img src="red_x.gif" alt="Failed" height="30" width="30"></td></tr>';
								}
								$is_missmatch == 1;
							}
						}
						if(strpos($dcmtag, "(0008,0018)") !== FALSE) {
							$is_valid=1;
						}
					}
				}
				if($foundpn == 0 && $done == 0) {
					//Could not read the file so it must not be in standard DICOM format.  Let's try a simple conversion and read it again.
					$command = "dd if=" . $name . " of=" . $name . ".dcm conv=swab 2>&1";
					session_write_close();
					unset($retval);
					exec($command, $retval, $return);
					if($return == 0)
					{
						unlink($name);
						$name = $name . ".dcm";
						$is_converted=1;
						$this_converted=1;
					}
					//Need to check and see if we have 8,0016
					exec("/home/dicom/bin/dcmdump " . $name . " 2>&1|grep \"0008,0016\"|wc -l", $retval, $return);
					if($return == 0)
					{
						//0008,0016 is not there.  This is a known issue on MRs that need to be converted.  Let's see if this is a MR.
						if($shown_816 == 0) {
							echo '<tr><td>0008,0016 is not there.  This is a known issue on MRs that need to be converted from ACR/NEMA format.</td><td><img src="alert_triangle.gif" alt="Warning" height="30" width="30"></td></tr>';
							$shown_816++;
						}
						exec("/home/dicom/bin/dcmdump " . $name . " 2>&1", $retval2, $return);
						$is_found_mr = 0;
						foreach ($retval2 as $dcmtag2) {
							//echo "<tr><td>Value = " . $dcmtag2 . "</td></tr>";
							if(strpos($dcmtag2, "(0008,0060)") !== FALSE) {
								if(strpos($dcmtag2, "[MR]") !== FALSE) {
									//echo "<tr><td>Value = " . $dcmtag2 . "</td></tr>";
									$is_found_mr = 1;
								}
							}
						}
						if($is_found_mr == 1)
						{
							//This is a MR and 0008,0016 is missing.  Let's attempt to add the tag.
							exec("/home/dicom/bin/dcmodify -i \"(0008,0016)==MRImageStorage\" " . $name . " 2>&1", $retval, $return);
							if($return == 0) {
								if($shown_add_mr == 0) {
									echo '<tr><td>Sucessfully added tag 0008,0016</td><td><img src="green_Check2.gif" alt="Success" height="30" width="30"></td></tr>';
									$shown_add_mr++;
								}
							} else {
								echo '<tr><td>Could not add tag 0008,0016</td><td><img src="red_x.gif" alt="Failed" height="30" width="30"></td></tr>';
							$is_error=1;
							}
						}
					}
				} elseif($foundpn == 0 && $done > 0) {
					//If we tried converting and still couldn't read the file, we are done...
					$is_error=1;
					$is_converted++;
					$is_corrupted++;
				}
				$done++;
			}
			if($is_valid != 1 && $foundpn > 0) {
				//There are some archives that contain Siemens Magic spicific or corrupted files.  We don't need them, kill them.
				unlink($name);
				$LC1--;
				if($this_converted == 1) {
					$this_converted = 0;
					$is_converted--;
				}
			} else {
				//Finally, move the file to the top directory so we can remove all the created subdirectories.
				//PRIMAL will not crawl subdirectories and look for DICOM files to send
				$pos2=strrpos($name, "/");
				$justname=substr($name, $pos2 + 1);
				rename($name, "./" . $justname);
			}
			$LC1++;
		}
	}

	//Done reading the files.  Let's let the user know if we had to convert any.  In case there are issues.
	if($is_converted == 1) {
		echo '<tr><td>Converted files from ACR/NEMA v1/2 to DICOM (v3)</td><td><img src="green_Check2.gif" alt="Success" height="30" width="30"></td></tr>';
	}
	if($is_corrupted == 1) {
		$is_warn = 1;
		$err_msg .= '<tr><td>Warning:  This archive contains ' . $is_corrupted . ' non-DICOM files.</td><td><img src="alert_triangle.gif" alt="Warning" height="30" width="30"></td></tr>';
	} elseif ($is_corrupted > 1) {
		$is_error = 1;
		$err_msg .= '<tr><td>Error:  This archive contains ' . $is_corrupted . ' non-DICOM files.</td><td><img src="red_x.gif" alt="Failed" height="30" width="30"></td></tr>';
	}

	if($LC1 > 0) {
		if($is_warn > 0) {
			echo $warn_msg;
		}
		if($is_error != 1) {
			//If there are no errors, we can send this study.
			if($LC1 == $simg) {
				echo "<tr><td>Found " . $LC1 . " images in the archive but the study says there should be " . $simg . ' images.</td><td><img src="alert_triangle.gif" alt="Warning" height="30" width="30"></td></tr>';
			} elseif($LC1 < ($simg-1) || $LC1 > ($simg+1)) {
				echo "<tr><td>Found " . $LC1 . " images in the archive but the study says there should be " . $simg . ' images (+/- 1).</td><td><img src="alert_triangle.gif" alt="Warning" height="30" width="30"></td></tr>';
				$is_warn = 1;
			}
			echo "<tr><td>Found " . $LC1 . ' images in tar archive.</td><td><img src="green_Check2.gif" alt="Success" height="30" width="30"></td></tr>';
			//Need to generate a unique PRIMAL ID since we are bypassing the receiver which usually does this.
			$primal_id = "3_" . date('YmdHis') . "_" . rand(0,1000);
			//The receiver also usually creates the directory to put the files in.
			$destname = "/home/dicom/inbound/" . $primal_id;
			//Remove subdirectories.  They will cause errors, even if empty.
			if ($handle = opendir('.')) {
				while (false !== ($entry = readdir($handle))) {
					if ($entry != "." && $entry != "..") {
						if(is_dir($entry)) {
							clean_dir($entry);
						}
					}
				}
				closedir($handle);
			}
		} else {
			$err_msg .= '<tr><td>There was an error.  If this issue persists, please contact your system administrator.</td><td><img src="red_x.gif" alt="Failed" height="30" width="30"></td></tr>';
		}
	} elseif($is_file > 0) {
		$is_error = 1;
        $err_msg .= '<tr><td>Error:  This archive contains no readable files and is probably corrupt!</td><td><img src="red_x.gif" alt="Failed" height="30" width="30"></td></tr>';
	}
}

if ($is_error > 0) {
	echo $err_msg;
}

echo '</table>';
echo '<br><br>';

if($is_error == 0) {
	echo '<H2>What would you like to do?</H2>';
	//echo '<form action="send.php?r=' . $_GET['r'] . '" method="post">';
	echo '<form action="send.php" method="post">';
	if(substr($_SESSION['login_sec_bit'], 1, 1) == 1)  {
		echo '<input type="radio" name="dest" value="Legacy" checked/>Send to Legacy<br>';
	}
	if(substr($_SESSION['login_sec_bit'], 4, 1) == 1)  {
		echo '<input type="radio" name="dest" value="Impax"/>Send to Impax (FIXUP REQUIRED!!!)<br>';
	}
	if(substr($_SESSION['login_sec_bit'], 2, 1) == 1)  {
		echo '<input type="radio" name="dest" value="SMN02"/>Send to LiteBox Backup (Nexus02)<br>';
	}
	if(substr($_SESSION['login_sec_bit'], 3, 1) == 1) {
		//If the user has high enough security, they will be given a choice where to send the files.
		echo '<input type="radio" name="dest" value="Other" />Send Somewhere Else<br><br>';
		echo "<table>";
		echo '<tr><td>Enter host name or IP of send destination:</td><td> <input type="text" name="senddest" /></td></tr>';
		echo '<tr><td>Enter port number of send destination:</td><td><input type="text" name="sendport" /></td></tr>';
		echo '<tr><td>Enter the AET you wish to use:</td><td><input type="text" name="sendaet" value="RADPRIMAL01" /></td></tr>';
		echo '<tr><td>Enter the AEC you wish to use:</td><td><input type="text" name="sendaec" /></td></tr>';
		echo '</table>';
	}
	echo '<input type="hidden" name="senddir" value="' . $dirname . '" />';
	echo '<input type="hidden" name="senddirshort" value="' . $dirnameshort . '" />';
	echo '<input type="hidden" name="sendsiuid" value="' . $_GET['r'] . '" />';
	echo '<br><input type="submit" value="submit"><br>';
	echo '</form>';
}

echo '<br><br>';
echo '<a href="http://' . $_SERVER['HTTP_HOST'] . '/migration/cleanup.php">New Query</a>';
echo '<br><br>';

Display_Footer();
echo '</BODY>';
echo '</HTML>';
unset($_SESSION['add_error']);
?>
