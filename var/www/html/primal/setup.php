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
Display_Header2();

if ($_GET['action'] == 'L') {
	fLoadConfig();
}
echo "<H2>System Setup</H2>";
if($_GET['action'] == 'L') {
	fLoadConfig();
} elseif($_GET['action'] == 'N') {
	echo '<form action="setup.php" method="post">';
	echo '<table border="1">';
	echo '<tr><td>' . 'Config Name:</td>';
	echo '<td><input type="text" name="conf_name" value ="' . $_SESSION["conf_name"] . '" /></td></tr>';
	echo '<tr><td>' . 'Server FQDN or IP' . '</td>';
	echo '<td><input type="text" name="conf_server" value ="' . $_SESSION["conf_server"] . '" /></td></tr>';
	echo '<tr><td>' . '<label for="rec_type">Receiver Type:</label></td><td>';
	echo '<select name="rec_type" id="rec_type">';
	echo '<option value="1" selected>Dicom</option>';
	echo '<option value="2">Directory</option>';
	echo '</select></td></tr>';
	echo '<tr><td>' . 'Port number to listen on (not used for directory type)' . '</td>';
	echo '<td><input type="text" name="rec_port" value ="' . $_SESSION["rec_port"] . '" /></td></tr>';
	echo '<tr><td>' . 'Absolute directory path for receiver data' . '</td>';
	echo '<td><input type="text" name="rec_dir" value ="' . $_SESSION["rec_dir"] . '" /></td></tr>';
	echo '<tr><td>' . 'Absolute directory path for receiver logs' . '</td>';
	echo '<td><input type="text" name="rec_log_full_path" value ="' . $_SESSION["rec_log_full_path"] . '" /></td></tr>';
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
	echo '<td><input type="text" name="rec_aet" value ="' . $_SESSION["rec_aet"] . '" /></td></tr>';
	echo '<tr><td>' . 'Time Out' . '</td>';
	echo '<td><input type="text" name="rec_time_out" value ="' . $_SESSION["rec_time_out"] . '" /></td></tr>';
	echo '<tr><td>' . 'Absolute directory path for moving data to for processing' . '</td>';
	echo '<td><input type="text" name="proc_dir" value ="' . $_SESSION["proc_dir"] . '" /></td></tr>';
	echo '<tr><td>' . 'Absolute directory path for processing logs' . '</td>';
	echo '<td><input type="text" name="proc_log_full_path" value ="' . $_SESSION["proc_log_full_path"] . '" /></td></tr>';
	echo '<tr><td>' . 'Absolute directory path for moving outging data to' . '</td>';
	echo '<td><input type="text" name="out_dir" value ="' . $_SESSION["out_dir"] . '" /></td></tr>';
	echo '<tr><td>' . 'Absolute directory path for outgoing logs' . '</td>';
	echo '<td><input type="text" name="out_log_full_path" value ="' . $_SESSION["out_log_full_path"] . '" /></td></tr>';
	echo '<tr><td>' . 'Absolute directory path for sent files' . '</td>';
	echo '<td><input type="text" name="sent_dir" value ="' . $_SESSION["sent_dir"] . '" /></td></tr>';
	echo '<tr><td>' . 'Absolute directory path for held files' . '</td>';
	echo '<td><input type="text" name="hold_dir" value ="' . $_SESSION["hold_dir"] . '" /></td></tr>';
	echo '<tr><td>' . 'Absolute directory path for errored files' . '</td>';
	echo '<td><input type="text" name="error_dir" value ="' . $_SESSION["error_dir"] . '" /></td></tr>';
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
	echo '<td><input type="text" name="ret_period" value ="' . $_SESSION["ret_period"] . '" /></td></tr>';
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
} else {
	echo '<a href="/primal/setup.php?action=L">Load Config</a><BR><BR>';
	$strQuery = "SELECT count(*) FROM conf_rec;";
	$result = mysqli_query($conn, $strQuery);
	$row = mysqli_fetch_assoc($result);
	$intNumRows = $row['count(*)'];
	if($intNumRows > 0) {
		echo '<div id="rec">';
		echo '</div>';
		echo '<svg width="300" height="500"><line x1="100" y1="70" x2="350" y2="70" stroke="black"/></svg>';
		echo '<div id="proc">';
		echo '</div>';
		echo '<svg width="300" height="500"><line x1="5" y1="70" x2="350" y2="70" stroke="black"/></svg>';
		echo '<div id="out">';
		echo '</div>';
	} else {
		echo "No receivers found.  Please create a new one.<br><br>";
	}
	echo '<a href="/primal/setup.php?action=N">New Config</a><BR><BR>';
}
Display_Footer();
echo '</BODY>';
echo '</HTML>';

