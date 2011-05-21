<?php
session_start("captcha");
error_reporting (0);
$key2=$_SESSION["hash"];
$key=trim($_REQUEST['key']);

if (strcasecmp ($key,$key2) != 0)
	$result = "";
else
	$result = "Correct!";

header("Content-type: text/plain");
$x = json_encode($result);
echo $x;

?>