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

if ($_SESSION['is_started'] != "index.php" && isset($_SESSION['is_started'])) {
	$_SESSION['is_started'] = "index.php";
	$_SESSION['is_error']=1;
	$_SESSION['add_error'] .= "Error:  It appears that you used your broswer buttons!  Your session has been reset.  Please try again...<br><br>";
} else {
	$_SESSION['is_started'] = "index.php";
}

header( "Refresh: 1800; URL=http://" . $_SERVER['HTTP_HOST'] . "/migration/logout.php");

if($_SERVER['REQUEST_METHOD'] == 'POST' && $_GET['t']!=1)
{
	if(isset($_POST['reset']))
	{
		unset($_POST['pname']);
		unset($_POST['pid']);
		unset($_POST['pdob']);
		unset($_POST['saccn']);
		unset($_POST['sdate']);
		unset($_POST['emod']);
		unset($_POST['edesc']);
		unset($_POST['psource']);
		unset($_SESSION['pname2']);
		unset($_SESSION['pid2']);
		unset($_SESSION['pdob2']);
		unset($_SESSION['saccn2']);
		unset($_SESSION['sdate2']);
		unset($_SESSION['emod2']);
		unset($_SESSION['edesc2']);
		unset($_SESSION['psource2']);
	}
	if(isset($_POST['pname']) && $_POST['pname'] != "")
	{
		if (strlen($_POST['pname']) > 127) 
		{
			$_SESSION['add_error'] .= "Error:  Patient name can not be longer than 127 characters...<br>";
			$_SESSION['is_error']=1;
		}
		if ($_SESSION['is_error'] != 1)
		{ 
			$_SESSION['pname'] = $_POST['pname'];
		}
	}
	$aValid = array('-', '_', ' ', '.', '%');
	if(isset($_POST['pid']) && $_POST['pid'] != "")
	{
		if(!ctype_alnum(str_replace($aValid, 'A', $_POST['pid'])))
		{
			$_SESSION['add_error'] .= "Error:  Patient MRN must only contain alpha/numeric characters...<br>";
			$_SESSION['is_error']=1;
		}
		if ($_SESSION['is_error'] != 1)
		{ 
			$_SESSION['pid'] = $_POST['pid'];
		}
	}
	$curyear=date('Y');
	if(isset($_POST['pdob']) && $_POST['pdob'] != "")
	{
		if(strlen($_POST['pdob']) != 10)
		{
			$_SESSION['add_error'] .= "Error:  Patient DOB must be in the format of YYYY-MM-DD";
			$_SESSION['is_error']=1;
		} else {
			$pyear=substr($_POST['pdob'], 0, 4);
			if($pyear < 1800 || $pyear > $curyear)
			{
				$_SESSION['add_error'] .= "Error:  Patient DOB year must be from 1800-" . $curyear . "<br>";
				$_SESSION['is_error']=1;
			}
			$pmon=substr($_POST['pdob'], 5, 2);
			if($pmon < 1 || $pmon > 12)
			{
				$_SESSION['add_error'] .= "Error:  Patient DOB month must be from 1 - 12...<br>";
				$_SESSION['is_error']=1;
			}
			$pday=substr($_POST['pdob'], 8, 2);
			if($pday < 1 || $pmon > 31)
			{
				$_SESSION['add_error'] .= "Error:  Patient DOB day must be from 1 - 31...<br>";
				$_SESSION['is_error']=1;
			}
		}
		if ($_SESSION['is_error'] != 1)
        {
			$_SESSION['pdob'] = $_POST['pdob'];
		}
	}
	$aValid = array('-', '_', ' ', '.', '%');
	if(isset($_POST['saccn']) && $_POST['saccn'] != "")
	{
		if(strlen($_POST['saccn']) < 1 || strlen($_POST['saccn']) > 32)
		{
			$_SESSION['add_error'] .= "Error:  Accession number must be from 1-32 characters long...<br>";
			$_SESSION['is_error']=1;
		}
		if(!ctype_alnum(str_replace($aValid, 'A', $_POST['saccn'])))
		{
			$_SESSION['add_error'] .= "Error:  Accession number must only contain alpha/numeric characters...<br>";
            $_SESSION['is_error']=1;
		}
		if($_SESSION['is_error'] != 1)
		{
			$_SESSION['saccn'] = $_POST['saccn'];
		}
	}
	if(isset($_POST['sdate']) && $_POST['sdate'] != "")
	{
		if(strlen($_POST['sdate']) != 10)
		{
			$_SESSION['add_error'] .= "Error:  Study date must be in the format of YYYY-MM-DD";
			$_SESSION['is_error']=1;
		} else {
			$pyear=substr($_POST['sdate'], 0, 4);
			if($pyear < 1800 || $pyear > $curyear)
			{
				$_SESSION['add_error'] .= "Error:  Study date year must be from 1800-" . $curyear . "<br>";
				$_SESSION['is_error']=1;
			}
			$pmon=substr($_POST['sdate'], 5, 2);
			if($pmon < 1 || $pmon > 12)
			{
				$_SESSION['add_error'] .= "Error:  Study date month must be from 1 - 12...<br>";
				$_SESSION['is_error']=1;
			}
			$pday=substr($_POST['sdate'], 8, 2);
			if($pday < 1 || $pmon > 31)
			{
				$_SESSION['add_error'] .= "Error:  Study date day must be from 1 - 31...<br>";
				$_SESSION['is_error']=1;
			}
		}
		if($_SESSION['is_error'] != 1)
		{
			$_SESSION['sdate'] = $_POST['sdate'];
		}
	}
	if(isset($_POST['emod']) && $_POST['emod'] != "")
	{
		if(strlen($_POST['emod']) < 1 || strlen($_POST['emod']) > 6)
		{
			$_SESSION['add_error'] .= "Error:  Modality type must be from 1 to 6 characters in length...<br>";
			$_SESSION['is_error']=1;
		}
		if($_SESSION['is_error'] != 1)
		{
			$_SESSION['emod'] = $_POST['emod'];
		}
	}
	if(isset($_POST['edesc']) && $_POST['edesc'] != "")
	{
		if(strlen($_POST['edesc']) < 1 || strlen($_POST['edesc']) > 32)
		{
			$_SESSION['add_error'] .= "Error:  Study description must be from 1 to 32 characters in length...<br>";
			$_SESSION['is_error']=1;
		}
		if($_SESSION['is_error'] != 1)
        {
			$_SESSION['edesc'] = $_POST['edesc'];
		}
	}
	if($_POST['pname'] == "" && $_POST['pid'] == "" && $_POST['saccn'] == "")
	{
		$_SESSION['add_error'] .= "Error:  Patient Name, Patient ID or Accession # must be entered....<br>";
		$_SESSION['is_error']=1;
	}
	$_SESSION['psource'] = $_POST['psource'];
	if($_SESSION['is_error']!=1)
	{
		$query = "insert into audit set aname = '" . $_SESSION['loginid'] . "', adate = '" . date('Y-m-d');
		$query .= "', atime = '" . date('H:i:s') . "', atype = 'search', amisc = '" . mysql_real_escape_string($_POST['pname']) . "|";
		$query .= $_POST['pid'] . "|" . $_POST['pdob'] . "|" . $_POST['saccn'] . "|" . $_POST['sdate'] . "|";
		$query .= $_POST['emod'] . "|" . mysql_real_escape_string($_POST['edesc']) . "'";
		$result = mysql_query($query);
		if(! $result)
		{
			echo "Error trying to insert into audit table!<br>";
			echo "Error in query: " . $query . "<br>";
			die("Error in MySQL qeury " . mysql_error() . ".<br>");
		}
		header("Location: results.php");
		exit();
	}
}		

