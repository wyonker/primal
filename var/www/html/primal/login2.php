<?php
    //License GPLv3
    session_start();
    header( "Expires: Mon, 20 Dec 1998 01:00:00 GMT" );
    header( "Last-Modified: " . gmdate("D, d M Y H:i:s") . " GMT" );
    header( "Cache-Control: no-cache, must-revalidate" );
    header( "Pragma: no-cache" );
	require_once ('config.php');
	require_once ('functions.php');
	date_default_timezone_set('America/New_York');
	$_SESSION['callerid']=$_SERVER['PHP_SELF'];
	$name_entered = $_POST['username'];
	$password_entered = base64_encode($_POST['password']);
	$query = "SELECT * from user " .
			 "WHERE loginid = '" . $name_entered . "'";
	$result = $conn->query($query);
    $num_rows = $result->num_rows;
    if ($num_rows <= 0) {
        $_SESSION['retry'] = 2;
        $_SESSION['active'] = 0;
    } elseif ($num_rows >= 2) {
        $_SESSION['retry'] = 3;
        $_SESSION['active'] = 0;
        $_SESSION['login_username'] = $name_entered;
    } else {
		$row = mysql_fetch_assoc($result);
		if ($row['password'] == $password_entered) {
			$_SESSION['loginid'] = $row['loginid'];
			$_SESSION['login_sec_level'] = $row['login_sec_level'];
			$_SESSION['login_username'] = $row['username'];
			$_SESSION['active'] = $row['active'];
			$_SESSION['page_size'] = $row['page_size'];
			$_SESSION['login_sec_bit'] = $row['sec_bit'];
			$_SESSION['rec_sec'] = $row['rec_sec'];
			if(!isset($_SESSION['page_size']) || (!ctype_digit($_SESSION['page_size']))) {
				$_SESSION['page_size'] = 0;
			}
            $_SESSION['retry'] = 0;
            if ($row['active'] != 1) {
				if ($row['active'] == -1) {
					$_SESSION['perror'] = 5;
					$_SESSION['callerid']=$_SERVER['PHP_SELF'];
					header ("location: password.php");
					exit ();
				}
                $_SESSION['retry'] = 4;
                $_SESSION['active'] = 0;
            }
		} else {
			$_SESSION['retry'] = 1;
			$_SESSION['active'] = 0;
		}
    }
    if ($_SESSION['retry'] == 0) {
        $query = "UPDATE user " .
                 "SET access = '" . date('Y.m.d:H.i.s') . "' " .
                 "WHERE loginid = '" . $_SESSION['loginid'] . "'";
        $result = $conn->query($query);
		$strMessage = "User " . $_SESSION['login_username'] . " logged in at " . date('Y.m.d:H.i.s') . "<br />";
		write_to_log($strMessage);
		header ("location: index.php");
    } else {
        header ("location: login.php");
    }
?>
