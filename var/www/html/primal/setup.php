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
Display_Footer();
echo '</BODY>';
echo '</HTML>';

?>
