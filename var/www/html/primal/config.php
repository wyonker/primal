<?php
//Version 1.01.02
//Build 3
//2026-04-28
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
    try {
        $conn = new mysqli($DBHost, $DBUser, $DBPass, $DBName);
    } catch (Exception $e) {
        error_log('Connection error: ' . $e->getMessage());
        header('Location: error.php?e=1');
        /*
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
        */
        exit;
    }
?>
