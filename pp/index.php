<?php 

// Default processing code for data sent to this system
// We might get an IPN from PayPal, or a user might go here to start an order or see his
// status.

include "config.inc";
include "config_packages.inc";
include "includes/pp_functions.inc";
include "includes/PPUser.inc";

//$LINK = StartDatabase( MYSQLDB );

// If we get an 'id' sent by GET, we fetch the file for that ID and display it
// The ID is the ID of the record in the IPN database (not in the service's user db)

// If we get an 'item_number' sent by POST, we fetch the record for that ID and display it.
// Then, extract all POST variables and something with them, fuck if I can tell what.
// I think we use them to display the info?

// If no data is passed, we go directly to the ordering page
$page = "";

if($_GET['id']) {
	$ID = trim($_GET['id']);
	$subscriber = GetOrder ($ID); 
	$receipt_box = null;
} elseif ($_POST['item_number']) {
	$ID = trim($_POST['item_number']);
	$subscriber = GetOrder ($ID); 
	foreach ($_POST as $key => $value) {
		if (get_magic_quotes_gpc())
			$value = stripslashes ($value);
		if (!eregi("^[_0-9a-z-]{1,30}$",$key)) {
			unset ($key); 
			unset ($value); 
		}
		if ($key != '') {
			$subscriber[$key] = $value; 
			unset ($_POST); 
		}
	}
	$receipt_box = SetReceiptBox ($subscriber) ;	
} elseif ($_REQUEST['tos']) {
	$page = FetchSnippet ("tos");
	print $tos;
	$vars["head"] =	FetchSnippet ("head_tos");
	$vars["foot"] =	FetchSnippet ("foot_tos");
	
	$page = Substitutions ($page, $vars);
	$page = ReplaceSysVars ($page);
	print $page;
	mysqli_close($LINK);
	//$FP_MYSQL_LINK->close();
	exit;
	
} else {
	header("Location: order.php"); 
	mysqli_close($LINK);
	//$FP_MYSQL_LINK->close();
 	exit; 
}
$page = HandleResults ($subscriber, FetchSnippet ("return"));

$varlist = GetVarList ($page);
$vars = array ();
foreach ($varlist as $v) { $vars[$v] = ${$v};	}
$vars["head"] =	FetchSnippet ("head");
$vars["foot"] =	FetchSnippet ("foot");
$vars["receipt_box"] = $receipt_box;

$page = Substitutions ($page, $vars);
$page = ReplaceSysVars ($page);
print $page;

//mysqli_close($LINK);
//$FP_MYSQL_LINK->close();



// =============================================

function SetReceiptBox ($subscriber) {
	// It's a POST so we assume this is the return page coming
	// a PayPal payment. Therefore, show the Receipt box
	$subscriber && extract($subscriber);
	$receipt_box = FetchSnippet ("receipt_box");
	$varlist = GetVarList ($receipt_box);
	$vars = array ();
	foreach ($varlist as $v) { $vars[$v] = ${$v};	}
	$receipt_box = Substitutions ($receipt_box, $vars);
	return $receipt_box;
}


