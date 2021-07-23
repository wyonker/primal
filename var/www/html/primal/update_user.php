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
if (isset($_SESSION['obj']) || isset($_SESSION['obj1']))
{
    unset($_SESSION['obj']);
    unset($_SESSION['obj1']);
}

$query="SELECT SCHEMA_NAME FROM INFORMATION_SCHEMA.SCHEMATA WHERE SCHEMA_NAME = 'primalarc';";
$result=mysql_query($query);
$has_migration = mysql_num_rows($result);
$_SESSION['callerid']=$_SERVER['PHP_SELF'];
if($_SERVER['REQUEST_METHOD'] == 'POST')
{
	if(isset($_POST['cancel']))
	{
		header("Location: http://" . $_SERVER['HTTP_HOST'] . "/primal/setup.php");
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
		if($_POST['add_login_sec_level'] > 254 && $_POST['add_loginid'] != 'primal')
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
		$_SESSION['add_error'] .= "Error:  User Name can not be blank and must contain only alpha/number charctersi (including -, _, ., and space.   Please try again...<br>";
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
	if(!isset($_SESSION['add_error']))
	{
		if(isset($_POST["update"]))
		{
			if($has_migration > 0) {
				$sec_bit = $_POST['access_migration'] . $_POST['add_send_legacy'] . $_POST['add_send_smn02'] . $_POST['add_send_anywhere'] . $_POST['add_send_impax'] . $_POST['access_audit'];
			} else {
				$sec_bit = "000000";
			}
			$query = "update user set loginid = '" . $_POST['add_loginid'] . "', login_sec_level = '" . $_POST['add_login_sec_level'] . "', ";
			$query .= "username = '" . $_POST["add_user_name"] . "', active = '" . $_POST["add_active"] . "', page_size = '";
			$query .= $_POST['add_page_size'] . "', sec_bit = '" . $sec_bit . "'";
		    $Rec_Names=explode(" ", $_SESSION['rec_list']);
			foreach ($Rec_Names as &$value) {
				$strValue=$value . " ";
				if (isset($_POST[$value])) {
					$strRecAccess .= $strValue;
				}
			}
			$query .= ", rec_sec = '" . $strRecAccess . "'";
			$query .= " where loginid = '" . $_SESSION['orig_loginid'] . "';";
			echo $query . "<br>";
		}
		$result = mysql_query($query);
		unset($_SESSION['orig_loginid']);
		if(!$result)
		{
			echo 'Query = ' . $query . '<br>';
			echo mysql_errno() . ": " . mysql_error(). "<br>";
		}
		header("Location: http://" . $_SERVER['HTTP_HOST'] . "/primal/setup.php");
	} else {
		echo $_SESSION['add_error'] . '<br>';
	}
} elseif(isset($_GET["e"])) {
    if(! isset($_SESSION['rec_list'])) {
        $handle = fopen("/etc/primal/primal.conf", "r") or die("Unable to open primal.conf file!");
        $intNumRec=0;
        while (($buffer = fgets($handle, 4096)) !== false) {
            #Strip off comments
            $intPOS=strpos($buffer, "#");
            if($intPOS !== FALSE && $intPOS != 0) {
                $strLINE=substr($buffer, 0, ($intPOS - 1));
            } elseif($intPOS !== FALSE && $intPOS == 0) {
                $strLINE="";
            } else {
                $strLINE=$buffer;
            }
            $strLINE=trim($strLINE);
            if(stripos($strLINE, "<scp")  !== FALSE) {
                $intPOS=stripos($strLINE, "<scp");
                $intePOS=stripos($strLINE, ">");
                $strRecName=substr($strLINE, ($intPOS+4), ($intePOS-$intPOS-4));
                $strRecName=$strRecName . " ";
                $_SESSION['rec_list']=$_SESSION['rec_list'] . $strRecName;
            }
        }
    }
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
		$_SESSION['orig_loginid'] = $row['loginid'];
		$_POST['add_login_sec_level'] = $row['login_sec_level'];
		$_POST["add_user_name"] = $row['username'];
		$_POST["add_active"] = $row['active'];
		$_POST['add_page_size'] = $row['page_size'];
		$_POST['access_migration'] = substr($row['sec_bit'], 0, 1);
		$_POST['add_send_legacy'] = substr($row['sec_bit'], 1, 1);
		$_POST['add_send_smn02'] = substr($row['sec_bit'], 2, 1);
		$_POST['add_send_anywhere'] = substr($row['sec_bit'], 3, 1);
		$_POST['add_send_impax'] = substr($row['sec_bit'], 4, 1);
		$_POST['access_audit'] = substr($row['sec_bit'], 5, 1);
		$strTemp = $row['rec_sec'];
		$Rec_Names=explode(" ", $strTemp);
	    foreach ($Rec_Names as &$value) {
			$strValue=$value . " ";
			if(strpos($_SESSION['rec_list'], $strValue) !== FALSE) {
				$_POST[$value] = 1;
			} else {
				$_POST[$value] = 0;
			}
		}
		unset($_POST["add_password1"]);
		unset($_POST["add_password2"]);
	} 
	if($_POST['add_login_sec_level'] > $_SESSION['login_sec_level']) {
		$_SESSION['add_error'] .= "Error:  Cannot modify a user with more permissions than yourself.  Please try again...<br>";
	}
	if (isset($_GET["c"])) {
		if (isset($_GET["h"])) {
			#If c and h are set, then we want to hide (or unhide) a column.
			$query="update user_columns set column" . $_GET["c"] . "_Visible = '" . $_GET["h"] . "' where user_id = '" . $_GET["e"] . "';";
			$result = mysql_query($query);
		} elseif (isset($_GET["o"])) {
			$query = "select count(*) from page_columns;";
			$result = mysql_query($query);
			while ($row = mysql_fetch_assoc($result)) {
				$num_columns = $row ['count(*)'];
			}
			$query="select Column" . $_GET["c"] . "_Order from user_columns where user_id = '" . $_GET["e"] . "';";
			$result = mysql_query($query);
			if (!$result) {
				echo "query = " . $query . "<br>";
				die('Invalid query: ' . mysql_error());
			}
			while ($row = mysql_fetch_assoc($result)) {
				$curpos = $row['Column' . $_GET["c"] . "_Order"];
			}
			$is_error = 0;
			if($_GET["o"] == "u" && $curpos > 1) {
				$newpos = $curpos - 1;
			} elseif ($_GET["o"] == "d" && $_GET["o"] < ($num_columns - 1)) {
				$newpos = $curpos + 1;
			} else {
				$is_error = 1;
			}
			if($is_error != 1) {
				$lc5 = 1;
				$intFound=0;
				#Search the database for any column that reports to be in the same position as where we want to move to.
				while($lc5 < $num_columns) {
					$query="select Column" . $lc5 . "_Order from user_columns where user_id = '" . $_GET["e"] . "';";
					$result = mysql_query($query);
					while ($row = mysql_fetch_assoc($result)) {
						if($row['Column' . $lc5 . "_Order"] == $newpos) {
							#We _shouldn't_ have multiple columns reporting the same position.  But let's increment just to be sure.
							$intFound++;
							#Save the last found ID so we have something to move (most times, this is the only guy in this position)
							$intFoundID=$lc5;
						}
					}
					$lc5++;
				}
				if($intFound == 1 || $intFound == 0) {
					#We only found one guy at this position (expected result).  Swap the found guy and the guy we want to move's positions.
					$query1="update user_columns set column" . $intFoundID . "_Order = '" . $curpos . "' where user_id = '" . $_GET["e"] . "';";
					$result1 = mysql_query($query1);
					$query1="update user_columns set column" . $_GET["c"] . "_Order = '" . $newpos . "' where user_id = '" . $_GET["e"] . "';";
					$result1 = mysql_query($query1);
				} else {
					#Uh oh found more than one guy at that position.
					#Fist let's deal with the case where we don't find anyone at the address we want to use
					if($intFound == 0) {
						$lc5=1;
						while($lc5 < $num_columns) {
							#Query the database and put each result in it's position in an array
							$query="select Column" . $lc5 . "_Order from user_columns where user_id = '" . $_GET["e"] . "';";
							$result = mysql_query($query);
							#Increment the positions so we can see who gets hit more than once
							$positions[$row['Column' . $lc5 . "_Order"]]++;
							$lc5++;
						}
						$lc5=1;
						#Loop through array and see who got hit more than once.
						while($lc5 < $num_columns) {
							#Set intFoundID to the duplicate
							if($positions[$lc5] > 1) {
								$intFoundID=$lc5;
							}
						}
					}
					$is_resolved=0;
					$lc5=1;
					$intFound=0;
					while($is_resolved==0) {
						unset($intPOS);
						while($lc5 < $num_columns) {
							$query="select Column" . $lc5 . "_Order from user_columns where user_id = '" . $_GET["e"] . "';";
							$result = mysql_query($query);
							if(! is_numeric($row['Column' . $lc5 . "_Order"]) || $row['Column' . $lc5 . "_Order"] > $num_columns || $row['Column' . $lc5 . "_Order"] < 1) {
								$query1="update user_columns set column" . $intFoundID . "_Order = '" . $lc5 . "' where user_id = '" . $_GET["e"] . "';";
								$result1 = mysql_query($query1);
								$is_resolved=1;
							}
							$lc5++;
						}
						if($is_resolved == 0) {
							echo "ERROR:  Database is in an unusable state!  Please contact your administrator!!!  Exiting...<br>";
							exit(1);
						}
						$lc5 = 1;
						$intFound=0;
						while($lc5 < $num_columns) {
							$query="select Column" . $lc5 . "_Order from user_columns where user_id = '" . $_GET["e"] . "';";
							$result = mysql_query($query);
							while ($row = mysql_fetch_assoc($result)) {
								if($row['Column' . $lc5 . "_Order"] == $newpos) {
									$intFound++;
									$intFoundID=$lc5;
								}
							}
						}
						if($intFound > 1) {
							$is_resolved=0;
						}
					}
				}
			}
		}
	} 
	if(isset($_SESSION['add_error']))
	{
		echo $_SESSION['add_error'] . '<br>';
	}
}