function fLoadConfig() {
	$strQuery = "SELECT * FROM conf_rec;";
	$result = mysqli_query($conn, $strQuery);
	$intNumRows = mysqli_num_rows($result);
	if($intNumRows > 0) {
		$intLC=1;
		while($intNumRows >= $intLC) {
			$row = mysqli_fetch_assoc($result);
			$_SESSION['conf'][$intLC]['conf_rec_id'] = $row['conf_rec_id'];
			$_SESSION['conf'][$intLC]['conf_name'] = $row['conf_name'];
			$_SESSION['conf'][$intLC]['conf_server'] = $row['conf_server'];
			$_SESSION['conf'][$intLC]['rec_type'] = $row['rec_type'];
			$_SESSION['conf'][$intLC]['rec_port'] = $row['rec_port'];
			$_SESSION['conf'][$intLC]['rec_dir'] = $row['rec_dir'];
			$_SESSION['conf'][$intLC]['rec_log_full_path'] = $row['rec_log_full_path'];
			$_SESSION['conf'][$intLC]['rec_log_level'] = $row['rec_log_level'];
			$_SESSION['conf'][$intLC]['rec_aet'] = $row['rec_aet'];
			$_SESSION['conf'][$intLC]['rec_time_out'] = $row['rec_time_out'];
			$_SESSION['conf'][$intLC]['proc_dir'] = $row['proc_dir'];
			$_SESSION['conf'][$intLC]['proc_log_full_path'] = $row['proc_log_full_path'];
			$_SESSION['conf'][$intLC]['out_dir'] = $row['out_dir'];
			$_SESSION['conf'][$intLC]['out_log_full_path'] = $row['out_log_full_path'];
			$_SESSION['conf'][$intLC]['sent_dir'] = $row['sent_dir'];
			$_SESSION['conf'][$intLC]['hold_dir'] = $row['hold_dir'];
			$_SESSION['conf'][$intLC]['error_dir'] = $row['error_dir'];
			$_SESSION['conf'][$intLC]['dupe'] = $row['dupe'];
			$_SESSION['conf'][$intLC]['out_comp_level'] = $row['out_comp_level'];
			$_SESSION['conf'][$intLC]['pass_through'] = $row['pass_through'];
			$_SESSION['conf'][$intLC]['ret_period'] = $row['ret_period'];
			$_SESSION['conf'][$intLC]['active'] = $row['active'];
			$intLC++;
		}
	} else {
		echo "No configuration found.  Please contact your administrator.";
	}

	$strQuery = "SELECT * FROM conf_proc;";
	$result = mysqli_query($conn, $strQuery);
	$intNumRows = mysqli_num_rows($result);
	if($intNumRows > 0) {
		$intLC=1;
		while($intNumRows >= $intLC) {
			$row = mysqli_fetch_assoc($result);
			$_SESSION['conf_proc'][$intLC]['conf_proc_id'] = $row['conf_proc_id'];
			$_SESSION['conf_proc'][$intLC]['conf_rec_id'] = $row['conf_rec_id'];
			$_SESSION['conf_proc'][$intLC]['proc_name'] = $row['proc_name'];
			$_SESSION['conf_proc'][$intLC]['proc_type'] = $row['proc_type'];
			$_SESSION['conf_proc'][$intLC]['proc_perator'] = $row['proc_perator'];
			$_SESSION['conf_proc'][$intLC]['proc_cond'] = $row['proc_cond'];
			$_SESSION['conf_proc'][$intLC]['proc_action'] = $row['proc_action'];
			$_SESSION['conf_proc'][$intLC]['proc_order'] = $row['proc_order'];
			$_SESSION['conf_proc'][$intLC]['proc_dest'] = $row['proc_dest'];
			$_SESSION['conf_proc'][$intLC]['active'] = $row['active'];
			$intLC++;
		}
	}

	$strQuery = "SELECT * FROM conf_send;";
	$result = mysqli_query($conn, $strQuery);
	$intNumRows = mysqli_num_rows($result);
	if($intNumRows > 0) {
		$intLC=1;
		while($intNumRows >= $intLC) {
			$row = mysqli_fetch_assoc($result);
			$_SESSION['conf_send'][$intLC]['conf_send_id'] = $row['conf_send_id'];
			$_SESSION['conf_send'][$intLC]['conf_rec_id'] = $row['conf_rec_id'];
			$_SESSION['conf_send'][$intLC]['send_name'] = $row['send_name'];
			$_SESSION['conf_send'][$intLC]['send_aet'] = $row['send_aet'];
			$_SESSION['conf_send'][$intLC]['send_aec'] = $row['send_aec'];
			$_SESSION['conf_send'][$intLC]['send_hip'] = $row['send_hip'];
			$_SESSION['conf_send'][$intLC]['send_type'] = $row['send_type'];
			$_SESSION['conf_send'][$intLC]['send_port'] = $row['send_port'];
			$_SESSION['conf_send'][$intLC]['send_time_out'] = $row['send_time_out'];
			$_SESSION['conf_send'][$intLC]['send_comp_level'] = $row['send_comp_level'];
			$_SESSION['conf_send'][$intLC]['send_retry'] = $row['send_retry'];
			$_SESSION['conf_send'][$intLC]['send_username'] = $row['send_username'];
			$_SESSION['conf_send'][$intLC]['send_password'] = $row['send_password'];
			$_SESSION['conf_send'][$intLC]['active'] = $row['active'];
			$intLC++;
		}
	}
}

