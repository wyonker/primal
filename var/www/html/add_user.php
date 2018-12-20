<?php
    //Version 2.99.00
    //Build 2
    //2015-09-12
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
if (isset($_SESSION['obj']) || isset($_SESSION['obj1']))
{
    unset($_SESSION['obj']);
    unset($_SESSION['obj1']);
}

$query="SELECT SCHEMA_NAME FROM INFORMATION_SCHEMA.SCHEMATA WHERE SCHEMA_NAME = 'primalarc';";
$result=mysql_query($query);
$has_migration = mysql_num_rows($result);
if($_SERVER['REQUEST_METHOD'] == 'POST')
{
	if(isset($_POST['cancel']))
	{
		header("Location: http://" . $_SERVER['HTTP_HOST'] . "/setup.php");
		//http_redirect('http://' . $_SERVER['HTTP_HOST'] . '/setup.php', true, HTTP_REDIRECT_PERM);
	}
	if(isset($_POST['reset']))
	{
		unset($_POST['add_loginid']);
		unset($_POST['add_login_sec_level']);
		unset($_POST["add_user_name"]);
	}
	if (!ctype_alnum($_POST['add_loginid']))
	{
		$_SESSION['add_error'] .= "Error:  Login ID must contain only alpha/numeric characters.  Please try again...<br>";
	} elseif(strlen($_POST['add_loginid']) > 63) {
		$_SESSION['add_error'] .= "Error:  Login Id can not be longer than 63 characters.  Please try again...<br>";
	} else {
		$query="select count(*) as total from user where loginid = '" . $_POST['add_loginid'] . "';";
		$result=mysql_query($query);
		if ($result['total'] > 0 )
		{
			$_SESSION['add_error'] = "Error:  Username exists.  Please try again...<br>";
		}
	}
	if ($_POST['add_login_sec_level'] < 1 || $_POST['add_login_sec_level'] > 254 || !is_numeric($_POST['add_login_sec_level']))
	{
		$_SESSION['add_error'] .= "Error:  Secuirty Level must be a number from 1 - 254.  Please try again...<br>";
	} elseif ($_POST['add_login_sec_level'] > $_SESSION['login_sec_level']) {
		$_SESSION['add_error'] .= "Error:  Cannot grant a user more permissions than yourself.  Please try again...<br>";
	}
	if(!isset($_GET["e"]))
	{
		$query = "select * from user where loginid = '" . trim($_GET["e"]) . "';";
		$result = mysql_query($query);
	    $num_rows = mysql_num_rows($result);
		if($num_rows > 0)
		{
			$_SESSION['add_error'] .= "Error:  User Name exists.  Please try again...<br>";
		}
	}
	$aValid = array('-', '_', ' ', '.'); 
	if (!isset($_POST["add_user_name"]) || !ctype_alnum(str_replace($aValid, 'A', $_POST['add_user_name'])))
	{
		$_SESSION['add_error'] .= "Error:  User Name can not be blank and must contain only alpha/number charcters (including -, _ and . .   Please try again...<br>";
	} elseif (strlen($_POST["add_user_name"]) > 127) {
		$_SESSION['add_error'] .= "Error:  User name can not be longer than 127 characters.  Please try again...<br>";
	}
	if(!isset($_POST['add_page_size']))
	{
		$_POST['add_page_size'] = 0;
	} elseif(!ctype_digit($_POST['add_page_size']) || $_POST['add_page_size'] < 0 || $_POST['add_page_size'] > 1000) {
		$_SESSION['add_error'] .= "Error:  Page_Size must be a number from 0 to 1000<br>";
	}
	if($has_migration > 0)
	{
		if(isset($_POST['access_migration'])) {
			$_POST['access_migration'] = 1;
		} else {
			$_POST['access_migration'] = 0;
		}
		if(isset($_POST['add_send_legacy'])) {
			$_POST['add_send_legacy'] = 1;
		} else {
			$_POST['add_send_legacy'] = 0;
		}
		if(isset($_POST['add_send_smn02'])) {
			$_POST['add_send_smn02'] = 1;
		} else {
			$_POST['add_send_smn02'] = 0;
		}
		if(isset($_POST['add_send_anywhere'])) {
			$_POST['add_send_anywhere'] = 1;
		} else {
			$_POST['add_send_anywhere'] = 0;
		}
		if(isset($_POST['add_send_impax'])) {
			$_POST['add_send_impax'] = 1;
		} else {
			$_POST['add_send_impax'] = 0;
		}
		if(isset($_POST['access_audit'])) {
			$_POST['access_audit'] = 1;
		} else {
			$_POST['access_audit'] = 0;
		}
	}
	if(isset($_POST["add_password1"]) && $_POST["add_password1"] != "")
	if(isset($_POST["add_password1"]) && $_POST["add_password1"] != "")
	{
		if(strlen($_POST["add_password1"]) < 8)
		{
			$_SESSION['add_error'] .= "Error:  Password must be a minimum of 8 characters, contain one capital and one number.  Please try again...<br>";
		} elseif(preg_match_all('/[A-Z]/', $_POST["add_password1"], $matches) === FALSE) {
			$_SESSION['add_error'] .= "Error:  Password must be a minimum of 8 characters, contain one capital and
one number.  Please try again...<br>";
		} elseif(preg_match_all('/[0-9]/', $_POST["add_password1"], $matches) === FALSE) {
			$_SESSION['add_error'] .= "Error:  Password must be a minimum of 8 characters, contain one capital and
one number.  Please try again...<br>";
		}
	}
	if(!isset($_SESSION['add_error']))
	{
		if($has_migration > 0) {
			$sec_bit = $_POST['access_migration'] . $_POST['add_send_legacy'] . $_POST['add_send_smn02'] . $_POST['add_send_anywhere'] . $_POST['add_send_impax'] . $_POST['access_audit'];
		} else {
			$sec_bit = "000000";
		}
		$query = "insert into user (loginid, login_sec_level, username, active, page_size, sec_bit) values('" . $_POST['add_loginid'] . "', '";
		$query .= $_POST['add_login_sec_level'] . "', '" . $_POST["add_user_name"] . "', '" . $_POST["add_active"] . "', '";
		$query .= $_POST['add_page_size'] . "', '" . $sec_bit . "')";
		$result = mysql_query($query);
		if(!$result)
		{
			echo 'Query = ' . $query . '<br>';
			echo mysql_errno() . ": " . mysql_error(). "<br>";
		}
		header("Location: http://" . $_SERVER['HTTP_HOST'] . "/setup.php");
	} else {
		echo $_SESSION['add_error'] . '<br>';
		unset($_SESSION['add_error']);
	}
} elseif(isset($_GET["e"])) {
	$query = "select * from user where loginid = '" . trim($_GET["e"]) . "';";
	$result = mysql_query($query);
	$num_rows = mysql_num_rows($result);
	if($num_rows > 1)
	{
		$_SESSION['add_error'] .= "Error:  Multiple results found for selected user.  Please contact your administrator...<br>";
	} elseif($num_rows < 1) {
		$_SESSION['add_error'] .= "Error:  Username not found.  Please try again.  If this error persists, please contact your administrator.<br>";
	}
	if(!isset($_SESSION['add_error']))
	{
		$row = mysql_fetch_assoc($result);
		$_POST['add_loginid'] = $row['loginid'];
		$_POST['add_login_sec_level'] = $row['login_sec_level'];
		$_POST["add_user_name"] = $row['username'];
		$_POST["add_active"] = $row['active'];
		$_POST['add_page_size'] = $row['page_size'];
		unset($_POST["add_password1"]);
		unset($_POST["add_password2"]);
	} 
	if($_POST['add_login_sec_level'] > $_SESSION['login_sec_level']) {
		$_SESSION['add_error'] .= "Error:  Cannot modify a user with more permissions than yourself.  Please try again...<br>";
	} 
	if(isset($_SESSION['add_error']))
	{
		echo $_SESSION['add_error'] . '<br>';
	}
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

echo '<form action="add_user.php" method="post">';
echo '<H1>PRIMAL Web Interface</H1>';
echo '<table border="1">';
echo '<tr>';
echo '<td>' . 'Login ID' . '</td>';
echo '<td>' . '<input type="text" name="add_loginid" value="' . $_POST['add_loginid'] . '" />' . '</td></tr>';
echo '<td>' . 'Security Level' . '</td>';
echo '<td>' . '<input type="text" name="add_login_sec_level" value="' . $_POST['add_login_sec_level'] . '" />' . '</td></tr>';
echo '<td>' . 'User Name' . '</td>';
echo '<td>' . '<input type="text" name="add_user_name" value ="' . $_POST["add_user_name"] . '" />' . '</td></tr>';
echo '<td>' . 'Active' . '</td>';
echo '<td>' . '<input type="text" name="add_active" value="' . $_POST["add_active"] . '" />' . '</td></tr>';
echo '<td>' . 'Page Size' . '</td>';
echo '<td>' . '<input type="text" name="add_page_size" value="' . $_POST["add_page_size"] . '" />' . '</td></tr>';
if($has_migration > 0)
{
	echo '<tr><td>' . 'Can access the Migration pages' . '</td>';
	if($_POST['access_migration'] == 1) {
		echo '<td><input type="checkbox" name="access_migration" value="1" checked />' . '</td></tr>';
	} else {
		echo '<td><input type="checkbox" name="access_migration" value="0" />' . '</td></tr>';
	}
	echo '<tr><td>' . 'Can send to Legacy' . '</td>';
	if($_POST['add_send_legacy'] == 1) {
		echo '<td><input type="checkbox" name="add_send_legacy" value="1" checked />' . '</td></tr>';
	} else {
		echo '<td><input type="checkbox" name="add_send_legacy" value="0" />' . '</td></tr>';
	}
	echo '<tr><td>' . 'Can send to Nexus02' . '</td>';
	if($_POST["add_send_smn02"] == 1) {
		echo '<td><input type="checkbox" name="add_send_smn02" value="1" checked />' . '</td></tr>';
	} else {
		echo '<td><input type="checkbox" name="add_send_smn02" value="0" />' . '</td></tr>';
	}
	echo '<tr><td>' . 'Can send Anywhere' . '</td>';
	if($_POST["add_send_anywhere"] == 1) {
		echo '<td><input type="checkbox" name="add_send_anywhere" value="1" checked />' . '</td></tr>';
	} else {
		echo '<td><input type="checkbox" name="add_send_anywhere" value="0" />' . '</td></tr>';
	}
	echo '<tr><td>' . 'Can send to Impax' . '</td>';
	if($_POST["add_send_impax"] == 1) {
		echo '<td><input type="checkbox" name="add_send_impax" value="1" checked />' . '</td></tr>';
	} else {
		echo '<td><input type="checkbox" name="add_send_impax" value="0" />' . '</td></tr>';
	}
	echo '<tr><td>' . 'Can access the Audit logs' . '</td>';
	if($_POST["access_audit"] == 1) {
		echo '<td><input type="checkbox" name="access_audit" value="1" checked />' . '</td></tr>';
	} else {
		echo '<td><input type="checkbox" name="access_audit" value="0" />' . '</td></tr>';
	}
}
echo '</table><br>';
echo 'Active should be -1 to force the user to change their password<br>';
echo '0 for an inactive account (user can not log in)<br>';
echo '1 for an active account<br>';
echo '<br><button type="submit" name="submit">Add</button>';
echo '<button type="submit" name="reset">Clear</button>';
echo '<button type="submit" name="cancel">Cancel</button>';
echo '   </form>';
Display_Footer();
echo '</BODY>';
echo '</HTML>';
unset($_SESSION['add_error']);
?>
