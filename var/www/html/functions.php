<?php
// License GPLv3
function Display_Header()
{
	header( "Expires: Mon, 20 Dec 1998 01:00:00 GMT" );
	header( "Last-Modified: " . gmdate("D, d M Y H:i:s") . " GMT" );
	header( "Cache-Control: no-cache, must-revalidate" );
	header( "Pragma: no-cache" );
}

function Display_Header2()
{
	echo '<H1>PRIMAL Web Interface</H1>';
	echo '<div id="logout">' . $_SESSION['login_username'] . '<br><a href="http://' . $_SERVER['HTTP_HOST'] . '/logout.php">Logout</a>';
	$query="SELECT SCHEMA_NAME FROM INFORMATION_SCHEMA.SCHEMATA WHERE SCHEMA_NAME = 'primalarc';";
	$result=mysql_query($query);
	$num_rows = mysql_num_rows($result);
	if($_SERVER['PHP_SELF'] != "/index.php")
	{
		echo '<br><a href="http://' . $_SERVER['HTTP_HOST'] . '/options.php">Options</a>';
		if($num_rows > 0 && substr($_SESSION['login_sec_bit'], 0, 1) == 1)
		{
			echo '<br><a href="http://' . $_SERVER['HTTP_HOST'] . '/migration/index.php">Migration</a>';
		}
		echo '<br><a href="http://' . $_SERVER['HTTP_HOST'] . '/index.php">Main Page</a></div>';
	} else {
		if($num_rows > 0 && substr($_SESSION['login_sec_bit'], 0, 1) == 1)
		{
			echo '<br><a href="http://' . $_SERVER['HTTP_HOST'] . '/migration/index.php">Migration</a>';
		}
		echo '<br><a href="http://' . $_SERVER['HTTP_HOST'] . '/options.php">Options</a></div>';
	}
}

function Display_Servers() {
	//Pop-up that lists servers.
	echo '<div class="form-popup" id="myForm">';
	echo '<form action="index.php" class="form-container">';
    echo '<h1>Select Servers to View</h1>';
	echo '<select name="Servers" multiple>';
	foreach($_SESSION['SERVERS'] AS $strServerName=>$intServerList) {
	  echo '<option value="' . $strServerName . '">' . $strServerName  . '</option>';
	}
	echo '</select>';
    echo '<button type="submit" class="btn">OK</button>';
    echo '<button type="button" class="btn cancel" onclick="closeForm()">Close</button>';
  	echo '</form>';
	echo '</div>';
}

function Display_Footer()
{
	echo "<br>";
	$query="select * from monitor";
	$result = mysql_query($query);
	while($row = mysql_fetch_assoc($result))
	{
		if($row['status'] == "1") {
			echo '<span class="recup">' . $row['SCP'] . '</span>';
		} else {
			echo '<span class="recdw">' . $row['SCP'] . '</span>';
		}
	}
	echo "<br>";
	if (! isset($SESSION['versionnum'])) {
		$SESSION['versionnum'] = file_get_contents('/etc/primal/primal.version');
	}
	echo "Version: " . $SESSION['versionnum'] . "<br>";
	echo "Date: 2015-08-26<br>";
}

function Error_Message($ERR_NUM)
{
   //echo "Error Number: $ERR_NUM<BR>";
	echo "<SCRIPT LANGUAGE=\"javascript1.2\">";
	echo 'function poponload()';
	echo '{';
	echo "testwindow= window.open ('error.php?errnum=$ERR_NUM', 'newwindow', 'location=1,status=1,scrollbars=1,width=400,height=100');";
	//echo "testwindow= window.open ('error.php?errnum=$ERR_NUM', 'newwindow', config='height=100, width=400, toolbar=no, menubar=no, scrollbars=no, resizable=no,  location=no, directories=no, status=no')";
	echo 'testwindow.moveTo(0,0);';
	echo '}';
   echo "</SCRIPT>";
}

function run_query($query)
{
    $result = mysql_query($query);
    if (!$result) {
        echo 'Could not run query: ' . $query . "<br />" . mysql_error();
        exit;
    }
    return $result;
}