//Put session variables passed from results page back into post variables
if($_GET['t']==1)
{
	if($_SESSION['pname2'] != "%")
	{
		$_POST['pname']=$_SESSION['pname2'];
	}
	if($_SESSION['pid2'] != "%")
	{
		$_POST['pid']=$_SESSION['pid2'];
	}
	if($_SESSION['pdob2'] != "%")
	{
		$_POST['pdob']=$_SESSION['pdob2'];
	}
	if($_SESSION['saccn2'] != "%")
	{
		$_POST['saccn']=$_SESSION['saccn2'];
	}
	if($_SESSION['sdate2'] != "%")
	{
		$_POST['sdate']=$_SESSION['sdate2'];
	}
	if($_SESSION['emod2'] != "%")
	{
		$_POST['emod']=$_SESSION['emod2'];
	}
	if($_SESSION['edesc2'] != "%")
	{
		$_POST['edesc']=$_SESSION['edesc2'];
	}
	if(isset($_SESSION['psource2'])) {
		$_POST['psource'] = $_SESSION['psource2'];
	}
	unset($_SESSION['pname2']);
	unset($_SESSION['pid2']);
	unset($_SESSION['pdob2']);
	unset($_SESSION['saccn2']);
	unset($_SESSION['sdate2']);
	unset($_SESSION['emod2']);
	unset($_SESSION['edesc2']);
	unset($_SESSION['psource2']);
}

