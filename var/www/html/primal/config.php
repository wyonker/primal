<?php
//Version 1.01.00
//Build 2
//2024-09-19
//License GPLv3
//Written by Will Yonker
    $DBHost = "localhost";
    $DBUser = "test";
    $DBPass = "test";
    $DBName = "test";
    $LDAPHost = "test";
    $LDAPDomain = "test.com";
    $LDAPBaseDN = "DC=test,DC=com";
    $LDAPGroup = "End Users";
    $LDAPDN = "test.com";
    $LDAPShortName = "test";
    //$conn = mysql_connect($DBHost, $DBUser, $DBPass) or die
    //                     ('Error connecting to mysql');
    //mysql_select_db($DBName);
    $conn = new mysqli($DBHost, $DBUser, $DBPass, $DBName);
    if ($conn->connect_errno) {
        error_log('Connection error: ' . $conn->connect_errno);
        echo '<!DOCTYPE html>';
        echo '<html lang="en">';
        echo '<head>';
        echo '<meta charset="UTF-8">';
        echo '</head>';
        echo '<body>';
        echo "<h1>PRIMAL Web Interface</h1>";
        echo "<BR>";
        echo '<h2>ERROR:  Cannot connect to database.  Exiting...</h2>';
        echo '</body>';
        echo '</html>';
        exit;
    }
?>
