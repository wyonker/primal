<?php
    //License GPLv3
    session_start();
    header( "Expires: Mon, 20 Dec 1998 01:00:00 GMT" );
    header( "Last-Modified: " . gmdate("D, d M Y H:i:s") . " GMT" );
    header( "Cache-Control: no-cache, must-revalidate" );
    header( "Pragma: no-cache" );
    require_once('config.php');
    require_once('functions.php');

if ($_SESSION['active'] != '1')
{
    header("Location: login.php");
    exit();
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

if($_SERVER['REQUEST_METHOD'] == 'POST') {
	if(isset($_POST['btnCancel'])) {
		unset($_SESSION["conf_rec"]);
		unset($_SESSION["conf_proc"]);
		unset($_SESSION["conf_send"]);
		header("Location: setup.php");
		exit();
	} elseif(isset($_POST['btnAdd'])) {
		$intReturn = fValidateInput(1, $_POST['conf_name']);
		if($intReturn == 1) {
			exit();
		}
		$strQuery = "INSERT INTO conf_rec SET ";
		$strQuery .= "conf_name = '" . $_POST['conf_name'] . "', ";
		$strQuery .= "conf_server = '" . $_POST['conf_server'] . "', ";
		$strQuery .= "rec_type = '" . $_POST['rec_type'] . "', ";
		$strQuery .= "rec_port = '" . $_POST['rec_port'] . "', ";
		$strQuery .= "rec_dir = '" . $_POST['rec_dir'] . "', ";
		$strQuery .= "rec_log_full_path = '" . $_POST['rec_log_full_path'] . "', ";
		$strQuery .= "rec_log_level = '" . $_POST['rec_log_level'] . "', ";
		$strQuery .= "rec_aet = '" . $_POST['rec_aet'] . "', ";
		$strQuery .= "rec_time_out = '" . $_POST['rec_time_out'] . "', ";
		$strQuery .= "proc_dir = '" . $_POST['proc_dir'] . "', ";
		$strQuery .= "proc_log_full_path = '" . $_POST['proc_log_full_path'] . "', ";
		$strQuery .= "out_dir = '" . $_POST['out_dir'] . "', ";
		$strQuery .= "out_log_full_path = '" . $_POST['out_log_full_path'] . "', ";
		$strQuery .= "sent_dir = '" . $_POST['sent_dir'] . "', ";
		$strQuery .= "hold_dir = '" . $_POST['hold_dir'] . "', ";
		$strQuery .= "error_dir = '" . $_POST['error_dir'] . "', ";
		$strQuery .= "dupe = '" . $_POST['dupe'] . "', ";
		$strQuery .= "out_comp_level = '" . $_POST['out_comp_level'] . "', ";
		$strQuery .= "pass_through = '" . $_POST['pass_through'] . "', ";
		$strQuery .= "ret_period = '" . $_POST['ret_period'] . "', ";
		$strQuery .= "active = '" . $_POST['active'] . "';";

		$result = mysqli_query($conn, $strQuery);
		if($result) {
			header("Location: setup.php");
		} else {
			echo "Error adding record: " . mysqli_error($conn) . "<br>";
			echo "Query = " . $strQuery . "<br>";
			exit();
		}
	} elseif(isset($_POST['btnUpdate'])) {
		$strQuery = "UPDATE conf_rec SET ";
		$strQuery .= "conf_name = '" . $_POST['conf_name'] . "', ";
		$strQuery .= "conf_server = '" . $_POST['conf_server'] . "', ";
		$strQuery .= "rec_type = '" . $_POST['rec_type'] . "', ";
		$strQuery .= "rec_port = '" . $_POST['rec_port'] . "', ";
		$strQuery .= "rec_dir = '" . $_POST['rec_dir'] . "', ";
		$strQuery .= "rec_log_full_path = '" . $_POST['rec_log_full_path'] . "', ";
		$strQuery .= "rec_log_level = '" . $_POST['rec_log_level'] . "', ";
		$strQuery .= "rec_aet = '" . $_POST['rec_aet'] . "', ";
		$strQuery .= "rec_time_out = '" . $_POST['rec_time_out'] . "', ";
		$strQuery .= "proc_dir = '" . $_POST['proc_dir'] . "', ";
		$strQuery .= "proc_log_full_path = '" . $_POST['proc_log_full_path'] . "', ";
		$strQuery .= "out_dir = '" . $_POST['out_dir'] . "', ";
		$strQuery .= "out_log_full_path = '" . $_POST['out_log_full_path'] . "', ";
		$strQuery .= "sent_dir = '" . $_POST['sent_dir'] . "', ";
		$strQuery .= "hold_dir = '" . $_POST['hold_dir'] . "', ";
		$strQuery .= "error_dir = '" . $_POST['error_dir'] . "', ";
		$strQuery .= "dupe = '" . $_POST['dupe'] . "', ";
		$strQuery .= "out_comp_level = '" . $_POST['out_comp_level'] . "', ";
		$strQuery .= "pass_through = '" . $_POST['pass_through'] . "', ";
		$strQuery .= "ret_period = '" . $_POST['ret_period'] . "', ";
		$strQuery .= "active = '" . $_POST['active'] . "' ";
		$strQuery .= "WHERE conf_rec_id = " . $_POST['conf_rec_id'] . " limit 1;";

		$result = mysqli_query($conn, $strQuery);
		if($result) {
			header("Location: setup.php");
		} else {
			echo "Error updating record: " . mysqli_error($conn) . "<br>";
			echo "Query = " . $strQuery . "<br>";
			exit();
		}
	} elseif(isset($_POST['btnBack'])) {
		header("Location: setup.php");
		exit();
	} else {
		echo "Error:  Unknown action...<br>";
		exit();
	}
}

Display_Header2();

echo "<H2>System Setup</H2>";
if($_GET['action'] == 'N') {
	if(isset($_GET['rec'])) {
		$strQuery = "SELECT * FROM conf_rec WHERE conf_rec_id = " . $_GET['rec'] . " limit 1;";
		$result = mysqli_query($conn, $strQuery);
		$row = mysqli_fetch_assoc($result);
		echo '<form action="setup.php?action=save" method="post">';
		echo '<input type="hidden" name="conf_rec_id" value ="' . $row["conf_rec_id"] . '" />';
		echo '<table border="1">';
		echo '<tr><td>' . 'Config Name:</td>';
		echo '<td><input type="text" name="conf_name" value ="' . $row["conf_name"] . '" /></td></tr>';
		echo '<tr><td>' . 'Server FQDN or IP' . '</td>';
		echo '<td><input type="text" name="conf_server" value ="' . $row["conf_server"] . '" /></td></tr>';
		echo '<tr><td>' . '<label for="rec_type">Receiver Type:</label></td><td>';
		echo '<select name="rec_type" id="rec_type">';
		echo '<option value="1" selected>Dicom</option>';
		echo '<option value="2">Directory</option>';
		echo '</select></td></tr>';
		echo '<tr><td>' . 'Port number to listen on (not used for directory type)' . '</td>';
		echo '<td><input type="text" name="rec_port" value ="' . $row["rec_port"] . '" /></td></tr>';
		echo '<tr><td>' . 'Absolute directory path for receiver data' . '</td>';
		echo '<td><input type="text" name="rec_dir" value ="' . $row["rec_dir"] . '" /></td></tr>';
		echo '<tr><td>' . 'Absolute directory path for receiver logs' . '</td>';
		echo '<td><input type="text" name="rec_log_full_path" value ="' . $row["rec_log_full_path"] . '" /></td></tr>';
		echo '<tr><td>' . '<label for="rec_log_level">Log Level:</label></td><td>';
		echo '<select name="rec_log_level" id="rec_log_level">';
		echo '<option value="1">fatal</option>';
		echo '<option value="2">error</option>';
		echo '<option value="3">warn</option>';
		echo '<option value="4">info</option>';
		echo '<option value="5" selected><b>debug</b></option>';
		echo '<option value="6">trace</option>';
		echo '</select></td></tr>';
		echo '<tr><td>' . 'AET' . '</td>';
		echo '<td><input type="text" name="rec_aet" value ="' . $row["rec_aet"] . '" /></td></tr>';
		echo '<tr><td>' . 'Time Out (seconds)' . '</td>';
		echo '<td><input type="text" name="rec_time_out" value ="' . $row["rec_time_out"] . '" /></td></tr>';
		echo '<tr><td>' . 'Absolute directory path for moving data to for processing' . '</td>';
		echo '<td><input type="text" name="proc_dir" value ="' . $row["proc_dir"] . '" /></td></tr>';
		echo '<tr><td>' . 'Absolute directory path for processing logs' . '</td>';
		echo '<td><input type="text" name="proc_log_full_path" value ="' . $row["proc_log_full_path"] . '" /></td></tr>';
		echo '<tr><td>' . 'Absolute directory path for moving outging data to' . '</td>';
		echo '<td><input type="text" name="out_dir" value ="' . $row["out_dir"] . '" /></td></tr>';
		echo '<tr><td>' . 'Absolute directory path for outgoing logs' . '</td>';
		echo '<td><input type="text" name="out_log_full_path" value ="' . $row["out_log_full_path"] . '" /></td></tr>';
		echo '<tr><td>' . 'Absolute directory path for sent files' . '</td>';
		echo '<td><input type="text" name="sent_dir" value ="' . $row["sent_dir"] . '" /></td></tr>';
		echo '<tr><td>' . 'Absolute directory path for held files' . '</td>';
		echo '<td><input type="text" name="hold_dir" value ="' . $row["hold_dir"] . '" /></td></tr>';
		echo '<tr><td>' . 'Absolute directory path for errored files' . '</td>';
		echo '<td><input type="text" name="error_dir" value ="' . $row["error_dir"] . '" /></td></tr>';
		echo '<tr><td>' . '<label for="dupe">Deduplication (WARNING!  This is a CPU hog):</label></td><td>';
		echo '<select name="dupe" id="dupe">';
		echo '<option value="1">Yes</option>';
		echo '<option value="2" selected><b>No</b></option>';
		echo '</select></td></tr>';
		echo '<tr><td>' . '<label for="out_comp_level">Compression Level (lower is faster):</label></td><td>';
		echo '<select name="out_comp_level" id="out_comp_level">';
		echo '<option value="1">1</option>';
		echo '<option value="2">2</option>';
		echo '<option value="3">3</option>';
		echo '<option value="4">4</option>';
		echo '<option value="5">5</option>';
		echo '<option value="6" selected><b>6</b></option>';
		echo '<option value="7">7</option>';
		echo '<option value="8">8</option>';
		echo '<option value="9">9</option>';
		echo '</select></td></tr>';
		echo '<tr><td>' . '<label for="pass_through">Use AET of sender?:</label></td><td>';
		echo '<select name="pass_through" id="pass_through">';
		echo '<option value="1">Yes</option>';
		echo '<option value="2" selected><b>No</b></option>';
		echo '</select></td></tr>';
		echo '<tr><td>' . 'Store sent files (minutes)' . '</td>';
		echo '<td><input type="text" name="ret_period" value ="' . $row["ret_period"] . '" /></td></tr>';
		echo '<tr><td>' . '<label for="active">Active?:</label></td><td>';
		echo '<select name="active" id="active" default=1>';
		echo '<option value="1"><b>Yes</b></option>';
		echo '<option value="2">No</option>';
		echo '</select></td></tr>';
		echo '</table>';
		echo '<br><button type="submit" id="btnUpdate" name="btnUpdate">Update</button>';
		echo '<button type="submit" id="btnReset" name="btnReset">Clear</button>';
		echo '<button type="submit" id="btnCancel" name="btnCancel">Cancel</button>';
		echo '</form>';
		} else {
		echo '<form action="setup.php?action=save" method="post">';
		echo '<table border="1">';
		echo '<tr><td>' . 'Config Name:</td>';
		echo '<td><input type="text" name="conf_name"></td></tr>';
		echo '<tr><td>' . 'Server FQDN or IP' . '</td>';
		echo '<td><input type="text" name="conf_server"></td></tr>';
		echo '<tr><td>' . '<label for="rec_type">Receiver Type:</label></td><td>';
		echo '<select name="rec_type" id="rec_type">';
		echo '<option value="1" selected>Dicom</option>';
		echo '<option value="2">Directory</option>';
		echo '</select></td></tr>';
		echo '<tr><td>' . 'Port number to listen on (not used for directory type)' . '</td>';
		echo '<td><input type="text" name="rec_port"></td></tr>';
		echo '<tr><td>' . 'Absolute directory path for receiver data' . '</td>';
		echo '<td><input type="text" name="rec_dir"></td></tr>';
		echo '<tr><td>' . 'Absolute directory path for receiver logs' . '</td>';
		echo '<td><input type="text" name="rec_log_full_path"></td></tr>';
		echo '<tr><td>' . '<label for="rec_log_level">Log Level:</label></td><td>';
		echo '<select name="rec_log_level" id="rec_log_level">';
		echo '<option value="1">fatal</option>';
		echo '<option value="2">error</option>';
		echo '<option value="3">warn</option>';
		echo '<option value="4">info</option>';
		echo '<option value="5" selected><b>debug</b></option>';
		echo '<option value="6">trace</option>';
		echo '</select></td></tr>';
		echo '<tr><td>' . 'AET' . '</td>';
		echo '<td><input type="text" name="rec_aet"></td></tr>';
		echo '<tr><td>' . 'Time Out (seconds)' . '</td>';
		echo '<td><input type="text" name="rec_time_out"></td></tr>';
		echo '<tr><td>' . 'Absolute directory path for moving data to for processing' . '</td>';
		echo '<td><input type="text" name="proc_dir"></td></tr>';
		echo '<tr><td>' . 'Absolute directory path for processing logs' . '</td>';
		echo '<td><input type="text" name="proc_log_full_path"></td></tr>';
		echo '<tr><td>' . 'Absolute directory path for moving outging data to' . '</td>';
		echo '<td><input type="text" name="out_dir"></td></tr>';
		echo '<tr><td>' . 'Absolute directory path for outgoing logs' . '</td>';
		echo '<td><input type="text" name="out_log_full_path"></td></tr>';
		echo '<tr><td>' . 'Absolute directory path for sent files' . '</td>';
		echo '<td><input type="text" name="sent_dir"></td></tr>';
		echo '<tr><td>' . 'Absolute directory path for held files' . '</td>';
		echo '<td><input type="text" name="hold_dir"></td></tr>';
		echo '<tr><td>' . 'Absolute directory path for errored files' . '</td>';
		echo '<td><input type="text" name="error_dir"></td></tr>';
		echo '<tr><td>' . '<label for="dupe">Deduplication (WARNING!  This is a CPU hog):</label></td><td>';
		echo '<select name="dupe" id="dupe">';
		echo '<option value="1">Yes</option>';
		echo '<option value="2" selected><b>No</b></option>';
		echo '</select></td></tr>';
		echo '<tr><td>' . '<label for="out_comp_level">Compression Level (lower is faster):</label></td><td>';
		echo '<select name="out_comp_level" id="out_comp_level">';
		echo '<option value="1">1</option>';
		echo '<option value="2">2</option>';
		echo '<option value="3">3</option>';
		echo '<option value="4">4</option>';
		echo '<option value="5">5</option>';
		echo '<option value="6" selected><b>6</b></option>';
		echo '<option value="7">7</option>';
		echo '<option value="8">8</option>';
		echo '<option value="9">9</option>';
		echo '</select></td></tr>';
		echo '<tr><td>' . '<label for="pass_through">Use AET of sender?:</label></td><td>';
		echo '<select name="pass_through" id="pass_through">';
		echo '<option value="1">Yes</option>';
		echo '<option value="2" selected><b>No</b></option>';
		echo '</select></td></tr>';
		echo '<tr><td>' . 'Store sent files (minutes)' . '</td>';
		echo '<td><input type="text" name="ret_period"></td></tr>';
		echo '<tr><td>' . '<label for="active">Active?:</label></td><td>';
		echo '<select name="active" id="active" default=1>';
		echo '<option value="1"><b>Yes</b></option>';
		echo '<option value="2">No</option>';
		echo '</select></td></tr>';
		echo '</table>';
		echo '<br><button type="submit" id="btnAdd" name="btnAdd">Add</button>';
		echo '<button type="submit" id="btnReset" name="btnReset">Clear</button>';
		echo '<button type="submit" id="btnCancel" name="btnCancel">Cancel</button>';
		echo '</form>';
		}
} elseif($_GET['action'] == 'E') {
	if(isset($_GET['rec'])) {
		$intRec = $_GET['rec'];
		$strQuery="SELECT * FROM conf_rec WHERE conf_rec_id = " . $intRec . ";";
		$result = mysqli_query($conn, $strQuery);
		$row = mysqli_fetch_assoc($result);

		echo '<form action="setup.php" method="post">';
		echo '<table border="1">';
		echo '<tr><td>' . 'Config Name:</td>';
		echo '<td><input type="text" name="conf_name" value ="' . $row["conf_name"] . '" /></td></tr>';
		echo '<tr><td>' . 'Server FQDN or IP' . '</td>';
		echo '<td><input type="text" name="conf_server" value ="' . $row["conf_server"] . '" /></td></tr>';
		echo '<tr><td>' . '<label for="rec_type">Receiver Type:</label></td><td>';
		echo '<select name="rec_type" id="rec_type">';
		echo '<option value="1" selected>Dicom</option>';
		echo '<option value="2">Directory</option>';
		echo '</select></td></tr>';
		echo '<tr><td>' . 'Port number to listen on (not used for directory type)' . '</td>';
		echo '<td><input type="text" name="rec_port" value ="' . $row["rec_port"] . '" /></td></tr>';
		echo '<tr><td>' . 'Absolute directory path for receiver data' . '</td>';
		echo '<td><input type="text" name="rec_dir" value ="' . $row["rec_dir"] . '" /></td></tr>';
		echo '<tr><td>' . 'Absolute directory path for receiver logs' . '</td>';
		echo '<td><input type="text" name="rec_log_full_path" value ="' . $row["rec_log_full_path"] . '" /></td></tr>';
		echo '<tr><td>' . '<label for="rec_log_level">Log Level:</label></td><td>';
		echo '<select name="rec_log_level" id="rec_log_level">';
		echo '<option value="1">fatal</option>';
		echo '<option value="2">error</option>';
		echo '<option value="3">warn</option>';
		echo '<option value="4">info</option>';
		echo '<option value="5" selected><b>debug</b></option>';
		echo '<option value="6">trace</option>';
		echo '</select></td></tr>';
		echo '<tr><td>' . 'AET' . '</td>';
		echo '<td><input type="text" name="rec_aet" value ="' . $row["rec_aet"] . '" /></td></tr>';
		echo '<tr><td>' . 'Time Out' . '</td>';
		echo '<td><input type="text" name="rec_time_out" value ="' . $_SESSION["rec_time_out"] . '" /></td></tr>';
		echo '<tr><td>' . 'Absolute directory path for moving data to for processing' . '</td>';
		echo '<td><input type="text" name="proc_dir" value ="' . $row["proc_dir"] . '" /></td></tr>';
		echo '<tr><td>' . 'Absolute directory path for processing logs' . '</td>';
		echo '<td><input type="text" name="proc_log_full_path" value ="' . $row["proc_log_full_path"] . '" /></td></tr>';
		echo '<tr><td>' . 'Absolute directory path for moving outging data to' . '</td>';
		echo '<td><input type="text" name="out_dir" value ="' . $row["out_dir"] . '" /></td></tr>';
		echo '<tr><td>' . 'Absolute directory path for outgoing logs' . '</td>';
		echo '<td><input type="text" name="out_log_full_path" value ="' . $row["out_log_full_path"] . '" /></td></tr>';
		echo '<tr><td>' . 'Absolute directory path for sent files' . '</td>';
		echo '<td><input type="text" name="sent_dir" value ="' . $row["sent_dir"] . '" /></td></tr>';
		echo '<tr><td>' . 'Absolute directory path for held files' . '</td>';
		echo '<td><input type="text" name="hold_dir" value ="' . $row["hold_dir"] . '" /></td></tr>';
		echo '<tr><td>' . 'Absolute directory path for errored files' . '</td>';
		echo '<td><input type="text" name="error_dir" value ="' . $row["error_dir"] . '" /></td></tr>';
		echo '<tr><td>' . '<label for="dupe">Deduplication (WARNING!  This is a CPU hog):</label></td><td>';
		echo '<select name="dupe" id="dupe">';
		echo '<option value="1">Yes</option>';
		echo '<option value="2" selected><b>No</b></option>';
		echo '</select></td></tr>';
		echo '<tr><td>' . '<label for="out_comp_level">Compression Level (lower is faster):</label></td><td>';
		echo '<select name="out_comp_level" id="out_comp_level">';
		echo '<option value="1">1</option>';
		echo '<option value="2">2</option>';
		echo '<option value="3">3</option>';
		echo '<option value="4">4</option>';
		echo '<option value="5">5</option>';
		echo '<option value="6" selected><b>6</b></option>';
		echo '<option value="7">7</option>';
		echo '<option value="8">8</option>';
		echo '<option value="9">9</option>';
		echo '</select></td></tr>';
		echo '<tr><td>' . '<label for="pass_through">Use AET of sender?:</label></td><td>';
		echo '<select name="pass_through" id="pass_through">';
		echo '<option value="1">Yes</option>';
		echo '<option value="2" selected><b>No</b></option>';
		echo '</select></td></tr>';
		echo '<tr><td>' . 'Minutes to store sent files' . '</td>';
		echo '<td><input type="text" name="ret_period" value ="' . $row["ret_period"] . '" /></td></tr>';
		echo '<tr><td>' . '<label for="active">Active?:</label></td><td>';
		echo '<select name="active" id="active" default=1>';
		echo '<option value="1"><b>Yes</b></option>';
		echo '<option value="2">No</option>';
		echo '</select></td></tr>';
		echo '</table>';
		echo '<br><button type="submit" id="btnAdd" name="btnAdd">Add</button>';
		echo '<button type="submit" id="btnReset" name="btnReset">Clear</button>';
		echo '<button type="submit" id="btnCancel" name="btnCancel">Cancel</button>';
		echo '</form>';
	}
} else {
	$strQuery = "SELECT conf_rec_id, conf_name FROM conf_rec;";
	$result = mysqli_query($conn, $strQuery);
	$intNumRows = mysqli_num_rows($result);
	$intLC=1;
	while($intLC <= ($intNumRows + 1)) {
		$row = mysqli_fetch_assoc($result);
		echo '<div id="rec" style="top: ' . $intLC * 150 . ';">';
		if($intLC > $intNumRows) {
			echo '<BR><BR><a href="/primal/setup.php?action=E&rec=0">New</a><BR><BR>';
		} else {
			echo '<BR><BR><a href="/primal/setup.php?action=N&rec=' . $row['conf_rec_id'] . '">' . $row['conf_name'] . '</a><BR><BR>';
		}
		echo '</div>';

		echo '<svg width="350" height="150"><line x1="100" y1="70" x2="350" y2="70" stroke="black"/></svg>';

		echo '<div id="proc" style="top: ' . $intLC * 150 . ';">';
		if(!isset($row['conf_rec_id'])) {
			echo '<BR><BR>No<BR>Receiver<BR>Selected';
		} else {
			$strQuery2 = "SELECT conf_proc_id, proc_name FROM conf_proc WHERE conf_rec_id = " . $row['conf_rec_id'] . ";";
			$result2 = mysqli_query($conn, $strQuery2);
			$intNumRows2 = mysqli_num_rows($result2);
			$intLC2=0;
			while($intLC2 <= $intNumRows2) {
				$row2 = mysqli_fetch_assoc($result2);
				echo '<BR><BR><a href="/primal/setup.php?action=E&proc=' . $row2['conf_proc_id'] . '">' . $row2['proc_name'] . '</a><BR><BR>';
				$intLC2++;
			}
		}
		echo '</div>';

		echo '<svg width="300" height="150"><line x1="50" y1="70" x2="350" y2="70" stroke="black"/></svg>';
		
		echo '<div id="out" style="top: ' . $intLC * 150 . ';">';
		if(!isset($row['conf_rec_id'])) {
			echo '<BR><BR>No<BR>Receiver<BR>Selected';
		} else {
			$strQuery2 = "SELECT conf_send_id, send_name FROM conf_send WHERE conf_rec_id = " . $row['conf_rec_id'] . ";";
			$result2 = mysqli_query($conn, $strQuery2);
			$intNumRows2 = mysqli_num_rows($result2);
			$intLC2=0;
			while($intLC2 <= $intNumRows2) {
				$row2 = mysqli_fetch_assoc($result2);
				echo '<BR><BR><a href="/primal/setup.php?action=N&proc=' . $row2['conf_send_id'] . '">' . $row2['send_name'] . '</a><BR><BR>';
				$intLC2++;
			}
		}
		echo '</div>';
		echo '<BR>';
		$intLC++;
	}
	echo '<BR><BR>';
}
Display_Footer();
echo '</BODY>';
echo '</HTML>';

fValidateInput($intType, $strUserInput) {
	if($intType == 1) {
		if(!preg_match("/^[a-zA-Z0-9_ ]*$/", $strUserInput)) {
			echo "Invalid input:  " . $strUserInput . "<br>";
			return 1;
		} else {
			return 0;
		}
	} elseif($intType == 2) {
		if(preg_match("/^((25[0-5]|(2[0-4]|1\d|[1-9]|)\d)\.?\b){4}$/", $strUserInput)) {
			return 0;
		}
		
	} elseif($intType == 3) {
		if(!preg_match("/^[a-zA-Z0-9_]*$/", $strUserInput)) {
			echo "Invalid input:  " . $strUserInput . "<br>";
			exit();
		}
	} elseif($intType == 4) {
		if(!preg_match("/^[a-zA-Z0-9_]*$/", $strUserInput)) {
			echo "Invalid input:  " . $strUserInput . "<br>";
			exit();
		}
	} elseif($intType == 5) {
		if(!preg_match("/^[a-zA-Z0-9_]*$/", $strUserInput)) {
			echo "Invalid input:  " . $strUserInput . "<br>";
			exit();
		}
	} elseif($intType == 6) {
		if(!preg_match("/^[a-zA-Z0-9_]*$/", $strUserInput)) {
			echo "Invalid input:  " . $strUserInput . "<br>";
			exit();
		}
	} elseif($intType == 7) {
		if(!preg_match("/^[a-zA-Z0-9_]*$/", $strUserInput)) {
			echo "Invalid input:  " . $strUserInput . "<br>";
			exit();
		}
	} elseif($intType == 8) {
		if(!preg_match("/^[a-zA-Z0-9_]*$/", $strUserInput)) {
			echo "Invalid input:  " . $strUserInput . "<br>";
			exit();
		}
	} elseif($intType == 9) {
		if(!preg_match("/^[a-zA-Z0-9_]*$/", $strUserInput)) {
			echo "Invalid input:  " . $strUserInput
}

function pingAddress($ip) {
    $pingresult = exec("/bin/ping -n 3 $ip", $outcome, $status);
    if (0 == $status) {
        $status = "alive";
    } else {
        $status = "dead";
    }
    echo "The IP address, $ip, is  ".$status;
}
?>
