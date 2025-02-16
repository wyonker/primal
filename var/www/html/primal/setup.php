<?php
	//License GPLv3
	//Version 1.00.01
	//2024-12-26
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

$arrFiles = scandir("/usr/local/scripts/");

$strOption = "";
foreach($arrFiles as $strFile) {
	if($strFile != "." && $strFile != "..") {
		$strOption .= '<option value=\"' . $strFile . '\">' . $strFile . '</option>';
	}
}
$strOption .= '</select>';

echo <<<EOT
<HTML>
<!DOCTYPE !DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN"
            "http://www.w3.org/TR/html4/loose.dtd">
<HEAD>
    <!-- Written by Will Yonker-->
    <TITLE>PRIMAL Web Interface</TITLE>
    <link rel="stylesheet" href="default.css">
</HEAD>
<SCRIPT>
function jsDelete() {
	var modal = document.getElementById('btnDelete2');
	var span = document.getElementsByClassName("close")[0];
	var yes = document.getElementById('btnDeleteRY');
	var no = document.getElementById('btnDeleteRN');
	modal.style.display = "block";
	span.onclick = function() {
		modal.style.display = "none";
	}
	yes.onclick = function() {
		modal.style.display = "none";
		document.getElementById('btnDeleteRY').click();
	}
	no.onclick = function() {
		modal.style.display = "none";
		document.getElementById('btnDeleteRN').click();
	}
}
function jsFunction(value) {
    value2 = document.getElementById("proc_result").value;
    if (value == "tag") {
        x="Tag ID:";
		y="<input type=\"text\" name=\"proc_tag\">";
	} else if (value == "date") {
		x="Date:";
		y="<input type=\"date\" name=\"proc_tag\">";
    } else if (value == "time") {
        x="Time:";
		y="<input type=\"time\" name=\"proc_tag\">";
    } else if (value == "datetime") {
	    x="Date/Time:";
		y="<input type=\"date\" name=\"proc_tag\">";
	} else if (value == "script") {
	    x="Script Name:";
		y="<select name=\"proc_tag\" id=\"proc_tag\">";
		y=y + "$strOption";
		value2 = value2 + '\"';
		y=y.replace(value2, value2 + " selected");
	} else if (value == "hl7") {
	    x="HL7 Segment:";
		y="<input type=\"text\" name=\"proc_tag\">";
	}
    document.getElementById("heading").innerHTML = x;
    document.getElementById("proc_tag1").innerHTML = y;
}
</SCRIPT>
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
			echo $strErrMsg;
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
		$strQuery .= "rec_comp_level = '" . $_POST['rec_comp_level'] . "', ";
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
		$intReturn = fValidateInput(1, $_POST['conf_name']);
		if($intReturn == 1) {
			echo $strErrMsg;
			exit();
		}
		$intReturn = fValidateInput(4, $_POST['rec_port']);
		if($intReturn == 1) {
			echo "Error:  Invalid input for port number...<br>";
			exit();
		}
		if(substr($_POST['rec_dir'], -1) != "/") {
			$_POST['rec_dir'] .= "/";
		}
		$intReturn = fValidateInput(5, $_POST['rec_dir']);
		if($intReturn == 1) {
			echo "Error:  Invalid input for receive directory...<br>";
			exit();
		}
		$intReturn = fValidateInput(6, $_POST['rec_log_full_path']);
		if($intReturn == 1) {
			echo "Error:  Invalid input for receive log (path and file must exist)...<br>";
			exit();
		}
		$intReturn = fValidateInput(3, $_POST['rec_aet']);
		if($intReturn == 1) {
			echo "Error:  Invalid input for AET <br>";
			exit();
		}
		$intReturn = fValidateInput(5, $_POST['proc_dir']);
		if($intReturn == 1) {
			echo "Error:  Invalid input for process directory...<br>";
			exit();
		}
		$intReturn = fValidateInput(6, $_POST['proc_log_full_path']);
		if($intReturn == 1) {
			echo "Error:  Invalid input for process log (path and file must exist)...<br>";
			exit();
		}
		$intReturn = fValidateInput(5, $_POST['out_dir']);
		if($intReturn == 1) {
			echo "Error:  Invalid input for out directory...<br>";
			exit();
		}
		$intReturn = fValidateInput(6, $_POST['out_log_full_path']);
		if($intReturn == 1) {
			echo "Error:  Invalid input for out log (path and file must exist)...<br>";
			exit();
		}
		$intReturn = fValidateInput(5, $_POST['sent_dir']);
		if($intReturn == 1) {
			echo "Error:  Invalid input for sent directory...<br>";
			exit();
		}
		$intReturn = fValidateInput(5, $_POST['hold_dir']);
		if($intReturn == 1) {
			echo "Error:  Invalid input for hold directory...<br>";
			exit();
		}
		$intReturn = fValidateInput(5, $_POST['error_dir']);
		if($intReturn == 1) {
			echo "Error:  Invalid input for error directory...<br>";
			exit();
		}
		$intReturn = fValidateInput(8, $_POST['ret_period']);
		if($intReturn == 1) {
			echo "Error:  Invalid input for retention period...<br>";
			exit();
		}
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
		$strQuery .= "rec_comp_level = '" . $_POST['rec_comp_level'] . "', ";
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
	} elseif(isset($_POST['btnDeleteRY'])) {
		$strQuery = "DELETE FROM conf_rec WHERE conf_rec_id = " . $_POST['conf_rec_id'] . " limit 1;";
		$result = mysqli_query($conn, $strQuery);
		if($result) {
			$strQuery = "DELETE FROM conf_proc WHERE conf_rec_id = " . $_POST['conf_rec_id'] . ";";
			$result = mysqli_query($conn, $strQuery);
			if($result) {
				$strQuery = "DELETE FROM conf_send WHERE conf_rec_id = " . $_POST['conf_rec_id'] . ";";
				$result = mysqli_query($conn, $strQuery);
				if($result) {
					header("Location: setup.php");
				} else {
					echo "Error deleting record: " . mysqli_error($conn) . "<br>";
					echo "Query = " . $strQuery . "<br>";
					exit();
				}
			} else {
				echo "Error deleting record: " . mysqli_error($conn) . "<br>";
				echo "Query = " . $strQuery . "<br>";
				exit();
			}
		} else {
			echo "Error deleting record: " . mysqli_error($conn) . "<br>";
			echo "Query = " . $strQuery . "<br>";
			exit();
		}
	} elseif(isset($_POST['btnDeleteRY'])) {
		$strQuery = "DELETE FROM conf_rec WHERE conf_rec_id = " . $_POST['conf_rec_id'] . " limit 1;";
		$result = mysqli_query($conn, $strQuery);
		if($result) {
			header("Location: setup.php");
		} else {
			echo "Error deleting record: " . mysqli_error($conn) . "<br>";
			echo "Query = " . $strQuery . "<br>";
			exit();
		}
	} elseif(isset($_POST['btnAddR'])) {
		$intReturn = fValidateInput(1, $_POST['proc_name']);
		if($intReturn == 1) {
			echo "Error:  Invalid input for Rule Name...<br>";
			exit();
		}
		$arrSearch = array("\\", '"');
		$strProcType = str_replace($arrSearch, "", $_POST['proc_type']);
		$strProcTag = str_replace($arrSearch, "", $_POST['proc_tag']);
		$strQuery = "INSERT INTO conf_proc SET ";
		$strQuery .= "proc_name = '" . $_POST['proc_name'] . "', ";
		$strQuery .= "proc_type = '" . $strProcType . "', ";
		$strQuery .= "proc_tag = '" . $strProcTag . "', ";
		$strQuery .= "proc_operator = '" . $_POST['proc_operator'] . "', ";
		$strQuery .= "proc_cond = '" . $_POST['proc_cond'] . "', ";
		$strQuery .= "proc_action = '" . $_POST['proc_action'] . "', ";
		$strQuery .= "proc_order = '" . $_POST['proc_order'] . "', ";
		$strQuery .= "proc_dest = '" . $_POST['proc_dest'] . "', ";
		$strQuery .= "active = '" . $_POST['proc_active'] . "', ";
		$strQuery .= "conf_rec_id = '" . $_POST['conf_rec_id'] . "';";

		$result = mysqli_query($conn, $strQuery);
		if($result) {
			header("Location: setup.php");
		} else {
			echo "Error adding record: " . mysqli_error($conn) . "<br>";
			echo "Query = " . $strQuery . "<br>";
			exit();
		}
	} elseif(isset($_POST['btnUpdateR'])) {
		$intReturn = fValidateInput(1, $_POST['proc_name']);
		if($intReturn == 1) {
			echo "Error:  Invalid input for Rule Name...<br>";
			exit();
		}
		$arrSearch = array("\\", '"');
		$strProcType = str_replace($arrSearch, "", $_POST['proc_type']);
		$strProcTag = str_replace($arrSearch, "", $_POST['proc_tag']);
		$strQuery = "UPDATE conf_proc SET ";
		$strQuery .= "proc_name = '" . $_POST['proc_name'] . "', ";
		$strQuery .= "proc_type = '" . $strProcType . "', ";
		$strQuery .= "proc_tag = '" . $strProcTag . "', ";
		$strQuery .= "proc_operator = '" . $_POST['proc_operator'] . "', ";
		$strQuery .= "proc_cond = '" . $_POST['proc_cond'] . "', ";
		$strQuery .= "proc_action = '" . $_POST['proc_action'] . "', ";
		$strQuery .= "proc_order = '" . $_POST['proc_order'] . "', ";
		$strQuery .= "proc_dest = '" . $_POST['proc_dest'] . "', ";
		$strQuery .= "active = '" . $_POST['proc_active'] . "' ";
		$strQuery .= "WHERE conf_proc_id = '" . $_POST['conf_proc_id'] . "' limit 1;";

		$result = mysqli_query($conn, $strQuery);
		if($result) {
			header("Location: setup.php");
		} else {
			echo "Error updating record: " . mysqli_error($conn) . "<br>";
			echo "Query = " . $strQuery . "<br>";
			exit();
		}
	} elseif(isset($_POST['btnDeleteR'])) {
		$strQuery = "DELETE FROM conf_proc WHERE conf_proc_id = " . $_POST['conf_proc_id'] . " limit 1;";
		$result = mysqli_query($conn, $strQuery);
		if($result) {
			header("Location: setup.php");
		} else {
			echo "Error deleting record: " . mysqli_error($conn) . "<br>";
			echo "Query = " . $strQuery . "<br>";
			exit();
		}
	} elseif(isset($_POST['btnAddD'])) {
		$intReturn = fValidateInput(1, $_POST['send_name']);
		if($intReturn == 1) {
			echo "Error:  Invalid input for destination Name...<br>";
			exit();
		}
		$strQuery = "INSERT INTO conf_send SET ";
		$strQuery .= "conf_rec_id = '" . $_POST['conf_rec_id'] . "', ";
		$strQuery .= "send_name = '" . $_POST['send_name'] . "', ";
		$strQuery .= "send_aet = '" . $_POST['send_aet'] . "', ";
		$strQuery .= "send_aec = '" . $_POST['send_aec'] . "', ";
		$strQuery .= "send_hip = '" . $_POST['send_hip'] . "', ";
		$strQuery .= "send_type = '" . $_POST['send_type'] . "', ";
		$strQuery .= "send_port = '" . $_POST['send_port'] . "', ";
		$strQuery .= "send_time_out = '" . $_POST['send_time_out'] . "', ";
		$strQuery .= "send_comp_level = '" . $_POST['send_comp_level'] . "', ";
		$strQuery .= "send_retry = '" . $_POST['send_retry'] . "', ";
		$strQuery .= "send_username = '" . $_POST['send_username'] . "', ";
		$strQuery .= "send_password = '" . $_POST['send_password'] . "', ";
		$strQuery .= "active = '" . $_POST['active'] . "';";
		$result = mysqli_query($conn, $strQuery);
		if($result) {
			header("Location: setup.php");
		} else {
			echo "Error adding record: " . mysqli_error($conn) . "<br>";
			echo "Query = " . $strQuery . "<br>";
			exit();
		}
	} elseif(isset($_POST['btnUpdateD'])) {
		$intReturn = fValidateInput(1, $_POST['send_name']);
		if($intReturn == 1) {
			echo "Error:  Invalid input for destination Name...<br>";
			exit();
		}
		$strQuery = "UPDATE conf_send SET ";
		$strQuery .= "send_name = '" . $_POST['send_name'] . "', ";
		$strQuery .= "send_aet = '" . $_POST['send_aet'] . "', ";
		$strQuery .= "send_org = '" . $_POST['send_org'] . "', ";
		$strQuery .= "send_aec = '" . $_POST['send_aec'] . "', ";
		$strQuery .= "send_hip = '" . $_POST['send_hip'] . "', ";
		$strQuery .= "send_type = '" . $_POST['send_type'] . "', ";
		$strQuery .= "send_port = '" . $_POST['send_port'] . "', ";
		$strQuery .= "send_time_out = '" . $_POST['send_time_out'] . "', ";
		$strQuery .= "send_comp_level = '" . $_POST['send_comp_level'] . "', ";
		$strQuery .= "send_retry = '" . $_POST['send_retry'] . "', ";
		$strQuery .= "send_username = '" . $_POST['send_username'] . "', ";
		$strQuery .= "send_password = '" . $_POST['send_password'] . "', ";
		$strQuery .= "active = '" . $_POST['active'] . "' ";
		$strQuery .= "WHERE conf_send_id = '" . $_POST['conf_send_id'] . "' limit 1;";

		$result = mysqli_query($conn, $strQuery);
		if($result) {
			header("Location: setup.php");
		} else {
			echo "Error updating record: " . mysqli_error($conn) . "<br>";
			echo "Query = " . $strQuery . "<br>";
			exit();
		}
	} elseif(isset($_POST['btnDeleteD'])) {
		$strQuery = "DELETE FROM conf_send WHERE conf_send_id = " . $_POST['conf_send_id'] . " limit 1;";
		$result = mysqli_query($conn, $strQuery);
		if($result) {
			header("Location: setup.php");
		} else {
			echo "Error deleting record: " . mysqli_error($conn) . "<br>";
			echo "Query = " . $strQuery . "<br>";
			exit();
		}
	} elseif(isset($_POST['btnReset'])) {
		header("Location: setup.php");
		exit();
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




if(!isset($_GET['rec'])) {
	$arrRec = [];
	echo '<table style="margin-right:auto;margin-left:0px;border="1">';
	$strQuery = "SELECT * FROM conf_rec WHERE conf_name = \"!Global!\";";
	$result = mysqli_query($conn, $strQuery);
	$row = mysqli_fetch_assoc($result);
	array_push($arrRec, $row['conf_rec_id']);
	echo '<tr><td><div id="rec"><br><br><a href="/primal/setup.php?action=Rec&rec=' . $row['conf_rec_id'] . '">' . $row['conf_name'] . '</a></div></td>';
	$strQuery = "SELECT * FROM conf_rec WHERE conf_name != \"!Global!\";";
	$result = mysqli_query($conn, $strQuery);
	$intNumRows = mysqli_num_rows($result);
	while($row = mysqli_fetch_assoc($result)) {
		$intLC=1;
		array_push($arrRec, $row['conf_rec_id']);
		echo '<td><div id="rec"><br><br><a href="/primal/setup.php?action=Rec&rec=' . $row['conf_rec_id'] . '">' . $row['conf_name'] . '</a></div></td>';
	}
	array_push($arrRec, "0");
	echo '<td><div id="rec"><br><br><a href="/primal/setup.php?action=Rec&rec=0">Add Receiver</a></div></td></tr>';
	//Now let's show the routes.
	echo "<tr>";
	foreach($arrRec as &$rec) {
		$strQuery = "SELECT conf_proc_id, proc_name FROM conf_proc WHERE conf_rec_id = " . $rec . ";";
		$result = mysqli_query($conn, $strQuery);
		echo '<td><div id="rec"><br><br>';
		while($row = mysqli_fetch_assoc($result)) {
			echo '<a href="/primal/setup.php?action=Rule&rec=' . $rec . '&rule=' . $row['conf_proc_id'] . '">' . $row['proc_name'] . '</a><br>';
		}
		echo '<div id="rec"><a href="/primal/setup.php?action=Rule&rec=' . $rec . '&rule=0">Add Rule</a></div></td>';
	}
	echo "</tr>";
	//Now let's show the destinations.
	echo "<tr>";
	foreach($arrRec as &$rec) {
		$strQuery = "SELECT conf_send_id, send_name FROM conf_send WHERE conf_rec_id = " . $rec . ";";
		$result = mysqli_query($conn, $strQuery);
		echo '<td><div id="rec"><br><br>';
		while($row = mysqli_fetch_assoc($result)) {
			echo '<a href="/primal/setup.php?action=Dest&rec=' . $rec . '&rule=' . $row['conf_send_id'] . '">' . $row['send_name'] . '</a><br>';
		}
		echo '<div id="rec"><a href="/primal/setup.php?action=Dest&rec=' . $rec . '&rule=0">Add Destination</a></div></td>';
	}
	echo "</tr>";
	echo "</table>";
}
if($_GET['action'] == 'Rec') {
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
		echo '<tr><td>' . '<label for="rec_comp_level">Compression Level (lower is faster):</label></td><td>';
		echo '<select name="rec_comp_level" id="rec_comp_level">';
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
		echo '<br><div class="btn-group">';
		echo '<button type="submit" id="btnUpdate" name="btnUpdate">Update</button>';
		echo '<button type="submit" id="btnReset" name="btnReset">Clear</button>';
		echo '<button type="submit" id="btnCancel" name="btnCancel">Cancel</button>';
		echo '<input type="button" onClick="jsDelete()" id="btnDelete" name="btnCancel" value=" Delete" size="3">';
		echo '<div id="btnDelete2" class="modal">';
  		echo '<!-- Modal content -->';
  		echo '<div class="modal-content">';
    	echo '<span class="close">&times;</span>';
    	echo '<p>Would you like to delete all rules and destiantions too?</p>';
    	echo '<button type="submit" id="btnDeleteRY" name="btnDeleteRY">Yes</button>';
		echo '<button type="submit" id="btnDeleteRN" name="btnDeleteRN">No</button>';
  		echo '</div>';
		echo '</div>';
		echo '</div>';
		echo '</form>';
	} else {
		echo '<form action="setup.php?action=save" method="post">';
		echo '<table border="1">';
		echo '<tr><td>' . 'Config Name:</td>';
		echo '<td><input type="text" name="conf_name" value="Default" /></td></tr>';
		echo '<tr><td>' . 'Server FQDN or IP' . '</td>';
		echo '<td><input type="text" name="conf_server" value="localhost" /></td></tr>';
		echo '<tr><td>' . '<label for="rec_type">Receiver Type:</label></td><td>';
		echo '<select name="rec_type" id="rec_type">';
		echo '<option value="1" selected>Dicom</option>';
		echo '<option value="2">Directory</option>';
		echo '</select></td></tr>';
		echo '<tr><td>' . 'Port number to listen on (not used for directory type)' . '</td>';
		echo '<td><input type="text" name="rec_port" value="2000"></td></tr>';
		echo '<tr><td>' . 'Absolute directory path for receiver data' . '</td>';
		echo '<td><input type="text" name="rec_dir" value="/home/dicom/inbound/"></td></tr>';
		echo '<tr><td>' . 'Absolute directory path + filename for receiver logs' . '</td>';
		echo '<td><input type="text" name="rec_log_full_path" value="/var/log/primal/in.log"></td></tr>';
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
		echo '<td><input type="text" name="rec_aet" value="PRIMAL" /></td></tr>';
		echo '<tr><td>' . 'Time Out (seconds)' . '</td>';
		echo '<td><input type="text" name="rec_time_out" value="30" /></td></tr>';
		echo '<tr><td>' . 'Absolute directory path for moving data to for processing' . '</td>';
		echo '<td><input type="text" name="proc_dir" value="/home/dicom/processing/" /></td></tr>';
		echo '<tr><td>' . 'Absolute directory path + filename for processing logs' . '</td>';
		echo '<td><input type="text" name="proc_log_full_path" value="/var/log/primal/process.log" /></td></tr>';
		echo '<tr><td>' . 'Absolute directory path for moving outging data to' . '</td>';
		echo '<td><input type="text" name="out_dir" value="/home/dicom/outbound/" /></td></tr>';
		echo '<tr><td>' . 'Absolute directory path + filename for outgoing logs' . '</td>';
		echo '<td><input type="text" name="out_log_full_path" value="/var/log/primal/out.log" /></td></tr>';
		echo '<tr><td>' . 'Absolute directory path for sent files' . '</td>';
		echo '<td><input type="text" name="sent_dir" value="/home/dicom/sent/" /></td></tr>';
		echo '<tr><td>' . 'Absolute directory path for held files' . '</td>';
		echo '<td><input type="text" name="hold_dir" value="/home/dicom/hold/" /></td></tr>';
		echo '<tr><td>' . 'Absolute directory path for errored files' . '</td>';
		echo '<td><input type="text" name="error_dir" value="/home/dicom/error/" /></td></tr>';
		echo '<tr><td>' . '<label for="dupe">Deduplication (WARNING!  This is a CPU hog):</label></td><td>';
		echo '<select name="dupe" id="dupe">';
		echo '<option value="1">Yes</option>';
		echo '<option value="2" selected><b>No</b></option>';
		echo '</select></td></tr>';
		echo '<tr><td>' . '<label for="rec_comp_level">Compression Level (lower is faster):</label></td><td>';
		echo '<select name="rec_comp_level" id="rec_comp_level">';
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
		echo '<td><input type="text" name="ret_period" value="10800" /></td></tr>';
		echo '<tr><td>' . '<label for="active">Active?:</label></td><td>';
		echo '<select name="active" id="active" default=1>';
		echo '<option value="1">Yes</option>';
		echo '<option value="2" selected>No</option>';
		echo '</select></td></tr>';
		echo '</table>';
		echo '<br><div class="btn-group">';
		echo '<button type="submit" id="btnAdd" name="btnAdd">Add</button>';
		echo '<button type="submit" id="btnReset" name="btnReset">Clear</button>';
		echo '<button type="submit" id="btnCancel" name="btnCancel">Cancel</button>';
		echo '</div>';
		echo '</form>';
	}
} elseif($_GET['action'] == 'Rule') {
	if($_GET['rule'] > '0') {
		$strQuery = "SELECT * FROM conf_proc WHERE conf_proc_id = " . $_GET['rule'] . " limit 1;";
		$result = mysqli_query($conn, $strQuery);
		$row = mysqli_fetch_assoc($result);
		echo '<form action="setup.php?action=save" method="post">';
		echo '<input type="hidden" name="conf_rec_id" value ="' . $row["conf_rec_id"] . '" />';
		echo '<input type="hidden" name="conf_proc_id" value ="' . $_GET['rule'] . '" />';
		echo '<input type="hidden" name="proc_result" id="proc_result" value ="' . $row["proc_tag"] . '" />';
		echo '<table border="1">';
		echo '<tr><td>' . 'Rule Name:</td>';
		echo '<td><input type="text" name="proc_name" value ="' . $row["proc_name"] . '" /></td></tr>';
		echo '<tr><td>' . '<label for="proc_type">Rule type:</label></td><td>';
		echo '<select name="proc_type" id="proc_type" onchange="jsFunction(this.value);">';
		echo '<option value="tag"';
		if ($row["proc_type"] == "tag") {
			echo ' selected';
		}
		echo '>tag</option>';
		echo '<option value="date"';
		if ($row["proc_type"] == "date") {
			echo ' selected';
		}
		echo '>date</option>';
		echo '<option value="time"';
		if ($row["proc_type"] == "time") {
			echo ' selected';
		}
		echo '>time</option>';
		echo '<option value="datetime"';
		if ($row["proc_type"] == "datetime") {
			echo ' selected';
		}
		echo '>date/time</option>';
		echo '<option value="script"';
		if ($row["proc_type"] == "script") {
			echo ' selected';
		}
		echo '>Script</option>';
		echo '<option value="hl7"';
		if ($row["proc_type"] == "hl7") {
			echo ' selected';
		}
		echo '>HL7</option>';
		echo '</select></td></tr>';
		if ($row["proc_type"] == "tag") {
			echo '<tr><td>' . '<span id="heading">Tag ID:</span></td>';
			echo '<td><span id="proc_tag1"><input type="text" name="proc_tag" value="' . $row["proc_tag"] . '"></span></td></tr>';
		} elseif ($row["proc_type"] == "date") {
			echo '<tr><td>' . '<span id="heading">Date:</span></td>';
			echo '<td><span id="proc_tag1"><input type="date" name="proc_tag" value="' . $row["proc_tag"] . '"></span></td></tr>';
		} elseif ($row["proc_type"] == "time") {
			echo '<tr><td>' . '<span id="heading">Time:</span></td>';
			echo '<td><span id="proc_tag1"><input type="time" name="proc_tag" value="' . $row["proc_tag"] . '"></span></td></tr>';
		} elseif ($row["proc_type"] == "datetime") {
			echo '<tr><td>' . '<span id="heading">Date:</span></td>';
			echo '<td><span id="proc_tag1"><input type="date" name="proc_tag" value="' . $row["proc_tag"] . '"></span></td></tr>';
			//echo '<tr><td>' . '<span id="heading">Time:</span></td>';
			//echo '<td><span id="proc_tag1"><input type="text" name="proc_tag"></span></td></tr>';
		} elseif ($row["proc_type"] == "script") {
			echo '<tr><td>' . '<span id="heading">Script:</span></td>';
			echo '<td><span id="proc_tag1"><select name="proc_tag">';
			$strOption2 = str_replace("\\", "", $strOption);
			$strOption2 = str_replace($row['proc_tag'] . "\"", $row['proc_tag'] . "\"" . " selected", $strOption2);
			echo $strOption2;
			echo '</span></td></tr>';
		} elseif ($row["proc_type"] == "hl7") {
			echo '<tr><td>' . '<span id="heading">HL7:</span></td>';
			echo '<td><span id="proc_tag1"><input type="text" name="proc_tag" value="' . $row["proc_tag"] . '"></span></td></tr>';
		}
		echo '<tr><td>' . '<label for="proc_operator">Operator Type:</label></td><td>';
		echo '<select name="proc_operator" id="proc_operator">';
		echo '<option value=">"';
		if($row["proc_operator"] == ">") {
			echo ' selected';
		}
		echo '>></option>';
		echo '<option value="<"';
		if($row["proc_operator"] == "<") {
			echo ' selected';
		}
		echo '><</option>';
		echo '<option value="equals"';
		if($row["proc_operator"] == "equals") {
			echo ' selected';
		}
		echo '>equals</option>';
		echo '<option value="!equals"';
		if($row["proc_operator"] == "!equals") {
			echo ' selected';
		}
		echo '>!equals</option>';
		echo '<option value="contains"';
		if($row["proc_operator"] == "contains") {
			echo ' selected';
		}
		echo '>contains</option>';
		echo '<option value="!contains"';
		if($row["proc_operator"] == "!contains") {
			echo ' selected';
		}
		echo '>!Contains</option>';
		echo '<option value="regex"';
		if($row["proc_operator"] == "regex") {
			echo ' selected';
		}
		echo '>regex</option>';
		echo '<option value="script"';
		if($row["proc_operator"] == "script") {
			echo ' selected';
		}
		echo '>script</option>';
		echo '<option value="begins"';
		if($row["proc_operator"] == "begins") {
			echo ' selected';
		}
		echo '>Begins</option>';
		echo '<option value="!begins"';
		if($row["proc_operator"] == "!begins") {
			echo ' selected';
		}
		echo '>!Begins</option>';
		echo '</select></td></tr>';
		echo '<tr><td>' . 'Rule Condition:</td>';
		echo '<td><input type="text" name="proc_cond" value ="' . $row["proc_cond"] . '" /></td></tr>';
		echo '<tr><td>' . '<label for="proc_action">Action Type:</label></td><td>';
		echo '<select name="proc_action" id="proc_action">';
		echo '<option value="1"';
		if($row["proc_action"] == 1) {
			echo ' selected';
		}
		echo '>Pass</option>';
		echo '<option value="2"';
		if($row["proc_action"] == 2) {
			echo ' selected';
		}
		echo '>Drop</option>';
		echo '<option value="3"';
		if($row["proc_action"] == 3) {
			echo ' selected';
		}
		echo '>Skip</option>';
		echo '<option value="4"';
		if($row["proc_action"] == 4) {
			echo ' selected';
		}
		echo '>Error</option>';
		echo '<option value="5"';
		if($row["proc_action"] == 5) {
			echo ' selected';
		}
		echo '>Send</option>';
		echo '<option value="6"';
		if($row["proc_action"] == 6) {
			echo ' selected';
		}
		echo '>Jump</option>';
		echo '</select></td></tr>';
		echo '<tr><td>' . 'Order:</td>';
		echo '<td><input type="text" name="proc_order" value ="' . $row["proc_order"] . '" /></td></tr>';
		echo '<tr><td>' . 'Destination (0 for all):</td>';
		echo '<td><input type="text" name="proc_dest" value ="' . $row["proc_dest"] . '" /></td></tr>';
		echo '<tr><td>' . '<label for="active">Active?:</label></td><td>';
		echo '<select name="proc_active" id="proc_active" default=1>';
		echo '<option value="1"';
		if($row["active"] == 0) {
			echo ' selected';
		}
		echo '>Yes</option>';
		echo '<option value="2"';
		if($row["active"] == 1) {
			echo ' selected';
		}
		echo '>No</option>';
		echo '</select></td></tr>';
		echo '</table>';
		echo '<br><div class="btn-group">';
		echo '<button type="submit" id="btnUpdateR" name="btnUpdateR">Update</button>';
		echo '<button type="submit" id="btnReset" name="btnReset">Clear</button>';
		echo '<button type="submit" id="btnCancel" name="btnCancel">Cancel</button>';
		echo '<button type="submit" id="btnDeleteR" name="btnDeleteR">Delete</button>';
		echo '</div>';
		echo '</form>';
} else {
		echo '<form action="setup.php?action=save" method="post">';
		echo '<input type="hidden" name="conf_rec_id" value ="' . $_GET["rec"] . '" />';
		echo '<input type="hidden" name="conf_proc_id" value ="0" />';
		echo '<input type="hidden" name="proc_result" id="proc_result" value ="tag" />';
		echo '<table border="1">';
		echo '<tr><td>' . 'Rule Name:</td>';
		echo '<td><input type="text" name="proc_name" value="default" /></td></tr>';
		echo '<tr><td>' . '<label for="proc_type">Rule type:</label></td><td>';
		echo '<select name="proc_type" id="proc_type" onchange="jsFunction(this.value);">';
		echo '<option value="tag" selected>tag</option>';
		echo '<option value="date">date</option>';
		echo '<option value="time">time</option>';
		echo '<option value="datetime">date/time</option>';
		echo '<option value="script">Script</option>';
		echo '<option value="hl7">HL7</option>';
		echo '</select></td></tr>';
		echo '<tr><td>' . '<span id="heading">Tag ID:</span></td>';
		echo '<td><span id="proc_tag1"><input type="text" name="proc_tag"></span></td></tr>';
		echo '<tr><td>' . '<label for="proc_operator">Operator Type:</label></td><td>';
		echo '<select name="proc_operator" id="proc_operator">';
		echo '<option value="1">></option>';
		echo '<option value="2"><</option>';
		echo '<option value="3">=</option>';
		echo '<option value="4">!=</option>';
		echo '<option value="5">contains</option>';
		echo '<option value="6">!Contains</option>';
		echo '<option value="7">regex</option>';
		echo '<option value="8">script</option>';
		echo '<option value="9">Begins</option>';
		echo '<option value="10">!Begins</option>';
		echo '</select></td></tr>';
		echo '<tr><td>' . 'Rule Condition:</td>';
		echo '<td><input type="text" name="proc_cond" /></td></tr>';
		echo '<tr><td>' . '<label for="proc_action">Action Type:</label></td><td>';
		echo '<select name="proc_action" id="proc_action">';
		echo '<option value="1">Pass</option>';
		echo '<option value="2">Drop</option>';
		echo '<option value="3">Skip</option>';
		echo '<option value="4">Error</option>';
		echo '<option value="5">Send</option>';
		echo '<option value="6">Jump</option>';
		echo '</select></td></tr>';
		echo '<tr><td>' . 'Order:</td>';
		echo '<td><input type="text" name="proc_order" value="1" /></td></tr>';
		echo '<tr><td>' . 'Destination (0 for all):</td>';
		echo '<td><input type="text" name="proc_dest" value="0" /></td></tr>';
		echo '<tr><td>' . '<label for="active">Active?:</label></td><td>';
		echo '<select name="proc_active" id="proc_active" default=1>';
		echo '<option value="0">Yes</option>';
		echo '<option value="1" selected>No</option>';
		echo '</select></td></tr>';
		echo '</table>';
		echo '<br><div class="btn-group">';
		echo '<button type="submit" id="btnAddR" name="btnAddR">Add</button>';
		echo '<button type="submit" id="btnReset" name="btnReset">Clear</button>';
		echo '<button type="submit" id="btnCancel" name="btnCancel">Cancel</button>';
		echo '</div>';
		echo '</form>';
	}
} elseif($_GET['action'] == 'Dest') {
	if($_GET['dest'] > '0') {
		$strQuery = "SELECT * FROM conf_send WHERE conf_send_id = " . $_GET['dest'] . " limit 1;";
		$result = mysqli_query($conn, $strQuery);
		$row = mysqli_fetch_assoc($result);
		echo '<form action="setup.php?action=save" method="post">';
		echo '<input type="hidden" name="conf_rec_id" value ="' . $row["conf_rec_id"] . '" />';
		echo '<input type="hidden" name="conf_send_id" value ="' . $_GET['dest'] . '" />';
		echo '<table border="1">';
		echo '<tr><td>' . 'Destination Name:</td>';
		echo '<td><input type="text" name="send_name" value ="' . $row["send_name"] . '" /></td></tr>';
		echo '<tr><td>' . 'Destination Org Code:</td>';
		echo '<td><input type="text" name="send_org" value ="' . $row["send_org"] . '" /></td></tr>';
		echo '<tr><td>' . 'Destination AET:</td>';
		echo '<td><input type="text" name="send_aet" value ="' . $row["send_aet"] . '" /></td></tr>';
		echo '<tr><td>' . 'Destination AEC:</td>';
		echo '<td><input type="text" name="send_aec" value ="' . $row["send_aec"] . '" /></td></tr>';
		echo '<tr><td>' . 'Destination Hostname or IP:</td>';
		echo '<td><input type="text" name="send_hip" value ="' . $row["send_hip"] . '" /></td></tr>';
		echo '<tr><td>' . '<label for="send_type">Send Type:</label></td><td>';
		echo '<select name="send_type" id="send_type">';
		echo '<option value="1"';
		if($row["send_type"] == 1) {
			echo ' selected';
		}
		echo '>DICOM</option>';
		echo '<option value="2"';
		if($row["send_type"] == 2) {
			echo ' selected';
		}
		echo '>SCP</option>';
		echo '<option value="3"';
		if($row["send_type"] == 3) {
			echo ' selected';
		}
		echo 'FTP</option>';
		echo '</select></td></tr>';
		echo '<tr><td>' . 'Destination Port Number:</td>';
		echo '<td><input type="text" name="send_port" value ="' . $row["send_port"] . '" /></td></tr>';
		echo '<tr><td>' . 'Time Out (in seconds):</td>';
		echo '<td><input type="text" name="send_time_out" value ="' . $row["send_time_out"] . '" /></td></tr>';
		echo '<tr><td>' . '<label for="send_comp_level">Compression Level (lower is faster):</label></td><td>';
		echo '<select name="send_comp_level" id="send_comp_level">';
		echo '<option value="1"';
		if($row["send_comp_level"] == 1) {
			echo ' selected';
		}
		echo '>1</option>';
		echo '<option value="2"';
		if($row["send_comp_level"] == 2) {
			echo ' selected';
		}
		echo '>2</option>';
		echo '<option value="3"';
		if($row["send_comp_level"] == 3) {
			echo ' selected';
		}
		echo '>3</option>';
		echo '<option value="4"';
		if($row["send_comp_level"] == 4) {
			echo ' selected';
		}
		echo '>4</option>';
		echo '<option value="5"';
		if($row["send_comp_level"] == 5) {
			echo ' selected';
		}
		echo '>5</option>';
		echo '<option value="6"';
		if($row["send_comp_level"] == 6) {
			echo ' selected';
		}
		echo '>6</option>';
		echo '<option value="7"';
		if($row["send_comp_level"] == 7) {
			echo ' selected';
		}
		echo '>7</option>';
		echo '<option value="8"';
		if($row["send_comp_level"] == 8) {
			echo ' selected';
		}
		echo '>8</option>';
		echo '<option value="9"';
		if($row["send_comp_level"] == 9) {
			echo ' selected';
		}
		echo '>9</option>';
		echo '</select></td></tr>';
		echo '<tr><td>' . 'Number of Retries:</td>';
		echo '<td><input type="text" name="send_retry" value ="' . $row["send_retry"] . '" /></td></tr>';
		echo '<tr><td>' . 'Username:</td>';
		echo '<td><input type="text" name="send_username" value ="' . $row["send_username"] . '" /></td></tr>';
		echo '<tr><td>' . 'Password:</td>';
		echo '<td><input type="text" name="send_password" value ="' . $row["send_password"] . '" /></td></tr>';
		echo '<tr><td>' . '<label for="active">Active?:</label></td><td>';
		echo '<select name="active" id="active" default=1>';
		echo '<option value="1"';
		if($row["active"] == 0) {
			echo ' selected';
		}
		echo '>Yes</option>';
		echo '<option value="2"';
		if($row["active"] == 1) {
			echo ' selected';
		}
		echo '>No</option>';
		echo '</select></td></tr>';
		echo '</table>';
		echo '<br><div class="btn-group">';
		echo '<button type="submit" id="btnUpdateD" name="btnUpdateD">Update</button>';
		echo '<button type="submit" id="btnReset" name="btnReset">Clear</button>';
		echo '<button type="submit" id="btnCancel" name="btnCancel">Cancel</button>';
		echo '<button type="submit" id="btnDeleteD" name="btnDeleteD">Delete</button>';
		echo '</div>';
		echo '</form>';
	} else {
		echo '<form action="setup.php?action=save" method="post">';
		echo '<input type="hidden" name="conf_rec_id" value ="' . $_GET["rec"] . '" />';
		echo '<input type="hidden" name="conf_send_id" />';
		echo '<table border="1">';
		echo '<tr><td>' . 'Destination Name:</td>';
		echo '<td><input type="text" name="send_name" value="Default" /></td></tr>';
		echo '<tr><td>' . 'Destination Org Code:</td>';
		echo '<td><input type="text" name="send_org" value="UNK" /></td></tr>';
		echo '<tr><td>' . 'Destination AET:</td>';
		echo '<td><input type="text" name="send_aet" /></td></tr>';
		echo '<tr><td>' . 'Destination AEC:</td>';
		echo '<td><input type="text" name="send_aec" value="PRIMAL" /></td></tr>';
		echo '<tr><td>' . 'Destination Hostname or IP:</td>';
		echo '<td><input type="text" name="send_hip" /></td></tr>';
		echo '<tr><td>' . '<label for="send_type">Send Type:</label></td><td>';
		echo '<select name="send_type" id="send_type">';
		echo '<option value="1" selected>DICOM</option>';
		echo '<option value="2">SCP</option>';
		echo '<option value="3">FTP</option>';
		echo '</select></td></tr>';
		echo '<tr><td>' . 'Destination Port Number:</td>';
		echo '<td><input type="text" name="send_port" value="104" /></td></tr>';
		echo '<tr><td>' . 'Time Out (in seconds):</td>';
		echo '<td><input type="text" name="send_time_out" value="30" /></td></tr>';
		echo '<tr><td>' . '<label for="send_comp_level">Compression Level (lower is faster):</label></td><td>';
		echo '<select name="send_comp_level" id="send_comp_level">';
		echo '<option value="1">1</option>';
		echo '<option value="2">2</option>';
		echo '<option value="3">3</option>';
		echo '<option value="4">4</option>';
		echo '<option value="5">5</option>';
		echo '<option value="6" selected>6</option>';
		echo '<option value="7">7</option>';
		echo '<option value="8">8</option>';
		echo '<option value="9">9</option>';
		echo '</select></td></tr>';
		echo '<tr><td>' . 'Number of Retries:</td>';
		echo '<td><input type="text" name="send_retry" value="3" /></td></tr>';
		echo '<tr><td>' . 'Username:</td>';
		echo '<td><input type="text" name="send_username" /></td></tr>';
		echo '<tr><td>' . 'Password:</td>';
		echo '<td><input type="text" name="send_password" /></td></tr>';
		echo '<tr><td>' . '<label for="active">Active?:</label></td><td>';
		echo '<tr><td>' . 'Order:</td>';
		echo '<td><input type="text" name="send_order" value="1" /></td></tr>';
		echo '<select name="active" id="active" default=1>';
		echo '<option value="1">Yes</option>';
		echo '<option value="2" selected>No</option>';
		echo '</select></td></tr>';
		echo '</table>';
		echo '<br><div class="btn-group">';
		echo '<button type="submit" id="btnAddD" name="btnAddD">Add</button>';
		echo '<button type="submit" id="btnReset" name="btnReset">Clear</button>';
		echo '<button type="submit" id="btnCancel" name="btnCancel">Cancel</button>';
		echo '</div>';
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
		echo '<tr><td>' . '<label for="rec_comp_level">Compression Level (lower is faster):</label></td><td>';
		echo '<select name="rec_comp_level" id="rec_comp_level">';
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
		echo '<br><div class="btn-group">';
		echo '<button type="submit" id="btnAdd" name="btnAdd">Add</button>';
		echo '<button type="submit" id="btnReset" name="btnReset">Clear</button>';
		echo '<button type="submit" id="btnCancel" name="btnCancel">Cancel</button>';
		echo '</div>';
		echo '</form>';
	}
/*
} else {
	$strQuery = "SELECT conf_rec_id, conf_name FROM conf_rec ORDER BY conf_name;";
	$result = mysqli_query($conn, $strQuery);
	$intNumRows = mysqli_num_rows($result);
	$intLC=1;
	while($intLC <= ($intNumRows + 1)) {
		$row = mysqli_fetch_assoc($result);
		echo '<div id="rec" style="top: ' . $intLC * 150 . ';">';
		if($intLC > $intNumRows) {
			echo '<BR><BR><a href="/primal/setup.php?action=Rec">New</a><BR><BR>';
		} else {
			echo '<BR><BR><a href="/primal/setup.php?action=Rec&rec=' . $row['conf_rec_id'] . '">' . $row['conf_name'] . '</a><BR><BR>';
		}
		echo '</div>';

		echo '<div id="proc" style="top: ' . $intLC * 150 . ';">';
		if(!isset($row['conf_rec_id'])) {
			echo '<BR><BR>N/A<BR><BR>';
		} else {
			$strQuery2 = "SELECT conf_proc_id, proc_name FROM conf_proc WHERE conf_rec_id = " . $row['conf_rec_id'] . ";";
			$result2 = mysqli_query($conn, $strQuery2);
			$intNumRows2 = mysqli_num_rows($result2);
			if($intNumRows2 > 0) {
				$intLC2=0;
				while($intLC2 < $intNumRows2) {
					$row2 = mysqli_fetch_assoc($result2);
					echo '<a href="/primal/setup.php?action=Rule&rec=' . $row2['conf_proc_id'] . '&rule=' . $row2['conf_proc_id'] . '">' . $row2['proc_name'] . '</a><BR>';
					$intLC2++;
				}
				echo '<a href="/primal/setup.php?action=Rule&rec=' . $row['conf_rec_id'] . '&rule=0">Add Rule</a><BR>';
			} else {
				echo '<a href="/primal/setup.php?action=Rule&rec=' . $row['conf_rec_id'] . '&rule=0">Add Rule</a><BR>';
			}
		}
		echo '</div>';

		echo '<div id="out" style="top: ' . $intLC * 150 . ';">';
		if(!isset($row['conf_rec_id'])) {
			echo '<BR><BR>N/A<BR><BR>';
		} else {
			$strQuery3 = "SELECT conf_send_id, send_name FROM conf_send WHERE conf_rec_id = " . $row['conf_rec_id'] . ";";
			$result3 = mysqli_query($conn, $strQuery3);
			$intNumRows3 = mysqli_num_rows($result3);
			if($intNumRows3 > 0) {
				$intLC3=0;
				while($intLC3 < $intNumRows3) {
					$row3 = mysqli_fetch_assoc($result3);
					echo '<a href="/primal/setup.php?action=Dest&dest=' . $row3['conf_send_id'] . '">' . $row3['send_name'] . '</a><BR>';
					$intLC3++;
				}
				echo '<a href="/primal/setup.php?action=Dest&rec=' . $row['conf_rec_id'] . '&dest=0">Add Destination</a><BR>';
			} else {
				echo '<a href="/primal/setup.php?action=Dest&rec=' . $row['conf_rec_id'] . '&dest=0">Add Destination</a><BR>';
			}
		}
		echo '</div>';
		echo '<BR>';
		$intLC++;
	}
	echo '<BR><BR>';
*/
}
Display_Footer();
echo '</BODY>';
echo '</HTML>';

