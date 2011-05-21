<?php
include "_config/sysconfig.inc";

// Print results?
$p = true;

$output = date("Y-m-d-H-i-s") . "<hr>\n";
$result = "";
$server = "69.36.15.169";
$list = file("fp_private/updater_site_list.txt", FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);

$results = array ();
foreach ($list as $line) {
	list ($title, $params) = explode("\t", $line);
	list ($domain, $account, $dir, $password) = explode(',', $params);
	
	$command = "/usr/bin/rsync -avzp --dry-run --delete -e \"ssh -i /Users/dgross/.ssh/mirror-rsync-key\" --include-from=/Users/dgross/Sites/fp/updater/include.txt --exclude-from=/Users/dgross/Sites/fp/updater/exclude.txt /Users/dgross/Sites/fp/* {$account}@{$server}:/home/{$account}/public_html/";
	
	$output .=   "<hr><h3>Update $title</h3>\n";
	$output .= "$account, $domain, $account, $dir, $password<br><br>\n";
	$output .= "COMMAND:<BR>$command<br><br>\n";

	exec($command, $result, $resultcode);
	$output .= "Result: $resultcode<BR>\n";
	
	foreach ($result as $line) {
		$output .= "$line<br>\n";
	}
	
}

print "$output<br><hr>END\n";

?>