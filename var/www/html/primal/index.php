<?php
    //License GPLv3
	//Version 1.00.07
	//2021-08-03
	session_start();
    header( "Expires: Mon, 20 Dec 1998 01:00:00 GMT" );
    header( "Last-Modified: " . gmdate("D, d M Y H:i:s") . " GMT" );
    header( "Cache-Control: no-cache, must-revalidate" );
    header( "Pragma: no-cache" );

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
if(isset($_POST['refresh']))
{
	if($_SESSION['refresh'] == 0)
	{
		$_SESSION['refresh'] = 10;
	} elseif($_SESSION['refresh'] == 10) {
		$_SESSION['refresh'] = 30;
	} elseif($_SESSION['refresh'] == 30) {
		$_SESSION['refresh'] = 300;
	} else {
		$_SESSION['refresh'] = 0;
	}
	unset($_POST['refresh']);
} elseif(!isset($_SESSION['refresh'])) {
	$_SESSION['refresh'] = 0;
}

if(isset($_SERVER['HTTPS']) && $_SERVER['HTTPS'] === 'on') {
	$strHTTP="https://";
} else {
	$strHTTP="http://";
}


if($_SESSION['refresh'] != 0)
{
	header( "Refresh: " . $_SESSION['refresh'] . '; URL=' . $strHTTP . $_SERVER['HTTP_HOST'] . $_SERVER['REQUEST_URI']);
}

require_once('config.php');
require_once('functions.php');

if(isset($_SESSION['HAVECONFIG']))
	unset($_SESSION['HAVECONFIG']);
if(isset($_SESSION['PRIDESTHIP']))
	unset($_SESSION['PRIDESTHIP']);
if(isset($_SESSION['PRIDESTAEC']))
	unset($_SESSION['PRIDESTAEC']);
if(isset($_SESSION['PRIDESTPORT']))
	unset($_SESSION['PRIDESTPORT']);
if(isset($_SESSION['PRIDESTCDCR']))
	unset($_SESSION['PRIDESTCDCR']);
if(isset($_SESSION['PRITAG']))
	unset($_SESSION['PRITAG']);
if(isset($_SESSION['PRIQRTAG']))
	unset($_SESSION['PRIQRTAG']);

$intFilter = 0;
if (isset($_POST['submit'])) {
	if(!empty($_POST["input_pname"])) {
		$_SESSION["input_pname"] = $_POST["input_pname"];
		$intFilter = $intFilter + 1;
	}
	if(!empty($_POST["input_pid"])) {
		$_SESSION["input_pid"] = $_POST["input_pid"];
		$intFilter = $intFilter + 1;
	}
	if(!empty($_POST["input_accn"])) {
		$_SESSION["input_accn"] = $_POST["input_accn"];
		$intFilter = $intFilter + 1;
	}
	if(!empty($_POST["input_puid"])) {
		$_SESSION["input_puid"] = $_POST["input_puid"];
		$intFilter = $intFilter + 1;
	}
	if(!empty($_POST["input_startdt"])) {
		$_SESSION["input_startdt"] = $_POST["input_startdt"];
		$intFilter = $intFilter + 1;
	}
	if(!empty($_POST["input_enddt"])) {
		$_SESSION["input_enddt"] = $_POST["input_enddt"];
		$intFilter = $intFilter + 1;
	}
}

if (isset($_POST['reset'])) {
	unset($_SESSION['cached_query']);
	unset($_SESSION["input_pname"]);
	unset($_SESSION["input_pid"]);
	unset($_SESSION["input_accn"]);
	unset($_SESSION["input_puid"]);
	unset($_SESSION["input_startdt"]);
	unset($_SESSION["input_enddt"]);
}

echo <<<EOT
<HTML lang="en">
<!DOCTYPE html>
<HEAD>
	<!-- Written by Will Yonker-->
	<TITLE>PRIMAL Web Interface</TITLE>
	<link rel="stylesheet" href="default.css">
EOT;

echo '</HEAD>';
echo '<BODY>';
Display_Header2();
echo '<br>';
echo '<form action="index.php" method="post">';
if($_SESSION['login_sec_level'] >= 250)
{
	echo '<div id="setup"><a href="' . $strHTTP . $_SERVER['HTTP_HOST'] . '/primal/setup.php">Setup</a></div>';
}
echo '<button type="submit" name="refresh">Refresh ' . $_SESSION['refresh'] . 's</button>';

