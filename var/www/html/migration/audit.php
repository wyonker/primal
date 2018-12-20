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

if (substr($_SESSION['login_sec_bit'], 5, 1) != 1) {
    header("Location:  http://" . $_SERVER['HTTP_HOST'] . '/index.php');
}

if(isset($_GET["p"])) {
	$cur_page = $_GET["p"];
} else {
	$cur_page = 1;
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

$query="select * from audit order by adate DESC, atime desc limit 1000" ;
//echo "<br>" . $query . "<br>";
$result = mysql_query($query);
$num_rows = mysql_num_rows($result);
if(! $result)
{
	die("Error in MySQL qeury " . mysql_error() . "<br>");
}
$count_rows = mysql_num_rows($result);

Display_Header3();
echo '<table border="1">';
echo '<tr>';
echo '<th>ID</th><th>User ID</th><th>Date</th><th>Time</th><th>Type</th><th>Destination</th><th>Misc</th></tr>';
$LC2=0;
$DISP_MIN=$_SESSION['page_size'] * ($cur_page -1);
$DISP_MAX=$_SESSION['page_size'] * $cur_page;
while($LC2 <= $DISP_MAX) {
	$row = mysql_fetch_assoc($result);
	if($LC2 >= $DISP_MIN) {
		echo '<tr>';
		echo '<td>' . $row['id'] . '</td>';
		echo '<td>' . $row['aname'] . '</td>';
		echo '<td>' . $row['adate'] . '</td>';
		echo '<td>' . $row['atime'] . '</td>'; 
		echo '<td>' . $row['atype'] . '</td>'; 
		echo '<td>' . $row['adest'] . '</td>'; 
		echo '<td>' . $row['amisc'] . '</td>';
		echo '</tr>';
	}
	$LC2++;
}
echo '</table><br>';
$start_page=$cur_page-3;
$end_page=$cur_page+3;
if($count_rows > $_SESSION['page_size'] && $_SESSION['page_size'] != 0) {
	$LC1=1;
	$num_pages = floor($num_rows/$_SESSION['page_size']);
	echo '<a href="http://' . $_SERVER['HTTP_HOST'] . '/migration/audit.php?p=1">First</a> ';
	if ($cur_page > 1) {
		echo '<a href="http://' . $_SERVER['HTTP_HOST'] . '/migration/audit.php?p=' . ($cur_page-1) . '"> << </a> ';
	}
	while($LC1 <= $num_pages) {
		if($LC1 >= $start_page && $LC1 <= $end_page) {
			if($LC1 == $cur_page) {
				echo " <b>" . ($LC1) . "</b> " ;
			} else {
				echo '<a href="http://' . $_SERVER['HTTP_HOST'] . '/migration/audit.php?p=' . $LC1 . '">' . ($LC1) . '</a> ';
			}
		}
		$LC1++;
	}
	if ($cur_page < $num_pages) {
		echo '<a href="http://' . $_SERVER['HTTP_HOST'] . '/migration/audit.php?p=' . ($cur_page+1) . '"> >> </a> ';
	}
	echo '<a href="http://' . $_SERVER['HTTP_HOST'] . '/migration/audit.php?p=' . ($LC1-1) . '">Last</a> ';
}
Display_Footer();
echo '</BODY>';
echo '</HTML>';
?>
