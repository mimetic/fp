<?php

class paypal_masspay
{
	var $nvp;				// The URI encoded params, e.g. NAME=Joe&PRICE=1.00
	var $url;				// URL to send to (real or sandbox)
	var $params;				// Array of security and other params for transaction
	var $payments;				// Array of separate payments 
	var $response;				// response from PayPal after a send
	var $currentrow;			// item set number, e.g. 1 as in L_AMT1
	var $timeout;				//
	var $error_email;			// email address to send error messages
		
	
	function log_masspay_payments () {
		global $BASEDIR, $LOGS;
		
		$i = 1;
		fp_error_log("\n-------------------------\nMasspay Payment begin:", 3, FP_PAYMENTS_LOG );
		foreach ($this->payments as $p) {
			fp_error_log("-----\n($i) Masspay Payment:".implode (", ", $p), 3, FP_PAYMENTS_LOG );
			$i++;
		}
		fp_error_log("\n-------------------------\n", 3, FP_PAYMENTS_LOG );
	}

	// Record payments in the payments DB
	function record_masspay_payments () {
		global $BASEDIR, $LOGS;
		
		$i = 1;

		foreach ($this->payments as $p) {
			$pairs = array (	
								"Amount"	=> $p["L_AMT"], 
								"Email"		=> $p["L_EMAIL"], 
								"UniqueID"	=> $p["L_UNIQUEID"], 
								"Note"		=> $p["L_NOTE"], 
								"SaleID"	=> $p["SaleID"], 
								"Payee"		=> $p["Payee"], 
								"DateTime"	=> $p["DateTime"]
							);
			AddRecord( DB_PAYMENTS, $pairs );
		}

	}


	function error_out($message, $mail_headers)
	{		
		$DEBUG = false;

		$mail_headers || $mail_headers = FP_EMAIL_HEADERS;
		
		$date = date("D M j G:i:s T Y", time());
		$message .= "\n\nThe following data was received from PayPal:\n\n";

		@reset($this->paypal_post_vars);
		while( @list($key,$value) = @each($this->paypal_post_vars)) {
			$message .= $key . ':' . " \t$value\n";
		}
		
		if ($DEBUG) {
			$message = str_replace ("\n", "<br>\n", $message);
			print $message;
		}
		mail($this->error_email, "[$date] paypay_ipn notification", $message, $mail_headers);
		fp_error_log("\n-------------------------\nMasspay Error: $message", 3, FP_PAYMENTS_LOG );
	}
	
	function alert_payees ($mail_headers)
	{
		$date = date("D M j G:i:s T Y", time());
		$subject = FetchSnippet ("masspay_alert_subject");
		$subject = ReplaceSysVars ($subject);
		$message_format = FetchSnippet ("masspay_alert_payee");
		foreach ($this->payments as $p) {
			$p = array_merge($this->params, $p);
			$p['DATE'] = $date;
			$message = Substitutions ( $message_format, $p);
			$message = ReplaceAllSnippets ($message);
			$message = ReplaceSysVars ($message);
			$message = DeleteUnusedSnippets ($message);
			mail($p["Email"], $subject, $message, $mail_headers);
			//echo __FILE__.":".__FUNCTION__.":".__LINE__.": Sent mail to ".$p["L_EMAIL"]." : $message\n";
		}
	}
	

	// Convert an array to a name-value pair array
	// $str can be an existing NVP string to add onto
	// $row can be a numbered set, and adds $row to the name of each pair
	// as for line items, e.g. Name0, Name1, etc.
	// *** we use URLENCODE ... don't do it twice! ***
	// *** Cannot have newlines in anything ... we strip them here ***
	// We also add only elements of the array that are proper masspay entries
	// because we might store other things in a payment array, such as the sale ID.
	function add_to_nvp ($arr) {
		$DEBUG = false;
		while (list($k,$v) = each($arr) ) {
			$v = trim(str_replace ("\n", "", $v));
			$p[] = $k . $this->currentrow . "=" . urlencode($v);
			$DEBUG && fp_error_log(__FUNCTION__.": Add $k{$this->currentrow} = $v to masspay NVP", 3, FP_PAYMENTS_LOG);
		}
		$this->nvp && $p[] = $this->nvp;
		$this->nvp = implode ("&", $p);
	}
	
