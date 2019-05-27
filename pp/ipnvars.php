<?php

$d = ("\n===== ".date("D M j G:i:s T Y")." =====\n");

$out = "";
if ($_REQUEST) {
	foreach ($_REQUEST as $k => $v) {
		$out .= "$k = $v\n";
	}
} else {
	$out = "NO DATA SENT";
}
file_put_contents ("POST_". time().".txt", $d.$out);

?>