<?php
    //License GPLv3
	//Version 1.00.06
	//2025-04-23
	//Written by Will Yonker

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
	$password_ldap = $_POST['password'];
	if(isset($_POST['cbLDAP'])) {
        $dn = "OU=" . $LDAPGroup . "," . $LDAPBaseDN;
        $ldap_dn = $name_entered . "@" . $LDAPDN;
        $ad = ldap_connect("ldap://" . $LDAPDomain . "/") or die('Could not connect to LDAP server.');
        ldap_set_option($ad, LDAP_OPT_PROTOCOL_VERSION, 3);
        ldap_set_option($ad, LDAP_OPT_REFERRALS, 0);
        $r = ldap_bind($ad, $LDAPShortName . "\\" . $name_entered, $password_ldap);
        if($r) {
            echo "Success!<br>";
        } else {
            header ("location: login.php");
            exit();
        }

        $filter="(samAccountName=" . $name_entered . ")";
        $arrEntries= array("memberOf");
        $search = "CN=*";
        $sr = ldap_search($ad, $dn, $filter, $arrEntries);
        $data = ldap_get_entries($ad, $sr);
        $strADGroups = print_r($data, true);
        if(strpos($strADGroups, "primal_admin") !== FALSE) {
            $_SESSION['loginid'] = $name_entered;
            $_SESSION['login_sec_level'] = 250;
            $_SESSION['login_username'] = $LDAPShortName . "//" . $name_entered;
            $_SESSION['active'] = 1;
            $_SESSION['login_type'] = 'AD';
			$_SESSION['login_sec_bit'] = "000000";
			$_SESSION['rec_sec'] = "1 2 3 4 5 6 7 8 9";
            $strLogMessage = "AD User: " . $_SESSION['loginid'] . " logged in with security level " . $_SESSION['login_sec_level'];
            write_to_log($strLogMessage);
            header ("location: index.php");
        } elseif(strpos($strADGroups, "primal_user") !== FALSE) {
            $_SESSION['loginid'] = $name_entered;
            $_SESSION['login_sec_level'] = 100;
            $_SESSION['login_username'] = $LDAPShortName . "//" . $name_entered;
            $_SESSION['active'] = 1;
            $_SESSION['login_type'] = 'AD';
			$_SESSION['login_sec_bit'] = "000000";
			$_SESSION['rec_sec'] = "1 2 3 4 5 6 7 8 9";
            $strLogMessage = "AD User: " . $_SESSION['loginid'] . " logged in with security level " . $_SESSION['login_sec_level'];
            write_to_log($strLogMessage);
            header ("location: index.php");
        } else {
            ldap_unbind($ad);
            header ("location: login.php");
        }
        ldap_unbind($ad);
	} else {
	    $strQuery = "SELECT * FROM user WHERE loginid = '" . $name_entered . "'";
	    $result = $conn->query($strQuery);
	    $num_rows = $result->num_rows;
	    if ($num_rows <= 0) {
			$_SESSION['retry'] = 2;
			$_SESSION['active'] = 0;
	    } elseif ($num_rows >= 2) {
			$_SESSION['retry'] = 3;
			$_SESSION['active'] = 0;
			$_SESSION['login_username'] = $name_entered;
	    } else {
			$row = mysqli_fetch_assoc($result);
			if ($row['password'] == $password_entered) {
				$_SESSION['loginid'] = $row['loginid'];
				$_SESSION['login_sec_level'] = $row['login_sec_level'];
				$_SESSION['login_username'] = $row['username'];
				$_SESSION['active'] = $row['active'];
				$_SESSION['login_type'] = 'LOCAL';
				if ($row['page_size'] < 10 || $row['page_size'] > 100) {
					$_SESSION['page_size'] = 30;
				} else {
					$_SESSION['page_size'] = $row['page_size'];
				}
				$_SESSION['login_sec_bit'] = $row['sec_bit'];
				$_SESSION['rec_sec'] = $row['rec_sec'];
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
			$strMessage = "Logged in.";
			write_to_log($strMessage);
			header ("location: index.php");
		} else {
			header ("location: login.php");
		}
	}
/*
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
		$row = mysqli_fetch_assoc($result);
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
*/
?>
