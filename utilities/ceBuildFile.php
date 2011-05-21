<?php 

$source = file ($_REQUEST['source']);

$arr = array ();
foreach ($source as $line) {
	$line = trim($line);
	print $line."<BR>";
	if ($line) {
		$entry = explode("\t", $line);
		for ($i=0;$i<count($entry);$i++) {
			$entry[$i] = trim ($entry[$i]);
		}
		$arr[] = $entry;
	}
}
file_put_contents ($_REQUEST['dest'], serialize ($arr));

?>