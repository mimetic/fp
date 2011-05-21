<?php
include "_config/sysconfig.inc";
include "_config/fpconfig.inc";

// Print results?
$p = 0;

$output = date("Y-m-d-H-i-s") . "\n";

$d = getenv('DOCUMENT_ROOT') . "/mysql_archives";
$d = "mysql_archives";
$output .=   __FILE__.": Save in $d<br>\n";

file_exists ($d) || mkdir ($d, 0750);
file_exists ($d) || $output .=   "Aw, fuck<br>\n";

// data only, no 'create table'
$backupFile =  "$d/".  MYSQLDB . "_data_" . date("Y-m-d-H-i-s") . '.sql.gz';
$command = "mysqldump  --opt --no-create-info --skip-add-drop-table --compatible=mysql40 --host=" . MYSQLHOST . " --user=" . MYSQLUSER . " --password=" . escapeshellarg(MYSQLPASSWORD) . " " . MYSQLDB . " | gzip > $backupFile";

$reply = system($command, $result);
($result === false) && $output .= "Command failed!<br>\n";

$output .=   "============<br>\nBackup to $d<br>\n";
$output .=   "$command<br>\n";
$result && ($output .=   "Result: $reply : $result<br>\n\n");

// data and create-table
$backupFile =  "$d/".  MYSQLDB . "_structure_" . date("Y-m-d-H-i-s") . '.sql.gz';
$command = "mysqldump  --opt --compatible=mysql40 --host=" . MYSQLHOST . " --user=" . MYSQLUSER . " --password=" . escapeshellarg(MYSQLPASSWORD) . " " . MYSQLDB . " | gzip > $backupFile";

$reply = system($command, $result);
($result === false) && $output .= "Command failed!<br>\n";

$output .=   "============<br>\nBackup to $d<br>\n";
$output .=   "$command<br>\n";
$result && ($output .=   "Result: $reply : $result<br>\n\n");

// Delete all backups older than 10 days

$maxage = 10;
$command = "find $d -type f -mtime +".$maxage." | xargs rm";
$filelist = exec($command, $result);

($result === false) && $output .= "Command failed!<br>\n";

$output .=   "============<br>\nCLEANUP to $d<br>\n";
$output .=   "$command<br>\n";
$reply && ($output .=   "Result: $reply : $result<br>\n\n");


$p && print $output;



function my_exec($cmd, $input='') {
	$proc = proc_open($cmd, array(0 => array('pipe', 'r'), 1 => array('pipe', 'w'), 2 => array('pipe', 'w')), $pipes);
	fwrite($pipes[0], $input);
	fclose($pipes[0]);
	$stdout = stream_get_contents($pipes[1]);
	fclose($pipes[1]);
	$stderr = stream_get_contents($pipes[2]);
	fclose($pipes[2]);
	$rtn = proc_close($proc);
	return array('stdout' => $stdout,
		'stderr' => $stderr,
		'return' => $rtn
	);
	var_export(my_exec('echo -e $(</dev/stdin) | wc -l', 'h\\nel\\nlo'));
}
?>