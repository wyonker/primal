<?php
    //Version 1.00.0
    //Build 2
    //2014-12-13
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
unset($_SESSION['result']);

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
echo '<a href="http://' . $_SERVER['HTTP_HOST'] . '/migration/setup.php?e=1">Edit</a>';
echo '<br><br><br>';
echo '<a href="http://' . $_SERVER['HTTP_HOST'] . '/migration/add_user.php">Add User</a></br>';
echo '<br><br>';
if ($_GET['e'] == 1)
{
	$myfile = fopen("/etc/primal/primal.conf", "r") or die("Unable to open primal.conf file!");
	echo fread($myfile,filesize("/etc/primal/primal.conf"));
	fclose($myfile);
} else {
	$query = "SELECT * from user";
	$result = run_query($query);
	$num_rows = mysql_num_rows($result);
	$lc1=0;
	while($row = mysql_fetch_assoc($result))
	{
		$users[$lc1]['loginid'] = $row['loginid'];
		$users[$lc1]['login_sec_level'] = $row['login_sec_level'];
		$users[$lc1]['username'] = $row['username'];
		$users[$lc1]['active'] = $row['active'];
		$users[$lc1]['access'] = $row['access'];
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
	echo '<th>Page Size</th>';
	echo '</th>';
	$lc1=0;
	foreach($users as $u)
	{
		if($users[$lc1]['login_sec_level'] <= $_SESSION['login_sec_level'])
		{
			echo '<tr>';
			echo '<td><a href="http://' . $_SERVER['HTTP_HOST'] . '/migration/update_user.php?e=' . $users[$lc1]['loginid'] . '">' . $users[$lc1]['loginid'] . '</a></td>';
			echo '<td>' . $users[$lc1]['login_sec_level'] . '</td>';
			echo '<td>' . $users[$lc1]['username'] . '</td>';
			echo '<td>' . $users[$lc1]['active'] . '</td>';
			echo '<td>' . $users[$lc1]['access'] . '</td>';
			echo '<td>' . $users[$lc1]['page_size'] . '</td>';
			echo '</tr>';
		}
		$lc1++;
	}
	echo '</table>';
}
echo '   </form><br>';
Display_Footer();
echo '</BODY>';
echo '</HTML>';

?>
