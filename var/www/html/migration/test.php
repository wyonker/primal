<?php

echo '<form action="test2.php" method="post">';
echo '<input type="radio" name="dest" value"Legacy">Send to Legacy<br>';
echo '<input type="radio" name="dest" value"Other">Send Somewhere Else<br>';
echo 'Enter host name or IP of send destination: <input type="text" name="senddest" /><br>';
echo 'Enter port number of send destination: <input type="text" name="sendport" /><br>';
echo 'Enter the AET you wish to use: <input type="text" name="sendaet" value="RADPRIMAL01" /><br>';
echo 'Enter the AEC you wish to use: <input type="text" name="sendaec" /><br>';
echo '<input type="hidden" name="senddir" value="1234589" />';
echo '<br><button type="submit" name="Submit">Submit</button><br>';
echo '</form>';


//echo "PHP Self = " . $_SERVER['PHP_SELF'] . "<br><br>";
// Show all information, defaults to INFO_ALL
//phpinfo();

// Show just the module information.
// phpinfo(8) yields identical results.
//phpinfo(INFO_MODULES);

//$_SERVER['REQUEST_URI'];

?>