unset($_SESSION["result"]);
unset($studies);

if ($_SESSION['page_size'] < 20) {
	$tempvar = 30;
} else {
	$tempvar = $_SESSION['page_size'];
}
if (!isset($_GET["p"]) || $_GET["p"] < 3) {
	$tempvar2 = 6;
} else {
	$tempvar2 = $_GET["p"] + 4;
}
if (!isset($_GET["o"])) {
	$sort_order = "DESC";
	$intSortOrder = 1;
} elseif ($_GET["o"] == 1) {
	$sort_order = "ASC";
	$intSortOrder = 0;
} else {
	$sort_order = "DESC";
	$intSortOrder = 1;
}

//echo "Time Index before select count: " . xdebug_time_index() . "<br>";

//$query="select a.puid, a.pname, a.pid, a.pdob, a.pmod, a.sdatetime, r.tstartrec, r.tendrec, r.rec_images, r.rerror, r.senderAET, p.tstartproc, p.tendproc, p.perror, s.tdest, s.tstartsend, s.tendsend, s.timages, s.serror, t.AccessionNum, t.StudyDate from patient as a left join receive as r on a.puid = r.puid left join process as p on a.puid = p.puid left join send as s on a.puid = s.puid left join study as t on a.puid = t.puid";
$query="select count(*) as total from send;";
$result = mysql_query($query);
$qdata=mysql_fetch_assoc($result);
$num_rows = $qdata['total'];;
$total_pages = floor($num_rows/$_SESSION['page_size']);
//echo "num_rows = " . $num_rows . " : total_pages = " . $total_pages . "<br>";
if(!isset($_GET["c"]) || $_GET["c"] == 0) {
	$sort_column = "r.tstartrec";
} elseif($_GET["c"] == 1) {
	$sort_column = "a.pname";
} elseif($_GET["c"] == 2) {
	$sort_column = "a.pid";
} elseif($_GET["c"] == 3) {
	$sort_column = "a.paccn";
} elseif($_GET["c"] == 4) {
	$sort_column = "a.pdob";
} elseif($_GET["c"] == 5) {
	$sort_column = "a.pmod";
} elseif($_GET["c"] == 6) {
	$sort_column = "a.sdatetime";
} elseif($_GET["c"] == 7) {
	$sort_column = "r.tendrec";
} elseif($_GET["c"] == 8) {
	$sort_column = "r.rec_images";
} elseif($_GET["c"] == 9) {
	$sort_column = "s.tstartsend";
} else {
	$sort_column = "r.tstartrec";
}

