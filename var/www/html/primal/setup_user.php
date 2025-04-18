<?php
    //License GPLv3
    session_start();
    header( "Expires: Mon, 20 Dec 1998 01:00:00 GMT" );
    header( "Last-Modified: " . gmdate("D, d M Y H:i:s") . " GMT" );
    header( "Cache-Control: no-cache, must-revalidate" );
    header( "Pragma: no-cache" );
    require_once('config.php');
    require_once('functions.php');
    $intRowsPerPage = 30;

if ($_SESSION['active'] != '1')
{
    header("Location: login.php");
    exit();
}
if (isset($_SESSION['obj']) || isset($_SESSION['obj1']))
{
    unset($_SESSION['obj']);
    unset($_SESSION['obj1']);
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
echo "<H2>User Management</H2>";
Display_Links();
echo '<br><br>';
$strQuery = "SELECT * from users ";
if($_SERVER['REQUEST_METHOD'] == 'POST') {
    if(isset($_POST['btnReset'])) {
        if(isset($_SESSION["rseclevel"])) {
            unset($_SESSION["rseclevel"]);
        }
        if(isset($_SESSION["ruserid"])) {
            unset($_SESSION["ruserid"]);
        }
        if(isset($_SESSION["rfullname"])) {
            unset($_SESSION["rfullname"]);
        }
        header("Location: setup.php");
        exit();
    }
    if(isset($_POST['btnQuery'])) {
        $intLC = 0;
        if((!isset($_POST['rseclevel'])) && (isset($_SESSION['rseclevel']))) {
            unset($_SESSION['rseclevel']);
        } elseif ((trim($_POST['rseclevel']) == "") && (isset($_SESSION['rseclevel']))) {
            unset($_SESSION['rseclevel']);
        }
        if((isset($_POST['rseclevel'])) && (trim($_POST['rseclevel']) != "")) {
            $_SESSION["rseclevel"] = str_replace('*', '%', $_POST['rseclevel']);
            $strWord = "WHERE";
            $intPOS = strpos($_SESSION["rseclevel"], '%');
            if($intPOS === FALSE) {
                $strQuery .= $strWord . ' login_sec_level = "' . $_SESSION["rseclevel"] . '" ';
            } else {
                $strQuery .= $strWord . ' login_sec_level like "' . $_SESSION["rseclevel"] . '" ';
            }
        }
        if((!isset($_POST['ruserid'])) && (isset($_SESSION['ruserid']))) {
            unset($_SESSION['ruserid']);
        } elseif ((trim($_POST['ruserid']) == "") && (isset($_SESSION['ruserid']))) {
            unset($_SESSION['ruserid']);
        }
        if((isset($_POST['ruserid'])) && (trim($_POST['ruserid']) != "")) {
            $_SESSION["ruserid"] = str_replace('*', '%', $_POST['ruserid']);
            $intPOS = strpos($strQuery, 'WHERE');
            if($intPOS === FALSE) {
                $strWord = "WHERE";
            } else {
                $strWord = "AND";
            }
            $intPOS = strpos($_SESSION["ruserid"], '%');
            if($intPOS === FALSE) {
                $strQuery .= $strWord . ' userid = "' . $_SESSION["ruserid"] . '" ';
            } else {
                $strQuery .= $strWord . ' userid like "' . $_SESSION["ruserid"] . '" ';
            }
        }
        if((!isset($_POST['rfullname'])) && (isset($_SESSION['rfullname']))) {
            unset($_SESSION['rfullname']);
        } elseif ((trim($_POST['rfullname']) == "") && (isset($_SESSION['rfullname']))) {
            unset($_SESSION['rfullname']);
        }
        if((isset($_POST['rfullname'])) && (trim($_POST['rfullname']) != "")) {
            $_SESSION["rfullname"] = str_replace('*', '%', $_POST['rfullname']);
            $intPOS = strpos($strQuery, 'WHERE');
            if($intPOS === FALSE) {
                $strWord = "WHERE";
            } else {
                $strWord = "AND";
            }
            $intPOS = strpos($_SESSION["rfullname"], '%');
            if($intPOS === FALSE) {
                $strQuery .= $strWord . ' username = "' . $_SESSION["rfullname"] . '" ';
            } else {
                $strQuery .= $strWord . ' username like "' . $_SESSION["rfullname"] . '" ';
            }
        }
    }
} else {
    if(isset($_SESSION['rseclevel'])) {
        $strWord = "WHERE";
        $intPOS = strpos($_SESSION["rseclevel"], '%');
        if($intPOS === FALSE) {
            $strQuery .= $strWord . ' login_sec_level = "' . $_SESSION["rseclevel"] . '" ';
        } else {
            $strQuery .= $strWord . ' login_sec_level like "' . $_SESSION["rseclevel"] . '" ';
        }
    }
    if((isset($_SESSION['ruserid'])) && (trim($_SESSION['ruserid']) != "")) {
        $intPOS = strpos($strQuery, 'WHERE');
        if($intPOS === FALSE) {
            $strWord = "WHERE";
        } else {
            $strWord = "AND";
        }
        $intPOS = strpos($_SESSION["ruserid"], '%');
        if($intPOS === FALSE) {
            $strQuery .= $strWord . ' userid = "' . $_SESSION["ruserid"] . '" ';
        } else {
            $strQuery .= $strWord . ' userid like "' . $_SESSION["ruserid"] . '" ';
        }
    }
    if((isset($_SESSION['rfullname'])) && (trim($_SESSION['rfullname']) != "")) {
        $intPOS = strpos($strQuery, 'WHERE');
        if($intPOS === FALSE) {
            $strWord = "WHERE";
        } else {
            $strWord = "AND";
        }
        $intPOS = strpos($_SESSION["rfullname"], '%');
        if($intPOS === FALSE) {
            $strQuery .= $strWord . ' username = "' . $_SESSION["rfullname"] . '" ';
        } else {
            $strQuery .= $strWord . ' username like "' . $_SESSION["rfullname"] . '" ';
        }
    }
}

$strQuery2 = str_replace("SELECT * ", "SELECT COUNT(*) AS count ", $strQuery);
$result = mysqli_query($conn, $strQuery2);
$row = mysqli_fetch_assoc($result);
$intTotalNumRows = $row['count'];
$intTotalPages = ceil($intTotalNumRows / $intRowsPerPage);

if(isset($_GET['page']) && is_numeric($_GET['page'])) {
    $intCurPage = (int) $_GET['page'];
} else {
    $intCurPage = 1;
}
if($intCurPage > $intTotalPages) {
    $intCurPage = $intTotalPages;
}
if($intCurPage < 1) {
    $intCurPage = 1;
}
$intOffset = ($intCurPage -1) * $intRowsPerPage;

if(isset($_GET['r0'])) {
    if ($_GET['r0'] == 'a') {
        $strQuery .= "order by login_sec_level ASC limit " . $intOffset . ", " . $intRowsPerPage . ";";
    } elseif ($_GET['r0'] == 'd') {
        $strQuery .= "order by login_sec_level DESC limit " . $intOffset . ", " . $intRowsPerPage . ";";
    }
}
if(isset($_GET['r1'])) {
    if ($_GET['r1'] == 'a') {
        $strQuery .= "order by userid ASC limit " . $intOffset . ", " . $intRowsPerPage . ";";
    } elseif ($_GET['r1'] == 'd') {
        $strQuery .= "order by userid DESC limit " . $intOffset . ", " . $intRowsPerPage . ";";
    }
}
if(isset($_GET['r2'])) {
    if ($_GET['r2'] == 'a') {
        $strQuery .= "order by username ASC limit " . $intOffset . ", " . $intRowsPerPage . ";";
    } elseif ($_GET['r2'] == 'd') {
        $strQuery .= "order by username DESC limit " . $intOffset . ", " . $intRowsPerPage . ";";
    }
}

$result = mysqli_query($conn, $strQuery);
$intNumRows = mysqli_num_rows($result);
$num_rows = $result->num_rows;
$lc1=0;
while($row = mysqli_fetch_assoc($result)) {
    $users[$lc1]['loginid'] = $row['loginid'];
    $users[$lc1]['userid'] = $row['userid'];
    $users[$lc1]['login_sec_level'] = $row['login_sec_level'];
    $users[$lc1]['username'] = $row['username'];
    $users[$lc1]['active'] = $row['active'];
    $lc1++;
}
echo '<form action="setup.php" method="post">';
echo '<table border="1">';
echo '<thead>';
echo '<tr>';
if(isset($_SESSION["rseclevel"])) {
    echo '<th><input type="text" name="rseclevel" value ="' . $_SESSION["rseclevel"] . '" /><BR>';
} else {
    echo '<th><input type="text" name="rseclevel"><BR>';
}
echo '<a href="setup.php?page=' . $intCurPage;
if(!isset($_GET['r0'])) {
    echo '&r0=a">Security Level</a></th>';
} elseif ($_GET['r0'] == 'a') {
    echo '&r0=d">Security Level &#8593</a></th>';
} elseif ($_GET['r0'] == 'd') {
    echo '">Security Level &#8595</a></th>';
}
if(isset($_SESSION["ruserid"])) {
    echo '<th><input type="text" name="ruserid" value ="' . $_SESSION["ruserid"] . '" /><BR>';
} else {
    echo '<th><input type="text" name="ruserid"><BR>';
}
echo '<a href="setup.php?page=' . $intCurPage;
if(!isset($_GET['r1'])) {
    echo '&r1=a">User ID</a></th>';
} elseif ($_GET['r1'] == 'a') {
    echo '&r1=d">User ID &#8593</a></th>';
} elseif ($_GET['r1'] == 'd') {
    echo '">User ID &#8595</a></th>';
}
if(isset($_SESSION["rfullname"])) {
    echo '<th><input type="text" name="rfullname" value ="' . $_SESSION["rfullname"] . '" /><BR>';
} else {
    echo '<th><input type="text" name="rfullname"><BR>';
}
echo '<a href="setup.php?page=' . $intCurPage;
if(!isset($_GET['r2'])) {
    echo '&r2=a">Full Name</a></th>';
} elseif ($_GET['r2'] == 'a') {
    echo '&r2=d">Full Name &#8593</a></th>';
} elseif ($_GET['r2'] == 'd') {
    echo '">Full Name &#8595</a></th>';
}
echo '<th>Active</th>';
echo '</tr>';
echo '</thead>';
echo '<tbody>';
$lc1=0;
foreach($users as $u) {
    if($users[$lc1]['login_sec_level'] <= $_SESSION['login_sec_level']) {
        echo '<tr>';
        echo '<td>' . $users[$lc1]['login_sec_level'] . '</td>';
        echo '<td><a href="/primfax/update_user.php?e=' . $users[$lc1]['loginid'] . '">' . $users[$lc1]['userid'] . '</a></td>';
        echo '<td><a href="/primfax/update_user.php?e=' . $users[$lc1]['loginid'] . '">' . $users[$lc1]['username'] . '</a></td>';
        echo '<td>' . $users[$lc1]['active'] . '</td>';
        echo '</tr>';
    }
    $lc1++;
}
echo "</tbody>";
echo '</table>';
echo '<button type="submit" id="btnQuery" name="btnQuery">Query</button>';
echo '<button type="submit" id="btnReset" name="btnReset">Clear</button>';
echo '</form><br>';
echo "<BR>";

$range = 3;
if ($intCurPage > 1) {
    echo " <a href='setup.php?page=1'><<</a> ";
    $intPrevPage = $intCurPage - 1;
    echo " <a href='setup.php?page=$intPrevPage'><</a> ";
}
 
for ($x = ($intCurPage - $range); $x < (($intCurPage + $range) + 1); $x++) {
    if (($x > 0) && ($x <= $intTotalPages)) {
       if ($x == $intCurPage) {
          echo " [<b>$x of $intTotalPages</b>] ";
       } else {
          echo " <a href='setup.php?page=$x'>$x</a> ";
       }
    }
}
 
if ($intCurPage != $intTotalPages) {
    $intNextPage = $intCurPage + 1;
    echo " <a href='setup.php?page=$intNextPage'>></a> ";
    echo " <a href='setup.php?page=$intTotalPages'>>></a> ";
}
Display_Footer();
echo '</BODY>';
echo '</HTML>';
