<?php
	//License GPLv3
	//Version 1.00.00
	//2026-08-05
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
if($_SERVER['REQUEST_METHOD'] == 'POST') {
    if(isset($_POST['btnTest'])) {
        $strTestConditionValue = $_POST['condition_value'];
        $strTestActionValue = $_POST['action_value'];
        if($strTestConditionValue != "" && $strTestConditionValue != " ") {
            $strTestConditionResult = exec("prim_server -tc " . $strTestConditionValue, $outcome, $status);
        }
        if($strTestActionValue != "" && $strTestActionValue != " ") {
            $strTestActionResult = exec("prim_server -ta " . $strTestActionValue, $outcome, $status);
        }

    }
}

if($_GET['action'] == 'Rule') {
    $intRule = $_GET['rec'];
    $sql = "SELECT * FROM conf_proc WHERE conf_proc_id = '$intRule'";
    $result = mysqli_query($conn, $sql);
    $row = mysqli_fetch_assoc($result);
    $strTestConditionRule = $row["proc_cond"];
    $strTestActionCondition = $row["proc_action_value"];

    echo '<form method="post" action="setup_test.php?action=Rule&rec=' . $intRule . '">';
    echo '<table>';
    echo '<tr><td>Condition Value to be tested:</td>';
    echo '<td><input type="text" name="condition_value" value ="' . $strTestConditionValue . '" /></td></tr>';
    echo '<tr><td>' . 'Rule Condition:</td>';
    echo '<td><input type="text" name="condition_rule" value ="' . $strTestConditionRule . '" /></td></tr>';
    echo '<tr><td>Result:</td>';
    echo '<td><input type="text" name="condition_result" value ="' . $strTestConditionResult . '" /></td></tr>';
    echo '<br>';
    echo '<tr><td>Action Value to be tested:</td>';
    echo '<td><input type="text" name="action_value" value ="' . $strTestActionValue . '" /></td></tr>';
    echo '<tr><td>' . 'Rule Condition:</td>';
    echo '<td><input type="text" name="action_condition" value ="' . $strTestActionCondition . '" /></td></tr>';
    echo '<tr><td>Result:</td>';
    echo '<td><input type="text" name="action_result" value ="' . $strTestActionResult . '" /></td></tr>';
    echo '<br>';
    echo '<tr><td><input type="submit" name="btnTest" value="Test" /></td></tr>';
    echo '</table>';
    echo '</form>';
}
?>

