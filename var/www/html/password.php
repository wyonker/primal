<?php
	//License GPLv3
    session_start();
    header( "Expires: Mon, 20 Dec 1998 01:00:00 GMT" );
    header( "Last-Modified: " . gmdate("D, d M Y H:i:s") . " GMT" );
    header( "Cache-Control: no-cache, must-revalidate" );
    header( "Pragma: no-cache" );
	require_once ('config.php');
	require_once ('functions.php');

if($_SESSION['callerid']!= "/update_user.php" && $_SESSION['callerid']!= "/login2.php" && $_SESSION['callerid']!= "/options.php")
{
	header ("location: login.php");
}
if($_SERVER['REQUEST_METHOD'] == 'POST')
{
    $curpasswd = base64_encode($_POST['curpassword']);
    $newpasswd = base64_encode($_POST['password']);
    $newpasswd2 = base64_encode($_POST['password2']);

    $_SESSION['perror'] = 0;

    if ($newpasswd != $newpasswd2)
    {
        $_SESSION['perror'] = 1;
    } else {
		$query = "SELECT * from user WHERE loginid = '" . $_SESSION['loginid'] . "'";
		$result = run_query($query);
		$num_rows = mysql_num_rows($result);
		if ($num_rows <= 0)
		{
			$_SESSION['perror'] = 2;
		}

		if ($num_rows >= 2)
		{
			$_SESSION['perror'] = 3;
		}
		$row = mysql_fetch_assoc($result);
	}
	if($_SESSION['callerid']!= "/update_user.php")
	{
		if ($row['password'] != $curpasswd)
		{
			$_SESSION['perror'] = 4;
		}
	}
    if ($_SESSION['perror'] == 0)
    {
        if ($row['active'] == -1)
        {
            $isactive = 1;
        } else {
            $isactive = $row['active'];
        }
        $query = "UPDATE user " .
                 "SET password = '" . $newpasswd . "', " .
                 "active = '" . $isactive . "' ";
		if ($_SESSION['callerid']== "/update_user.php")
		{
			$query = $query . "WHERE loginid = '" . $_SESSION['orig_loginid'] . "'";
		} else {
			$query = $query . "WHERE loginid = '" . $_SESSION['loginid'] . "'";
		}
        $result = run_query($query);
		if($_SESSION['callerid']== "/update_user.php")
		{
			header ("location: setup.php");
		} elseif ($_SESSION['callerid']== "/index.php") {
			header ("location: index.php");
		} else {
			header ("location: login.php");
		}
    } else {
        header ("location: password.php");
    }
} else {
echo <<<EOT
	<HTML>
	<!DOCTYPE !DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN"
				"http://www.w3.org/TR/html4/loose.dtd">
	<HEAD>
	   <TITLE>PRIAML Web Interface</TITLE>
	   <!-- Written by Will Yonker-->
		<link rel="stylesheet" href="default.css">
	</HEAD>
	<BODY>
EOT;
Display_Header2();
	echo '<br><br><br>';
	echo '<h2>Setup Screen</h2><br />';
if($_SESSION['loginid'] == "primal")
{
	echo '<b><font color="red">WARNING:  NEVER CHANG THE PRIMAL USER\'S PASSWORD!!!</font></b><br><br>';
}
if($_SESSION['callerid']== "/update_user.php")
{
	echo '<h3>Modifying password for ' . $_SESSION['orig_loginid'] . '</h3><br>';
}
echo '<form name="form1" method="post" action="password.php">';
echo '<table>';
if($_SESSION['callerid']!= "/update_user.php")
{
	echo '<tr><td>Enter Current Password:</td><td>';
	echo '<input type="password" name="curpassword"></td></tr>';
}
echo <<<EOT
			 <tr><td>Enter New Password:</td><td>
			 <input type="password" name="password"></td></tr>
			 <tr><td>Enter New Password Again:</td><td>
			 <input type="password" name="password2"></td></tr>
		  </table> <br>
		  <input type="submit" value="Submit">
		  &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
		  <input type="reset" value="Clear" /><br><br>
		  <br>
EOT;
}
if ($_SESSION['perror'] == 1)
{
    $err_msg = "Error:  New passwords did not match!";
} elseif ($_SESSION['perror'] == 2) {
    $err_msg = "Error:  Username not found!";
} elseif ($_SESSION['perror'] == 3) {
    $err_msg = "Error:  Database inconsistancy!  Please contact your administrator.";
} elseif ($_SESSION['perror'] == 4) {
    $err_msg = "Error:  Current password did not match!";
} elseif ($_SESSION['perror'] == 5) {
    $err_msg = "Error:  Your password has expired.  Please change.";
}

echo '<div style="color:#FF0000;"><blink>' . $err_msg . "</blink></div><br>";

echo '   </form>';
Display_Footer();
echo '</BODY>';
echo '</HTML>';
$_SESSION['perror'] = 0;
?>

