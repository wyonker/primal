<?php
	//License GPLv3
	//Version 1.00.00
	//2026-06-15
    session_start();
    header( "Expires: Mon, 20 Dec 1998 01:00:00 GMT" );
    header( "Last-Modified: " . gmdate("D, d M Y H:i:s") . " GMT" );
    header( "Cache-Control: no-cache, must-revalidate" );
    header( "Pragma: no-cache" );
    require_once('config.php');
    require_once('functions.php');

if ($_SESSION['active'] != '1') {
    header("Location: login.php");
    exit();
}

require_once('config.php');
require_once('functions.php');

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

if($_SERVER['REQUEST_METHOD'] == 'POST') {
	if(isset($_POST['btnCancel'])) {
		header("Location: setup.php");
		exit();
	} elseif(isset($_POST['btnAdd'])) {
		exit();
	} elseif(isset($_POST['btnUpdate'])) {
		exit();
	} elseif(isset($_POST['btnDelete'])) {
		exit();
	} elseif(isset($_POST['btnReset'])) {
		header("Location: setup_db.php");
		exit();
	} else {
		header("Location: setup.php");
		exit();
	}
}

Display_Header2();
echo "<H2>System Setup DB</H2>";
$strQuery = "SELECT * FROM config";
$result = mysqli_query($conn, $strQuery);
while($row = mysqli_fetch_assoc($result)) {
	$arrConfig[$row['name']] = $row['value'];
}
echo '<form action="setup_db.php" method="post">';
echo '<table border="1">';
echo '<thead>';
echo '<tr>';
echo '<th>Config Name</th>';
echo '<th>Config Value</th>';
echo '</tr>';
echo '</thead>';
echo '<tbody>';
foreach($arrConfig as $key => $value) {
	echo '<tr>';
	echo '<td>'.$key.'</td>';
	echo '<td><input type="text" name="'.$key.'" value="'.$value.'"></td>';
	echo '</tr>';
}
echo '</tbody>';
echo '</table>';
echo '<input type="submit" name="btnUpdate" value="Update">';
echo '<input type="submit" name="btnReset" value="Reset">';
echo '<input type="submit" name="btnCancel" value="Cancel">';
echo '</form>';

?>
