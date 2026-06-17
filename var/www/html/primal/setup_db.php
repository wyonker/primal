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
$intUpdate = 0;
$intDelete = 0;
if($_SERVER['REQUEST_METHOD'] == 'POST') {
	if(isset($_POST['btnCancel'])) {
		header("Location: setup.php");
		exit();
	} elseif(isset($_POST['btnAdd'])) {
		$intAdd = 1;
	} elseif(isset($_POST['btnAdd2'])) {
		if(trim($_POST['new_conf_name']) != " " && trim($_POST['new_conf_value']) != " " && trim($_POST['new_conf_name']) != "" && trim($_POST['new_conf_value']) != "") {
			//First let's check to make sure the config name doesn't already exist.  We don't want duplicates.
			$strQuery = "SELECT * FROM config WHERE conf_name = '".$_POST['new_conf_name']."'";
			$result = mysqli_query($conn, $strQuery);
			if(mysqli_num_rows($result) > 0) {
				$intAdd = 2;
				$strNewConfName = $_POST['new_conf_name'];
				$strNewConfValue = $_POST['new_conf_value'];
			} else {
				$strQuery = "INSERT INTO config (conf_name, conf_value) VALUES ('".$_POST['new_conf_name']."', '".$_POST['new_conf_value']."')";
				mysqli_query($conn, $strQuery);
				header("Location: setup_db.php");
				exit();
			}
		} elseif(trim($_POST['new_conf_name']) != "" && trim($_POST['new_conf_name']) != " ") {
			//They entered a config name but not a value.  We need both to add a new config item.
			$intAdd = 2;
			$strNewConfName = $_POST['new_conf_name'];
			$strNewConfValue = $_POST['new_conf_value'];
		}
	} elseif(isset($_POST['btnUpdate'])) {
		$intUpdate = 1;
	} elseif(isset($_POST['btnUpdate2'])) {
		//We will loop through the posted values and look for checkboxes that are checked.  The name of the checkbox will be the conf_name and the value will be the old conf_value.  We will then look for a text box with the name of the conf_name + "_value" to get the new value.  If the checkbox is checked we will update that config item with the new value.
		foreach($_POST as $key => $value) {
			if(strpos($key, '_value') !== false) {
				//This is a text box value.  We need to check if the corresponding checkbox is checked.
				$conf_name = str_replace('_value', '', $key);
				if(isset($_POST[$conf_name])) {
					//The checkbox is checked.  We will update this config item with the new value.
					$strQuery = "UPDATE config SET conf_value = '".$value."' WHERE conf_name = '".$conf_name."'";
					mysqli_query($conn, $strQuery);
				}
			}
		}
		header("Location: setup_db.php");
		exit();
	} elseif(isset($_POST['btnDelete'])) {
		$intDelete = 1;
	} elseif(isset($_POST['btnDelete2'])) {
		//We will loop through the posted values and look for checkboxes that are checked.  The name of the checkbox will be the conf_name and the value will be the conf_value.  If the checkbox is checked we will delete that config item.
		foreach($_POST as $key => $value) {
			if(strpos($key, '_value') === false) {
				//This is a checkbox value.  We need to check if it is checked.
				if(isset($_POST[$key])) {
					//The checkbox is checked.  We will delete this config item.
					$strQuery = "DELETE FROM config WHERE conf_name = '".$key."'";
					mysqli_query($conn, $strQuery);
				}
			}
		}
		header("Location: setup_db.php");
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
if($intUpdate == 1 || $intDelete == 1) {
	//Put a checkbox in front of each config item so they can select which ones to update.  We will use the conf_name as the name of the checkbox and the conf_value as the value.  When they submit the form we will check which checkboxes are checked and update those config items with the new values they entered in the text boxes.
	foreach($arrConfig as $key => $value) {
		if($intDelete == 1 && $key == "use_db") {
			//Don't allow them to delete the use_db config item.  This could cause problems if they delete it and then can't get back into the setup page to add it back.
			continue;
		}
		echo '<tr>';
		echo '<td><input type="checkbox" name="'.$key.'" value="'.$value.'"> '.$key.'</td>';
		echo '<td><input type="text" name="'.$key.'_value" value="'.$value.'"></td>';
		echo '</tr>';
	}
} else {
	//Just show the config items with text boxes to edit the values.  If they want to update they will click the update button and we will show the checkboxes then.
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
	} elseif($intAdd == 2) {
		echo '<tr>';
		echo '<td><input type="text" name="new_conf_name" value="'.$strNewConfName.'"></td>';
		echo '<td><input type="text" name="new_conf_value" value="'.$strNewConfValue.'"></td>';
		echo '</tr>';
	}
}
echo '</tbody>';
echo '</table>';
if($intUpdate == 1) {
	echo '<input type="submit" name="btnUpdate2" value="Update">';
} else {
	echo '<input type="submit" name="btnUpdate" value="Update">';
}
echo '<input type="submit" name="btnReset" value="Reset">';
echo '<input type="submit" name="btnCancel" value="Cancel">';
if($intAdd == 0) {
	echo '<input type="submit" name="btnAdd" value="Add">';
} else {
	echo '<input type="submit" name="btnAdd2" value="Add">';
}
if($intDelete == 1) {
	echo '<input type="submit" name="btnDelete2" value="Delete">';
} else {
	echo '<input type="submit" name="btnDelete" value="Delete">';
}
echo '<br><br>';
echo '<h2>Currently Implemented Config Items</h2>';
echo '<ul>';
echo '<li><strong>use_db</strong> - Use the DB for configuration values or /etc/primal/primal.conf  (only one will be used)</li>';
echo '</ul>';
echo '</form>';

?>
