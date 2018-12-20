<?php
/**
 * Require the library
 */
require 'PHPTail.php';


/*
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
*/

/**
 * Initilize a new instance of PHPTail
 * @var PHPTail
 */
$tail = new PHPTail("/home/dicom/logs/in.log");
$tailproc = new PHPTail("/home/dicom/logs/process.log");
$tailout = new PHPTail("/home/dicom/logs/out.log");

/**
 * We're getting an AJAX call
 */
if(isset($_GET['ajax']))  {
        echo $tail->getNewLines($_GET['lastsize'], $_GET['grep'], $_GET['invert']);
        die();
}
/**
 * Regular GET/POST call, print out the GUI
 */
$tail->generateGUI();


