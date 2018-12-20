<?php
    $DBHost = "localhost";
    $DBUser = "primal";
    $DBPass = "primal";
    $DBName = "primal";
    $conn = mysql_connect($DBHost, $DBUser, $DBPass) or die
                         ('Error connecting to mysql');
    mysql_select_db($DBName);
?>
