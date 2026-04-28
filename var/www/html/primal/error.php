<?php
	//License GPLv3
    //Version 1.00.00
    //2026-04-28
    //Written by Will Yonker
    
    session_start();
    header( "Expires: Mon, 20 Dec 1998 01:00:00 GMT" );
    header( "Last-Modified: " . gmdate("D, d M Y H:i:s") . " GMT" );
    header( "Cache-Control: no-cache, must-revalidate" );
    header( "Pragma: no-cache" );
	date_default_timezone_set('America/New_York');
if($_SERVER['REQUEST_METHOD'] === 'POST') {
    $strConfig = file_get_contents('config.php');
    if((isset($_POST['DBHost'])) && (trim($_POST['DBHost']) != "") && (trim($_POST['DBHost']) != " ")) {
        $DBHost = $_POST['DBHost'];
    } else {
        $strError = "DBHost is required.";
    }
    if((isset($_POST['DBUser'])) && (trim($_POST['DBUser']) != "") && (trim($_POST['DBUser']) != " ")) {
        $DBUser = $_POST['DBUser'];
    } else {
        $strError = "DBUser is required.";
    }
    if((isset($_POST['DBPass'])) && (trim($_POST['DBPass']) != "") && (trim($_POST['DBPass']) != " ")) {
        $DBPass = $_POST['DBPass'];
    } else {
        $strError = "DBPass is required.";
    }
    if((isset($_POST['DBName'])) && (trim($_POST['DBName']) != "") && (trim($_POST['DBName']) != " ")) {
        $DBName = $_POST['DBName'];
    } else {
        $strError = "DBName is required.";
    }
    if((isset($_POST['LDAPHost'])) && (trim($_POST['LDAPHost']) != "") && (trim($_POST['LDAPHost']) != " ")) {
        $LDAPHost = $_POST['LDAPHost'];
    } else {
        $LDAPHost = " ";
    }
    if((isset($_POST['LDAPDomain'])) && (trim($_POST['LDAPDomain']) != "") && (trim($_POST['LDAPDomain']) != " ")) {
        $LDAPDomain = $_POST['LDAPDomain'];
    } else {
        $LDAPDomain = " ";
    }
    if((isset($_POST['LDAPBaseDN'])) && (trim($_POST['LDAPBaseDN']) != "") && (trim($_POST['LDAPBaseDN']) != " ")) {
        $LDAPBaseDN = $_POST['LDAPBaseDN'];
    } else {
        $LDAPBaseDN = " ";
    }
    if((isset($_POST['LDAPGroup'])) && (trim($_POST['LDAPGroup']) != "") && (trim($_POST['LDAPGroup']) != " ")) {
        $LDAPGroup = $_POST['LDAPGroup'];
    } else {
        $LDAPGroup = " ";
    }
    if((isset($_POST['LDAPDN'])) && (trim($_POST['LDAPDN']) != "") && (trim($_POST['LDAPDN']) != " ")) {
        $LDAPDN = $_POST['LDAPDN'];
    } else {
        $LDAPDN = " ";
    }

    //Update config.php with new values
    $strConfig = preg_replace("/\\\$DBHost = \".*\";/", "\$DBHost = \"$DBHost\";", $strConfig);
    $strConfig = preg_replace("/\\\$DBUser = \".*\";/", "\$DBUser = \"$DBUser\";", $strConfig);
    $strConfig = preg_replace("/\\\$DBPass = \".*\";/", "\$DBPass = \"$DBPass\";", $strConfig);
    $strConfig = preg_replace("/\\\$DBName = \".*\";/", "\$DBName = \"$DBName\";", $strConfig);
    $strConfig = preg_replace("/\\\$LDAPHost = \".*\";/", "\$LDAPHost = \"$LDAPHost\";", $strConfig);
    $strConfig = preg_replace("/\\\$LDAPDomain = \".*\";/", "\$LDAPDomain = \"$LDAPDomain\";", $strConfig);
    $strConfig = preg_replace("/\\\$LDAPBaseDN = \".*\";/", "\$LDAPBaseDN = \"$LDAPBaseDN\";", $strConfig);
    $strConfig = preg_replace("/\\\$LDAPGroup = \".*\";/", "\$LDAPGroup = \"$LDAPGroup\";", $strConfig);
    $strConfig = preg_replace("/\\\$LDAPDN = \".*\";/", "\$LDAPDN = \"$LDAPDN\";", $strConfig);
    file_put_contents('config.php', $strConfig);

    //Redirect back to login page after updating config
    header('Location: login.php');
    exit;
}
echo '<HTML>';
echo '<!DOCTYPE html>';
echo '<HEAD>';
echo '	<TITLE>PRIMAL Web Interface</TITLE>';
echo '	<!-- Written by Will Yonker-->';
echo '    <link rel="stylesheet" href="default.css">';
echo '</HEAD>';
if(isset ($_GET['a'])) {
    $intAction = $_GET['a'];
    if($intAction == 1) {
        //User wants to rewrite config.php
        echo '<BODY>';
        echo '<H1>PRIMAL Web Interface</H1>';
        echo '<BR>';
        $strConfig = file_get_contents('config.php');
        $DBHost = get_config_value($strConfig, 'DBHost');
        $DBUser = get_config_value($strConfig, 'DBUser');
        $DBPass = get_config_value($strConfig, 'DBPass');
        $DBName = get_config_value($strConfig, 'DBName');
        $LDAPHost = get_config_value($strConfig, 'LDAPHost');
        $LDAPDomain = get_config_value($strConfig, 'LDAPDomain');
        $LDAPBaseDN = get_config_value($strConfig, 'LDAPBaseDN');
        $LDAPGroup = get_config_value($strConfig, 'LDAPGroup');
        $LDAPDN = get_config_value($strConfig, 'LDAPDN');
        echo '<BODY>';
        echo '<H1>PRIMAL Web Interface</H1>';
        echo '<BR>';
        echo '<form name="form1" method="post" action="error.php">';
        echo '<table>';
        echo '<tr><td>Database Host:</td><td><input type="text" name="DBHost" value="' . htmlspecialchars($DBHost) . '"></td></tr>';
        echo '<tr><td>Database User:</td><td><input type="text" name="DBUser" value="' . htmlspecialchars($DBUser) . '"></td></tr>';
        echo '<tr><td>Database Pass (no special characters):</td><td><input type="password" name="DBPass" value="' . htmlspecialchars($DBPass) . '"></td></tr>';
        echo '<tr><td>Database Name:</td><td><input type="text" name="DBName" value="' . htmlspecialchars($DBName) . '"></td></tr>';
        echo '<tr><td>LDAP Host:</td><td><input type="text" name="LDAPHost" value="' . htmlspecialchars($LDAPHost) . '"></td></tr>';
        echo '<tr><td>LDAP Domain:</td><td><input type="text" name="LDAPDomain" value="' . htmlspecialchars($LDAPDomain) . '"></td></tr>';
        echo '<tr><td>LDAP Base DN:</td><td><input type="text" name="LDAPBaseDN" value="' . htmlspecialchars($LDAPBaseDN) . '"></td></tr>';
        echo '<tr><td>LDAP Group:</td><td><input type="text" name="LDAPGroup" value="' . htmlspecialchars($LDAPGroup) . '"></td></tr>';
        echo '<tr><td>LDAP DN:</td><td><input type="text" name="LDAPDN" value="' . htmlspecialchars($LDAPDN) . '"></td></tr>';
        echo '</table> <br>';
        echo '<input type="submit" value="Submit">';
        echo '&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;';
        echo '<input type="reset" value="Clear" /><br><br>';
        echo '<br>';
        echo '</form>';

        echo '</BODY>';
        echo '</HTML>';
    } elseif($intAction == 2) {
        //User wants to recreate database
        $strConfig = file_get_contents('config.php');
        echo '<BODY>';
        echo '<H1>PRIMAL Web Interface</H1>';
        echo '<BR>';
        echo '<h2>In order to recreate the database, you will need to run the following command on the command line of the server hosting PRIMAL:</h2>';
        echo '<h2>Note:  Please be sure to update the configuration values in config.php before running the commands below.</h2>';
        echo '<h3>mysql -u [username] -p [database name] < /home/dicom/install/install.sql</h3>';
        echo '<h3>grant all privileges on primal.* to \'' . htmlspecialchars($DBUser) . '\'@\'localhost\' identified by \'' . htmlspecialchars($DBPass) . '\' with grant option;</h3>';
        echo '</BODY>';
        echo '</HTML>';
    }
} elseif(isset($_GET['e'])) {
    $intError = $_GET['e'];
    if($intError == 1) {
        $strConfig = file_get_contents('config.php');
        $DBHost = get_config_value($strConfig, 'DBHost');
        $DBUser = get_config_value($strConfig, 'DBUser');
        $DBPass = get_config_value($strConfig, 'DBPass');
        $DBName = get_config_value($strConfig, 'DBName');
        $LDAPHost = get_config_value($strConfig, 'LDAPHost');
        $LDAPDomain = get_config_value($strConfig, 'LDAPDomain');
        $LDAPBaseDN = get_config_value($strConfig, 'LDAPBaseDN');
        $LDAPGroup = get_config_value($strConfig, 'LDAPGroup');
        $LDAPDN = get_config_value($strConfig, 'LDAPDN');
        echo '<BODY>';
        echo '<H1>PRIMAL Web Interface</H1>';
        echo '<BR>';
        echo '<form name="form1" method="post" action="error.php">';
        echo '<table>';
        echo '<tr><td>Database Host:</td><td><input type="text" name="DBHost" value="' . htmlspecialchars($DBHost) . '"></td></tr>';
        echo '<tr><td>Database User:</td><td><input type="text" name="DBUser" value="' . htmlspecialchars($DBUser) . '"></td></tr>';
        echo '<tr><td>Database Pass:</td><td><input type="password" name="DBPass" value="' . htmlspecialchars($DBPass) . '"></td></tr>';
        echo '<tr><td>Database Name:</td><td><input type="text" name="DBName" value="' . htmlspecialchars($DBName) . '"></td></tr>';
        echo '<tr><td>LDAP Host:</td><td><input type="text" name="LDAPHost" value="' . htmlspecialchars($LDAPHost) . '"></td></tr>';
        echo '<tr><td>LDAP Domain:</td><td><input type="text" name="LDAPDomain" value="' . htmlspecialchars($LDAPDomain) . '"></td></tr>';
        echo '<tr><td>LDAP Base DN:</td><td><input type="text" name="LDAPBaseDN" value="' . htmlspecialchars($LDAPBaseDN) . '"></td></tr>';
        echo '<tr><td>LDAP Group:</td><td><input type="text" name="LDAPGroup" value="' . htmlspecialchars($LDAPGroup) . '"></td></tr>';
        echo '<tr><td>LDAP DN:</td><td><input type="text" name="LDAPDN" value="' . htmlspecialchars($LDAPDN) . '"></td></tr>';
        echo '</table> <br>';
        echo '<input type="submit" value="Submit">';
        echo '&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;';
        echo '<input type="reset" value="Clear" /><br><br>';
        echo '<br>';
        echo '</form>';

        echo '</BODY>';
        echo '</HTML>';
    } elseif($interror == 2) {
        //DB exists but is the wrong version
        echo '<BODY>';
        echo '<H1>PRIMAL Web Interface</H1>';
        echo '<BR>';
        echo '<h2>The database exists but is the wrong version.</h2>';
        echo '<h2>We will need to perform a migration but the migration process is not yet complete.  Alternatively, you can recreate the database using existing configuration information by clicking this link.</h2>';
        echo '<h3><a href="error.php?a=2">Recreate Database</a></h3>';
        echo '</BODY>';
        echo '</HTML>';
    }
}

function get_config_value($strConfig, $strKey) {
    preg_match("/\\\$$strKey = \"(.*)\";/", $strConfig, $matches);
    if(count($matches) > 1) {
        return $matches[1];
    } else {
        return "";
    }
}

?>