<?php
//Version 1.01.02
//Build 4
//2026-05-07
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
    if($DBUser == "test") {
        //The config file has not be setup.  Need to redirect.
        header("Location: newsetup.php");
        exit();
    } else {
        try {
            $conn = new mysqli($DBHost, $DBUser, $DBPass, $DBName);
        } catch (Exception $e) {
            error_log('Connection error: ' . $e->getMessage());
            header('Location: error.php?e=1');
            exit;
        }
    }
?>