//echo "Time Index before select: " . xdebug_time_index() . "<br>";
/*
if($_SERVER['REQUEST_METHOD'] == 'POST')
{
	$query=Check_Input($sort_column, $sort_order, $tempvar, $tempvar2);
} elseif(isset($_SESSION['cached_query'])) {
	//$query=$_SESSION['cached_query'];
} else {
	$query="select a.puid, a.pname, a.pid, a.pdob, a.pmod, a.sdatetime, r.tstartrec, r.tendrec, r.rec_images, r.rerror, r.senderAET, ";
	$query.="p.tstartproc, p.tendproc, p.perror, s.tdest, s.tstartsend, s.tendsend, s.timages, s.serror, t.AccessionNum, t.StudyModType, ";
	$query.="t.StudyDate , t.sCaseID from patient as a left join receive as r on a.puid = r.puid left join process as p on a.puid = p.puid ";
	$query.="left join send as s on a.puid = s.puid left join study as t on a.puid = t.puid order by " . $sort_column . " " . $sort_order;
}
*/
if(!empty($_SESSION['cached_query'])) {
	$query = $_SESSION['cached_query'];
} else {
	$query="select a.puid, a.pname, a.pid, a.pdob, r.tstartrec, r.tendrec, r.rec_images, r.rerror, r.senderAET, ";
	$query.="p.tstartproc, p.tendproc, p.perror, s.tdest, s.tstartsend, s.tendsend, s.timages, s.serror, t.AccessionNum, t.StudyModType, ";
	$query.="t.StudyDate , t.sCaseID from patient as a left join receive as r on a.puid = r.puid left join process as p on a.puid = p.puid ";
	$query.="left join send as s on a.puid = s.puid left join study as t on a.puid = t.puid ";

	if ($intFilter > 0) {
		$query.=' where';
	}
	if(!empty($_SESSION["input_startdt"]) && !empty($_SESSION["input_enddt"])) {
		$query.=' (r.tstartrec between "' . $_SESSION["input_startdt"] . '" and "' . $_SESSION["input_enddt"] . '")';
	} elseif (!empty($_SESSION["input_startdt"])) {
		$query.=' (r.tstartrec between "' . $_SESSION["input_startdt"] . '" and NOW())';
	} elseif (!empty($_SESSION["input_enddt"])) {
		$query.=' (r.tstartrec between NOW() and "' . $_SESSION["input_startdt"] . '")';
	}
	if(!empty($_SESSION["input_pname"])) {
		$intPos = strpos($_SESSION["input_pname"], '%');
		if($intPos === false) {
			$query.=' a.pname = "' . $_SESSION["input_pname"] . '" ';
		} else {
			$query.=' a.pname like "' . $_SESSION["input_pname"] . '" ';
		}
	}
	if(!empty($_SESSION["input_pid"])) {
		$intPos = strpos($_SESSION["input_pid"], '%');
		if($intPos === false) {
			$query.=' a.pid = "' . $_SESSION["input_pid"] . '" ';
		} else {
			$query.=' a.pid like "' . $_SESSION["input_pid"] . '" ';
		}
	}
	if(!empty($_SESSION["input_accn"])) {
		$intPos = strpos($_SESSION["input_accn"], '%');
		if($intPos === false) {
			$query.=' t.AccessionNum = "' . $_SESSION["input_accn"] . '" ';
		} else {
			$query.=' t.AccessionNum like "' . $_SESSION["input_accn"] . '" ';
		}
	}
	if(!empty($_SESSION["input_puid"])) {
		$intPos = strpos($_SESSION["input_puid"], '%');
		if($intPos === false) {
			$query.=' a.puid = "' . $_SESSION["input_puid"] . '" ';
		} else {
			$query.=' a.puid like "' . $_SESSION["input_puid"] . '" ';
		}
	}
	$query.="order by " . $sort_column . " " . $sort_order . " ";
	$query.="limit " . ($tempvar * $tempvar2);
	if($intFilter != 0) {
		$_SESSION['cached_query'] = $query;
	}
}
/*
if(!empty($_POST["input_startdt"])) {
	echo "<br>Start Receive date: " . $_POST["input_startdt"] . "<br>";
}
if(!empty($_POST["input_enddt"])) {
	echo "<br>Start Receive date end range: " . $_POST["input_enddt"] . "<br>";
}
*/
//echo "<br>query = " . $query . "<br>";
//echo "query = " . $_SESSION["query"] . "</br>";
$result = mysql_query($query);
$lc1=0;
if(!isset($_GET["p"]))
	$_GET["p"] = 0;
if(!isset($_get['o']))
	$_GET['o']=0;
if(!isset($_get['c']))
	$_GET['c']=0;

//We need to limit the amount of RAM used by this array.  
//First let's set the first study we want to store to be from the page prior to the one we want on the screen ( in case of overlap)
//The maximum should be taken care of by the limit on the query

//echo "Time Index before sort: " . xdebug_time_index() . "<br>";