function HandleResults ($subscriber, $page) {
	global $PP_SERVICES, $PP_PACKAGES, $service_options;
	global $msg, $error;
	
	// Display User/Subscription Information
	
	$subscriber && extract($subscriber);

	$Servicename = $PP_SERVICES["$option_selection1"]['desc'];
//	$servicename = $service_options[$service]['desc'];
	$Packagename = $PP_PACKAGES["$option_selection1"]["$option_selection2"]["desc"];
	$Username = $subscriber['pp_username'];
	$Password = $subscriber['pp_password'];
	$Status = $subscriber['pp_status'];
	$URL = $PP_SERVICES["$option_selection1"]['URL'];
	$loginURL = $PP_SERVICES["$option_selection1"]['loginURL'];
	$loginURL = Substitutions ($loginURL, array ("Username"	=>	urlencode ($Username), "Password" => urlencode ($Password)));	
	// Display database entry
	// ID is the 'id' of the GET from above
	// OR the result of the POST item_number
	
	$varlist = GetVarList ($page);
	$vars = array ();
	foreach ($varlist as $v) { $$v && ($vars[$v] = ${$v});	}
	$page = Substitutions ($page, $vars);
	
	// Send confirmation email to user
	if ( SEND_EMAIL ) {
		$plaintext=html2text($paypal_results_page);
		$boundary="+_+_+_".time()."_+_+_+";
		$to="$address_name <$payer_email>";
		$subject="$option_selection1 Account Information";
		$from_address="$receiver_email";
		$from_name="Service Department";
		$header ="From: $from_name <$from_address>\n";
		$header.="Reply-to: <$from_address>\n";
		$header.="Return-path: <$from_address>\n";
		$header.="To: $to\n";
		$header.="X-Mailer: TinyTool MIME Mail ( http://www.teatoast.com/TinyTool/ )\n";
		$header.="MIME-Version: 1.0\n";
		$header.="Content-Type: multipart/alternative;\n boundary=\"$boundary\"\n\n";
		//
		$body="This is a MIME-encoded message\n\n".
					"--".$boundary."\nContent-Type: text/plain;\n charset=\"iso-8859-1\"\n".
					"Content-Transfer-Encoding: 7bit\n\n".
					$plaintext."\n\n".
					"--".$boundary."\nContent-Type: text/html;\n charset=\"iso-8859-1\"\n".
					"Content-Transfer-Encoding: 7bit\n\n".
					$paypal_results_page."\n\n".
					"--".$boundary."--\n\n";
		//
		
		// Mail results to subscriber
		$result=mail($to, $subject, $body, $header);
	}
	return $page;
}

function html2text($html)
	{
	$width = 70;
	$hr = str_pad($hr, $width, "+-");
	//
	$search = array(
	        "/\r/",										// Non-legal carriage return
	        "/[\n\t]+/",								// Newline and tab
	        '/<script[^>]*>.*?<\/script>/i',// <script> and </script>
	        '/<!-- .+? -->/',							// HTML Comments
	        '/<\/title>/i',
	        '/<h[123456][^>]*>(.+?)<\/h[123456]>/ie',	// H1 - H6
	        '/<\/p>/i',									// <p> or </p>
	        '/<br[^>]*>/i',							// <br>
	        '/<\/ul>/i',									// </ul>
	        '/<\/ol>/i',									// </ol>
	        '/<\/dl>/i',									// </dl>
	        '/<\/dt>/i',         				        // </dt>
	        '/<dd[^>]*>/i',							// <dd>
	        '/<\/dd>/i',								// </dd>
	        '/<li[^>]*>/i',							// <li>
	        '/<hr[^>]*>/i',							// <hr>
	        '/(<table[^>]*>|<\/table>)/i',	// <table> or </table>
	        '/<\/tr>/i',									// 	</tr>
	        '/<\/td>/i',								// </td>
	        '/&nbsp;/i',
	        '/&quot;/i',
	        '/&gt;/i',
	        '/&lt;/i',
	        '/&amp;/i',
	        '/&copy;/i',
	        '/&trade;/i',
	        '/&reg;/i',
	        '/&bull;/'
	    );
	//
	$replace = array(
	        "",									// Non-legal carriage return
	        "",									// Newline and tab
	        "",									// <script>s -- which strip_tags supposedly has problems with
	        "",									// Comments -- which strip_tags might have problem a with
	        "\n",
	        "ucwords(\"\n\n\\1\n\n\")", 		// H1 - H6
	        "\n",								// <p> or </p>
	        "\n",								// <br>
	        "\n",								// <ul> or </ul>
	        "\n",								// <ol> or </ol>
	        "\n",								// </dl>
	        "\n",								// </dt>
	        "\t\t",							// <dd>
	        "\n",								// </dd>
	        "\t*\t",							// <li>
	        "\n".$hr."\n",				// <hr>
	        "\n\n",							// <table> or </table>
	        "\n",								// 	</tr>
	        "\t\t\t\t\t",					// </td>
	        ' ',									// &nbsp;
	        '"',								// &quot;
	        '>',								// &gt;
	        '<',								// &lt;
	        '&',								// &amp;
	        '(c)',								// &copy;
	        '(tm)',							// &trade;
	        '(R)',								// &reg;
	        '*'									// &bull;
	    );
	//
	$text = trim(stripslashes($html));
	$text = preg_replace($search, $replace, $text);
	$text = strip_tags($text);
	$text = preg_replace("/\n[[:space:]]+/", "\n\n", $text);
	$text = preg_replace("/[\n]{3,}/", "\n\n", $text);
	$text = wordwrap($text, $width);
	return $text;
}
?>
