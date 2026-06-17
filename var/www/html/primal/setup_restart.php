<?php
	//License GPLv3
	//Version 1.00.00
	//2026-06-17
session_start();
header( "Expires: Mon, 20 Dec 1998 01:00:00 GMT" );
header( "Last-Modified: " . gmdate("D, d M Y H:i:s") . " GMT" );
header( "Cache-Control: no-cache, must-revalidate" );
header( "Pragma: no-cache" );
header('Content-Encoding: none');
header('Connection: keep-alive');
header('X-Accel-Buffering: no'); 
ob_end_clean();

require_once('config.php');
require_once('functions.php');

if ($_SESSION['active'] != '1')
{
    header("Location: login.php");
    exit();
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

Display_Header2();

set_time_limit(300);
$output = [];
$result_code = null;

echo '<pre>';
echo 'Stopping PRIMAL services...<br><br>';
ob_flush();
flush();
exec("sudo /home/dicom/startup.bash stop ALL", $output, $result_code);
echo implode("\n", $output);
ob_flush();
flush();
unset($output);
unset($result_code);
$output = [];
$result_code = null;
sleep(15);
echo '<br><br>Starting PRIMAL services...<br><br>';
ob_flush();
flush();
exec("sudo /home/dicom/startup.bash start ALL", $output, $result_code);
echo implode("\n", $output);
ob_flush();
flush();
sleep(15);
echo "</pre>";

echo '<H3><a href="setup.php">Return to Setup</a></H3>';

Display_Footer();
echo '</BODY>';
echo '</HTML>';
?>