while($row = mysql_fetch_assoc($result))
{
	//Per receiver security
	$intPOS=strpos($row["puid"], "_");
	$strRecName=substr($row["puid"], 0, ($intPOS));
	$strRecName .= " ";
    //Need to get the modality.
	$query10="select Modality from series where puid = '" . $row["puid"] . "' group by Modality";
	$result10 = mysql_query($query10);
	unset($temppmod);
	$temppmod="";
	$intLC10=0;
	while($row2 =  mysql_fetch_assoc($result10)) {
			$intLC10++;
			if ($intLC10 > 1) {
					$temppmod .= " " . $row2["Modality"];
			} else {
					$temppmod .= $row2["Modality"];
			}
	}
	if(strpos($_SESSION['rec_sec'], $strRecName) !== FALSE) {
		$studies[$lc1]["tstartrec"] = $row["tstartrec"];
		$studies[$lc1]["caseid"] = $row["sCaseID"];
		$studies[$lc1]["pname"] = $row["pname"];
		$studies[$lc1]["pid"] = $row["pid"];
		$studies[$lc1]["paccn"] = $row["AccessionNum"];
		$studies[$lc1]["pdob"] = $row["pdob"];
		$studies[$lc1]["sdatetime"] = $row["StudyDate"];
		$studies[$lc1]["tendrec"] = $row["tendrec"];
		$studies[$lc1]["rec_images"] = $row["rec_images"];
		$studies[$lc1]["tstartproc"] = $row["tstartproc"];
		$studies[$lc1]["tendproc"] = $row["tendproc"];
		$studies[$lc1]["tdest"] = $row["tdest"];
		$studies[$lc1]["tstartsend"] = $row["tstartsend"];
		$studies[$lc1]["tendsend"] = $row["tendsend"];
		$studies[$lc1]["timages"] = $row["timages"];
		$studies[$lc1]["puid"] = $row["puid"];
		$studies[$lc1]["rerror"] = $row["rerror"];
		$studies[$lc1]["perror"] = $row["perror"];
		$studies[$lc1]["serror"] = $row["serror"];
		$studies[$lc1]["senderAET"] = $row["senderAET"];
		//$studies[$lc1]["pmod"] = $row["StudyModType"];
		$studies[$lc1]["pmod"] = $temppmod;
		/*
		if(is_null($row["StudyModType"])) {
			$query4="select count(*) as total from study where puid = '" . $studies[$lc1]["puid"] . "';";
			$result4=mysql_query($query4);
			$qdata=mysql_fetch_assoc($result);
			$is_empty_study = $qdata['total'];;
			if($is_empty_study == 0) {
				$query4="delete from patient where puid = '" . $studies[$lc1]["puid"] . "';";
				$result4=mysql_query($query4);
				$query4="delete from receive where puid = '" . $studies[$lc1]["puid"] . "';";
				$result4=mysql_query($query4);
				$query4="delete from process where puid = '" . $studies[$lc1]["puid"] . "';";
				$result4=mysql_query($query4);
				$query4="delete from send where puid = '" . $studies[$lc1]["puid"] . "';";
				$result4=mysql_query($query4);
				$query4="delete from series where puid = '" . $studies[$lc1]["puid"] . "';";
				$result4=mysql_query($query4);
				$query4="delete from image where puid = '" . $studies[$lc1]["puid"] . "';";
				$result4=mysql_query($query4);
				echo "Purghing puid " . $studies[$lc1]["puid"] . " from the database...<br>";
			}
			echo "Getting here for " . $studies[$lc1]["puid"] . "...<br>";
			$query2="select Modality from series where puid ='" . $studies[$lc1]["puid"] . "';";
			$result2 = mysql_query($query2);
			$lc2=0;
			while($row2 = mysql_fetch_assoc($result2)) {
				if($lc2 > 0) {
					if(strstr($studies[$lc1]["pmod"], $row2["Modality"]) === FALSE) {
						$studies[$lc1]["pmod"] .= "/" . $row2["Modality"];
					}
				} else {
					$studies[$lc1]["pmod"] = $row2["Modality"];
				}
				$lc2++;
			}
			$query3="update study set StudyModType ='" . $studies[$lc1]["pmod"] . "' where puid = '" . $studies[$lc1]["puid"] . "';";
			$result3 = mysql_query($query3);
		}
		*/
		$lc1++;
	}
	//$_SESSION["studies"] = $studies;
}

$count_rows=$lc1-1;
//echo "Sorted " . $count_rows . " rows of data.<br>";

if ($_GET["c"] >= 1 && $_GET["c"] <= 8)
{
	$sort_column = $_GET["c"];
} else {
	$sort_column = 0;
}

if ($_GET["o"] == "1")
{
	$sort_order = 1;
} else {
	$sort_order = 0;
}

/*
if (isset($_GET["c"]))
{
	$studies=Sort_Data($studies, $sort_column, $sort_order);
} else {
	$studies=Sort_Data($studies, 0, 1);
	$_GET["c"]=0;
	$_GET["o"]=1;
}
*/

//echo "Time Index before select from page_columns: " . xdebug_time_index() . "<br>";

$lc21=0;
$query="select * from page_columns;";
$result = mysql_query($query);
while($row = mysql_fetch_assoc($result))
{
    $ucolumns[$lc21]["id"] = $row["id"];
    $ucolumns[$lc21]["name"] = $row["name"];
    $ucolumns[$lc21]["dorder"] = $row["dorder"];
    $ucolumns[$lc21]["order"] = $row["dorder"];
    $ucolumns[$lc21]["visible"] = 0;

    $lc21++;
}