function write_to_log($message1)
{
    //Need to write the log file here.
    $the_date = date("Ymd");
    $to_write = "Date/Time: " . date("Ymd:His") . " Username: <b>" . $_SESSION['login_username'] . "</b> " . $temp_stuff . " " . $message1 . "<br /><br />";
    $filename = "logs/" . $the_date . ".logs";
    if (!$handle = fopen($filename, 'a')) {
        echo "Cannot open file ($filename)";
    } else {
        if (fwrite($handle, $to_write) === FALSE)
        {
            echo "Cannot write to file ($filename)";
            fclose($handle);
        }
    }
}

function Sort_Data($studies, $sort_column, $sort_order)
{
if ($sort_column == 0 && $sort_order == 0)
	{
		$price = array();
		foreach ($studies as $key => $row)
		{
			$price[$key] = $row['tstartrec'];
		}
		array_multisort($price, SORT_ASC, $studies);
	} elseif ($sort_column == 0 && $sort_order == 1) {
		$price = array();
		foreach ($studies as $key => $row)
		{
			$price[$key] = $row['tstartrec'];
		}
		array_multisort($price, SORT_DESC, $studies);
	} elseif ($sort_column == 1 && $sort_order == 0) {
		$price = array();
		foreach ($studies as $key => $row)
		{
			$price[$key] = $row['pname'];
		}
		array_multisort($price, SORT_ASC, $studies);
	} elseif ($sort_column == 1 && $sort_order == 1) {
		$price = array();
		foreach ($studies as $key => $row)
		{
			$price[$key] = $row['pname'];
		}
		array_multisort($price, SORT_DESC, $studies);
	} elseif ($sort_column == 2 && $sort_order == 0) {
		$price = array();
		foreach ($studies as $key => $row)
		{
			$price[$key] = $row['pid'];
		}
		array_multisort($price, SORT_ASC, $studies);
	} elseif ($sort_column == 2 && $sort_order == 1) {
		$price = array();
		foreach ($studies as $key => $row)
		{
			$price[$key] = $row['pid'];
		}
		array_multisort($price, SORT_DESC, $studies);
	} elseif ($sort_column == 3 && $sort_order == 0) {
		$price = array();
		foreach ($studies as $key => $row)
		{
			$price[$key] = $row['paccn'];
		}
		array_multisort($price, SORT_ASC, $studies);
	} elseif ($sort_column == 3 && $sort_order == 1) {
		$price = array();
		foreach ($studies as $key => $row)
		{
			$price[$key] = $row['paccn'];
		}
		array_multisort($price, SORT_DESC, $studies);
	} elseif ($sort_column == 4 && $sort_order == 0) {
		$price = array();
		foreach ($studies as $key => $row)
		{
			$price[$key] = $row['pdob'];
		}
		array_multisort($price, SORT_ASC, $studies);
	} elseif ($sort_column == 4 && $sort_order == 1) {
		$price = array();
		foreach ($studies as $key => $row)
		{
			$price[$key] = $row['pdob'];
		}
		array_multisort($price, SORT_DESC, $studies);
	} elseif ($sort_column == 5 && $sort_order == 0) {
		$price = array();
		foreach ($studies as $key => $row)
		{
			$price[$key] = $row['pmod'];
		}
		array_multisort($price, SORT_ASC, $studies);
	} elseif ($sort_column == 5 && $sort_order == 1) {
		$price = array();
		foreach ($studies as $key => $row)
		{
			$price[$key] = $row['pmod'];
		}
		array_multisort($price, SORT_DESC, $studies);
	} elseif ($sort_column == 6 && $sort_order == 0) {
		$price = array();
		foreach ($studies as $key => $row)
		{
			$price[$key] = $row['sdatetime'];
		}
		array_multisort($price, SORT_ASC, $studies);
	} elseif ($sort_column == 6 && $sort_order == 1) {
		$price = array();
		foreach ($studies as $key => $row)
		{
			$price[$key] = $row['sdatetime'];
		}
		array_multisort($price, SORT_DESC, $studies);
	} elseif ($sort_column == 7 && $sort_order == 0) {
		$price = array();
		foreach ($studies as $key => $row)
		{
			$price[$key] = $row['tendrec'];
		}
		array_multisort($price, SORT_ASC, $studies);
	} elseif ($sort_column == 7 && $sort_order == 1) {
		$price = array();
		foreach ($studies as $key => $row)
		{
			$price[$key] = $row['tendrec'];
		}
		array_multisort($price, SORT_DESC, $studies);
	} elseif ($sort_column == 8 && $sort_order == 0) {
		$price = array();
		foreach ($studies as $key => $row)
		{
			$price[$key] = $row['rec_images'];
		}
		array_multisort($price, SORT_ASC, $studies);
	} elseif ($sort_column == 8 && $sort_order == 1) {
		$price = array();
		foreach ($studies as $key => $row)
		{
			$price[$key] = $row['rec_images'];
		}
		array_multisort($price, SORT_DESC, $studies);
	}
	return $studies;
}