#Need to pull column information from the database
$lc1=0;
$query="select * from page_columns;";
$result = mysql_query($query);
while($row = mysql_fetch_assoc($result))
{
	$ucolumns[$lc1]["id"] = $row["id"];
	$ucolumns[$lc1]["name"] = $row["name"];
	$ucolumns[$lc1]["dorder"] = $row["dorder"];
	$ucolumns[$lc1]["order"] = $row["dorder"];
	$ucolumns[$lc1]["visible"] = 0;

	$lc1++;
}

#Now that we have all the page columns, let's retrieve the user's settings for them
$lc2=0;
$query="select * from user_columns where user_id = '" . $_GET["e"] . "';";
$result = mysql_query($query);
$num_rows = mysql_num_rows($result);
#If we didn't get any results, we need to add this user to the user_columns table.
#This will set default values for all columns.
if ($num_rows < 1) {
	$query="insert into user_columns (user_id) values ('" . $_GET["e"] . "');";
	$result2 = mysql_query($query);
}

#Now that the user is added, let's start over and query again so that we can get the default values	
$query="select * from user_columns where user_id = '" . $_GET["e"] . "';";
$result = mysql_query($query);
$num_rows = mysql_num_rows($result);

while($row = mysql_fetch_assoc($result))
{
	$lc2=0;
	while($lc2 < count($ucolumns)) {
		$ucolumns[$lc2]["visible"] = $row["Column" . $ucolumns[$lc2]["id"] . "_Visible"];
		$ucolumns[$lc2]["order"] = $row["Column" . $ucolumns[$lc2]["id"] . "_Order"];
		$lc2++;
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

echo '<form action="update_user.php" method="post">';
Display_Header2();
echo '<a href="http://' . $_SERVER['HTTP_HOST'] . '/primal/password.php">Reset Password</a><br><br>';
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
echo '<br><br>';
echo '<table border="1">';
echo '<tr><td colspan="2"><p style="font-size:20px">Receiver Access</p></td></tr>';
echo '<tr><th>Receiver</th><th>Visible</th></tr>';
$Rec_Names=explode(" ", $_SESSION['rec_list']);
foreach ($Rec_Names as &$value) {
	if (trim($value) != "") {
		echo '<tr><td>' . $value . '</td>';
		if($_POST[$value] == 1) {
			echo '<td><input type="checkbox" name="' . $value . '" value="1" checked />' . '</td></tr>';
		} else {
			echo '<td><input type="checkbox" name="' . $value . '" value="0" />' . '</td></tr>';
		}
	}
}

echo '</table><br>';
echo '<br><br>';
echo '<table border="1">';
echo '<tr><td colspan="4"><p style="font-size:20px">Column Settings</p></td></tr>';
echo '<tr><th>Column Name</th><th>Visible</th><th colspan="2">Movement</th></tr>';
$lc3=0;
while ($lc3 <= count($ucolumns)) {
	$lc4=0;
	while ($lc4 < count($ucolumns)) {
		if ( $ucolumns[$lc4]["order"] == $lc3 ) {
			echo '<tr>';
			if ( $ucolumns[$lc4]["visible"] == 1) {
				echo '<td>Hidden: ' . $ucolumns[$lc4]["name"] . '</td>';
				echo '<td><a href="http://' . $_SERVER['HTTP_HOST'] . '/primal/update_user.php?e=' . $_GET["e"] . '&c=' . $ucolumns[$lc4]["dorder"] . '&h=0' . '">Unhide</a></td>';
			} else {
				echo '<td>' . $ucolumns[$lc4]["name"] . '</td>';
				echo '<td><a href="http://' . $_SERVER['HTTP_HOST'] . '/primal/update_user.php?e=' . $_GET["e"] . '&c=' . $ucolumns[$lc4]["dorder"] . '&h=1' . '">Hide</a></td>';
			}
			if ($lc3 > 1) {
				echo '<td><a href="http://' . $_SERVER['HTTP_HOST'] . '/primal/update_user.php?e=' . $_GET["e"] . '&c=' . $ucolumns[$lc4]["dorder"] . '&o=u">Up</a></td>';
			} else {
				echo '<td></td>';
			}
			if ($lc3 < count($ucolumns)) {
				echo '<td><a href="http://' . $_SERVER['HTTP_HOST'] . '/primal/update_user.php?e=' . $_GET["e"] . '&c=' . $ucolumns[$lc4]["dorder"] . '&o=d">Down</a></td>';
			} else {
				echo '<td></td>';
			}
			echo '</tr>';
		}
		$lc4++;
	}
	$lc3++;
}

echo '</table><br>';
echo 'Active should be -1 to force the user to change their password<br>';
echo '0 for an inactive account (user can not log in)<br>';
echo '1 for an active account<br>';
echo '<br><button type="submit" name="update">Update</button>';
echo '<button type="submit" name="reset">Clear</button>';
echo '<button type="submit" name="cancel">Cancel</button>';
echo '   </form>';
Display_Footer();
echo '</BODY>';
echo '</HTML>';
unset($_SESSION['add_error']);
?>
