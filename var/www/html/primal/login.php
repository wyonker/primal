<?php
	//License GPLv3
    //Version 1.10.00
    //2024-09-19
    //Written by Will Yonker
    session_start();
    header( "Expires: Mon, 20 Dec 1998 01:00:00 GMT" );
    header( "Last-Modified: " . gmdate("D, d M Y H:i:s") . " GMT" );
    header( "Cache-Control: no-cache, must-revalidate" );
    header( "Pragma: no-cache" );
	require_once ('config.php');
	require_once ('functions.php');
	date_default_timezone_set('America/New_York');
echo <<<EOT
<HTML>
<!DOCTYPE html>
<HEAD>
	<TITLE>PRIMAL Web Interface</TITLE>
	<!-- Written by Will Yonker-->
    <link rel="stylesheet" href="default.css">
</HEAD>
<BODY>
   <H1>PRIMAL Web Interface</H1>
   <br><br><br>
   <h2>Login Screen</h2><br />
   <form name="form1" method="post" action="login2.php">
      <table>
         <tr><td>User Name:</td><td>
         <input type="text" name="username"></td></tr>
         <tr><td>Password:</td><td>
         <input type="password" name="password"></td></tr>
         <tr><td>AD Auth?</td>
         <td><input type="checkbox" id="cbLDAP" name="cbLDAP"></td></tr>
      </table> <br>
      <input type="submit" value="Submit">
      &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
      <input type="reset" value="Clear" /><br><br>
      <br>
EOT;
if(isset($_SESSION['retry'])) {
    if ($_SESSION['retry'] == 1) {
        $err_msg = "Error:  Invalid username or password entered!";
    } elseif ($_SESSION['retry'] == 2) {
        $err_msg = "Error:  Invalid username or password entered!";
    } elseif ($_SESSION['retry'] == 3) {
        $err_msg = "Error:  Database inconsistancy!  Please contact your administrator.";
    } elseif ($_SESSION['retry'] == 4) {
        $err_msg = "Error:  Invalid username or password entered!";
    }
    echo '<div style="color:#FF0000;"><blink>' . $err_msg . "</blink></div><br>";
}

echo '   </form>';
Display_Footer();
echo '</BODY>';
echo '</HTML>';
?>
