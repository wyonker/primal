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

//Need to detect if the user has tried to use a browser navigation button
if($_SESSION['is_started'] != "index.php")
{
    header("Location: index.php");
    exit();
} else {
    $_SESSION['is_started'] = "results.php";
}

if (substr($_SESSION['login_sec_bit'], 0, 1) != 1) {
    header("Location:  http://" . $_SERVER['HTTP_HOST'] . '/index.php');
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

header( "Refresh: 1800; URL=http://" . $_SERVER['HTTP_HOST'] . "/migration/logout.php");

unset($_SESSION['is_error']);
unset($_SESSION['add_error']);
//Need an additional check to see if the variable is set to prevent warnings in web log file
if(isset($_SESSION['pname'])) {
	if($_SESSION['pname'] == "")
	{ 
		$_SESSION['pname'] = "%"; 
	}
} else {
	$_SESSION['pname'] = "%";
}
if(isset($_SESSION['pid'])) {
	if($_SESSION['pid'] == "")
	{
		$_SESSION['pid'] = "%";
	}
} else {
	$_SESSION['pid'] = "%";
}
if(isset($_SESSION['pdob'])) {
	if($_SESSION['pdob'] == "")
	{
		$_SESSION['pdob'] = "%";
	}
} else {
	$_SESSION['pdob'] = "%";
}
if(isset($_SESSION['saccn'])) {
	if($_SESSION['saccn'] == "")
	{
		$_SESSION['saccn'] = "%";
	}
} else {
	$_SESSION['saccn'] = "%";
}
if(isset($_SESSION['sdate'])) {
	if($_SESSION['sdate'] == "")
	{
		$_SESSION['sdate'] = "%";
	}
} else {
	$_SESSION['sdate'] = "%";
}
if(isset($_SESSION['emod'])) {
	if($_SESSION['emod'] == "")
	{
		$_SESSION['emod'] = "%";
	}
} else {
	$_SESSION['emod'] = "%";
}
if(isset($_SESSION['edesc'])) {
	if($_SESSION['edesc'] == "")
	{
		$_SESSION['edesc'] = "%";
	}
} else {
	$_SESSION['edesc'] = "%";
}

//Session variables in case user wants to refine their serach
$_SESSION['pname2']=$_SESSION['pname'];
$_SESSION['pid2']=$_SESSION['pid'];
$_SESSION['pdob2']=$_SESSION['pdob'];
$_SESSION['saccn2']=$_SESSION['saccn'];
$_SESSION['sdate2']=$_SESSION['sdate'];
$_SESSION['emod2']=$_SESSION['emod'];
$_SESSION['edesc2']=$_SESSION['edesc'];
$_SESSION['psource2']=$_SESSION['psource'];

//$query="select * from patient where pname like '" . $_SESSION['pname'] . "' and pid like '" . $_SESSION['pid'] . "' and pdob like '" . $_SESSION['pdob'] . "';";
$query="select * from patient as p inner join study as s on p.puid = s.puid " ;
$query.="inner join series as e on p.puid = e.puid and s.siuid = e.siuid inner join image as i on p.puid = i.puid and s.siuid = i.siuid ";
$query.="where pname like '" . mysql_real_escape_string($_SESSION['pname']) . "' and pid like '" . $_SESSION['pid'] . "' ";
$query.="and pdob like '" . $_SESSION['pdob'] . "' and saccn like '" . $_SESSION['saccn'];
$query.="' and sdate like '" . $_SESSION['sdate'] . "' and e.rmodtype like '" . $_SESSION['emod'];
$query.="' and psource = '" . $_SESSION['psource'];
$query.="' and sdesc like '" . mysql_real_escape_string($_SESSION['edesc']) . "' and s.siuid is not NULL ";
//$query.="group by s.siuid having count(*) = 1 limit 101 ;";
//$query.="order by s.siuid;";
$query.="limit 1000;";

//echo "<br>" . $query . "<br>";
$result = mysql_query($query);
if(! $result)
{
	die("Error in MySQL qeury " . mysql_error() . "<br>");
}
$num_rows = mysql_num_rows($result);
$is_warn=0;
if($num_rows > 100)
{
	$warn_msg = "Warning:  Not all search results displayed.  Please refine your query.<br><br>";
	$is_warn =1;
}
#echo "num_rows = " . $num_rows . "<br>";
$lc1=0;
while($row = mysql_fetch_array($result, MYSQL_BOTH))
{
	//echo $row[8] . " " . $row[18] . "<br>";
    $patients[$lc1]["puid"] = $row["puid"];
    $patients[$lc1]["pname"] = $row["pname"];
    $patients[$lc1]["pid"] = $row["pid"];
    $patients[$lc1]["pdob"] = $row["pdob"];
    $patients[$lc1]["psex"] = $row["psex"];
	$patients[$lc1]["saccn"] = $row["saccn"];
	$patients[$lc1]["sdate"] = $row["sdate"];
	$patients[$lc1]["stime"] = $row["stime"];
	$patients[$lc1]["rmodtype"] = $row["rmodtype"];
	$patients[$lc1]["sdesc"] = $row["sdesc"];
	$patients[$lc1]["simg"] = $row["simg"];
	$patients[$lc1]["siuid"] = $row[9];

	$lc1++;
}

Display_Header3();
if($is_warn == 1)
{
	echo $warn_msg;
}
print_r($row);
echo '<table border="1">';
echo '<tr>';
echo '<tr><th colspan="10">Search Criteria (click row to modify)</th></tr>';
echo '<tr>';
echo '<th><a href="http://' . $_SERVER['HTTP_HOST'] . '/migration/cleanup.php?t=1">' . $_SESSION['pname'] . '</a></th>';
echo '<th><a href="http://' . $_SERVER['HTTP_HOST'] . '/migration/cleanup.php?t=1">' . $_SESSION['pid'] . '</a></th>';
echo '<th><a href="http://' . $_SERVER['HTTP_HOST'] . '/migration/cleanup.php?t=1">' . $_SESSION['pdob'] . '</a></th>';
echo '<th></th>';
echo '<th><a href="http://' . $_SERVER['HTTP_HOST'] . '/migration/cleanup.php?t=1">' . $_SESSION['saccn'] . '</a></th>';
echo '<th><a href="http://' . $_SERVER['HTTP_HOST'] . '/migration/cleanup.php?t=1">' . $_SESSION['edesc'] . '</a></th>';
echo '<th><a href="http://' . $_SERVER['HTTP_HOST'] . '/migration/cleanup.php?t=1">' . $_SESSION['sdate'] . '</a></th>';
echo '<th></th>';
echo '<th></th>';
echo '<th><a href="http://' . $_SERVER['HTTP_HOST'] . '/migration/index.php?t=1">' . $_SESSION['emod'] . '</a></th>';
echo '</tr>';
if($_SESSION['psource'] == "si") {
	echo '<tr><th colspan="10">Syngo Imaging Search Results (click assession # to send study)</th></tr>';
} else {
	echo '<tr><th colspan="10">Magic Search Results (click assession # to send study)</th></tr>';
}
echo '<th>Patient Name</th><th>MRN</th><th>DOB</th><th>Sex</th><th>Accession #</th><th>Study Description</th><th>Study Date</th>';
echo '<th>Study Time</th><th># Images</th><th>Modaity</th></tr>';
$lc3=0;
//while($lc3 < count($patients))
$first_row=0;
$lc4=0;
while($lc4 <= 100 && $lc3 < $num_rows)
{
	if($lc3 == 0) {
		if(trim($patients[$lc3]["saccn"]) == "" || trim($patients[$lc3]["saccn"]) == NULL)
        {
            $patients[$lc3]["saccn"] = "*Not Found*";
        }
		echo '<tr><td>' . $patients[$lc3]["pname"] . '</td>';
        echo '<td>' . $patients[$lc3]["pid"] . '</td>';
        echo '<td>' . $patients[$lc3]["pdob"] . '</td>';
        echo '<td>' . $patients[$lc3]["psex"] . '</td>';
		if( substr($_SESSION['login_sec_bit'], 1, 4) == "0000") {
			echo '<td>' . $patients[$lc3]["saccn"] . '</td>';
		} else {
			echo '<td><a href="http://' . $_SERVER['HTTP_HOST'] . '/migration/retrieve.php?r=' . $patients[$lc3]["siuid"] . '">' . $patients[$lc3]["saccn"] . '</a></td>';
		}
		echo '<td>' . $patients[$lc3]["sdesc"] . '</td>';
		echo '<td>' . $patients[$lc3]["sdate"] . '</td>';
		echo '<td>' . $patients[$lc3]["stime"] . '</td>';
		echo '<td>' . $patients[$lc3]["simg"] . '</td>';
		echo '<td>' . $patients[$lc3]["rmodtype"] . '</td></tr>';
		$lc4++;
	} elseif($patients[$lc3]["siuid"] != $patients[$lc3 - 1]["siuid"]) {
		if(trim($patients[$lc3]["saccn"]) == "" || trim($patients[$lc3]["saccn"]) == NULL)
		{
			$patients[$lc3]["saccn"] = "*Not Found*";
		}
		if($patients[$lc3]["puid"] == $patients[$lc3-1]["puid"])
		{
			echo '<tr><td></td><td></td><td></td><td></td>';
			$first_row++;
		} else {
			echo '<tr><td>' . $patients[$lc3]["pname"] . '</td>';
			echo '<td>' . $patients[$lc3]["pid"] . '</td>';
			echo '<td>' . $patients[$lc3]["pdob"] . '</td>';
			echo '<td>' . $patients[$lc3]["psex"] . '</td>';
		}
		if( substr($_SESSION['login_sec_bit'], 1, 4) == "0000") {
			echo '<td>' . $patients[$lc3]["saccn"] . '</td>';
		} else {
			echo '<td><a href="http://' . $_SERVER['HTTP_HOST'] . '/migration/retrieve.php?r=' . $patients[$lc3]["siuid"] . '">' . $patients[$lc3]["saccn"] . '</a></td>';
		}
		echo '<td>' . $patients[$lc3]["sdesc"] . '</td>';
		echo '<td>' . $patients[$lc3]["sdate"] . '</td>';
		echo '<td>' . $patients[$lc3]["stime"] . '</td>';
		echo '<td>' . $patients[$lc3]["simg"] . '</td>';
		echo '<td>' . $patients[$lc3]["rmodtype"] . '</td></tr>';
		$lc4++;
	}
	$lc3++;
}
echo '</table><br>';

echo '<br><br>';
echo '<a href="http://' . $_SERVER['HTTP_HOST'] . '/migration/cleanup.php">New Query</a>';
echo '<br><br>';
unset($_SESSION['pname']);
unset($_SESSION['pid']);
unset($_SESSION['pdob']);
unset($_SESSION['saccn']);
unset($_SESSION['sdate']);
unset($_SESSION['emod']);
unset($_SESSION['edesc']);
unset($_SESSION['psource']);

Display_Footer();
echo '</BODY>';
echo '</HTML>';
unset($_SESSION['add_error']);
?>
