<?php
    session_start();
    header( "Expires: Mon, 20 Dec 1998 01:00:00 GMT" );
    header( "Last-Modified: " . gmdate("D, d M Y H:i:s") . " GMT" );
    header( "Cache-Control: no-cache, must-revalidate" );
    header( "Pragma: no-cache" );
    require_once('config.php');
    require_once('functions.php');
    date_default_timezone_set('America/New_York');

if ($_SESSION['active'] != '1')
{
    header("Location: login.php");
    exit();
}
if (isset($_SESSION['obj']) || isset($_SESSION['obj1']))
{
    unset($_SESSION['obj']);
    unset($_SESSION['obj1']);
}

if (substr($_SESSION['login_sec_bit'], 0, 1) != 1) {
    header("Location:  http://" . $_SERVER['HTTP_HOST'] . '/index.php');
}

//Need to detect if the user has tried to use a browser navigation button
if(isset($_SESSION['is_started']) && $_SESSION['is_started'] != "retrieve.php")
{
	header("Location: index.php");
	exit();
} else {
	$_SESSION['is_started'] = "send.php";
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

$query="select * from image where siuid = '" . $_POST['sendsiuid'] . "';";
$result = mysql_query($query);
if (!$result)
{
    echo $query . "<br>";
    die('Invalid query: ' . mysql_error());
}
//echo $query . "<br>";
$num_rows = mysql_num_rows($result);
$row = mysql_fetch_assoc($result);
$ilocation=$row["ilocation"];
$puid=$row["puid"];
$siuid=$_POST['sendsiuid'];

$query="select * from patient where puid = '" . $puid . "';";
$result = mysql_query($query);
if (!$result)
{
    echo $query . "<br>";
    die('Invalid query: ' . mysql_error());
}
$num_rows = mysql_num_rows($result);
if($num_rows > 1)
{
    echo "More than one Patient with the unique (ish) id of " . $puid . " found...<br>";
    exit;
}
$row2 = mysql_fetch_assoc($result);
$pname=$row2["pname"];
$pid=$row2["pid"];
$pdob=$row2["pdob"];
$psex=$row2["psex"];

$query="select * from study where siuid = '" . $_POST['sendsiuid'] . "';";
$result = mysql_query($query);
if (!$result)
{
    echo $query . "<br>";
    die('Invalid query: ' . mysql_error());
}
$num_rows = mysql_num_rows($result);
if($num_rows > 1)
{
    echo "More than one study with the unique (ish) id of " . $_POST['sendsiuid'] . " found...<br>";
    exit;
}
$row3 = mysql_fetch_assoc($result);
$saccn=$row3["saccn"];
$simg=$row3["simg"];
$sdate=$row3["sdate"];
$stime=$row3["stime"];

echo 'Sending patient: <b>' . $pname . '</b> MRN: <b>' . $pid . '</b> DOB: <b>' . $pdob . '</b><br>';
echo 'Accesssion number: <b>' . $saccn . '</b> Study date: <b>' . $sdate . '</b> Study time: <b>' . $stime . '</b><br>';
echo "<br><br>";
echo "<table>";
echo "<th>Action</th><th>Result</th>";

chdir($_POST['senddir']);
if($_POST['dest']=="Other")
{
	system('export DCMDICTPATH="/home/dicom/share/dcmtk/dicom.dic"');
	unset($retval);
	$sendline = "/home/dicom/bin/storescu -xs +sd -ll debug -aet " . $_POST['sendaet'] . " -aec " . $_POST['sendaec'] . " " . $_POST['senddest'] . " " . $_POST['sendport'] . " " . "*";
	//echo $sendline . "<br>";
	exec($sendline, $retval, $return);
	if($return == 0)
	{
		echo '<tr><td>Images have been routed to ' . $_POST['senddest'] . ":" . $_POST['sendport'] . '.</td><td><img src="green_Check2.gif" alt="Success" height="30" width="30"></td></tr>';
	} else {
		 echo '<tr><td>There was an error sending the study.  Please try again or contact your administrator for assistance.</td><td><img src="red_x.gif" alt="Failed" height="30" width="30"></td></tr>';
	}
	$query = "insert into audit set aname = '" . $_SESSION['loginid'] . "', adate = '" . date('Y-m-d');
	$query .= "', atime = '" . date('H:i:s') . "', atype = 'send', adest = '" . $_POST['sendaet'] . " " . $_POST['sendport'];
	$query .= "', amisc = '" . $siuid . "'";
	$result = mysql_query($query);
	if(! $result)
	{
		echo "Error trying to insert into audit table!<br>";
		echo "Error in query: " . $query . "<br>";
		die("Error in MySQL qeury " . mysql_error() . ".<br>");
	}
} elseif($_POST['dest']=="Impax") {
	$primal_id = "4_" . date('YmdHis') . "_" . rand(0,1000);
	$destname = "/home/dicom/inbound/" . $primal_id;
	if ($handle = opendir('.'))
	{
		while (false !== ($entry = readdir($handle)))
		{
			if ($entry != "." && $entry != "..")
			{
				if(is_dir($entry))
				{
					clean_dir($entry);
				}
			}
		}
		closedir($handle);
	}
	recurse_copy($_POST['senddirshort'], $destname);
	$query = "insert into audit set aname = '" . $_SESSION['loginid'] . "', adate = '" . date('Y-m-d');
	$query .= "', atime = '" . date('H:i:s') . "', atype = 'send', adest = 'PACSSTOREC1 2104";
	$query .= "', amisc = '" . $siuid . "'";
	$result = mysql_query($query);
	if(! $result)
	{
		echo "Error trying to insert into audit table!<br>";
		echo "Error in query: " . $query . "<br>";
		die("Error in MySQL qeury " . mysql_error() . ".<br>");
	}
	echo '<tr><td>Files have been moved to PRIMAL for routing.  Please check the PRIMAL web interface <b><a href="http://' . $_SERVER['HTTP_HOST'] . '/index.php">HERE</a></b> for further information.</td><td><img src="green_Check2.gif" alt="Success" height="30" width="30"></td></tr>';
} elseif($_POST['dest']=="SMN02") {
    $primal_id = "5_" . date('YmdHis') . "_" . rand(0,1000);
    $destname = "/home/dicom/inbound/" . $primal_id;
    if ($handle = opendir('.'))
    {
        while (false !== ($entry = readdir($handle)))
        {
            if ($entry != "." && $entry != "..")
            {
                if(is_dir($entry))
                {
                    clean_dir($entry);
                }
            }
        }
        closedir($handle);
    }
    recurse_copy($_POST['senddirshort'], $destname);
    $query = "insert into audit set aname = '" . $_SESSION['loginid'] . "', adate = '" . date('Y-m-d');
    $query .= "', atime = '" . date('H:i:s') . "', atype = 'send', adest = 'PACSSTOREC1 2104";
    $query .= "', amisc = '" . $siuid . "'";
    $result = mysql_query($query);
    if(! $result)
    {
        echo "Error trying to insert into audit table!<br>";
        echo "Error in query: " . $query . "<br>";
        die("Error in MySQL qeury " . mysql_error() . ".<br>");
    }
    echo '<tr><td>Files have been moved to PRIMAL for routing.  Please check the PRIMAL web interface <b><a href="http://' . $_SERVER['HTTP_HOST'] . '/index.php">HERE</a></b> for further information.</td><td><img src="green_Check2.gif" alt="Success" height="30" width="30"></td></tr>';
} else {
	$primal_id = "3_" . date('YmdHis') . "_" . rand(0,1000);
	$destname = "/home/dicom/inbound/" . $primal_id;
	if ($handle = opendir('.'))
	{
		while (false !== ($entry = readdir($handle)))
		{
			if ($entry != "." && $entry != "..")
			{
				if(is_dir($entry))
				{
					clean_dir($entry);
				}
			}
		}
		closedir($handle);
	}
	recurse_copy($_POST['senddirshort'], $destname);
	$query = "insert into audit set aname = '" . $_SESSION['loginid'] . "', adate = '" . date('Y-m-d');
	$query .= "', atime = '" . date('H:i:s') . "', atype = 'send', adest = 'RADIDC3SAS 11112";
	$query .= "', amisc = '" . $siuid . "'";
	$result = mysql_query($query);
	if(! $result)
	{
		echo "Error trying to insert into audit table!<br>";
		echo "Error in query: " . $query . "<br>";
		die("Error in MySQL qeury " . mysql_error() . ".<br>");
	}
	echo '<tr><td>Files have been moved to PRIMAL for routing.  Please check the PRIMAL web interface <b><a href="http://' . $_SERVER['HTTP_HOST'] . '/index.php">HERE</a></b> for further information.</td><td><img src="green_Check2.gif" alt="Success" height="30" width="30"></td></tr>';
}

if($is_error != 1)
{
    chdir("/var/www/html/migration/");
    $dir = "tmp/" . $_POST['senddirshort'];
    $it = new RecursiveDirectoryIterator($dir, RecursiveDirectoryIterator::SKIP_DOTS);
    $files = new RecursiveIteratorIterator($it,
                 RecursiveIteratorIterator::CHILD_FIRST);
    foreach($files as $file) {
        if ($file->isDir()){
            rmdir($file->getRealPath());
        } else {
            unlink($file->getRealPath());
        }
    }
    rmdir($dir);
	unset($_SESSION['cleanupdir']);
}

echo '</table>';
echo '<br><br>';
echo '<a href="http://' . $_SERVER['HTTP_HOST'] . '/migration/cleanup.php">New Query</a>';

Display_Footer();
echo '</BODY>';
echo '</HTML>';
unset($_SESSION['add_error']);
?>
