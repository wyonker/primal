<?php
	//License GPLv3
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

require_once('config.php');
require_once('functions.php');

$names=array("Erie", "Akron General", "Hawaii", "TeleStroke", "Magic");

for($intLC=0; $intLC<=11; $intLC++) {
	$link2 = mysql_connect('172.27.40.90:3306', 'stats', 'primal');
	if (!$link2) {
		die('Could not connect: ' . mysql_error());
	}
	mysql_select_db('primal', $link2);

	$intLastMonth=date("m", strtotime("first day of -" . $intLC . " month"));
	$intYearSearch=date("Y", strtotime("first day of -" . $intLC . " month"));

	$query="select count(*) as total from patient as p join receive as r on p.puid=r.puid join study as s on s.puid=p.puid where r.tstartrec like '" . $intYearSearch . "-" . $intLastMonth . "%' and s.StudyDate between date_sub(r.tstartrec, interval 48 hour) and date_add(r.tstartrec, interval 48 hour) and s.StudyDate like '" . $intYearSearch . "-" . $intLastMonth . "%';";
	$result = mysql_query($query, $link2);
	$qdata=mysql_fetch_assoc($result);
	$intCurStudies["Erie"][$intLC] = $qdata['total'];

	$query="select count(*) as total from patient as p join receive as r on p.puid=r.puid join study as s on s.puid=r.puid where r.tstartrec like '" . $intYearSearch . "-" . $intLastMonth . "%' and s.StudyDate like '" . $intYearSearch . "-" . $intLastMonth . "%';";
	$result = mysql_query($query, $link2);
	$qdata=mysql_fetch_assoc($result);
	$intTotalStudies["Erie"][$intLC] = $qdata['total'];
	$intTotalTotal[$intLC] = $intTotalStudies["Erie"][$intLC];

	mysql_close($link2);

	$link2 = mysql_connect('167.90.248.69:3306', 'stats', 'primal');
	if (!$link2) {
		die('Could not connect: ' . mysql_error());
	}
	mysql_select_db('primal', $link2);

	$query="select count(*) as total from patient as p join receive as r on p.puid=r.puid join study as s on s.puid=p.puid where r.tstartrec like '" . $intYearSearch . "-" . $intLastMonth . "%' and s.StudyDate between date_sub(r.tstartrec, interval 48 hour) and date_add(r.tstartrec, interval 48 hour) and s.StudyDate like '" . $intYearSearch . "-" . $intLastMonth . "%';";
	$result = mysql_query($query, $link2);
	$qdata=mysql_fetch_assoc($result);
	$intCurStudies["Akron General"][$intLC] = $qdata['total'];

	$query="select count(*) as total from patient as p join receive as r on p.puid=r.puid join study as s on s.puid=r.puid where r.tstartrec like '" . $intYearSearch . "-" . $intLastMonth . "%' and s.StudyDate like '" . $intYearSearch . "-" . $intLastMonth . "%';";
	$result = mysql_query($query, $link2);
	$qdata=mysql_fetch_assoc($result);
	$intTotalStudies["Akron General"][$intLC] = $qdata['total'];

	mysql_close($link2);

	$link2 = mysql_connect('167.90.248.70:3306', 'stats', 'primal');
	if (!$link2) {
		die('Could not connect: ' . mysql_error());
	}
	mysql_select_db('primal', $link2);

	$query="select count(*) as total from patient as p join receive as r on p.puid=r.puid join study as s on s.puid=p.puid where s.StudyDate between date_sub(r.tstartrec, interval 48 hour) and date_add(r.tstartrec, interval 48 hour) and s.StudyDate like '" . $intYearSearch . "-" . $intLastMonth . "%';";
	$result = mysql_query($query, $link2);
	$qdata=mysql_fetch_assoc($result);
	$intCurStudiesAG["Akron General"][$intLC] += $qdata['total'];

	$query="select count(*) as total from patient as p join receive as r on p.puid=r.puid join study as s on s.puid=r.puid where s.StudyDate like '" . $intYearSearch . "-" . $intLastMonth . "%';";
	$result = mysql_query($query, $link2);
	$qdata=mysql_fetch_assoc($result);
	$intTotalStudiesAG["Akron General"][$intLC] += $qdata['total'];
	$intTotalTotal[$intLC] += $intTotalStudies["Akron General"][$intLC];

	mysql_close($link2);

	$link2 = mysql_connect('10.88.48.151:3306', 'stats', 'primal');
	if (!$link2) {
		die('Could not connect: ' . mysql_error());
	}
	mysql_select_db('primal', $link2);

	$query="select count(*) as total from patient as p join receive as r on p.puid=r.puid where r.tstartrec like '" . $intYearSearch . "-" . $intLastMonth . "%' and p.sdatetime between date_sub(r.tstartrec, interval 48 hour) and date_add(r.tstartrec, interval 48 hour) and p.puid REGEXP '^2_|^4_' and p.sdatetime like '" . $intYearSearch . "-" . $intLastMonth . "%';";
	$result = mysql_query($query, $link2);
	$qdata=mysql_fetch_assoc($result);
	$intCurStudies["Hawaii"][$intLC] = $qdata['total'];

	$query="select count(*) as total from patient as p join receive as r on p.puid=r.puid where r.tstartrec like '" . $intYearSearch . "-" . $intLastMonth . "%' and p.puid REGEXP '^2_|^4_' and p.sdatetime like '" . $intYearSearch . "-" . $intLastMonth . "%';";
	$result = mysql_query($query, $link2);
	$qdata=mysql_fetch_assoc($result);
	$intTotalStudies["Hawaii"][$intLC] = $qdata['total'];
	$intTotalTotal[$intLC] += $intTotalStudies["Hawaii"][$intLC];

	$query="select count(*) as total from (select count(*) from patient as p join receive as r on p.puid=r.puid where r.tstartrec like '" . $intYearSearch . "-" . $intLastMonth . "%' and p.sdatetime between date_sub(r.tstartrec, interval 48 hour) and date_add(r.tstartrec, interval 48 hour) and p.puid like '1_%' and p.sdatetime like '" . $intYearSearch . "-" . $intLastMonth . "%' group by p.pname, p.sdatetime) final;";
	$result = mysql_query($query, $link2);
	$qdata=mysql_fetch_assoc($result);
	$intCurStudies["TeleStroke"][$intLC] = $qdata['total'];

	$query="select count(*) as total from (select count(*) from patient as p join receive as r on p.puid=r.puid where r.tstartrec like '" . $intYearSearch . "-" . $intLastMonth . "%' and p.puid like '1_%' and p.sdatetime like '" . $intYearSearch . "-" . $intLastMonth . "%' group by p.pname, p.sdatetime) final;";
	$result = mysql_query($query, $link2);
	$qdata=mysql_fetch_assoc($result);
	$intTotalStudies["TeleStroke"][$intLC] = $qdata['total'];
	$intTotalTotal[$intLC] += $intTotalStudies["TeleStroke"][$intLC];

	mysql_close($link2);

	$link2 = mysql_connect('10.65.208.22:3306', 'stats', 'primal');
	if (!$link2) {
		die('Could not connect: ' . mysql_error());
	}
	mysql_select_db('primal', $link2);

	$query="select count(*) as total from patient as p join receive as r on p.puid=r.puid where r.tstartrec like '" . $intYearSearch . "-" . $intLastMonth . "%' and p.sdatetime between date_sub(r.tstartrec, interval 48 hour) and date_add(r.tstartrec, interval 48 hour) and p.sdatetime like '" . $intYearSearch . "-" . $intLastMonth . "%';";
	$result = mysql_query($query, $link2);
	$qdata=mysql_fetch_assoc($result);
	$intCurStudies["Magic"][$intLC] = $qdata['total'];

	$query="select count(*) as total from patient as p join receive as r on p.puid=r.puid where r.tstartrec like '" . $intYearSearch . "-" . $intLastMonth . "%' and r.tstartrec like '" . $intYearSearch . "-" . $intLastMonth . "%';";
	$result = mysql_query($query, $link2);
	$qdata=mysql_fetch_assoc($result);
	$intTotalStudies["Magic"][$intLC] = $qdata['total'];;
	$intTotalTotal[$intLC] += $intTotalStudies["Magic"][$intLC];

	mysql_close($link2);
}


