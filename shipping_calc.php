<?php
$logfile = "log/shipping.log";

if(isset($_REQUEST['query']) && isset($_REQUEST['shipper'])) {
	$query = urldecode($_REQUEST['query']);
	$shipper = $_REQUEST['shipper'];
	
	switch ($shipper) {
		case "ups" :
		default :
			$result = queryUPS ($query);
			break;
	}
} else {
	$result = __FILE__.": Error: No shipper or query";
}
//error_log (date('Y-m-d H:i:s') . ": {$result}\n", 3, $logfile);
echo $result;

// ============
function queryUPS ($query) {
	global $logfile;
	
	$Url = "https://www.ups.com/using/services/rave/qcostcgi.cgi?accept_UPS_license_agreement=yes&" . $query;
		
	//var_dump ($Url);
	
	$Resp = fopen($Url, "r");
	while(!feof($Resp))
	{	 
		$ResultS = fgets($Resp, 500);
		$Result = explode("%", $ResultS);
		$Err = substr($Result[0], -1);

		if ($Err) {
			//error_log (date('Y-m-d H:i:s') . ": {$Err} : {$ResultS}\n", 3, $logfile);
		}
		switch($Err)
		{
			case 3:
			$ResCode = $Result[8];
			break;
			case 4:
			$ResCode = $Result[8];
			break;
			case 5:
			$ResCode = $Result[1];
			break;
			case 6:
			$ResCode = $Result[1];
			break;
		}
	}
	fclose($Resp);
	if(!$ResCode)
	{
		$ResCode = "An error occured.";
	}
	return $ResCode;
}

?>


