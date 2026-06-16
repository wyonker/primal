<?php
	//License GPLv3
	//Version 1.00.01
	//2026-06-16
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

$intAdd = 0;
if($_SERVER['REQUEST_METHOD'] == 'POST') {
	if(isset($_POST['btnCancel'])) {
		header("Location: setup.php");
		exit();
	} elseif(isset($_POST['btnAdd'])) {
		$intAdd = 1;
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
	if($row['conf_name'] != "DBVER") {
		//Don't want to show the DBVER config item.  It should only be changed by dev when updating the database structure.
		$arrConfig[$row['conf_name']] = $row['conf_value'];
	}
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
if($intAdd == 1) {
	echo '<tr>';
	echo '<td><input type="text" name="new_conf_name"></td>';
	echo '<td><input type="text" name="new_conf_value"></td>';
	echo '</tr>';
}
echo '</tbody>';
echo '</table>';
echo '<input type="submit" name="btnUpdate" value="Update">';
echo '<input type="submit" name="btnReset" value="Reset">';
echo '<input type="submit" name="btnCancel" value="Cancel">';
if($intAdd == 0) {
	echo '<input type="submit" name="btnAdd" value="Add">';
}
echo '</form>';

?>
