<?php

$d = ("\n===== ".date("D M j G:i:s T Y")." =====\n");

$out = "";
if ($_REQUEST) {
	while (list($k,$v) = each ($_REQUEST))
		$out .= "$k = $v\n";
} else {
	$out = "NO DATA SENT";
}
file_put_contents ("POST_". time().".txt", $d.$out);

?>