function fValidateInput($intType, $strUserInput) {
	if($intType == 1) {
		if(!preg_match("/^[a-zA-Z0-9_ \[\]\(\)\-\_\*\^]*$/", $strUserInput)) { // Alpha numeric
			$strErrMsg = "Invalid input:  " . $strUserInput . "<br>";
			return 1;
		} elseif($strUserInput == "!Global!") {
			$strErrMsg = "Invalid input:  " . $strUserInput . ".  !Global! is a reserved name.<br>";
			return 1;
		} else {
			return 0;
		}
	} elseif($intType == 2) {
		if(preg_match("/^((25[0-5]|(2[0-4]|1\d|[1-9]|)\d)\.?\b){4}$/", $strUserInput)) { // IP address
			return 0;
		}
		return 0;
	} elseif($intType == 3) {
		if(!preg_match("/^[a-zA-Z0-9_\-\*\^]*$/", $strUserInput)) { // AET
			echo "Invalid input:  " . $strUserInput . "<br>";
			exit();
		}
	} elseif($intType == 4) { // Port number
		if(!preg_match("/^[0-9]*$/", $strUserInput)) {
			return 1;
		} else {
			if($strUserInput < 1 || $strUserInput > 65535) {
				return 1;
			}
			return 0;
		}
	} elseif($intType == 5) { // Directory
		if(!preg_match("/^[a-zA-Z0-9_\/]*$/", $strUserInput)) {
			return 1;
		} else {
			if(!is_dir($strUserInput)) {
				mkdir($strUserInput, 0777, true);
				if(!is_dir($strUserInput)) {
					return 1;
				} else {
					return 0;
				}
			} else {
				return 0;
			}
		}
	} elseif($intType == 6) { // File
		if(!file_exists($strUserInput)) {
			return 1;
		} else {
			return 0;
		}
	} elseif($intType == 7) {
		if(!is_numeric($strUserInput)) {
			return 1;
		} else {
			if($strUserInput < 1 || $strUserInput > 10) {
				return 1;
			} else {
				return 0;
			}
		}
	} elseif($intType == 8) {
		if(!is_numeric($strUserInput)) {
			return 1;
		} else {
			if($strUserInput < 1 || $strUserInput > 5256000) {
				return 1;
			} else {
				return 0;
			}
		}
	}
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
