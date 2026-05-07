<?php
	//License GPLv3
    //Version 1.10.01
    //2026-05-07
    //Written by Will Yonker
    
    session_start();
    header( "Expires: Mon, 20 Dec 1998 01:00:00 GMT" );
    header( "Last-Modified: " . gmdate("D, d M Y H:i:s") . " GMT" );
    header( "Cache-Control: no-cache, must-revalidate" );
    header( "Pragma: no-cache" );
	//require_once ('config.php');
	require_once ('functions.php');
    if($_SERVER['REQUEST_METHOD'] == 'POST') {
        //Process the form data and write to config.php
        $dbhost = $_POST['dbhost'];
        $dbname = $_POST['dbname'];
        $dbuser = $_POST['dbuser'];
        $dbpass = $_POST['dbpass'];
        $config_data = "<?php\n";
        $config_data .= "//Version 1.01.02\n";
        $config_data .= "//Build 4\n";
        $config_data .= "//2026-05-07\n";
        $config_data .= "//License GPLv3\n";
        $config_data .= "//Written by Will Yonker\n";
        $config_data .= "    \$DBHost = \"$dbhost\";\n";
        $config_data .= "    \$DBUser = \"$dbuser\";\n";
        if(!empty($dbpass) && $dbpass != "" && trim($dbpass) != " ") {
            $config_data .= "    \$DBPass = \"$dbpass\";\n";
        }
        $config_data .= "    \$DBName = \"$dbname\";\n";
        $config_data .= "    \$LDAPHost = \"test\";\n";
        $config_data .= "    \$LDAPDomain = \"test.com\";\n";
        $config_data .= "    \$LDAPBaseDN = \"DC=test,DC=com\";\n";
        $config_data .= "    \$LDAPGroup = \"End Users\";\n";
        $config_data .= "    \$LDAPDN = \"test.com\";\n";
        $config_data .= "    \$LDAPShortName = \"test\";\n";
        $config_data .= "    if(\$DBUser == \"test\") {\n";
        $config_data .= "        //The config file has not be setup.  Need to redirect.\n";
        $config_data .= "        header(\"Location: newsetup.php\");\n";
        $config_data .= "        exit();\n";
        $config_data .= "    } else {\n";
        $config_data .= "        try {\n";
        $config_data .= "            \$conn = new mysqli(\$DBHost, \$DBUser, \$DBPass, \$DBName);\n";
        $config_data .= "        } catch (Exception \$e) {\n";
        $config_data .= "            error_log('Connection error: ' . \$e->getMessage());\n";
        $config_data .= "            header('Location: error.php?e=1');\n";
        $config_data .= "            exit;\n";
        $config_data .= "        }\n";
        $config_data .= "    }\n";
        $config_data .= "?>\n";

        file_put_contents('config.php', $config_data);
        //Redirect to login page
        header("Location: login.php");
        exit();
    }
	date_default_timezone_set('America/New_York');
    echo '<!DOCTYPE html>';
    echo '<HTML>';
    echo '<HEAD>';
    echo '	<TITLE>PRIMAL Web Interface</TITLE>';
    echo '	<!-- Written by Will Yonker-->';
    echo '    <link rel="stylesheet" href="default.css">';
    echo '</HEAD>';
    //Need to see if the current user can even write to the config file before we display the form.  If not, we need to display an error message and instructions for how to fix the permissions.
    if(!is_writable('config.php')) {
        echo '<BODY>';
        echo '   <H1>PRIMAL Web Interface</H1>';
        echo '   <H2>Initial Setup</H2><br />';
        echo '   <p style="color:#FF0000;"><strong>ERROR:</strong>  The web server does not have permissions to write to the config.php file.  Please change the permissions on the config.php file to allow write access and try again.</p>';
        echo '   <BR>';
        echo '   <a href="newsetup.php">Try Again...</a>';
        echo '</BODY>';
        echo '</HTML>';
        exit();
    }
    //Read the config file and save existing values.
    $strConfig = file_get_contents('config.php');
    preg_match('/\$DBHost\s*=\s*"([^"]+)";/', $strConfig, $matches);
    $dbhost = $matches[1];
    preg_match('/\$DBUser\s*=\s*"([^"]+)";/', $strConfig, $matches);
    $dbuser = $matches[1];
    //preg_match('/\$DBPass\s*=\s*"([^"]+)";/', $strConfig, $matches);
    //$dbpass = $matches[1];
    preg_match('/\$DBName\s*=\s*"([^"]+)";/', $strConfig, $matches);
    $dbname = $matches[1];
    preg_match('/\$LDAPHost\s*=\s*"([^"]+)";/', $strConfig, $matches);
    $ldaphost = $matches[1];
    preg_match('/\$LDAPDomain\s*=\s*"([^"]+)";/', $strConfig, $matches);
    $ldapdomain = $matches[1];
    preg_match('/\$LDAPBaseDN\s*=\s*"([^"]+)";/', $strConfig, $matches);
    $ldapbasedn = $matches[1];
    preg_match('/\$LDAPGroup\s*=\s*"([^"]+)";/', $strConfig, $matches);
    $ldapgroup = $matches[1];
    preg_match('/\$LDAPDN\s*=\s*"([^"]+)";/', $strConfig, $matches);
    $ldapdn = $matches[1];
    preg_match('/\$LDAPShortName\s*=\s*"([^"]+)";/', $strConfig, $matches);
    $ldapshortname = $matches[1];
    echo '<BODY>';
    echo '   <H1>PRIMAL Web Interface</H1>';
    echo '   <H2>Initial Setup</H2><br />';
    echo '   <p>It appears that the PRIMAL Web Interface has not been configured yet (you cannot use test as the DB user name).  Please fill out the form below to get started.</p>';
    echo '   <p><strong>NOTE:</strong>  The database user you specify must have permissions to create tables and write data to the database.  The database itself must already exist.</p>';
    echo '   <p><strong>WARNING:</strong>  If you specify a DB password, it will overwrite whatever is currently set.  For security reasons, the original value is not shown.</p>';
    echo '   <form name="form1" method="post" action="newsetup.php">';
    echo '      <table>';
    echo '         <tr><td>Database Host:</td><td>';
    echo '         <input type="text" name="dbhost" value="' . htmlspecialchars($dbhost) . '"></td></tr>';
    echo '         <tr><td>Database Name:</td><td>';
    echo '         <input type="text" name="dbname" value="' . htmlspecialchars($dbname) . '"></td></tr>';
    echo '         <tr><td>Database User:</td><td>';
    echo '         <input type="text" name="dbuser" value="' . htmlspecialchars($dbuser) . '"></td></tr>';
    echo '         <tr><td>Database Password:</td><td>';
    echo '         <input type="password" name="dbpass"></td></tr>';
    echo '         <BR>';
    echo '         <tr><td>LDAP Host:</td><td>';
    echo '         <input type="text" name="ldaphost" value="' . htmlspecialchars($ldaphost) . '"></td></tr>';
    echo '         <tr><td>LDAP Domain:</td><td>';
    echo '         <input type="text" name="ldapdomain" value="' . htmlspecialchars($ldapdomain) . '"></td></tr>';
    echo '         <tr><td>LDAP Base DN:</td><td>';
    echo '         <input type="text" name="ldapbasedn" value="' . htmlspecialchars($ldapbasedn) . '"></td></tr>';
    echo '         <tr><td>LDAP Group:</td><td>';
    echo '         <input type="text" name="ldapgroup" value="' . htmlspecialchars($ldapgroup) . '"></td></tr>';
    echo '         <tr><td>LDAP DN:</td><td>';
    echo '         <input type="text" name="ldapdn" value="' . htmlspecialchars($ldapdn) . '"></td></tr>';
    echo '         <tr><td>LDAP Short Name:</td><td>';
    echo '         <input type="text" name="ldapshortname" value="' . htmlspecialchars($ldapshortname) . '"></td></tr>';
    echo '      </table> <br>';
    echo '      <input type="submit" value="Submit">';
    echo '      &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;';
    echo '      <input type="reset" value="Clear" /><br><br>';
    echo '      <br>';
    echo '   </form>';
    echo '</BODY>';
    echo '</HTML>';
?>
