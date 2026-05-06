<?php
	//License GPLv3
    //Version 1.10.01
    //2025-07-31
    //Written by Will Yonker
    
    session_start();
    header( "Expires: Mon, 20 Dec 1998 01:00:00 GMT" );
    header( "Last-Modified: " . gmdate("D, d M Y H:i:s") . " GMT" );
    header( "Cache-Control: no-cache, must-revalidate" );
    header( "Pragma: no-cache" );
	require_once ('config.php');
	require_once ('functions.php');
	date_default_timezone_set('America/New_York');
    echo '<!DOCTYPE html>';
    echo '<HTML>';
    echo '<HEAD>';
    echo '	<TITLE>PRIMAL Web Interface</TITLE>';
    echo '	<!-- Written by Will Yonker-->';
    echo '    <link rel="stylesheet" href="default.css">';
    echo '</HEAD>';
    echo '<BODY>';
    echo '   <H1>PRIMAL Web Interface</H1>';
    echo '   <H2>Initial Setup</H2><br />';
    echo '   <p>It appears that the PRIMAL Web Interface has not been configured yet.  Please fill out the form below to get started.</p>';
    echo '   <form name="form1" method="post" action="newsetup.php">';
    echo '      <table>';
    echo '         <tr><td>Database Host:</td><td>';
    echo '         <input type="text" name="dbhost"></td></tr>';
    echo '         <tr><td>Database Name:</td><td>';
    echo '         <input type="text" name="dbname"></td></tr>';
    echo '         <tr><td>Database User:</td><td>';
    echo '         <input type="text" name="dbuser"></td></tr>';
    echo '         <tr><td>Database Password:</td><td>';
    echo '         <input type="password" name="dbpass"></td></tr>';
    echo '      </table> <br>';
    echo '      <input type="submit" value="Submit">';
    echo '      &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;';
    echo '      <input type="reset" value="Clear" /><br><br>';
    echo '      <br>';
    echo '   </form>';
    echo '</BODY>';
    echo '</HTML>';
?>
