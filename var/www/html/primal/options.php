<?php
	//License GPLv3
    session_start();
    header( "Expires: Mon, 20 Dec 1998 01:00:00 GMT" );
    header( "Last-Modified: " . gmdate("D, d M Y H:i:s") . " GMT" );
    header( "Cache-Control: no-cache, must-revalidate" );
    header( "Pragma: no-cache" );
    require_once ('config.php');
    require_once ('functions.php');

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
$_SESSION['callerid']=$_SERVER['PHP_SELF'];
if($_SERVER['REQUEST_METHOD'] == 'POST') {
    if(isset($_POST['cancel'])) {
        unset($_POST["add_user_name"]);
        unset($_POST["add_page_size"]);
        header("Location: http://" . $_SERVER['HTTP_HOST'] . "/primal/index.php");
        //http_redirect('http://' . $_SERVER['HTTP_HOST'] . '/setup.php', true, HTTP_REDIRECT_PERM);
    }
    if(isset($_POST['reset']))
    {
        unset($_POST["add_user_name"]);
        unset($_POST["add_page_size"]);
        unset($_POST["add_refresh_dekay"]);
        $query = "SELECT * from user where loginid = '" . $_SESSION['loginid'] . "';";
        $result = $conn->query($query);
        $num_rows = $result->num_rows;
        while($row = mysqli_fetch_assoc($result)) {
            $_POST['add_user_name'] = $row['username'];
            $_POST['add_page_size'] = $row['page_size'];
            $_POST['add_refresh_dekay'] = $row['refresh_dekay'];
        }
    }
    if(!ctype_digit($_POST['add_page_size']) || $_POST['add_page_size'] < 0 || $_POST['add_page_size'] > 1000) {
        $_SESSION['add_error'] .= "Error:  Page_Size must be a number from 0 to 1000<br>";
    }
    if(!ctype_digit($_POST['add_refresh_dekay']) || $_POST['add_refresh_dekay'] < 10 || $_POST['add_refresh_dekay'] > 600) {
        $_SESSION['add_error'] .= "Error:  Refresh delay must be a number from 10 to 600<br>";
    }
    $aValid = array('-', '_', ' ');
    if (!isset($_POST["add_user_name"]) || !ctype_alnum(str_replace($aValid, 'A', $_POST['add_user_name']))) {
        $_SESSION['add_error'] .= "Error:  User Name can not be blank and must contain only alpha/number charcters.   Please try again...<br>";
    } elseif (strlen($_POST["add_user_name"]) > 127) {
        $_SESSION['add_error'] .= "Error:  User name can not be longer than 127 characters.  Please try again...<br>";
    }
    if(!isset($_SESSION['add_error'])) {
        if(isset($_POST["update"])) {
            $query = "update user set page_size = '" . $_POST['add_page_size'];
            $query .= "', username = '" . $_POST['add_user_name'];
            $query .= "', refresh_dekay = '" . $_POST['add_refresh_dekay'];
            $query .= "' where username = '" . $_SESSION['loginid'] . "';";
        }
        $result = $conn->query($query);
        unset($_SESSION['orig_loginid']);
        if(!$result) {
            echo 'Query = ' . $query . '<br>';
            echo mysql_errno() . ": " . mysql_error(). "<br>";
        } else {
            echo 'User ' . $_SESSION['loginid'] . ' has been updated.<br><br>';
            $_SESSION['login_username'] = $_POST["add_user_name"];
            $_SESSION['page_size'] = $_POST['add_page_size'];
            $_SESSION['refresh_dekay'] = $_POST['add_refresh_dekay'];
            unset($_POST['add_user_name']);
            unset($_POST['add_page_size']);
            unset($_POST['add_refresh_dekay']);
        }
        //header("Location: http://" . $_SERVER['HTTP_HOST'] . "/setup.php");
        $query = "SELECT * from user where loginid = '" . $_SESSION['loginid'] . "';";
        $result = $conn->query($query);
        $num_rows = $result->num_rows;
        while($row = mysqli_fetch_assoc($result)) {
            $_POST['add_user_name'] = $row['username'];
            $_POST['add_page_size'] = $row['page_size'];
            $_POST['add_refresh_dekay'] = $row['refresh_dekay'];
        }
    } else {
        echo $_SESSION['add_error'] . '<br>';
		$num_rows = 1;
		unset($_SESSION['add_error']);
	}
} else {
    if (isset($_GET["c"])) {
        if (isset($_GET["h"])) {
            #If c and h are set, then we want to hide (or unhide) a column.
            $query="update user_columns set column" . $_GET["c"] . "_Visible = '" . $_GET["h"] . "' where user_id = '" . $_SESSION['loginid'] . "';";
            $result = $conn->query($query);
        } elseif (isset($_GET["o"])) {
            $query = "select count(*) from page_columns;";
            $result = $conn->query($query);
            while ($row = mysqli_fetch_assoc($result)) {
                $num_columns = $row ['count(*)'];
            }
            $query="select Column" . $_GET["c"] . "_Order from user_columns where user_id = '" . $_SESSION['loginid'] . "';";
            $result = $conn->query($query);
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
                    $query="select Column" . $lc5 . "_Order from user_columns where user_id = '" . $_SESSION['loginid'] . "';";
                    $result = $conn->query($query);
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
                    $query1="update user_columns set column" . $intFoundID . "_Order = '" . $curpos . "' where user_id = '" . $_SESSION['loginid'] . "';";
                    $result1 = $conn->query($query1);
                    $query1="update user_columns set column" . $_GET["c"] . "_Order = '" . $newpos . "' where user_id = '" . $_SESSION['loginid'] . "';";
                    $result1 = $conn->query($query1);
                } else {
                    #Uh oh found more than one guy at that position.
                    #Fist let's deal with the case where we don't find anyone at the address we want to use
                    if($intFound == 0) {
                        $lc5=1;
                        while($lc5 < $num_columns) {
                            #Query the database and put each result in it's position in an array
                            $query="select Column" . $lc5 . "_Order from user_columns where user_id = '" . $_SESSION['loginid'] . "';";
                            $result = $conn->query($query);
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
                            $query="select Column" . $lc5 . "_Order from user_columns where user_id = '" . $_SESSION['loginid'] . "';";
                            $result = $conn->query($query);
                            if(! is_numeric($row['Column' . $lc5 . "_Order"]) || $row['Column' . $lc5 . "_Order"] > $num_columns || $row['Column' . $lc5 . "_Order"] < 1) {
                                $query1="update user_columns set column" . $intFoundID . "_Order = '" . $lc5 . "' where user_id = '" . $_SESSION['loginid'] . "';";
                                $result1 = $conn->query($query1);
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
                            $query="select Column" . $lc5 . "_Order from user_columns where user_id = '" . $_SESSION['loginid'] . "';";
                            $result = $conn->query($query);
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
	$query = "SELECT * from user where loginid = '" . $_SESSION['loginid'] . "';";
	$result = run_query($query);
	$num_rows = mysql_num_rows($result);
	while($row = mysql_fetch_assoc($result)) {
            $_POST['add_user_name'] = $row['username'];
            $_POST['add_page_size'] = $row['page_size'];
            $_POST['add_refresh_dekay'] = $row['refresh_dekay'];
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
	$query="select * from user_columns where user_id = '" . $_SESSION['loginid'] . "';";
	$result = mysql_query($query);
	$num_rows = mysql_num_rows($result);
	#If we didn't get any results, we need to add this user to the user_columns table.
	#This will set default values for all columns.
	if ($num_rows < 1) {
		$query="insert into user_columns (user_id) values ('" . $_SESSION['loginid'] . "');";
		$result2 = mysql_query($query);
	}
	#Now that the user is added, let's start over and query again so that we can get the default values
	$query="select * from user_columns where user_id = '" . $_SESSION['loginid'] . "';";
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
}

echo <<<EOT
<HTML>
<!DOCTYPE html> 
<HEAD>
    <!-- Written by Will Yonker-->
    <TITLE>PRIMAL Web Interface</TITLE>
    <link rel="stylesheet" href="default.css">
</HEAD>
<BODY>
EOT;
echo '<form action="options.php" method="post">';
Display_Header2();
echo '<a href="http://' . $_SERVER['HTTP_HOST'] . '/primal/password.php">Reset Password</a><br><br>';
if($num_rows != 1)
{
	echo "Error: Invalid number of users found.  Please try again.  If you continue to get this message, please contact your administrator";
} else {
	echo '<table border="1">';
	echo '<tr>';
	echo '<td>' . 'User Name' . '</td>';
	echo '<td>' . '<input type="text" name="add_user_name" value ="' . $_POST["add_user_name"] . '" />' . '</td></tr>';
	echo '<td>' . 'Page Size' . '</td>';
	echo '<td>' . '<input type="text" name="add_page_size" value="' . $_POST["add_page_size"] . '" />' . '</td></tr>';
	echo '<td>' . 'Refresh Delay (sec)' . '</td>';
	echo '<td>' . '<input type="text" name="add_refresh_delay" value="' . $_POST["add_refresh_dekay"] . '" />' . '</td></tr>';
	echo '</table><br>';
	echo '<br><button type="submit" name="update">Update</button>';
	echo '<button type="submit" name="reset">Clear</button>';
	echo '<button type="submit" name="cancel">Cancel</button>';
	echo '   </form>';
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
                echo '<td><a href="http://' . $_SERVER['HTTP_HOST'] . '/primal/options.php?e=' . $_session['loginid'] . '&c=' . $ucolumns[$lc4]["dorder"] . '&h=0' . '">Unhide</a></td>';
            } else {
                echo '<td>' . $ucolumns[$lc4]["name"] . '</td>';
                echo '<td><a href="http://' . $_SERVER['HTTP_HOST'] . '/primal/options.php?e=' . $_session['loginid'] . '&c=' . $ucolumns[$lc4]["dorder"] . '&h=1' . '">Hide</a></td>';
            }
            if ($lc3 > 1) {
                echo '<td><a href="http://' . $_SERVER['HTTP_HOST'] . '/primal/options.php?e=' . $_session['loginid'] . '&c=' . $ucolumns[$lc4]["dorder"] . '&o=u">Up</a></td>';
            } else {
                echo '<td></td>';
            }
            if ($lc3 < count($ucolumns)) {
                echo '<td><a href="http://' . $_SERVER['HTTP_HOST'] . '/primal/options.php?e=' . $_session['loginid'] . '&c=' . $ucolumns[$lc4]["dorder"] . '&o=d">Down</a></td>';
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

}
Display_Footer();
echo '</BODY>';
echo '</HTML>';
?>
