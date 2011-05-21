<?php


$fl = glob ("updates/*updater_mysql.sql");
sort ($fl);
foreach ($fl as $f) {
//	print "$f<br>";
}
reset ($fl);
$mrg = "";
$prev = "";
foreach ($fl as $f) {
	$c = file_get_contents ($f);
	if ($c != $prev) {
		//$mrg .= "-- $f\n";
		$mrg .= "$c\n";
		$prev = $c;
	}
}

file_put_contents ("all.sql", $mrg);

?>