echo <<<EOT
<HTML>
<!DOCTYPE !DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN"
            "http://www.w3.org/TR/html4/loose.dtd">
<HEAD>
    <!-- Written by Will Yonker-->
    <TITLE>PRIMAL-M Web Interface</TITLE>
    <link rel="stylesheet" href="default.css">
</HEAD>
<BODY>
EOT;
Display_Header3();

echo '<form action="index.php" method="post">';
echo '<H2>Enter your search criteria below</H2>';
if($_SESSION['is_error']==1)
{
	echo '<div id="error">' . $_SESSION['add_error'] . "</div>";
	unset($_SESSION['is_error']);
	unset($_SESSION['add_error']);
}
echo '% is the wildcard character<br>';
echo 'Queries are case insensitive<br><br><br>';
echo '<table border="1">';
echo '<tr>';
echo '<td>' . 'Patient Name:' . '</td>';
echo '<td>' . '<input type="text" name="pname" value="' . $_POST['pname'] . '" />' . '</td></tr>';
echo '<td>' . 'Patient MRN:' . '</td>';
echo '<td>' . '<input type="text" name="pid" value="' . $_POST['pid'] . '" />' . '</td></tr>';
echo '<td>' . 'Date of Birth (YYYY-MM-DD)' . '</td>';
echo '<td>' . '<input type="text" name="pdob" value="' . $_POST['pdob'] . '" />' . '</td></tr>';
echo '<td>' . 'Accession #' . '</td>';
echo '<td>' . '<input type="text" name="saccn" value="' . $_POST['saccn'] . '" />' . '</td></tr>';
echo '<td>' . 'Study Date (YYYY-MM-DD)' . '</td>';
echo '<td>' . '<input type="text" name="sdate" value="' . $_POST['sdate'] . '" />' . '</td></tr>';
echo '<td>' . 'Modality Type (US, CT, MR, etc)' . '</td>';
echo '<td>' . '<input type="text" name="emod" value="' . $_POST['emod'] . '" />' . '</td></tr>';
echo '<td>' . 'Study Description' . '</td>';
echo '<td>' . '<input type="text" name="edesc" value="' . $_POST['edesc'] . '" />' . '</td></tr>';
if(isset($_POST['psource'])) {
	if($_POST['psource']=="si") {
		echo '<tr><td>' . '<input type="radio" name="psource" value="magic" />Magic</td>';
		echo '<td>' . '<input type="radio" name="psource" value="si" checked/>Syngo Imaging</td></tr>';
	} else {
		echo '<tr><td>' . '<input type="radio" name="psource" value="magic" checked/>Magic</td>';
		echo '<td>' . '<input type="radio" name="psource" value="si" />Syngo Imaging</td></tr>';
	}
} else {
	echo '<tr><td>' . '<input type="radio" name="psource" value="magic" checked/>Magic</td>';
	echo '<td>' . '<input type="radio" name="psource" value="si" />Syngo Imaging</td></tr>';
}
		
echo '</table><br>';
echo '<br><button type="submit" name="query">Query</button>';
echo '<button type="submit" name="reset">Clear</button>';
echo '</form>';
if($_SESSION['login_sec_level'] >= 250)
{
	echo '<br><br><div id="setup"><a href="http://' . $_SERVER['HTTP_HOST'] . '/setup.php">Setup</a></div>';
}
Display_Footer();
echo '</BODY>';
echo '</HTML>';
unset($_SESSION['add_error']);
?>