	// We have to strip out extraneous entries in the payments array which are used for other purposes.
	function add_payments_to_nvp () {
		$DEBUG = false;
		$this->currentrow = 0;
		$keys = array (	"L_EMAIL"=>"", "L_AMT"=>"", "L_UNIQUEID"=>"", "L_NOTE"=>"");
		foreach ($this->payments as $p) {
			$DEBUG && fp_error_log(__FUNCTION__.": ".print_r($p), 3, FP_PAYMENTS_LOG);
			$p = array_intersect_key ($p, $keys);
			$DEBUG && fp_error_log(__FUNCTION__.": ".print_r($p), 3, FP_PAYMENTS_LOG);
			$this->add_to_nvp ($p) ;
			$this->currentrow++;
		}
	}

	function add_params_to_nvp () {
		$this->add_to_nvp ($this->params) ;
	}

	/**
	 * Send HTTP POST Request
	 *
	 * $this->url	string	The request URL
	 * $this->nvp	string	The urlencoded POST Message, e.g L_EMAIL0=a@b.com&L_AMT0=1&...
	 * @param	bool		determines whether to return a parsed array (true) or a raw array (false)
	 * @return	array		Contains a bool status, error_msg, error_no,
	 * 		and the HTTP Response body(parsed=response  or non-parsed=httpResponse) if successful
	 *
	 */
	function PPHttpPost($parsed_)
	{
		$DEBUG = false;
		
		$this->currentrow = null;
		$this->add_params_to_nvp () ;
		$this->add_payments_to_nvp ();
		
		$DEBUG && fp_error_log(__FUNCTION__.": this->nvp = {$this->nvp}", 3, FP_PAYMENTS_LOG);

		//setting the curl parameters.
		$ch = curl_init();
		curl_setopt($ch, CURLOPT_URL,$this->url);
		curl_setopt($ch, CURLOPT_VERBOSE, 1);

		//turning off the server and peer verification(TrustManager Concept).
		curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, FALSE);
		curl_setopt($ch, CURLOPT_SSL_VERIFYHOST, FALSE);

		curl_setopt($ch, CURLOPT_RETURNTRANSFER,1);
		curl_setopt($ch, CURLOPT_POST, 1);

		//setting the nvpreq as POST FIELD to curl
		curl_setopt($ch,CURLOPT_POSTFIELDS,$this->nvp);

		//getting response from server
		$httpResponse = curl_exec($ch);

		if(!$httpResponse) {
			$this->response =  array("status" => false, "error_msg" => curl_error($ch), "error_no" => curl_errno($ch));
			return $this->response;
		}

		if(!$parsed_) {
			$this->response = array("status" => true, "httpResponse" => $httpResponse);
			return $this->response;
		}
		
		$this->response = deformatNVP ($httpResponse);

		if(0 == sizeof($this->response)) {
			$error = "Invalid HTTP Response for POST request(".$this-nvp.") to ".$this->url;
			$this->response = array("status" => false, "error_msg" => $error, "error_no" => 0);
			return $this->response;
		}
		
		if ('Failure' == $this->response['ACK']) {
			$this->response = array("status" => true, "response" => $this->response);
		} else {
			$this->response = array("status" => true, "response" => $this->response);
		}

		return $this->response;

	} // PPHttpPost



	// This function will take NVPString and convert it to an Associative Array and it will decode the response.
	//   * It is usefull to search for a particular key and displaying arrays.
	//   * @nvpstr is NVPString.
	//   * @nvpArray is Associative Array.
	


} 

// ===== FUNCTIONS

function deformatNVP($nvpstr)
{

	$intial=0;
	$nvpArray = array();


	while(strlen($nvpstr)){
		//postion of Key
		$keypos= strpos($nvpstr,'=');
		//position of value
		$valuepos = strpos($nvpstr,'&') ? strpos($nvpstr,'&'): strlen($nvpstr);

		// getting the Key and Value values and storing in a Associative Array
		$keyval=substr($nvpstr,$intial,$keypos);
		$valval=substr($nvpstr,$keypos+1,$valuepos-$keypos-1);
		//decoding the respose
		$nvpArray[urldecode($keyval)] =urldecode( $valval);
		$nvpstr=substr($nvpstr,$valuepos+1,strlen($nvpstr));
	 }
	return $nvpArray;
}


?>