$query="select * from user_columns where user_id = '" . $_SESSION['loginid'] . "';";
$result = mysql_query($query);
$num_rows = mysql_num_rows($result);
while($row = mysql_fetch_assoc($result))
{
    $lc22=0;
    while($lc22 < count($ucolumns)) {
        $ucolumns[$lc22]["visible"] = $row["Column" . $ucolumns[$lc22]["id"] . "_Visible"];
        $ucolumns[$lc22]["order"] = $row["Column" . $ucolumns[$lc22]["id"] . "_Order"];
        $lc22++;
    }
}

echo '<table border="1">';
echo '<tr>';
$lc23=0;
while ($lc23 <= count($ucolumns)) {
	$lc24=0;
	while ($lc24 < count($ucolumns)) {
		if ($lc23 == $ucolumns[$lc24]["order"]) {
			Show_Headers($ucolumns[$lc24]["id"], $ucolumns, $sort_column, $intSortOrder);
		}
		$lc24++;
	}
	$lc23++;
}
echo '</th>';


if(!isset($_SESSION['page_size']) || !ctype_digit($_SESSION['page_size']))
{
	$_SESSION['page_size']=0;
}

$lc1=0;
$lc1=(($_GET["p"]) * $_SESSION['page_size']);
$pend=(($_GET["p"]+1) * $_SESSION['page_size']);
if($pend == 0 || $pend > count($studies))
{
	$pend = count($studies);
}

while($lc1 < $pend)
{
	$temperror=0;
	$temperror=$studies[$lc1]["rerror"] + $studies[$lc1]["perror"] + $studies[$lc1]["serror"];
	echo '<tr>';
	$lc23=0;
	while ($lc23 <= count($ucolumns)) {
		$lc24=0;
		while ($lc24 < count($ucolumns)) {
			if ($lc23 == $ucolumns[$lc24]["order"]) {
				Show_Rows($ucolumns[$lc24]["id"], $lc1, $studies, $ucolumns);
			}
			$lc24++;
		}
		$lc23++;
	}
	echo '</tr>';

	$lc1++;
}
echo '</table><br>';
echo '<button type="submit" name="reset">Clear</button>';
echo '<button type="submit" name="submit">Query</button>';
echo '<br><br>';
$start_page=$_GET["p"]-3;
$end_page=$_GET["p"]+3;
if($count_rows > $_SESSION['page_size'] && $_SESSION['page_size'] != 0)
{
	$count_pages = floor($count_rows/$_SESSION['page_size']);
	$lc1=0;
	echo '<a href="/primal/index.php?c=' . $_GET["c"] . '&o=' . $_GET["o"] . '&p=' . $lc1 . '">First</a> ';
	if ($_GET["p"] > 0)
	{
		echo '<a href="/primal/index.php?c=' . $_GET["c"] . '&o=' . $_GET["o"] . '&p=' . ($_GET["p"]-1) . '"> << </a> ';
	}
	while($lc1 <= $count_pages)
	{
		if($lc1 >= $start_page && $lc1 <= $end_page)
		{
			if($lc1 == $_GET["p"])
			{
				echo " <b>" . ($lc1+1) . "</b> " ;
			} else {
				echo '<a href="/primal/index.php?c=' . $_GET["c"] . '&o=' . $_GET["o"] . '&p=' . $lc1 . '">' . ($lc1+1) . '</a> ';
			}
		}
		$lc1++;
	}
	if ($_GET["p"] < $count_pages)
	{
		echo '<a href="/primal/index.php?c=' . $_GET["c"] . '&o=' . $_GET["o"] . '&p=' . ($_GET["p"]+1) . '"> >> </a> ';
	}
	echo '<a href="/primal/index.php?c=' . $_GET["c"] . '&o=' . $_GET["o"] . '&p=' . ($total_pages) . '">Last</a> ';
}
if(isset($err_msg))
	echo '<div style="color:#FF0000;"><blink>' . $err_msg . "</blink></div><br>";
echo '   </form>';
Display_Footer();
echo '</BODY>';
echo '</HTML>';
unset($studies);

