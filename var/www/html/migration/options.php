<?php
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
mysql_select_db("primal");
$_SESSION['callerid']=$_SERVER['PHP_SELF'];
if($_SERVER['REQUEST_METHOD'] == 'POST')
{
    if(isset($_POST['cancel']))
    {
		unset($_POST["add_user_name"]);
		unset($_POST["add_page_size"]);
        header("Location: http://" . $_SERVER['HTTP_HOST'] . "/migration/index.php");
        //http_redirect('http://' . $_SERVER['HTTP_HOST'] . '/setup.php', true, HTTP_REDIRECT_PERM);
    }
    if(isset($_POST['reset']))
    {
        unset($_POST["add_user_name"]);
        unset($_POST["add_page_size"]);
		$query = "SELECT * from user where loginid = '" . $_SESSION['loginid'] . "';";
		$result = run_query($query);
		$num_rows = mysql_num_rows($result);
		while($row = mysql_fetch_assoc($result))
		{
			$_POST['add_user_name'] = $row['username'];
			$_POST['add_page_size'] = $row['page_size'];
		}
    }
	if(!ctype_digit($_POST['add_page_size']) || $_POST['add_page_size'] < 0 || $_POST['add_page_size'] > 1000) {
        $_SESSION['add_error'] .= "Error:  Page_Size must be a number from 0 to 1000<br>";
    }
	$aValid = array('-', '_', ' ');
	if (!isset($_POST["add_user_name"]) || !ctype_alnum(str_replace($aValid, 'A', $_POST['add_user_name'])))
    {
        $_SESSION['add_error'] .= "Error:  User Name can not be blank and must contain only alpha/number charcters.   Please try again...<br>";
    } elseif (strlen($_POST["add_user_name"]) > 127) {
        $_SESSION['add_error'] .= "Error:  User name can not be longer than 127 characters.  Please try again...<br>";
    }
    if(!isset($_SESSION['add_error']))
    {
        if(isset($_POST["update"]))
        {
            $query = "update user set username = '" . $_POST["add_user_name"] . "', page_size = '";
            $query .= $_POST['add_page_size'] . "'";
            $query .= " where loginid = '" . $_SESSION['loginid'] . "';";
        }
        $result = mysql_query($query);
        unset($_SESSION['orig_loginid']);
        if(!$result)
        {
            echo 'Query = ' . $query . '<br>';
            echo mysql_errno() . ": " . mysql_error(). "<br>";
        } else {
			echo 'User ' . $_SESSION['loginid'] . ' has been updated.<br><br>';
			$_SESSION['login_username'] = $_POST["add_user_name"];
			$_SESSION['page_size'] = $_POST['add_page_size'];
			unset($_POST["add_user_name"]);
			unset($_POST['add_page_size']);
		}
        //header("Location: http://" . $_SERVER['HTTP_HOST'] . "/setup.php");
		$query = "SELECT * from user where loginid = '" . $_SESSION['loginid'] . "';";
		$result = run_query($query);
		$num_rows = mysql_num_rows($result);
		while($row = mysql_fetch_assoc($result))
		{
			$_POST['add_user_name'] = $row['username'];
			$_POST['add_page_size'] = $row['page_size'];
		}
    } else {
        echo $_SESSION['add_error'] . '<br>';
		$num_rows = 1;
		unset($_SESSION['add_error']);
	}
} else {
	$query = "SELECT * from user where loginid = '" . $_SESSION['loginid'] . "';";
	$result = run_query($query);
	$num_rows = mysql_num_rows($result);
	while($row = mysql_fetch_assoc($result))
    {
		$_POST['add_user_name'] = $row['username'];
		$_POST['add_page_size'] = $row['page_size'];
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
echo '<form action="options.php" method="post">';
Display_Header3();
echo '<a href="http://' . $_SERVER['HTTP_HOST'] . '/migration/password.php">Reset Password</a><br><br>';
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
	echo '</table><br>';
	echo '<br><button type="submit" name="update">Update</button>';
	echo '<button type="submit" name="reset">Clear</button>';
	echo '<button type="submit" name="cancel">Cancel</button>';
	echo '   </form>';
}
Display_Footer();
mysql_select_db($DBName);
echo '</BODY>';
echo '</HTML>';
?>
