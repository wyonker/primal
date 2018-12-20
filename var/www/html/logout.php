<?php
	session_start();
	require_once('functions.php');
	date_default_timezone_set('America/New_York');
	$strMessage = "User " . $_SESSION['login_username'] . " logged out at " . date('Y.m.d:H.i.s') . "<br />";
	write_to_log($strMessage);
	unset($_SESSION['obj']);
	unset($_SESSION['obj1']);
	// Unset all of the session variables.
	$_SESSION = array();

	// If it's desired to kill the session, also delete the session cookie.
	// Note: This will destroy the session, and not just the session data!
	if (isset($_COOKIE[session_name()])) {
		setcookie(session_name(), '', time()-42000, '/');
	}

	// Finally, destroy the session.
	session_destroy();
	header ("location: index.php");
?>