function Show_Headers($Column_Number, $ucolumns, $sort_column, $intSortOrder) {
switch ($Column_Number) {
case 1:
	if($ucolumns[0]["visible"] == 0) {
		if ($sort_column == 0 && $intSortOrder == 0)
		{
			echo '<th><a href="/primal/index.php?c=0&o=1&p=' . $_GET["p"] . '">Start Receive</a><br/>';
			if(isset($_SESSION['input_startdt']))
			{
				echo '<input type="date" name="input_startdt" value="' . $_SESSION['input_startdt'] . '"/></br>';
			} else {
				echo '<input type="date" name="input_startdt" /></br>';
			}
			if(isset($_SESSION['input_enddt']))
			{
				echo '<input type="date" name="input_enddt" value="' . $_SESSION['input_enddt'] . '"/></br>';
			} else {
				echo '<input type="date" name="input_enddt" /></br>';
			}
		} else {
			echo '<th><a href="/primal/index.php?c=0&o=0&p=' . $_GET["p"] . '">Start Receive</a><br/>';
			if(isset($_SESSION['input_startdt']))
			{
				echo '<input type="date" name="input_startdt" value="' . $_SESSION['input_startdt'] . '"/></br>';
			} else {
				echo '<input type="date" name="input_startdt" /></br>';
			}
			if(isset($_SESSION['input_enddt']))
			{
				echo '<input type="date" name="input_enddt" value="' . $_SESSION['input_enddt'] . '"/></br>';
			} else {
				echo '<input type="date" name="input_enddt" /></br>';
			}
		}
	}
break;
case 2:
	if($ucolumns[1]["visible"] == 0) {
		if ($sort_column == 1 && $intSortOrder == 0)
		{
			echo '<th><a href="/primal/index.php?c=1&o=1&p=' . $_GET["p"] . '">Patient Name</a><br/>';
			if(isset($_SESSION['input_pname']))
			{
				echo '<input type="text" name="input_pname" value="' . $_SESSION['input_pname'] . '"/></th>';
			} else {
				echo '<input type="text" name="input_pname" /></th>';
			}
		} else {
			echo '<th><a href="/primal/index.php?c=1&o=0&p=' . $_GET["p"] . '">Patient Name</a><br/>';
			if(isset($_SESSION['input_pname']))
			{
				echo '<input type="text" name="input_pname" value="' . $_SESSION['input_pname'] . '"/></th>';
			} else {
				echo '<input type="text" name="input_pname" /></th>';
			}
		}
	}
break;
case 3:
	if($ucolumns[2]["visible"] == 0) {
		if ($sort_column == 2 && $intSortOrder == 0)
		{
			echo '<th><a href="/primal/index.php?c=2&o=1&p=' . $_GET["p"] . '">Patient ID</a></br>';
			if(isset($_SESSION['input_pid'])) {
				echo '<input type="text" name="input_pid" />' . $_SESSION["input_pid"] . '</th>';
			} else {
				echo '<input type="text" name="input_pid" /></th>';
			}
		} else {
			echo '<th><a href="/primal/index.php?c=2&o=0&p=' . $_GET["p"] . '">Patient ID</a></br>';
			if(isset($_SESSION['input_pid'])) {
				echo '<input type="text" name="input_pid" />' . $_SESSION["input_pid"] . '</th>';
			} else {
				echo '<input type="text" name="input_pid" /></th>';
			}
		}
	}
break;
case 4:
	if($ucolumns[3]["visible"] == 0) {
		if ($sort_column == 3 && $intSortOrder == 0)
		{
			echo '<th><a href="/primal/index.php?c=3&o=1&p=' . $_GET["p"] . '">Accession #</a></br>';
			if(isset($_SESSION['input_accn']))
			{
				echo '<input type="text" name="input_accn" value="' . $_SESSION['input_accn'] . '"/></th>';
			} else {
				echo '<input type="text" name="input_accn" /></th>';
			}
		} else {
			echo '<th><a href="/primal/index.php?c=3&o=0&p=' . $_GET["p"] . '">Accession #</a></br>';
			if(isset($_SESSION['input_accn']))
			{
				echo '<input type="text" name="input_accn" value="' . $_SESSION['input_accn'] . '"/></th>';
			} else {
				echo '<input type="text" name="input_accn" /></th>';
			}
		}
	}
break;
case 5:
	if($ucolumns[4]["visible"] == 0) {
		if ($sort_column == 4 && $intSortOrder == 0)
		{
			echo '<th><a href="/primal/index.php?c=4&o=1&p=' . $_GET["p"] . '">DOB</a></th>';
		} else {
			echo '<th><a href="/primal/index.php?c=4&o=0&p=' . $_GET["p"] . '">DOB</a></th>';
		}
	}
break;
case 6:
	if($ucolumns[5]["visible"] == 0) {
		if ($sort_column == 5 && $intSortOrder == 0)
		{
			echo '<th><a href="/primal/index.php?c=5&o=1&p=' . $_GET["p"] . '">MOD</a></th>';
		} else {
			echo '<th><a href="/primal/index.php?c=5&o=0&p=' . $_GET["p"] . '">MOD</a></th>';
		}
	}
break;
case 7:
	if($ucolumns[6]["visible"] == 0) {
		if ($sort_column == 6 && $intSortOrder == 0)
		{
			echo '<th><a href="/primal/index.php?c=6&o=1&p=' . $_GET["p"] . '">Study Date</a></th>';
		} else {
			echo '<th><a href="/primal/index.php?c=6&o=0&p=' . $_GET["p"] . '">Study Date</a></th>';
		}
	}
break;
case 8:
	if($ucolumns[7]["visible"] == 0) {
		if ($sort_column == 7 && $intSortOrder == 0)
		{
			echo '<th><a href="/primal/index.php?c=7&o=1&p=' . $_GET["p"] . '">End Receive</a></th>';
		} else {
			echo '<th><a href="/primal/index.php?c=7&o=0&p=' . $_GET["p"] . '">End Receive</a></th>';
		}
	}
break;
case 9:
	if($ucolumns[8]["visible"] == 0) {
		if ($sort_column == 8 && $intSortOrder == 0) {
			echo '<th><a href="/primal/index.php?c=8&o=1&p=' . $_GET["p"] . '">#<br>images<br>rec</a></th>';
		} else {
			echo '<th><a href="/primal/index.php?c=8&o=0&p=' . $_GET["p"] . '">#<br>images<br>rec</a></th>';
		}
	}
break;
case 10:
	if($ucolumns[9]["visible"] == 0) {
		echo '<th>Start Process Date</th>';
	}
break;
case 11:
	if($ucolumns[10]["visible"] == 0) {
		echo '<th>End Process Date</th>';
	}
break;
case 12:
	if($ucolumns[11]["visible"] == 0) {
		echo '<th>Destination</th>';
	}
break;
case 13:
	if($ucolumns[12]["visible"] == 0) {
		if ($sort_column == 9 && $intSortOrder == 0) {
			echo '<th><a href="/primal/index.php?c=9&o=1&p=' . $_GET["p"] . '">Start Send Date</a></th>';
		} else {
			echo '<th><a href="/primal/index.php?c=9&o=0&p=' . $_GET["p"] . '">Start Send Date</a></th>';
		}
	}
break;
case 14:
	if($ucolumns[13]["visible"] == 0) {
		echo '<th>End Send Date</th>';
	}
break;
case 15:
	if($ucolumns[14]["visible"] == 0) {
		echo '<th>#<br>images<br>sent</th>';
	}
break;
case 16:
	if($ucolumns[15]["visible"] == 0) {
		echo '<th>PRIMAL ID</BR>';
		if(isset($_SESSION['input_puid'])) {
			echo '<input type="text" name="input_puid" value="' . $_SESSION['input_puid'] . '"/></th>';
		} else {
			echo '<input type="text" name="input_puid" /></th>';
		}
	}
break;
case 17:
	if($ucolumns[16]["visible"] == 0) {
		if ($sort_column == 10 && $intSortOrder == 0) {
			$bolSortOrder=1;
		} else {
			$bolSortOrder=0;
		}
		echo '<th><a href="/primal/index.php?c=10&o=' . $bolSortOrder . '&p=' . $_GET["p"] . '">Sender<br>AET</a></th>';
	}
break;
case 18:
	if($ucolumns[17]["visible"] == 0) {
		echo '<th>Case ID</th>';
	}
break;
}
}

