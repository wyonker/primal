<?php
	//License GPLv3
	//Version 1.00.01
	//2026-08-10
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
            exec("prim_server -tc " . $strTestConditionValue, $strTestConditionResult, $status);
        }
        if($strTestActionValue != "" && $strTestActionValue != " ") {
            exec("prim_server -ta " . $strTestActionValue, $strTestActionResult, $status);
        }

    }
}

Display_Header2();
echo "<H2>System Setup</H2>";
echo '<H3><a href="setup_user.php">Setup User</a></H3>';
echo '<H3><a href="setup_db.php">Setup Database</a></H3>';
echo '<H3><a href="setup_restart.php">Restart PRIMAL</a></H3>';
if($_GET['action'] != 'defaults') {
	echo '<H3><a href="setup.php?action=defaults">Modify Default Settings</a></H3>';
} else {
	echo '<H3><a href="setup.php">Normal</a></H3>';
}
if($_GET['action'] == 'Rule') {
    $intRule = $_GET['rec'];
    $strQuery = "SELECT * FROM conf_proc WHERE conf_proc_id = '$intRule'";
    $result = mysqli_query($conn, $strQuery);
    $row = mysqli_fetch_assoc($result);
    $strTestConditionRule = $row["proc_cond"];
    $strTestActionCondition = $row["proc_action_value"];

    echo '<form method="post" action="setup_test.php?action=Rule2&rec=' . $intRule . '">';
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
    echo '<tr><td colspan="2"><input type="submit" name="btnTest" value="Test" /></td></tr>';
    echo '</table>';
    echo '</form>';
}elseif($_GET['action'] == 'Rule2') {
    $intRule = $_GET['rec'];
    $strQuery = "SELECT * FROM conf_proc WHERE conf_proc_id = '$intRule'";
    $result = mysqli_query($conn, $strQuery);
    $row = mysqli_fetch_assoc($result);
    $strTestConditionRule = $row["proc_cond"];
    $strTestActionCondition = $row["proc_action_value"];

    $strQuery = "SELECT conf_value FROM config WHERE conf_name = 'test_rule_condition_result'";
    $result = mysqli_query($conn, $strQuery);
    $row = mysqli_fetch_assoc($result);
    $strTestConditionResult = $row["conf_value"];
    $strQuery = "SELECT conf_value FROM config WHERE conf_name = 'test_rule_action_result'";
    $result = mysqli_query($conn, $strQuery);
    $row = mysqli_fetch_assoc($result);
    $strTestActionResult = $row["conf_value"];
    
    echo '<form method="post" action="setup_test.php?action=Rule2&rec=' . $intRule . '">';
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
    echo '<tr><td colspan="2"><input type="submit" name="btnTest" value="Test" /></td></tr>';
    echo '</table>';
    echo '</form>';
}
?>

