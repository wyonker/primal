<?php
    //License GPLv3
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
if (isset($_SESSION['obj']) || isset($_SESSION['obj1']))
{
    unset($_SESSION['obj']);
    unset($_SESSION['obj1']);
}
unset($_SESSION['result']);
if(isset($_GET['zz'])) {
if ($_GET['zz'] == 1) {
	if(isset($_SESSION['PRIDESTHIP']))
		unset($_SESSION['PRIDESTHIP']);
	if(isset($_SESSION['PRIDESTHIP']))
		unset($_SESSION['PRIDESTHIP']);
	if(isset($_SESSION['PRIDESTCDCR']))
		unset($_SESSION['PRIDESTCDCR']);
	if(isset($_SESSION['PRIDESTAEC']))
		unset($_SESSION['PRIDESTAEC']);
	if(isset($_SESSION['PRIQRHIP']))
		unset($_SESSION['PRIQRHIP']);
	if(isset($_SESSION['PRIQRPORT']))
		unset($_SESSION['PRIQRPORT']);
	if(isset($_SESSION['PRIQRAEC']))
		unset($_SESSION['PRIQRAEC']);
	if(isset($_SESSION['PRIQRWAIT']))
		unset($_SESSION['PRIQRWAIT']);
	if(isset($_SESSION['PRIQRDESTHIP']))
		unset($_SESSION['PRIQRDESTHIP']);
	if(isset($_SESSION['PRIQRDESTPORT']))
		unset($_SESSION['PRIQRDESTPORT']);
	if(isset($_SESSION['PRIQRDESTAEC']))
		unset($_SESSION['PRIQRDESTAEC']);
	if(isset($_SESSION['PRITAGNAME']))
		unset($_SESSION['PRITAGNAME']);
	if(isset($_SESSION['PRITAGVAL']))
		unset($_SESSION['PRITAGVAL']);
	if(isset($_SESSION['PRITAGMALL']))
		unset($_SESSION['PRITAGMALL']);
	if(isset($_SESSION['PRITAGADD']))
		unset($_SESSION['PRITAGADD']);
	if(isset($_SESSION['PRIQRTAGNAME']))
		unset($_SESSION['PRIQRTAGNAME']);
	if(isset($_SESSION['PRIQRTAGVAL']))
		unset($_SESSION['PRIQRTAGVAL']);
	if(isset($_SESSION['PRIQRTAGMALL']))
		unset($_SESSION['PRIQRTAGMALL']);
	if(isset($_SESSION['PRIQRTAGADD']))
		unset($_SESSION['PRIQRTAGADD']);
	if(isset($_SESSION['PRINUM']))
		unset($_SESSION['PRINUM']);
	if(isset($_SESSION['PRIAET']))
		unset($_SESSION['PRIAET']);
	if(isset($_SESSION['PRIPORT']))
		unset($_SESSION['PRIPORT']);
	if(isset($_SESSION['PRILL']))
		unset($_SESSION['PRILL']);
	if(isset($_SESSION['PRILFOUT']))
		unset($_SESSION['PRILFOUT']);
	if(isset($_SESSION['PRILFIN']))
		unset($_SESSION['PRILFIN']);
	if(isset($_SESSION['PRILFPROC']))
		unset($_SESSION['PRILFPROC']);
	if(isset($_SESSION['PRILFQR']))
		unset($_SESSION['PRILFQR']);
	if(isset($_SESSION['PRIIF']))
		unset($_SESSION['PRIIF']);
	if(isset($_SESSION['PRIPROC']))
		unset($_SESSION['PRIPROC']);
	if(isset($_SESSION['PRIOUT']))
		unset($_SESSION['PRIOUT']);
	if(isset($_SESSION['PRISENT']))
		unset($_SESSION['PRISENT']);
	if(isset($_SESSION['PRIRET']))
		unset($_SESSION['PRIRET']);
	if(isset($_SESSION['PRIQRREC']))
		unset($_SESSION['PRIQRREC']);
	if(isset($_SESSION['PRIQRAGE']))
		unset($_SESSION['PRIQRAGE']);
	if(isset($_SESSION['PRIQRMAX']))
		unset($_SESSION['PRIQRMAX']);
	if(isset($_SESSION['PRIERROR']))
		unset($_SESSION['PRIERROR']);
	if(isset($_SESSION['PRILOGDIR']))
		unset($_SESSION['PRILOGDIR']);
	if(isset($_SESSION['PRICL']))
		unset($_SESSION['PRICL']);
	if(isset($_SESSION['PRIDCOM']))
		unset($_SESSION['PRIDCOM']);
	if(isset($_SESSION['PRIPASSTU']))
		unset($_SESSION['PRIPASSTU']);
	if(isset($_SESSION['PRIRECTO']))
		unset($_SESSION['PRIRECTO']);
	if(isset($_SESSION['PRIDUPE']))
		unset($_SESSION['PRIDUPE']);
	if(isset($_SESSION['PRIHOLD']))
		unset($_SESSION['PRIHOLD']);
	$handle = fopen("/etc/primal/primal.conf", "r") or die("Unable to open primal.conf file!");
	$intNumRec=0;
	$bolInRec=0;
    while (($buffer = fgets($handle, 4096)) !== false) {
		#Strip off comments
        $intPOS=strpos($buffer, "#");
        if($intPOS !== FALSE && $intPOS != 0) {
            $strLINE=substr($buffer, 0, ($intPOS - 1));
        } elseif($intPOS !== FALSE && $intPOS == 0) {
            $strLINE="";
        } else {
            $strLINE=$buffer;
        }
        $strLINE=trim($strLINE);
		if(stripos($strLINE, "<scp")  !== FALSE) {
			$intLC1=0;
			$bolInRec=1;
			$intPOS=stripos($strLINE, "<scp");
			$intePOS=stripos($strLINE, ">");
			$_SESSION['PRINUM'][$intNumRec]=substr($strLINE, ($intPOS+4), ($intePOS-$intPOS-4));
			$intNumTags=0;
			$intNumQRTags=0;
		} elseif (stripos($strLINE, "</scp")  !== FALSE) {
			$bolInRec=0;
			$intNumRec++;
		} elseif (stripos($strLINE, "PRIDESTHIP") !== FALSE) {
            $intPOS = stripos($strLINE, "PRIDESTHIP");
            $intPOS2=strpos($strLINE, "=");
            if ($intPOS2 !== FALSE) {
                $strDestNum=substr($strLINE, ($intPOS + 10), ($intPOS2 - ($intPOS + 10)));
                $strValue=substr($strLINE, $intPOS2+1);
                $_SESSION['PRIDESTHIP'][$intNumRec][$strDestNum] = $strValue;
            }
		} elseif (stripos($strLINE, "PRIDESTPORT") !== FALSE) {
			$intPOS = stripos($strLINE, "PRIDESTPORT");
			$intPOS2=strpos($strLINE, "=");
			if ($intPOS2 !== FALSE) {
				$strDestNum=substr($strLINE, ($intPOS + 11), ($intPOS2 - ($intPOS + 11)));
				$strValue=substr($strLINE, $intPOS2+1);
				$_SESSION['PRIDESTPORT'][$intNumRec][$strDestNum] = $strValue;
			}
		} elseif (stripos($strLINE, "PRIDESTCDCR") !== FALSE) {
			$intPOS = stripos($strLINE, "PRIDESTCDCR");
			$intPOS2=strpos($strLINE, "=");
			if ($intPOS2 !== FALSE) {
				$strDestNum=substr($strLINE, ($intPOS + 11), ($intPOS2 - ($intPOS + 11)));
				$strValue=substr($strLINE, $intPOS2+1);
				$_SESSION['PRIDESTCDCR'][$intNumRec][$strDestNum] = $strValue;
			}
		} elseif (stripos($strLINE, "PRIDESTAEC") !== FALSE) {
			$intPOS = stripos($strLINE, "PRIDESTAEC");
			$intPOS2=strpos($strLINE, "=");
			if ($intPOS2 !== FALSE) {
				$strDestNum=substr($strLINE, ($intPOS + 10), ($intPOS2 - ($intPOS + 10)));
				$strValue=substr($strLINE, $intPOS2+1);
				$_SESSION['PRIDESTAEC'][$intNumRec][$strDestNum] = $strValue;
			}
		} elseif (stripos($strLINE, "PRIQRHIP") !== FALSE) {
            $intPOS = stripos($strLINE, "PRIQRHIP");
            $intPOS2=strpos($strLINE, "=");
            if ($intPOS2 !== FALSE) {
                $strDestNum=substr($strLINE, ($intPOS + 8), ($intPOS2 - ($intPOS + 8)));
                $strValue=substr($strLINE, $intPOS2+1);
                $_SESSION['PRIQRHIP'][$intNumRec][$strDestNum] = $strValue;
            }
		} elseif (stripos($strLINE, "PRIQRDESTHIP") !== FALSE) {
            $intPOS = stripos($strLINE, "PRIQRDESTHIP");
            $intPOS2=strpos($strLINE, "=");
            if ($intPOS2 !== FALSE) {
                $strDestNum=substr($strLINE, ($intPOS + 12), ($intPOS2 - ($intPOS + 12)));
                $strValue=substr($strLINE, $intPOS2+1);
                $_SESSION['PRIQRDESTHIP'][$intNumRec][$strDestNum] = $strValue;
            }
		} elseif (stripos($strLINE, "PRIQRPORT") !== FALSE) {
			$intPOS = stripos($strLINE, "PRIQRPORT");
			$intPOS2=strpos($strLINE, "=");
			if ($intPOS2 !== FALSE) {
				$strDestNum=substr($strLINE, ($intPOS + 9), ($intPOS2 - ($intPOS + 9)));
				$strValue=substr($strLINE, $intPOS2+1);
				$_SESSION['PRIQRPORT'][$intNumRec][$strDestNum] = $strValue;
			}
		} elseif (stripos($strLINE, "PRIQRDESTPORT") !== FALSE) {
			$intPOS = stripos($strLINE, "PRIQRDESTPORT");
			$intPOS2=strpos($strLINE, "=");
			if ($intPOS2 !== FALSE) {
				$strDestNum=substr($strLINE, ($intPOS + 13), ($intPOS2 - ($intPOS + 13)));
				$strValue=substr($strLINE, $intPOS2+1);
				$_SESSION['PRIQRDESTPORT'][$intNumRec][$strDestNum] = $strValue;
			}
		} elseif (stripos($strLINE, "PRIQRAEC") !== FALSE) {
			$intPOS = stripos($strLINE, "PRIQRAEC");
			$intPOS2=strpos($strLINE, "=");
			if ($intPOS2 !== FALSE) {
				$strDestNum=substr($strLINE, ($intPOS + 8), ($intPOS2 - ($intPOS + 8)));
				$strValue=substr($strLINE, $intPOS2+1);
				$_SESSION['PRIQRAEC'][$intNumRec][$strDestNum] = $strValue;
			}
		} elseif (stripos($strLINE, "PRIQRDESTAEC") !== FALSE) {
			$intPOS = stripos($strLINE, "PRIQRDESTAEC");
			$intPOS2=strpos($strLINE, "=");
			if ($intPOS2 !== FALSE) {
				$strDestNum=substr($strLINE, ($intPOS + 12), ($intPOS2 - ($intPOS + 12)));
				$strValue=substr($strLINE, $intPOS2+1);
				$_SESSION['PRIQRDESTAEC'][$intNumRec][$strDestNum] = $strValue;
			}
		} elseif (stripos($strLINE, "PRIAET") !== FALSE) {
			$intPOS2=strpos($strLINE, "=");
			if ($intPOS2 !== FALSE) {
				$_SESSION['PRIAET'][$intNumRec]=substr($strLINE, $intPOS2+1);
			}
		} elseif (stripos($strLINE, "PRIPORT") !== FALSE) {
			$intPOS2=strpos($strLINE, "=");
			if ($intPOS2 !== FALSE) {
				$_SESSION['PRIPORT'][$intNumRec]=substr($strLINE, $intPOS2+1);
			}
		} elseif (stripos($strLINE, "PRILL") !== FALSE) {
			$intPOS2=strpos($strLINE, "=");
			if ($intPOS2 !== FALSE) {
				$_SESSION['PRILL'][$intNumRec][]=substr($strLINE, $intPOS2+1);
			}
		} elseif (stripos($strLINE, "PRILFOUT") !== FALSE) {
			$intPOS2=strpos($strLINE, "=");
			if ($intPOS2 !== FALSE) {
				$_SESSION['PRILFOUT'][$intNumRec][]=substr($strLINE, $intPOS2+1);
			}
		} elseif (stripos($strLINE, "PRILFIN") !== FALSE) {
			$intPOS2=strpos($strLINE, "=");
			if ($intPOS2 !== FALSE) {
				$_SESSION['PRILFIN'][$intNumRec][]=substr($strLINE, $intPOS2+1);
			}
		} elseif (stripos($strLINE, "PRILFPROC") !== FALSE) {
			$intPOS2=strpos($strLINE, "=");
			if ($intPOS2 !== FALSE) {
				$_SESSION['PRILFPROC'][$intNumRec]=substr($strLINE, $intPOS2+1);
			}
		} elseif (stripos($strLINE, "PRILFQR") !== FALSE) {
			$intPOS2=strpos($strLINE, "=");
			if ($intPOS2 !== FALSE) {
				$_SESSION['PRILFQR'][$intNumRec]=substr($strLINE, $intPOS2+1);
			}
		} elseif (stripos($strLINE, "PRIIF") !== FALSE) {
			$intPOS2=strpos($strLINE, "=");
			if ($intPOS2 !== FALSE) {
				$_SESSION['PRIIF'][$intNumRec]=substr($strLINE, $intPOS2+1);
			}
		} elseif (stripos($strLINE, "PRIPROC") !== FALSE) {
			$intPOS2=strpos($strLINE, "=");
			if ($intPOS2 !== FALSE) {
				$_SESSION['PRIPROC'][$intNumRec]=substr($strLINE, $intPOS2+1);
			}
		} elseif (stripos($strLINE, "PRIOUT") !== FALSE) {
			$intPOS2=strpos($strLINE, "=");
			if ($intPOS2 !== FALSE) {
				$_SESSION['PRIOUT'][$intNumRec]=substr($strLINE, $intPOS2+1);
			}
		} elseif (stripos($strLINE, "PRISENT") !== FALSE) {
			$intPOS2=strpos($strLINE, "=");
			if ($intPOS2 !== FALSE) {
				$_SESSION['PRISENT'][$intNumRec]=substr($strLINE, $intPOS2+1);
			}
		} elseif (stripos($strLINE, "PRIRET") !== FALSE) {
			$intPOS2=strpos($strLINE, "=");
			if ($intPOS2 !== FALSE) {
				$_SESSION['PRIRET'][$intNumRec]=substr($strLINE, $intPOS2+1);
			}
		} elseif (stripos($strLINE, "PRIQRREC") !== FALSE) {
			$intPOS2=strpos($strLINE, "=");
			if ($intPOS2 !== FALSE) {
				$_SESSION['PRIQRREC'][$intNumRec]=substr($strLINE, $intPOS2+1);
			}
		} elseif (stripos($strLINE, "PRIQRAGE") !== FALSE) {
			$intPOS2=strpos($strLINE, "=");
			if ($intPOS2 !== FALSE) {
				$_SESSION['PRIQRAGE'][$intNumRec]=substr($strLINE, $intPOS2+1);
			}
		} elseif (stripos($strLINE, "PRIQRWAIT") !== FALSE) {
			$intPOS2=strpos($strLINE, "=");
			if ($intPOS2 !== FALSE) {
				$_SESSION['PRIQRWAIT'][$intNumRec]=substr($strLINE, $intPOS2+1);
			}
		} elseif (stripos($strLINE, "PRIQRMAX") !== FALSE) {
			$intPOS2=strpos($strLINE, "=");
			if ($intPOS2 !== FALSE) {
				$_SESSION['PRIQRMAX'][$intNumRec]=substr($strLINE, $intPOS2+1);
			}
		} elseif (stripos($strLINE, "PRIERROR") !== FALSE) {
			$intPOS2=strpos($strLINE, "=");
			if ($intPOS2 !== FALSE) {
				$_SESSION['PRIERROR'][$intNumRec]=substr($strLINE, $intPOS2+1);
			}
		} elseif (stripos($strLINE, "PRILOGDIR") !== FALSE) {
			$intPOS2=strpos($strLINE, "=");
			if ($intPOS2 !== FALSE) {
				$_SESSION['PRILOGDIR'][$intNumRec]=substr($strLINE, $intPOS2+1);
			}
		} elseif (stripos($strLINE, "PRICL") !== FALSE) {
			$intPOS2=strpos($strLINE, "=");
			if ($intPOS2 !== FALSE) {
				$_SESSION['PRICL'][$intNumRec]=substr($strLINE, $intPOS2+1);
			}
		} elseif (stripos($strLINE, "PRIDCOM") !== FALSE) {
			$intPOS2=strpos($strLINE, "=");
			if ($intPOS2 !== FALSE) {
				$_SESSION['PRIDCOM'][$intNumRec]=substr($strLINE, $intPOS2+1);
			}
		} elseif (stripos($strLINE, "PRIPASSTU") !== FALSE) {
			$intPOS2=strpos($strLINE, "=");
			if ($intPOS2 !== FALSE) {
				$_SESSION['PRIPASSTU'][$intNumRec]=substr($strLINE, $intPOS2+1);
			}
		} elseif (stripos($strLINE, "PRIRECTO") !== FALSE) {
			$intPOS2=strpos($strLINE, "=");
			if ($intPOS2 !== FALSE) {
				$_SESSION['PRIRECTO'][$intNumRec]=substr($strLINE, $intPOS2+1);
			}
		} elseif (stripos($strLINE, "PRIDUPE") !== FALSE) {
			$intPOS2=strpos($strLINE, "=");
			if ($intPOS2 !== FALSE) {
				$_SESSION['PRIDUPE'][$intNumRec]=substr($strLINE, $intPOS2+1);
			}
		} elseif (stripos($strLINE, "PRIHOLD") !== FALSE) {
			$intPOS2=strpos($strLINE, "=");
			if ($intPOS2 !== FALSE) {
				$_SESSION['PRIHOLD'][$intNumRec]=substr($strLINE, $intPOS2+1);
			}
		} elseif (stripos($strLINE, "PRITAG") !== FALSE) {
			$intPOS2=strpos($strLINE, "=");
			if ($intPOS2 !== FALSE) {
				$strTemp=substr($strLINE, $intPOS2+1);
				$strTemp2=explode(":", $strTemp);
				$_SESSION['PRITAGNAME'][$intNumRec][$intNumTags]=$strTemp2[0];
				$_SESSION['PRITAGVAL'][$intNumRec][$intNumTags]=$strTemp2[1];
				$_SESSION['PRITAGMALL'][$intNumRec][$intNumTags]=$strTemp2[2];
				$_SESSION['PRITAGADD'][$intNumRec][$intNumTags]=$strTemp2[3];
				$intNumTags++;
			}
		} elseif (stripos($strLINE, "PRIQRTAG") !== FALSE) {
			$intPOS2=strpos($strLINE, "=");
			if ($intPOS2 !== FALSE) {
				$strTemp=substr($strLINE, $intPOS2+1);
				$strTemp2=explode(":", $strTemp);
				$_SESSION['PRIQRTAGNAME'][$intNumRec][$intNumQRTags]=$strTemp2[0];
				$_SESSION['PRIQRTAGVAL'][$intNumRec][$intNumQRTags]=$strTemp2[1];
				$_SESSION['PRIQRTAGGROUP'][$intNumRec][$intNumQRTags]=$strTemp2[2];
				$intNumQRTags++;
			}
		}
	}
	fclose($handle);
}
} 
if($_SERVER['REQUEST_METHOD'] == 'POST') {
	$_SESSION['PRIAET'][$_GET['v']] = $_POST['PRIAET'];
	$_SESSION['PRIPORT'][$_GET['v']] = $_POST['PRIPORT'];
	$_SESSION['PRILL'][$_GET['v']] = $_POST['PRILL'];
	$_SESSION['PRILFOUT'][$_GET['v']] = $_POST['PRILFOUT'];
	$_SESSION['PRILFIN'][$_GET['v']] = $_POST['PRILFIN'];
	$_SESSION['PRILFPROC'][$_GET['v']] = $_POST['PRILFPROC'];
	$_SESSION['PRILFQR'][$_GET['v']] = $_POST['PRILFQR'];
	$_SESSION['PRIIF'][$_GET['v']] = $_POST['PRIIF'];
	$_SESSION['PRIPROC'][$_GET['v']] = $_POST['PRIPROC'];
	$_SESSION['PRIOUT'][$_GET['v']] = $_POST['PRIOUT'];
	$_SESSION['PRISENT'][$_GET['v']] = $_POST['PRISENT'];
	$_SESSION['PRIRET'][$_GET['v']] = $_POST['PRIRET'];
	$_SESSION['PRIQRREC'][$_GET['v']] = $_POST['PRIQRREC'];
	$_SESSION['PRIQRAGE'][$_GET['v']] = $_POST['PRIQRAGE'];
	$_SESSION['PRIQRWAIT'][$_GET['v']] = $_POST['PRIQRWAIT'];
	$_SESSION['PRIQRMAX'][$_GET['v']] = $_POST['PRIQRMAX'];
	$_SESSION['PRIERROR'][$_GET['v']] = $_POST['PRIERROR'];
	$_SESSION['PRILOGDIR'][$_GET['v']] = $_POST['PRILOGDIR'];
	$_SESSION['PRICL'][$_GET['v']] = $_POST['PRICL'];
	$_SESSION['PRIDCOM'][$_GET['v']] = $_POST['PRIDCOM'];
	$_SESSION['PRIPASSTU'][$_GET['v']] = $_POST['PRIPASSTU'];
	$_SESSION['PRIRECTO'][$_GET['v']] = $_POST['PRIRECTO'];
	$_SESSION['PRIDUPE'][$_GET['v']] = $_POST['PRIDUPE'];
	$_SESSION['PRIHOLD'][$_GET['v']] = $_POST['PRIHOLD'];
    $intLC2=0;
    while(isset($_POST['PRIDESTHIP' . $intLC2])) {
        $_SESSION['PRIDESTHIP'][$_GET['v']][$intLC2] = $_POST['PRIDESTHIP' . $intLC2];
        $_SESSION['PRIDESTPORT'][$_GET['v']][$intLC2] = $_POST['PRIDESTPORT' . $intLC2];
        $_SESSION['PRIDESTCDCR'][$_GET['v']][$intLC2] = $_POST['PRIDESTCDCR' . $intLC2];
        $_SESSION['PRIDESTAEC'][$_GET['v']][$intLC2] = $_POST['PRIDESTAEC' . $intLC2];
        $intLC2++;
    }
    $intLC2=0;
    while(isset($_POST['PRIQRHIP' . $intLC2])) {
        $_SESSION['PRIQRHIP'][$_GET['v']][$intLC2] = $_POST['PRIQRHIP' . $intLC2];
        $_SESSION['PRIQRDESTHIP'][$_GET['v']][$intLC2] = $_POST['PRIQRDESTHIP' . $intLC2];
        $_SESSION['PRIQRPORT'][$_GET['v']][$intLC2] = $_POST['PRIQRPORT' . $intLC2];
        $_SESSION['PRIQRDESTPORT'][$_GET['v']][$intLC2] = $_POST['PRIQRDESTPORT' . $intLC2];
        $_SESSION['PRIQRAEC'][$_GET['v']][$intLC2] = $_POST['PRIQRAEC' . $intLC2];
        $_SESSION['PRIQRDESTAEC'][$_GET['v']][$intLC2] = $_POST['PRIQRDESTAEC' . $intLC2];
        $intLC2++;
    }
    $intLC2=0;
    while(isset($_POST['PRITAGNAME' . $intLC2])) {
        $_SESSION['PRITAGNAME'][$_GET['v']][$intLC2] = $_POST['PRITAGNAME' . $intLC2];
        $_SESSION['PRITAGVAL'][$_GET['v']][$intLC2] = $_POST['PRITAGVAL' . $intLC2];
        $_SESSION['PRITAGMALL'][$_GET['v']][$intLC2] = $_POST['PRITAGMALL' . $intLC2];
        $_SESSION['PRITAGADD'][$_GET['v']][$intLC2] = $_POST['PRITAGADD' . $intLC2];
        $intLC2++;
    }
    $intLC2=0;
    while(isset($_POST['PRIQRTAGNAME' . $intLC2])) {
        $_SESSION['PRIQRTAGNAME'][$_GET['v']][$intLC2] = $_POST['PRIQRTAGNAME' . $intLC2];
        $_SESSION['PRIQRTAGVAL'][$_GET['v']][$intLC2] = $_POST['PRIQRTAGVAL' . $intLC2];
        $_SESSION['PRIQRTAGGROUP'][$_GET['v']][$intLC2] = $_POST['PRIQRTAGGROUP' . $intLC2];
        $intLC2++;
    }
} elseif(isset($_GET['ad'])) {
	$_SESSION['PRIDESTHIP'][$_GET['v']][$_GET['ad']] = " ";
	$_SESSION['PRIDESTPORT'][$_GET['v']][$_GET['ad']] = " ";
	$_SESSION['PRIDESTCDCR'][$_GET['v']][$_GET['ad']] = " ";
	$_SESSION['PRIDESTAEC'][$_GET['v']][$_GET['ad']] = " ";
} elseif(isset($_GET['aq'])) {
	$_SESSION['PRIQRHIP'][$_GET['v']][$_GET['aq']] = " ";
	$_SESSION['PRIQRPORT'][$_GET['v']][$_GET['aq']] = " ";
	$_SESSION['PRIQRAEC'][$_GET['v']][$_GET['aq']] = " ";
} elseif(isset($_GET['dd'])) {
	$intLC1=0;
	while(isset($_SESSION['PRIDESTHIP'][$_GET['v']][$intLC1])) {
		if($intLC1 == $_GET['dd']) {
			$_SESSION['PRIDESTHIP'][$_GET['v']][$intLC1] = " ";
			$_SESSION['PRIDESTPORT'][$_GET['v']][$intLC1] = " ";
			$_SESSION['PRIDESTCDCR'][$_GET['v']][$intLC1] = " ";
			$_SESSION['PRIDESTAEC'][$_GET['v']][$intLC1] = " ";
		} elseif($intLC1 > $_GET['dd']) {
			$_SESSION['PRIDESTHIP'][$_GET['v']][$intLC1 - 1] = $_SESSION['PRIDESTHIP'][$_GET['v']][$intLC1];
			$_SESSION['PRIDESTPORT'][$_GET['v']][$intLC1 - 1] = $_SESSION['PRIDESTPORT'][$_GET['v']][$intLC1];
			$_SESSION['PRIDESTCDCR'][$_GET['v']][$intLC1 - 1] = $_SESSION['PRIDESTCDCR'][$_GET['v']][$intLC1];
			$_SESSION['PRIDESTAEC'][$_GET['v']][$intLC1 - 1] = $_SESSION['PRIDESTAEC'][$_GET['v']][$intLC1];
		}
		if(! isset($_SESSION['PRIDESTHIP'][$_GET['v']][$intLC1 + 1])) {
			unset($_SESSION['PRIDESTHIP'][$_GET['v']][$intLC1]);
			unset($_SESSION['PRIDESTPORT'][$_GET['v']][$intLC1]);
			unset($_SESSION['PRIDESTCDCR'][$_GET['v']][$intLC1]);
			unset($_SESSION['PRIDESTAEC'][$_GET['v']][$intLC1]);
		}
		$intLC1++;
	}
} elseif(isset($_GET['dq'])) {
	$intLC1=0;
	while(isset($_SESSION['PRIQRHIP'][$_GET['v']][$intLC1])) {
		if($intLC1 == $_GET['dq']) {
			$_SESSION['PRIQRHIP'][$_GET['v']][$intLC1] = " ";
			$_SESSION['PRIQRDESTHIP'][$_GET['v']][$intLC1] = " ";
			$_SESSION['PRIQRPORT'][$_GET['v']][$intLC1] = " ";
			$_SESSION['PRIQRDESTPORT'][$_GET['v']][$intLC1] = " ";
			$_SESSION['PRIQRAEC'][$_GET['v']][$intLC1] = " ";
			$_SESSION['PRIQRDESTAEC'][$_GET['v']][$intLC1] = " ";
		} elseif($intLC1 > $_GET['dq']) {
			$_SESSION['PRIQRHIP'][$_GET['v']][$intLC1 - 1] = $_SESSION['PRIQRHIP'][$_GET['v']][$intLC1];
			$_SESSION['PRIQRDESTHIP'][$_GET['v']][$intLC1 - 1] = $_SESSION['PRIQRDESTHIP'][$_GET['v']][$intLC1];
			$_SESSION['PRIQRPORT'][$_GET['v']][$intLC1 - 1] = $_SESSION['PRIQRPORT'][$_GET['v']][$intLC1];
			$_SESSION['PRIQRDESTPORT'][$_GET['v']][$intLC1 - 1] = $_SESSION['PRIQRDESTPORT'][$_GET['v']][$intLC1];
			$_SESSION['PRIQRAEC'][$_GET['v']][$intLC1 - 1] = $_SESSION['PRIQRAEC'][$_GET['v']][$intLC1];
			$_SESSION['PRIQRDESTAEC'][$_GET['v']][$intLC1 - 1] = $_SESSION['PRIQRDESTAEC'][$_GET['v']][$intLC1];
		}
		if(! isset($_SESSION['PRIQRHIP'][$_GET['v']][$intLC1 + 1])) {
			unset($_SESSION['PRIQRHIP'][$_GET['v']][$intLC1]);
			unset($_SESSION['PRIQRDESTHIP'][$_GET['v']][$intLC1]);
			unset($_SESSION['PRIQRPORT'][$_GET['v']][$intLC1]);
			unset($_SESSION['PRIQRDESTPORT'][$_GET['v']][$intLC1]);
			unset($_SESSION['PRIQRAEC'][$_GET['v']][$intLC1]);
			unset($_SESSION['PRIQRDESTAEC'][$_GET['v']][$intLC1]);
		}
		$intLC1++;
	}
} elseif(isset($_GET['at'])) {
	$_SESSION['PRITAGNAME'][$_GET['v']][$_GET['at']] = " ";
	$_SESSION['PRITAGVAL'][$_GET['v']][$_GET['at']] = " ";
	$_SESSION['PRITAGMALL'][$_GET['v']][$_GET['at']] = " ";
	$_SESSION['PRITAGADD'][$_GET['v']][$_GET['at']] = " ";
} elseif(isset($_GET['aqt'])) {
	$_SESSION['PRIQRTAGNAME'][$_GET['v']][$_GET['aqt']] = " ";
	$_SESSION['PRIQRTAGVAL'][$_GET['v']][$_GET['aqt']] = " ";
} elseif(isset($_GET['dt'])) {
	$intLC1=0;
	while(isset($_SESSION['PRITAGNAME'][$_GET['v']][$intLC1])) {
		if($intLC1 == $_GET['dt']) {
			$_SESSION['PRITAGNAME'][$_GET['v']][$intLC1] = " ";
			$_SESSION['PRITAGVAL'][$_GET['v']][$intLC1] = " ";
			$_SESSION['PRITAGMALL'][$_GET['v']][$intLC1] = " ";
			$_SESSION['PRITAGADD'][$_GET['v']][$intLC1] = " ";
		} elseif($intLC1 > $_GET['dt']) {
			$_SESSION['PRITAGNAME'][$_GET['v']][$intLC1 - 1] = $_SESSION['PRITAGNAME'][$_GET['v']][$intLC1];
			$_SESSION['PRITAGVAL'][$_GET['v']][$intLC1 - 1] = $_SESSION['PRITAGVAL'][$_GET['v']][$intLC1];
			$_SESSION['PRITAGMALL'][$_GET['v']][$intLC1 - 1] = $_SESSION['PRITAGMALL'][$_GET['v']][$intLC1];
			$_SESSION['PRITAGADD'][$_GET['v']][$intLC1 - 1] = $_SESSION['PRITAGADD'][$_GET['v']][$intLC1];
		}
		if(! isset($_SESSION['PRITAGNAME'][$_GET['v']][$intLC1 + 1])) {
			unset($_SESSION['PRITAGNAME'][$_GET['v']][$intLC1]);
			unset($_SESSION['PRITAGVAL'][$_GET['v']][$intLC1]);
			unset($_SESSION['PRITAGMALL'][$_GET['v']][$intLC1]);
			unset($_SESSION['PRITAGADD'][$_GET['v']][$intLC1]);
		}
		$intLC1++;
	}
} elseif(isset($_GET['dqt'])) {
	$intLC1=0;
	while(isset($_SESSION['PRIQRTAGNAME'][$_GET['v']][$intLC1])) {
		if($intLC1 == $_GET['dqt']) {
			$_SESSION['PRIQRTAGNAME'][$_GET['v']][$intLC1] = " ";
			$_SESSION['PRIQRTAGVAL'][$_GET['v']][$intLC1] = " ";
			$_SESSION['PRIQRTAGGROUP'][$_GET['v']][$intLC1] = " ";
		} elseif($intLC1 > $_GET['dqt']) {
			$_SESSION['PRIQRTAGNAME'][$_GET['v']][$intLC1 - 1] = $_SESSION['PRIQRTAGNAME'][$_GET['v']][$intLC1];
			$_SESSION['PRIQRTAGVAL'][$_GET['v']][$intLC1 - 1] = $_SESSION['PRIQRTAGVAL'][$_GET['v']][$intLC1];
			$_SESSION['PRIQRTAGGROUP'][$_GET['v']][$intLC1 - 1] = $_SESSION['PRIQRTAGGROUP'][$_GET['v']][$intLC1];
		}
		if(! isset($_SESSION['PRIQRTAGNAME'][$_GET['v']][$intLC1 + 1])) {
			unset($_SESSION['PRIQRTAGNAME'][$_GET['v']][$intLC1]);
			unset($_SESSION['PRIQRTAGVAL'][$_GET['v']][$intLC1]);
			unset($_SESSION['PRIQRTAGGROUP'][$_GET['v']][$intLC1]);
		}
		$intLC1++;
	}
} elseif(isset($_GET['ar'])) {
	$_SESSION['PRINUM'][$_GET['ar']] = $_GET['ar']+1;
	$_SESSION['PRIAET'][$_GET['ar']] = " ";
	$_SESSION['PRIPORT'][$_GET['ar']] = " ";
	$_SESSION['PRILL'][$_GET['ar']] = " ";
	$_SESSION['PRILFOUT'][$_GET['ar']] = " ";
	$_SESSION['PRILFIN'][$_GET['ar']] = " ";
	$_SESSION['PRILFPROC'][$_GET['ar']] = " ";
	$_SESSION['PRILFQR'][$_GET['ar']] = " ";
	$_SESSION['PRIIF'][$_GET['ar']] = " ";
	$_SESSION['PRIPROC'][$_GET['ar']] = " ";
	$_SESSION['PRIOUT'][$_GET['ar']] = " ";
	$_SESSION['PRISENT'][$_GET['ar']] = " ";
	$_SESSION['PRIQRREC'][$_GET['ar']] = " ";
	$_SESSION['PRIQRAGE'][$_GET['ar']] = " ";
	$_SESSION['PRIQRWAIT'][$_GET['ar']] = " ";
	$_SESSION['PRIQRMAX'][$_GET['ar']] = " ";
	$_SESSION['PRIRET'][$_GET['ar']] = " ";
	$_SESSION['PRIERROR'][$_GET['ar']] = " ";
	$_SESSION['PRILOGDIR'][$_GET['ar']] = " ";
	$_SESSION['PRICL'][$_GET['ar']] = " ";
	$_SESSION['PRIDCOM'][$_GET['ar']] = " ";
	$_SESSION['PRIPASSTU'][$_GET['ar']] = " ";
	$_SESSION['PRIRECTO'][$_GET['ar']] = " ";
	$_SESSION['PRIDUPE'][$_GET['ar']] = " ";
	$_SESSION['PRIHOLD'][$_GET['ar']] = " ";
} elseif(isset($_GET['dr'])) {
	$intLC1=0;
	while(isset($_SESSION['PRINUM'][$intLC1])) {
		if($intLC1 == $_GET['dr']) {
			$_SESSION['PRINUM'][$intLC1] = " ";
			$_SESSION['PRIAET'][$intLC1] = " ";
			$_SESSION['PRIPORT'][$intLC1] = " ";
			$_SESSION['PRILL'][$intLC1] = " ";
			$_SESSION['PRILFOUT'][$intLC1] = " ";
			$_SESSION['PRILFIN'][$intLC1] = " ";
			$_SESSION['PRILFPROC'][$intLC1] = " ";
			$_SESSION['PRILFQR'][$intLC1] = " ";
			$_SESSION['PRIIF'][$intLC1] = " ";
			$_SESSION['PRIPROC'][$intLC1] = " ";
			$_SESSION['PRIOUT'][$intLC1] = " ";
			$_SESSION['PRISENT'][$intLC1] = " ";
			$_SESSION['PRIQRREC'][$intLC1] = " ";
			$_SESSION['PRIQRAGE'][$intLC1] = " ";
			$_SESSION['PRIQRWAIT'][$intLC1] = " ";
			$_SESSION['PRIQRMAX'][$intLC1] = " ";
			$_SESSION['PRIRET'][$intLC1] = " ";
			$_SESSION['PRIERROR'][$intLC1] = " ";
			$_SESSION['PRILOGDIR'][$intLC1] = " ";
			$_SESSION['PRICL'][$intLC1] = " ";
			$_SESSION['PRIDCOM'][$intLC1] = " ";
			$_SESSION['PRIPASSTU'][$intLC1] = " ";
			$_SESSION['PRIRECTO'][$intLC1] = " ";
			$_SESSION['PRIDUPE'][$intLC1] = " ";
			$_SESSION['PRIHOLD'][$intLC1] = " ";
			$intLC2=0;
			while(isset($_SESSION['PRIDESTHIP'][$intLC1][$intLC2])) {
				unset($_SESSION['PRIDESTHIP'][$intLC1][$intLC2]);;
				unset($_SESSION['PRIDESTPORT'][$intLC1][$intLC2]);;
				unset($_SESSION['PRIDESTCDCR'][$intLC1][$intLC2]);;
				unset($_SESSION['PRIDESTAEC'][$intLC1][$intLC2]);;
				$intLC2++;
			}
			$intLC2=0;
			while(isset($_SESSION['PRIQRHIP'][$intLC1][$intLC2])) {
				unset($_SESSION['PRIQRHIP'][$intLC1][$intLC2]);;
				unset($_SESSION['PRIQRDESTHIP'][$intLC1][$intLC2]);;
				unset($_SESSION['PRIQRPORT'][$intLC1][$intLC2]);;
				unset($_SESSION['PRIQRDESTPORT'][$intLC1][$intLC2]);;
				unset($_SESSION['PRIQRAEC'][$intLC1][$intLC2]);;
				unset($_SESSION['PRIQRDESTAEC'][$intLC1][$intLC2]);;
				$intLC2++;
			}
			$intLC2=0;
			while(isset($_SESSION['PRITAGNAME'][$intLC1][$intLC2])) {
				unset($_SESSION['PRITAGNAME'][$intLC1][$intLC2]);
				unset($_SESSION['PRITAGVAL'][$intLC1][$intLC2]);
				unset($_SESSION['PRITAGMALL'][$intLC1][$intLC2]);
				unset($_SESSION['PRITAGADD'][$intLC1][$intLC2]);
			}
			$intLC2=0;
			while(isset($_SESSION['PRIQRTAGNAME'][$intLC1][$intLC2])) {
				unset($_SESSION['PRIQRTAGNAME'][$intLC1][$intLC2]);
				unset($_SESSION['PRIQRTAGVAL'][$intLC1][$intLC2]);
				unset($_SESSION['PRIQRTAGGROUP'][$intLC1][$intLC2]);
			}
		} elseif($intLC1 > $_GET['dr']) {
			$_SESSION['PRINUM'][$intLC1 - 1] = $intLC1;
			$_SESSION['PRIAET'][$intLC1 - 1] = $_SESSION['PRIAET'][$intLC1];
			$_SESSION['PRIPORT'][$intLC1 - 1] = $_SESSION['PRIPORT'][$intLC1];
			$_SESSION['PRILL'][$intLC1 - 1] = $_SESSION['PRILL'][$intLC1];
			$_SESSION['PRILFOUT'][$intLC1 - 1] = $_SESSION['PRILFOUT'][$intLC1];
			$_SESSION['PRILFIN'][$intLC1 - 1] = $_SESSION['PRILFIN'][$intLC1];
			$_SESSION['PRILFPROC'][$intLC1 - 1] = $_SESSION['PRILFPROC'][$intLC1];
			$_SESSION['PRILFQR'][$intLC1 - 1] = $_SESSION['PRILFPROC'][$intLC1];
			$_SESSION['PRIIF'][$intLC1 - 1] = $_SESSION['PRIIF'][$intLC1];
			$_SESSION['PRIPROC'][$intLC1 - 1] = $_SESSION['PRIPROC'][$intLC1];
			$_SESSION['PRIOUT'][$intLC1 - 1] = $_SESSION['PRIOUT'][$intLC1];
			$_SESSION['PRIQRREC'][$intLC1 - 1] = $_SESSION['PRIQRREC'][$intLC1];
			$_SESSION['PRIQRAGE'][$intLC1 - 1] = $_SESSION['PRIQRAGE'][$intLC1];
			$_SESSION['PRIQRWAIT'][$intLC1 - 1] = $_SESSION['PRIQRWAIT'][$intLC1];
			$_SESSION['PRIQRMAX'][$intLC1 - 1] = $_SESSION['PRIQRMAX'][$intLC1];
			$_SESSION['PRISENT'][$intLC1 - 1] = $_SESSION['PRISENT'][$intLC1];
			$_SESSION['PRIRET'][$intLC1 - 1] = $_SESSION['PRIRET'][$intLC1];
			$_SESSION['PRIERROR'][$intLC1 - 1] = $_SESSION['PRIERROR'][$intLC1];
			$_SESSION['PRILOGDIR'][$intLC1 - 1] = $_SESSION['PRILOGDIR'][$intLC1];
			$_SESSION['PRICL'][$intLC1 - 1] = $_SESSION['PRICL'][$intLC1];
			$_SESSION['PRIDCOM'][$intLC1 - 1] = $_SESSION['PRIDCOM'][$intLC1];
			$_SESSION['PRIPASSTU'][$intLC1 - 1] = $_SESSION['PRIPASSTU'][$intLC1];
			$_SESSION['PRIRECTO'][$intLC1 - 1] = $_SESSION['PRIRECTO'][$intLC1];
			$_SESSION['PRIDUPE'][$intLC1 - 1] = $_SESSION['PRIDUPE'][$intLC1];
			$_SESSION['PRIHOLD'][$intLC1 -1] = $_SESSION['PRIHOLD'][$intLC1];
			$intLC2=0;
			while(isset($_SESSION['PRIDESTHIP'][$intLC1][$intLC2])) {
				$_SESSION['PRIDESTHIP'][$intLC1 - 1][$intLC2] = $_SESSION['PRIDESTHIP'][$intLC1][$intLC2];
				$_SESSION['PRIDESTPORT'][$intLC1 - 1][$intLC2] = $_SESSION['PRIDESTPORT'][$intLC1][$intLC2];
				$_SESSION['PRIDESTCDCR'][$intLC1 - 1][$intLC2] = $_SESSION['PRIDESTCDCR'][$intLC1][$intLC2];
				$_SESSION['PRIDESTAEC'][$intLC1 - 1][$intLC2] = $_SESSION['PRIDESTAEC'][$intLC1][$intLC2];
				$intLC2++;
			}
			$intLC2=0;
			while(isset($_SESSION['PRIQRHIP'][$intLC1][$intLC2])) {
				$_SESSION['PRIQRHIP'][$intLC1 - 1][$intLC2] = $_SESSION['PRIQRHIP'][$intLC1][$intLC2];
				$_SESSION['PRIQRDESTHIP'][$intLC1 - 1][$intLC2] = $_SESSION['PRIQRDESTHIP'][$intLC1][$intLC2];
				$_SESSION['PRIQRPORT'][$intLC1 - 1][$intLC2] = $_SESSION['PRIQRPORT'][$intLC1][$intLC2];
				$_SESSION['PRIQRDESTPORT'][$intLC1 - 1][$intLC2] = $_SESSION['PRIQRDESTPORT'][$intLC1][$intLC2];
				$_SESSION['PRIQRAEC'][$intLC1 - 1][$intLC2] = $_SESSION['PRIQRAEC'][$intLC1][$intLC2];
				$_SESSION['PRIQRDESTAEC'][$intLC1 - 1][$intLC2] = $_SESSION['PRIQRDESTAEC'][$intLC1][$intLC2];
				$intLC2++;
			}
			$intLC2=0;
			while(isset($_SESSION['PRITAGNAME'][$intLC1][$intLC2])) {
				$_SESSION['PRITAGNAME'][$intLC1 - 1][$intLC2] = $_SESSION['PRITAGNAME'][$intLC1][$intLC2];
				$_SESSION['PRITAGVAL'][$intLC1 - 1][$intLC2] = $_SESSION['PRITAGVAL'][$intLC1][$intLC2];
				$_SESSION['PRITAGMALL'][$intLC1 - 1][$intLC2] = $_SESSION['PRITAGMALL'][$intLC1][$intLC2];
				$_SESSION['PRITAGADD'][$intLC1 - 1][$intLC2] = $_SESSION['PRITAGADD'][$intLC1][$intLC2];
				$intLC2++;
			}
			$intLC2=0;
			while(isset($_SESSION['PRIQRTAGNAME'][$intLC1][$intLC2])) {
				$_SESSION['PRIQRTAGNAME'][$intLC1 - 1][$intLC2] = $_SESSION['PRIQRTAGNAME'][$intLC1][$intLC2];
				$_SESSION['PRIQRTAGVAL'][$intLC1 - 1][$intLC2] = $_SESSION['PRIQRTAGVAL'][$intLC1][$intLC2];
				$_SESSION['PRIQRTAGGROUP'][$intLC1 - 1][$intLC2] = $_SESSION['PRIQRTAGGROUP'][$intLC1][$intLC2];
				$intLC2++;
			}
		}
		if(! isset($_SESSION['PRINUM'][$intLC1 + 1])) {
			unset($_SESSION['PRINUM'][$intLC1]);
			unset($_SESSION['PRIAET'][$intLC1]);
			unset($_SESSION['PRIPORT'][$intLC1]);
			unset($_SESSION['PRILL'][$intLC1]);
			unset($_SESSION['PRILFOUT'][$intLC1]);
			unset($_SESSION['PRILFIN'][$intLC1]);
			unset($_SESSION['PRILFPROC'][$intLC1]);
			unset($_SESSION['PRILFQR'][$intLC1]);
			unset($_SESSION['PRIIF'][$intLC1]);
			unset($_SESSION['PRIPROC'][$intLC1]);
			unset($_SESSION['PRIOUT'][$intLC1]);
			unset($_SESSION['PRISENT'][$intLC1]);
			unset($_SESSION['PRIQRREC'][$intLC1]);
			unset($_SESSION['PRIQRMAX'][$intLC1]);
			unset($_SESSION['PRIQRAGE'][$intLC1]);
			unset($_SESSION['PRIQRWAIT'][$intLC1]);
			unset($_SESSION['PRIRET'][$intLC1]);
			unset($_SESSION['PRIERROR'][$intLC1]);
			unset($_SESSION['PRILOGDIR'][$intLC1]);
			unset($_SESSION['PRICL'][$intLC1]);
			unset($_SESSION['PRIDCOM'][$intLC1]);
			unset($_SESSION['PRIPASSTU'][$intLC1]);
			unset($_SESSION['PRIRECTO'][$intLC1]);
			unset($_SESSION['PRIDUPE'][$intLC1]);
			unset($_SESSION['PRIHOLD'][$intLC1]);
			$intLC2=0;
			while(isset($_SESSION['PRIDESTHIP'][$intLC1][$intLC2])) {
				unset($_SESSION['PRIDESTHIP'][$intLC1][$intLC2]);
				unset($_SESSION['PRIDESTPORT'][$intLC1][$intLC2]);
				unset($_SESSION['PRIDESTCDCR'][$intLC1][$intLC2]);
				unset($_SESSION['PRIDESTAEC'][$intLC1][$intLC2]);
				$intLC2++;
			}
			$intLC2=0;
			while(isset($_SESSION['PRIQRHIP'][$intLC1][$intLC2])) {
				unset($_SESSION['PRIQRHIP'][$intLC1][$intLC2]);
				unset($_SESSION['PRIQRDESTHIP'][$intLC1][$intLC2]);
				unset($_SESSION['PRIQRPORT'][$intLC1][$intLC2]);
				unset($_SESSION['PRIQRDESTPORT'][$intLC1][$intLC2]);
				unset($_SESSION['PRIQRAEC'][$intLC1][$intLC2]);
				unset($_SESSION['PRIQRDESTAEC'][$intLC1][$intLC2]);
				$intLC2++;
			}
			$intLC2=0;
			while(isset($_SESSION['PRITAGNAME'][$intLC1][$intLC2])) {
				unset($_SESSION['PRITAGNAME'][$intLC1][$intLC2]);
				unset($_SESSION['PRITAGVAL'][$intLC1][$intLC2]);
				unset($_SESSION['PRITAGMALL'][$intLC1][$intLC2]);
				unset($_SESSION['PRITAGADD'][$intLC1][$intLC2]);
				$intLC2++;
			}
			$intLC2=0;
			while(isset($_SESSION['PRIQRTAGNAME'][$intLC1][$intLC2])) {
				unset($_SESSION['PRIQRTAGNAME'][$intLC1][$intLC2]);
				unset($_SESSION['PRIQRTAGVAL'][$intLC1][$intLC2]);
				unset($_SESSION['PRIQRTAGGROUP'][$intLC1][$intLC2]);
				$intLC2++;
			}
		}
		$intLC1++;
	}
} elseif(isset($_GET['save'])) {
	if($_GET['save'] == 1) {
		//Edit lock on the config file so 2 users don't try to save at the same time.
		//First let's see if there is a lock already
		$strOutput="<b>Save log</b><br>";
		$strOutput.="Checking to see if anyone else is editing the config file.<br>";
		$query="select count(*) as total from locker where pproc = 'config_edit' and plock = '1'";
		$result = mysql_query($query);
		$qdata=mysql_fetch_assoc($result);
		$num_rows = $qdata['total'];;
		if($num_rows == 0) {
			$strOutput.="Config file is not locked.<br>";
			$path = '/home/dicom/bin';
			set_include_path(get_include_path() . PATH_SEPARATOR . $path);
			exec('sudo /home/dicom/startup.bash stop ALL', $return, $retval);
			$intLC3=0;
			while($intLC3 < sizeof($return)) {
				$strOutput.=$return[$intLC3] . "<br>";
				$intLC3++;
			}
			$_SESSION['thists']=time();
			//No lock.  Let's create a lock
			$query="update locker set ptimestamp = '" . $_SESSION['thists'] . "', ";
			$query.="puser = '" . $_SESSION['loginid'] . "', plock = '1' where pproc = 'config_edit';";
			$result = mysql_query($query);
			//Rename the config file so that we have a copy of the original
			rename("/etc/primal/primal.conf", "/etc/primal/primal.conf." . $_SESSION['thists']);
			$strOutput.="Moved old config file to /etc/primal/primal.conf." . $_SESSION['thists'] . "<br>";
			$strOutput.="Saving new config file.<br>";
			//Make sure the file is there.
			if(file_exists("/etc/primal/primal.conf." . $_SESSION['thists'])) {
				$handle2 = fopen("/etc/primal/primal.conf", "w");
				$intLC1=0;
				while($intLC1 < sizeof($_SESSION['PRINUM'])) {
					fwrite($handle2, '<scp' . $_SESSION['PRINUM'][$intLC1] . ">\n");
					fwrite($handle2, "\tPRIPORT=" . $_SESSION['PRIPORT'][$intLC1] . "\n");
					fwrite($handle2, "\tPRIAET=" . $_SESSION['PRIAET'][$intLC1] . "\n");
					fwrite($handle2, "\tPRILL=" . $_SESSION['PRILL'][$intLC1] . "\n");
					fwrite($handle2, "\tPRILFOUT=" . $_SESSION['PRILFOUT'][$intLC1] . "\n");
					fwrite($handle2, "\tPRILFIN=" . $_SESSION['PRILFIN'][$intLC1] . "\n");
					fwrite($handle2, "\tPRILFPROC=" . $_SESSION['PRILFPROC'][$intLC1] . "\n");
					fwrite($handle2, "\tPRILFQR=" . $_SESSION['PRILFQR'][$intLC1] . "\n");
					fwrite($handle2, "\tPRIIF=" . $_SESSION['PRIIF'][$intLC1] . "\n");
					fwrite($handle2, "\tPRIPROC=" . $_SESSION['PRIPROC'][$intLC1] . "\n");
					fwrite($handle2, "\tPRIOUT=" . $_SESSION['PRIOUT'][$intLC1] . "\n");
					fwrite($handle2, "\tPRIQRREC=" . $_SESSION['PRIQRREC'][$intLC1] . "\n");
					fwrite($handle2, "\tPRIQRAGE=" . $_SESSION['PRIQRAGE'][$intLC1] . "\n");
					if($_SESSION['PRIQRWAIT'][$intLC1] != 1 || !is_numeric($_SESSION['PRIQRWAIT'][$intLC1]))
						$_SESSION['PRIQRWAIT'][$intLC1]=0;
					fwrite($handle2, "\tPRIQRWAIT=" . $_SESSION['PRIQRWAIT'][$intLC1] . "\n");
					fwrite($handle2, "\tPRIQRMAX=" . $_SESSION['PRIQRMAX'][$intLC1] . "\n");
					fwrite($handle2, "\tPRISENT=" . $_SESSION['PRISENT'][$intLC1] . "\n");
					fwrite($handle2, "\tPRIRET=" . $_SESSION['PRIRET'][$intLC1] . "\n");
					fwrite($handle2, "\tPRIERROR=" . $_SESSION['PRIERROR'][$intLC1] . "\n");
					fwrite($handle2, "\tPRILOGDIR=" . $_SESSION['PRILOGDIR'][$intLC1] . "\n");
					fwrite($handle2, "\tPRICL=" . $_SESSION['PRICL'][$intLC1] . "\n");
					fwrite($handle2, "\tPRIDCOM=" . $_SESSION['PRIDCOM'][$intLC1] . "\n");
					fwrite($handle2, "\tPRIPASSTU=" . $_SESSION['PRIPASSTU'][$intLC1] . "\n");
					fwrite($handle2, "\tPRIRECTO=" . $_SESSION['PRIRECTO'][$intLC1] . "\n");
					fwrite($handle2, "\tPRIDUPE=" . $_SESSION['PRIDUPE'][$intLC1] . "\n");
					fwrite($handle2, "\tPRIHOLD=" . $_SESSION['PRIHOLD'][$intLC1] . "\n");
					$intLC2=0;
					while(isset($_SESSION['PRIDESTHIP'][$intLC1][$intLC2])) {
						fwrite($handle2, "\tPRIDESTHIP" . $intLC2 . '=' . $_SESSION['PRIDESTHIP'][$intLC1][$intLC2] . "\n");
						fwrite($handle2, "\tPRIDESTPORT" . $intLC2 . '=' . $_SESSION['PRIDESTPORT'][$intLC1][$intLC2] . "\n");
						fwrite($handle2, "\tPRIDESTCDCR" . $intLC2 . '=' . $_SESSION['PRIDESTCDCR'][$intLC1][$intLC2] . "\n");
						fwrite($handle2, "\tPRIDESTAEC" . $intLC2 . '=' . $_SESSION['PRIDESTAEC'][$intLC1][$intLC2] . "\n");
						$intLC2++;
					}
					$intLC2=0;
					while(isset($_SESSION['PRIQRHIP'][$intLC1][$intLC2])) {
						fwrite($handle2, "\tPRIQRHIP" . $intLC2 . '=' . $_SESSION['PRIQRHIP'][$intLC1][$intLC2] . "\n");
						fwrite($handle2, "\tPRIQRPORT" . $intLC2 . '=' . $_SESSION['PRIQRPORT'][$intLC1][$intLC2] . "\n");
						fwrite($handle2, "\tPRIQRAEC" . $intLC2 . '=' . $_SESSION['PRIQRAEC'][$intLC1][$intLC2] . "\n");
						fwrite($handle2, "\tPRIQRDESTHIP" . $intLC2 . '=' . $_SESSION['PRIQRDESTHIP'][$intLC1][$intLC2] . "\n");
						fwrite($handle2, "\tPRIQRDESTPORT" . $intLC2 . '=' . $_SESSION['PRIQRDESTPORT'][$intLC1][$intLC2] . "\n");
						fwrite($handle2, "\tPRIQRDESTAEC" . $intLC2 . '=' . $_SESSION['PRIQRDESTAEC'][$intLC1][$intLC2] . "\n");
						$intLC2++;
					}
					$intLC2=0;
					while(isset($_SESSION['PRITAGNAME'][$intLC1][$intLC2])) {
						fwrite($handle2, "\tPRITAG=");
						fwrite($handle2, $_SESSION['PRITAGNAME'][$intLC1][$intLC2] . ':');
						fwrite($handle2, $_SESSION['PRITAGVAL'][$intLC1][$intLC2] . ':');
						fwrite($handle2, $_SESSION['PRITAGMALL'][$intLC1][$intLC2] . ':');
						fwrite($handle2, $_SESSION['PRITAGADD'][$intLC1][$intLC2] . "\n");
						$intLC2++;
					}
					$intLC2=0;
					while(isset($_SESSION['PRIQRTAGNAME'][$intLC1][$intLC2])) {
						fwrite($handle2, "\tPRIQRTAG=");
						fwrite($handle2, $_SESSION['PRIQRTAGNAME'][$intLC1][$intLC2] . ':');
						fwrite($handle2, $_SESSION['PRIQRTAGVAL'][$intLC1][$intLC2] . ':');
						fwrite($handle2, $_SESSION['PRIQRTAGGROUP'][$intLC1][$intLC2] . "\n");
						$intLC2++;
					}
					fwrite($handle2, "</scp" . $_SESSION['PRINUM'][$intLC1] . ">\n");
					$intLC1++;
				}
			} else {
				$strOutput.="ERROR:  Could not copy config file.<br>";
			}
			fclose($handle2);
			//Release the lock
			$strOutput.="Unlocking config file<br>";
			$query="update locker set ptimestamp = '" . $_SESSION['thists'] . "', ";
			$query.="puser = '" . $_SESSION['loginid'] . "', plock = '0' where pproc = 'config_edit';";
			$result = mysql_query($query);
			unset($return);
			exec('sudo /home/dicom/startup.bash start ALL', $return, $retval);
			$intLC3=0;
			while($intLC3 < sizeof($return)) {
				$strOutput.=$return[$intLC3] . "<br>";
				$intLC3++;
			}
			$strOutput.="Done<br>";
		} else {
			$strOutput.="Config file is being edited by another user.  Please try again later...<br>";
		}
	}
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
if(isset($_GET['v'])) {
	echo '<form action="edit.php?v=' . $_GET['v'] . '" method="post">';
} else {
	echo '<form action="edit.php method="post">';
}
Display_Header2();
echo '<table border="1">';
echo '<tr><th>Receiver</th><th>Action</th></tr>';
$intLC1=0;
while($intLC1 < sizeof($_SESSION['PRINUM'])) {
	echo '<tr><td>' . $_SESSION['PRINUM'][$intLC1] . '<td><a href="http://' . $_SERVER['HTTP_HOST'] . '/edit.php?v=';
	echo $intLC1 . '">View</a></td><tr>';
	$intLC1++;
}
echo '</table>';
$intNumRec=count($_SESSION['PRINUM']);
if($intNumRec < 10) {
	echo '<br>';
	echo '<a href="http://' . $_SERVER['HTTP_HOST'];
	if(isset($_GET['v'])) {
		echo '/edit.php?v=' . $_GET['v'] . '&ar=' . $intLC1 . '">Add Receiver</a><br>';
	} else {
		echo '/edit.php?ar=' . $intLC1 . '">Add Receiver</a><br>';
	}
}
echo '<br><br>';
if(isset($_GET['v'])) {
	echo '<table border="1">';
	echo '<tr><th colspan="2">Receiver #' . $_SESSION['PRINUM'][$_GET['v']] . '</th><th>';
	echo '<a href="http://' . $_SERVER['HTTP_HOST'];
	echo '/edit.php?v=' . $_GET['v'] . '&dr=' . $_GET['v'] . '">Delete</a></th></tr>';
	echo '<tr><th>Description</th><th>Value</th></tr>';
	echo '<tr><td>AET</td>';
	echo '<td><input type="text" name="PRIAET" value="' . $_SESSION['PRIAET'][$_GET['v']] . '"></td></tr>';
	echo '<tr><td>Port</td>';
	echo '<td><input type="text" name="PRIPORT" value="' . $_SESSION['PRIPORT'][$_GET['v']] . '"></td></tr>';
	echo '<tr><td>Log Level</td>';
	echo '<td><input type="text" name="PRILL" value="' . $_SESSION['PRILL'][$_GET['v']] . '"></td></tr>';
	echo '<tr><td>Outbound Log File</td>';
	echo '<td><input type="text" name="PRILFOUT" value="' . $_SESSION['PRILFOUT'][$_GET['v']] . '"></td></tr>';
	echo '<tr><td>Inbound Log File</td>';
	echo '<td><input type="text" name="PRILFIN" value="' . $_SESSION['PRILFIN'][$_GET['v']] . '"></td></tr>';
	echo '<tr><td>Process Log File</td>';
	echo '<td><input type="text" name="PRILFPROC" value="' . $_SESSION['PRILFPROC'][$_GET['v']] . '"></td></tr>';
	echo '<tr><td>Q/R Log File</td>';
	echo '<td><input type="text" name="PRILFQR" value="' . $_SESSION['PRILFQR'][$_GET['v']] . '"></td></tr>';
	echo '<tr><td>Inbound Directory</td>';
	echo '<td><input type="text" name="PRIIF" value="' . $_SESSION['PRIIF'][$_GET['v']] . '"></td></tr>';
	echo '<tr><td>Process Directory</td>';
	echo '<td><input type="text" name="PRIPROC" value="' . $_SESSION['PRIPROC'][$_GET['v']] . '"></td></tr>';
	echo '<tr><td>Outbound Directory</td>';
	echo '<td><input type="text" name="PRIOUT" value="' . $_SESSION['PRIOUT'][$_GET['v']] . '"></td></tr>';
	echo '<tr><td>Sent Directory</td>';
	echo '<td><input type="text" name="PRISENT" value="' . $_SESSION['PRISENT'][$_GET['v']] . '"></td></tr>';
	echo '<tr><td>Retention Period</td>';
	echo '<td><input type="text" name="PRIRET" value="' . $_SESSION['PRIRET'][$_GET['v']] . '"></td></tr>';
	echo '<tr><td>Query/Retrive Priors Receiver</td>';
	echo '<td><input type="text" name="PRIQRREC" value="' . $_SESSION['PRIQRREC'][$_GET['v']] . '"></td></tr>';
	echo '<tr><td>Days to Query/Retrieve Priors</td>';
	echo '<td><input type="text" name="PRIQRAGE" value="' . $_SESSION['PRIQRAGE'][$_GET['v']] . '"></td></tr>';
	echo '<tr><td>Send priors before current study?</td>';
	echo '<td><input type="text" name="PRIQRWAIT" value="' . $_SESSION['PRIQRWAIT'][$_GET['v']] . '"></td></tr>';
	echo '<tr><td>Maximum number of priors to Query/Retrieve</td>';
	echo '<td><input type="text" name="PRIQRMAX" value="' . $_SESSION['PRIQRMAX'][$_GET['v']] . '"></td></tr>';
	echo '<tr><td>Error Directory</td>';
	echo '<td><input type="text" name="PRIERROR" value="' . $_SESSION['PRIERROR'][$_GET['v']] . '"></td></tr>';
	echo '<tr><td>Log Directory</td>';
	echo '<td><input type="text" name="PRILOGDIR" value="' . $_SESSION['PRILOGDIR'][$_GET['v']] . '"></td></tr>';
	echo '<tr><td>Compression Level</td>';
	echo '<td><input type="text" name="PRICL" value="' . $_SESSION['PRICL'][$_GET['v']] . '"></td></tr>';
	echo '<tr><td>Decompression</td>';
	echo '<td><input type="text" name="PRIDCOM" value="' . $_SESSION['PRIDCOM'][$_GET['v']] . '"></td></tr>';
	echo '<tr><td>AET Passthrough</td>';
	echo '<td><input type="text" name="PRIPASSTU" value="' . $_SESSION['PRIPASSTU'][$_GET['v']] . '"></td></tr>';
	echo '<tr><td>Recieve Timeout</td>';
	echo '<td><input type="text" name="PRIRECTO" value="' . $_SESSION['PRIRECTO'][$_GET['v']] . '"></td></tr>';
	echo '<tr><td>Duplicate Check</td>';
	echo '<td><input type="text" name="PRIDUPE" value="' . $_SESSION['PRIDUPE'][$_GET['v']] . '"></td></tr>';
	echo '<tr><td>Hold Directory</td>';
	echo '<td><input type="text" name="PRIHOLD" value="' . $_SESSION['PRIHOLD'][$_GET['v']] . '"></td></tr>';
	echo '<tr></tr>';
	$intLC2=0;
	while(isset($_SESSION['PRIDESTHIP'][$_GET['v']][$intLC2])) {
		echo '<tr><th colspan="2">Destination #' . $intLC2 . '</th><th>';
		echo '<a href="http://' . $_SERVER['HTTP_HOST'];
		echo '/edit.php?v=' . $_GET['v'] . '&dd=' . $intLC2 . '">Delete</a></th></tr>';
		echo '<tr><td>Destination Host/IP ' . $intLC2 . '</td>';
		echo '<td><input type="text" name="PRIDESTHIP' . $intLC2 . '"';
		echo ' value="' . $_SESSION['PRIDESTHIP'][$_GET['v']][$intLC2] . '"></td></tr>';
		echo '<tr><td>Destination port ' . $intLC2 . '</td>';
		echo '<td><input type="text" name="PRIDESTPORT' . $intLC2 . '"';
		echo ' value="' . $_SESSION['PRIDESTPORT'][$_GET['v']][$intLC2] . '"></td></tr>';
		echo '<tr><td>Compress or Decompress ' . $intLC2 . '</td>';
		echo '<td><input type="text" name="PRIDESTCDCR' . $intLC2 . '"';
		echo ' value="' . $_SESSION['PRIDESTCDCR'][$_GET['v']][$intLC2] . '"></td></tr>';
		echo '<tr><td>Destination AET ' . $intLC2 . '</td>';
		echo '<td><input type="text" name="PRIDESTAEC' . $intLC2 . '"';
		echo ' value="' . $_SESSION['PRIDESTAEC'][$_GET['v']][$intLC2] . '"></td></tr>';
		$intLC2++;
	}
	$intLC3=0;
	while(isset($_SESSION['PRIQRHIP'][$_GET['v']][$intLC3])) {
		echo '<tr><th colspan="2">Q/R Source #' . $intLC3 . '</th><th>';
		echo '<a href="http://' . $_SERVER['HTTP_HOST'];
		echo '/edit.php?v=' . $_GET['v'] . '&dq=' . $intLC3 . '">Delete</a></th></tr>';
		echo '<tr><td>Q/R Source Host/IP ' . $intLC3 . '</td>';
		echo '<td><input type="text" name="PRIQRHIP' . $intLC3 . '"';
		echo ' value="' . $_SESSION['PRIQRHIP'][$_GET['v']][$intLC3] . '"></td></tr>';
		echo '<tr><td>Q/R Source Port ' . $intLC3 . '</td>';
		echo '<td><input type="text" name="PRIQRPORT' . $intLC3 . '"';
		echo ' value="' . $_SESSION['PRIQRPORT'][$_GET['v']][$intLC3] . '"></td></tr>';
		echo '<tr><td>Q/R Source AET ' . $intLC3 . '</td>';
		echo '<td><input type="text" name="PRIQRAEC' . $intLC3 . '"';
		echo ' value="' . $_SESSION['PRIQRAEC'][$_GET['v']][$intLC3] . '"></td></tr>';
		echo '<tr><td>Q/R Destination Host/IP ' . $intLC3 . '</td>';
		echo '<td><input type="text" name="PRIQRDESTHIP' . $intLC3 . '"';
		echo ' value="' . $_SESSION['PRIQRDESTHIP'][$_GET['v']][$intLC3] . '"></td></tr>';
		echo '<tr><td>Q/R Destination port ' . $intLC3 . '</td>';
		echo '<td><input type="text" name="PRIQRDESTPORT' . $intLC3 . '"';
		echo ' value="' . $_SESSION['PRIQRDESTPORT'][$_GET['v']][$intLC3] . '"></td></tr>';
		echo '<tr><td>Q/R Destination AET ' . $intLC3 . '</td>';
		echo '<td><input type="text" name="PRIQRDESTAEC' . $intLC3 . '"';
		echo ' value="' . $_SESSION['PRIQRDESTAEC'][$_GET['v']][$intLC3] . '"></td></tr>';
		$intLC3++;
	}
	echo '</table><br>';
	if($intLC2 < 10) {
		echo '<a href="http://' . $_SERVER['HTTP_HOST'];
		echo '/edit.php?v=' . $_GET['v'] . '&ad=' . $intLC2 . '">Add Destination</a><br>';
	}
	if($intLC3 < 10) {
		echo '<a href="http://' . $_SERVER['HTTP_HOST'];
		echo '/edit.php?v=' . $_GET['v'] . '&aq=' . $intLC3 . '">Add Q/R Source</a><br>';
	}
	echo '<br>';
	echo '<table border="1">';
	echo '<tr><th>Tag Name</th><th>Modification</th><th>All?</th><th>Add?</th><th>Action</th></tr>';
	$intLC2=0;
	while(isset($_SESSION['PRITAGNAME'][$_GET['v']][$intLC2])) {
		echo '<tr><td><input type="text" name="PRITAGNAME' . $intLC2 . '"';
		echo ' value="' . $_SESSION['PRITAGNAME'][$_GET['v']][$intLC2] . '"></td>';
		echo '<td><input type="text" name="PRITAGVAL' . $intLC2 . '"';
		echo ' value="' . $_SESSION['PRITAGVAL'][$_GET['v']][$intLC2] . '"></td>';
		echo '<td><input type="text" name="PRITAGMALL' . $intLC2 . '"';
		echo ' value="' . $_SESSION['PRITAGMALL'][$_GET['v']][$intLC2] . '"></td>';
		echo '<td><input type="text" name="PRITAGADD' . $intLC2 . '"';
		echo ' value="' . $_SESSION['PRITAGADD'][$_GET['v']][$intLC2] . '"></td>';
		echo '<td><a href="http://' . $_SERVER['HTTP_HOST'] . '/edit.php?v=' . $_GET['v'];
		echo '&dt=' . $intLC2 . '">Delete</a></td></tr>';
		$intLC2++;
	}
	echo '</table>';
	echo '<br>';
	echo '<a href="http://' . $_SERVER['HTTP_HOST'];
	echo '/edit.php?v=' . $_GET['v'] . '&at=' . $intLC2 . '">Add Tag Modification</a><br>';
	echo '<br>';
	echo '<table border="1">';
	echo '<tr><th>Q/R Tag Name</th><th>Value</th><th>Grouping</th><th>Action</th></tr>';
	$intLC2=0;
	while(isset($_SESSION['PRIQRTAGNAME'][$_GET['v']][$intLC2])) {
		echo '<tr><td><input type="text" name="PRIQRTAGNAME' . $intLC2 . '"';
		echo ' value="' . $_SESSION['PRIQRTAGNAME'][$_GET['v']][$intLC2] . '"></td>';
		echo '<td><input type="text" name="PRIQRTAGVAL' . $intLC2 . '"';
		echo ' value="' . $_SESSION['PRIQRTAGVAL'][$_GET['v']][$intLC2] . '"></td>';
		echo '<td><input type="text" name="PRIQRTAGGROUP' . $intLC2 . '"';
		echo ' value="' . $_SESSION['PRIQRTAGGROUP'][$_GET['v']][$intLC2] . '"></td>';
		echo '<td><a href="http://' . $_SERVER['HTTP_HOST'] . '/edit.php?v=' . $_GET['v'];
		echo '&dqt=' . $intLC2 . '">Delete</a></td></tr>';
		$intLC2++;
	}
	echo '</table>';
	echo '<br>';
	echo '<a href="http://' . $_SERVER['HTTP_HOST'];
	echo '/edit.php?v=' . $_GET['v'] . '&aqt=' . $intLC2 . '">Add Q/R Tag Search Parameter</a><br>';
}

echo '</table>';
echo '<br><button type="submit" name="update">Update</button><br><br>';
echo '<a href="http://' . $_SERVER['HTTP_HOST'];
if(isset($_GET['v'])) {
	echo '/edit.php?v=' . $_GET['v'] . '&save=1">Save Changes</a><br>';
} else {
	echo '/edit.php?save=1">Save Changes</a><br>';
}
echo '   </form><br>';
if(isset($strOutput)) {
	echo $strOutput . "<br>";
}
Display_Footer();
echo '</BODY>';
echo '</HTML>';

?>