function Show_Rows($Column_Number, $lc1, $studies, $ucolumns) {
$temperror=$studies[$lc1]["serror"]+$studies[$lc1]["rerror"];
switch ($Column_Number) {
case 1:
	if($ucolumns[0]["visible"] == 0) {
		if($studies[$lc1]["rerror"] != 0)
		{
			echo '<td bgcolor="#FF0000"><font color="#FFFFFF">' . $studies[$lc1]["tstartrec"] . '</td>';
		} else {
			echo '<td>' . $studies[$lc1]["tstartrec"] . '</td>';
		}
	}
break;
case 2:
	if($ucolumns[1]["visible"] == 0) {
		echo '<td><a href="/primal/resend.php?p=' . $studies[$lc1]["puid"] . '"> ' . $studies[$lc1]["pname"] . '</a></td>';
	}
break;
case 3:
	if($ucolumns[2]["visible"] == 0) {
		echo '<td>' . $studies[$lc1]["pid"] . '</td>';
	}
break;
case 4:
	if($ucolumns[3]["visible"] == 0) {
		echo '<td>' . $studies[$lc1]["paccn"] . '</td>';
	}
break;
case 5:
	if($ucolumns[4]["visible"] == 0) {
		echo '<td>' . $studies[$lc1]["pdob"] . '</td>';
	}
break;
case 6:
	if($ucolumns[5]["visible"] == 0) {
		echo '<td>' . $studies[$lc1]["pmod"] . '</td>';
	}
break;
case 7:
	if($ucolumns[6]["visible"] == 0) {
		echo '<td>' . $studies[$lc1]["sdatetime"] . '</td>';
	}
break;
case 8:
	if($ucolumns[7]["visible"] == 0) {
		if($studies[$lc1]["rerror"] != 0)
		{
			echo '<td bgcolor="#FF0000">' . $studies[$lc1]["tendrec"] . '</td>';
		} else {
			echo '<td>' . $studies[$lc1]["tendrec"] . '</td>';
		}
	}
break;
case 9:
	if($ucolumns[8]["visible"] == 0) {
		if($studies[$lc1]["rerror"] != 0)
        {
			echo '<td bgcolor="#FF0000">' . $studies[$lc1]["rec_images"] . '</td>';
		} else {
			echo '<td>' . $studies[$lc1]["rec_images"] . '</td>';
		}
	}
break;
case 10:
	if($ucolumns[9]["visible"] == 0) {
		if($studies[$lc1]["perror"] != 0)
		{
			echo '<td bgcolor="#FF0000"><font color="#FFFFFF">' . $studies[$lc1]["tstartproc"] . '</td>';
		} else {
			echo '<td>' . $studies[$lc1]["tstartproc"] . '</td>';
		}
	}
break;
case 11:
	if($ucolumns[10]["visible"] == 0) {
		if($studies[$lc1]["perror"] != 0)
		{
			echo '<td bgcolor="#FF0000"><font color="#FFFFFF">' . $studies[$lc1]["tendproc"] . '</td>';
		} else {
			echo '<td>' . $studies[$lc1]["tendproc"] . '</td>';
		}
	}
break;
case 12:
	if($ucolumns[11]["visible"] == 0) {
		if($studies[$lc1]["serror"] != 0)
		{
			echo '<td bgcolor="#FF0000"><font color="#FFFFFF">' . $studies[$lc1]["tdest"] . '</td>';
		} else {
			echo '<td>' . $studies[$lc1]["tdest"] . '</td>';
		}
	}
break;
case 13:
	if($ucolumns[12]["visible"] == 0) {
		if($studies[$lc1]["serror"] != 0)
		{
			echo '<td bgcolor="#FF0000"><font color="#FFFFFF">' . $studies[$lc1]["tstartsend"] . '</td>';
		} else {
			echo '<td>' . $studies[$lc1]["tstartsend"] . '</td>';
		}
	}
break;
case 14:
	if($ucolumns[13]["visible"] == 0) {
		if($studies[$lc1]["serror"] != 0)
		{
			echo '<td bgcolor="#FF0000"><font color="#FFFFFF">' . $studies[$lc1]["tendsend"] . '</td>';
		} else {
			echo '<td>' . $studies[$lc1]["tendsend"] . '</td>';
		}
	}
break;
case 15:
	if($ucolumns[14]["visible"] == 0) {
		if($studies[$lc1]["serror"] != 0)
		{
			echo '<td bgcolor="#FF0000"><font color="#FFFFFF">' . $studies[$lc1]["timages"] . '</td>';
		} else {
			echo '<td>' . $studies[$lc1]["timages"] . '</td>';
		}
	}
break;
case 16:
	if($ucolumns[15]["visible"] == 0) {
		echo '<td>' . $studies[$lc1]["puid"] . '</td>';
	}
break;
case 17:
	if($ucolumns[16]["visible"] == 0) {
		if($studies[$lc1]["rerror"] != 0)
		{
			echo '<td bgcolor="#FF0000"><font color="#FFFFFF">' . $studies[$lc1]["senderAET"] . '</td>';
		} else {
			echo '<td>' . $studies[$lc1]["senderAET"] . '</td>';
		}
	}
break;
case 18:
	if($ucolumns[17]["visible"] == 0) {
		echo '<td>' . $studies[$lc1]["caseid"] . '</td>';
	}
}
}

?>