echo <<<EOT
<HTML>
<!DOCTYPE !DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN"
            "http://www.w3.org/TR/html4/loose.dtd">
<HEAD>
	<!-- Written by Will Yonker-->
	<TITLE>PRIMAL Web Interface</TITLE>
	<link rel="stylesheet" href="default.css">
EOT;

echo '</HEAD>';
echo '<BODY>';
Display_Header2();
echo '<br>';

echo '<table>';
echo '<tr>';
echo '<th>Site Name</th>';
for($i=0; $i<12; $i++) {
	echo '<th colspan="3">' . date("m", strtotime("first day of -" . $i . " month")) . "/" . date("Y", strtotime("first day of -" . $i . " month")) . '</th>';
}
echo '</tr>';
echo '<tr><th></th>';
for($i=0; $i<12; $i++) {
	echo '<th>Cur</th><th>Prior</th><th><span style="font-weight:bold">Total</span></th>';
}
echo '</tr>';
foreach($names as $value) {
	echo '<tr>';
	echo '<td>' . $value . '</td>';
	for($i=0; $i<12; $i++) {
		echo '<td>' . $intCurStudies[$value][$i] . '</td><td>' . ($intTotalStudies[$value][$i]-$intCurStudies[$value][$i]) . '</td><td><span style="font-weight:bold">' . $intTotalStudies[$value][$i] . '</span></td>';
	}
	echo '</tr>';
}
echo '<tr>';
echo '<td>Total</td>';
for($i=0; $i<12; $i++) {
	echo '<td></td><td></td><td>' . $intTotalTotal[$i] . '</td>';
}
echo '</tr>';
echo '</table>';

echo '</BODY>';
echo '</HTML>';
?>