/*
$strConfigFile = "/etc/primal/primal.conf";
if (file_exists($strConfigFile) == FALSE) {
	echo "Config file does not exist.<br><br>";
	echo '<a href="http://' . $_SERVER['HTTP_HOST'] . '/primal/edit_conf.php">New</a>';
}
if (is_file($strConfigFile) == FALSE) {
	echo "Config file is not a file.  Please contact your admistrator!<br><br>";
	exit(1);
}

if (isset($_SESSION['obj']) || isset($_SESSION['obj1']))
{
    unset($_SESSION['obj']);
    unset($_SESSION['obj1']);
}
unset($_SESSION['result']);
unset($_SESSION['rec_list']);

if(isset($_SESSION['HAVECONFIG']))
    unset($_SESSION['HAVECONFIG']);
if(isset($_SESSION['PRIDESTHIP']))
    unset($_SESSION['PRIDESTHIP']);
if(isset($_SESSION['PRIDESTAEC']))
    unset($_SESSION['PRIDESTAEC']);
if(isset($_SESSION['PRIDESTPORT']))
    unset($_SESSION['PRIDESTPORT']);
if(isset($_SESSION['PRIDESTCDCR']))
    unset($_SESSION['PRIDESTCDCR']);
if(isset($_SESSION['PRITAG']))
    unset($_SESSION['PRITAG']);
if(isset($_SESSION['PRIQRTAG']))
    unset($_SESSION['PRIQRTAG']);

echo '<a href="http://' . $_SERVER['HTTP_HOST'] . '/primal/edit.php?zz=1">Edit</a>';
echo '<br><br><br>';
echo '<a href="http://' . $_SERVER['HTTP_HOST'] . '/primal/add_user.php">Add User</a></br>';
echo '<br><br>';
if ($_GET['e'] == 1)
{
	$myfile = fopen("/etc/primal/primal.conf", "r") or die("Unable to open primal.conf file!");
	echo fread($myfile,filesize("/etc/primal/primal.conf"));
	fclose($myfile);
} else {
	$query = "SELECT * from user";
	$result = $conn->query($query);
	$num_rows = $result->num_rows;
	$lc1=0;
	while($row = mysqli_fetch_assoc($result)) {
		$users[$lc1]['loginid'] = $row['loginid'];
		$users[$lc1]['login_sec_level'] = $row['login_sec_level'];
		$users[$lc1]['username'] = $row['username'];
		$users[$lc1]['active'] = $row['active'];
		$users[$lc1]['access'] = $row['access'];
		$users[$lc1]['sec_bit'] = $row['sec_bit'];
		$users[$lc1]['page_size'] = $row['page_size'];
		$lc1++;
	}
	echo '<table border="1">';
	echo '<tr>';
	echo '<th>loginid</th>';
	echo '<th>Security Level</th>';
	echo '<th>username</th>';
	echo '<th>active</th>';
	echo '<th>Last Access</th>';
	echo '<th>Security Bits</th>';
	echo '<th>Page Size</th>';
	echo '</th>';
	$lc1=0;
	foreach($users as $u) {
		if($users[$lc1]['login_sec_level'] <= $_SESSION['login_sec_level']) {
			echo '<tr>';
			echo '<td><a href="http://' . $_SERVER['HTTP_HOST'] . '/primal/update_user.php?e=' . $users[$lc1]['loginid'] . '">' . $users[$lc1]['loginid'] . '</a></td>';
			echo '<td>' . $users[$lc1]['login_sec_level'] . '</td>';
			echo '<td>' . $users[$lc1]['username'] . '</td>';
			echo '<td>' . $users[$lc1]['active'] . '</td>';
			echo '<td>' . $users[$lc1]['access'] . '</td>';
			echo '<td>' . $users[$lc1]['sec_bit'] . '</td>';
			echo '<td>' . $users[$lc1]['page_size'] . '</td>';
			echo '</tr>';
		}
		$lc1++;
	}
	echo '</table>';
}
echo '   </form><br>';
*/
Display_Footer();
echo '</BODY>';
echo '</HTML>';

?>