function Check_Input($sort_column, $sort_order, $tempvar, $tempvar2)
{
	$query="select a.puid, a.pname, a.pid, a.pdob, a.pmod, a.sdatetime, r.tstartrec, r.tendrec, r.rec_images, r.rerror, r.senderAET, p.tstartproc, p.tendproc, p.perror, s.tdest, s.tstartsend, s.tendsend, s.timages, s.serror, t.AccessionNum, t.StudyDate, t.StudyModType from patient as a left join receive as r on a.puid = r.puid left join process as p on a.puid = p.puid left join send as s on a.puid = s.puid left join study as t on a.puid = t.puid";
	if(isset($_POST['reset']))
	{
		$query = $query . " limit 100;";
		unset($_POST['input_startdt']);
		unset($_POST['input_enddt']);
		unset($_POST['input_pname']);
		unset($_SESSION['cached_query']);
		unset($_SESSION['input_startdt']);
		unset($_SESSION['input_enddt']);
		unset($_SESSION['input_pname']);
		unset($_SESSION['input_pid']);
		unset($_SESSION['input_accn']);
		return $query;
	}
	if ($_POST['input_startdt'] != "*" && $_POST['input_startdt'] != NULL)
	{
		//have to validate the date format
		//Must be YYYY-MM-DD HH:MM:SS
		$query = $query . " where r.tstartrec between '" . $_POST['input_startdt'] . "' and '" . $_POST['input_enddt'] . "' ";
		$_SESSION['input_startdt']=$_POST['input_startdt'];
		$_SESSION['input_enddt']=$_POST['input_enddt'];
		$bAdded = TRUE;
	}
	if ($_POST['input_pname'] != "*" && $_POST['input_pname'] != NULL)
	{
		if($bAdded === TRUE)
		{
			$query = $query . " and";
		}
		$query = $query . " where a.pname like '%" . $_POST['input_pname'] . "%'";
		$_SESSION['input_pname']=$_POST['input_pname'];
		$bAdded = TRUE;
	}
	if ($_POST['input_pid'] != "*" && $_POST['input_pid'] != NULL)
	{
		if($bAdded === TRUE)
		{
			$query = $query . " and";
		}
		$query = $query . " where a.pid like '%" . $_POST['input_pid'] . "%'";
		$_SESSION['input_pid']=$_POST['input_pid'];
		$bAdded = TRUE;
	}
	if ($_POST['input_accn'] != "*" && $_POST['input_accn'] != NULL)
	{
		if($bAdded === TRUE)
		{
			$query = $query . " and";
		}
		$query = $query . " where t.AccessionNum like '%" . $_POST['input_accn'] . "%'";
		$_SESSION['input_accn']=$_POST['input_accn'];
		$bAdded = TRUE;
	}
	//echo "Query in funciton = " . $query . "<br/>";
	$query = $query . " order by " . $sort_column . " " . $sort_order;
	$_SESSION['cached_query']=$query;
	return $query;
}

function in_array_r($needle, $haystack, $strict = false) {
    foreach ($haystack as $item) {
        if (($strict ? $item === $needle : $item == $needle) || (is_array($item) && in_array_r($needle, $item, $strict))) {
            return true;
        }
    }

    return false;
}
?>
