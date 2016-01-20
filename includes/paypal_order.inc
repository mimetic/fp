<?php

class paypal_order
{
	var $paypal_post_vars;
	var $printer_specs;
	var $order_desc;
	var $error_email;

	function OrderSummary() {
		$summary = "time: " . $this->paypal_post_vars['payment_date'] . "; invoice: " . $this->paypal_post_vars['invoice'] .
		"; amount: ". $this->paypal_post_vars['mc_gross'].
		"; transaction fee: ". $this->paypal_post_vars['mc_fee'] ;
		return $summary;
	}
	
	// If param is an array, assume it is vars posted by paypal
	// If param is a single value, assume it is an ID of a previous
	// paypal transaction and fetch it from the DB
	// If empty, return NULL
	function paypal_order($paypal_post_vars) {
		if (is_array($paypal_post_vars)) {
			$this->paypal_post_vars = $paypal_post_vars;
			if (isset($this->paypal_post_vars['num_cart_items'])) {
				$this->paypal_post_vars['cmd'] = "_cart";
			} else {
				$this->paypal_post_vars['num_cart_items'] = 1;
				$this->paypal_post_vars['cmd'] = "_xclick";
			}
			
			// decode the specs
			$num_cart_items = $this->paypal_post_vars['num_cart_items'];
			for ($i=1;$i<=$num_cart_items;$i++) {
				//$this->paypal_post_vars["option_selection2_$i"] = urldecode(base64_decode($this->paypal_post_vars["option_selection2_$i"]));
			}
		} elseif ($vars = FetchPayPal($paypal_post_vars)) {
			$this->paypal_post_vars = $vars;
		} else {
			$this->paypal_post_vars = null;
		}
	}

	// Send the supplier for particular line item (a picture)
	// an order to print and ship. Each picture may have its own
	// printer, so we're not sending to just one supplier.
	function SendOrderToSupplier ($saleID = null)
	{
		global $BASEDIR, $LOGS;
		
		$DEBUG = 1;
		$NOSEND = FP_NO_ORDERS;	// if true, don't actually send payments but print them out.

		$DEBUG && fp_error_log(__FUNCTION__, 3, FP_ORDER_LOG );
		$sales = array ();
		
		if ($saleID) {
			$sales[] = FetchSale ($saleID);
			$txn_id = $sales[0]['txn_id'];
		} else {
			$txn_id = $this->paypal_post_vars['txn_id'];
			$sales = FetchSalesByTxnID ($txn_id);
		}

		$DEBUG && fp_error_log(__FUNCTION__.":".__LINE__.": $txn_id", 3, FP_ORDER_LOG );

		$fields = array ();
		$orders = array ();

		$quantities = array();
		// Get all individual items (sales) in this paypal order
		// and group them by supplier.
		foreach ($sales as $item) {
			$imageID = $item["item_id"];	// get picture ID


			$image = FetchImage ($imageID);
			
			// get artist record, but prepend "artist" to each key
			$artist1 = FetchArtist ($image['ArtistID']);
			while (list ($k,$v) = each ($artist1)) {
				$artist["artist_$k"] = $v;
			}
			
			$sale = array ();
			$sale['item'] = $item;
			$sale['image'] = $image;
			$sale['artist'] = $artist;
			$sale['spec'] = UnpackSpec ($item["item_spec"]);

			$salesinfo = ImageSalesInfo ($imageID);
			// Get amount sold + 1 => Print number in the edition
			// We must tell the printer what the print number is, e.g. 5/10
			// ??? $sale['printnum'] = $salesinfo['amount'];
			// Get the supplier for this image
			$supplierID = $salesinfo['supplierID'];
			
			$sale['EDITION_SIZE'] = $salesinfo['totaledition'];

			// Add to other sales of the same image in this order to find the total quantity sold in this order.
			if (in_array($imageID, array_keys($quantities))) {
				$quantities[$imageID] += $item['quantity'];
			} else {
				$quantities[$imageID] = $item['quantity'];
			}

			// We record the order as it is made, so the quantity is added to the number sold already, i.e. the 'totalsold' value
			// We need to count backward to find the first print number we are ordering.
			// If the order is for MORE than one print, then 'NUMBER_IN_EDITION' is the first in the series ordered,
			
			$sale['NUMBER_IN_EDITION'] = $salesinfo['totalsold'] - $quantities[$imageID] + 1;
			
			// HERE, record the Print Number in the edition into the sale. 
			// Why? Because we can be dead sure the number recorded matches the number we tell the printer.
			UpdateRecord( DB_SALES, $item['id'], array('PrintNumber'=>$sale['NUMBER_IN_EDITION']));
			
			$DEBUG && fp_error_log(__FUNCTION__.":".__LINE__.": imageID=$imageID, Sales info: number in edition={$sale['NUMBER_IN_EDITION']}, edition size = {$sale['EDITION_SIZE']}", 3, FP_ORDER_LOG );
			
			
			$orders[$supplierID][] = $sale;

		}

		// The multi-item cart doesn't use the Paypal info anymore. 
		//$paypal = FetchPayPalByTxnID($txn_id);
		$paypal = array();

		// All sales have the same ship-to info, so we just pull the last one from the loop above.
		$fields['ORDER_SHIPTO'] = $item['address_name']."<br>\n".
							$item['address_street'].", ".
							$item['address_city'].", ".
							$item['address_state']." ".
							$item['address_zip']."<br>\n".
							$item['address_country'];

		$fields['payer_email'] = $item['payer_email'];

		// Order fields: date, artist (from), ship to
		$date = date ("F j, Y, g:i a");
		$fields['ORDER_DATE'] = $date;

		
		$addr = join ("<br>\n", array ($artist['artist_Address1'], $artist['artist_Address2']));
		!empty($artist['artist_Address2']) ? $addr = $artist['artist_Address1']."<br>\n".$artist['artist_Address2'] : $addr = $artist['artist_Address1'];
		$fields['ORDER_FROM'] = 	$artist['artist_Fullname']."<br>\n".
							$addr."<br>\n".
							$artist['artist_City'].", ".
							$artist['artist_State']." ".
							$artist['artist_Zip']."<br>\n".
							$artist['artist_Country'];


		// Prepare an order for each supplier containing
		// all items for that supplier. There could be
		// multiple pictures ordered.
		$orderMessages = array ();
		$requireMatchprint = FetchSnippet ("supplier_order_require_matchprint");
		$existsMatchprint = FetchSnippet ("supplier_order_exists_matchprint");
		$noMatchprint = FetchSnippet ("supplier_order_no_matchprint");
		$editionInfoSingle = FetchSnippet ("supplier_order_editioning_oneprint");
		$editionInfoMulti = FetchSnippet ("supplier_order_editioning_multiprint");

		// Build the order for a picture
		$orderform = FetchSnippet ("supplier_order_form");
		$itemform = FetchSnippet ("supplier_order_item");
		$itemformOriginal = FetchSnippet ("supplier_order_item_original_art");
		
		while (list ($supplierID, $sale) = each ($orders)) {
			$supplier = FetchSupplier ($supplierID);
			$DEBUG && fp_error_log("<HR>Supplier: ".$supplier['Name']."<BR>", 3, FP_ORDER_LOG);
			$message = Substitutions ($orderform, $supplier);
			$orderitems = "";
			$itemstotal = 0;
			$rownum = 1;

			$supplierState = $supplier['State'];
			$supplierTaxRate = $supplier['SalesTaxRate'];
			$item['SalesTaxRate'] = $supplier['SalesTaxRate'];
			$salesTaxRate = $item['SalesTaxRate']/100;
			$fields['TAX'] = 0;
			
			foreach ($sale as $item) {
				$f = array ();
				$lineitem = $itemform;
				$image = $item['image'];
				$params = DecodeArrayFromDB ($image['Params']);
				$isOriginal = GetParam ($params, FP_PARAM_IMAGE_IS_ORIGINAL_ART);

				// If no shipping state set, assume this is a 'pickup' order and charge sales tax
				// Otherwise, charge tax if the shipper state == the destination state
				$chargeSalesTax = (!$item['item']['address_state'] || ($item['item']['address_state'] == $supplierState));

				if (!$isOriginal)
					// EDITIONED ITEMS/PRINTS
					{
					$DEBUG && fp_error_log(__FUNCTION__.":".__LINE__.":Build item ID = ".$item['item']['id']."<BR>", 3, FP_ORDER_LOG);
					$picIsOffsite = GetParam ($params, FP_PARAM_IMAGE_OFFSITE);
					if ($picIsOffsite) {
						$f['print_file_message'] = FetchSnippet ("supplier_order_has_printfile");
					} else {
						$f['print_file_message'] = FetchSnippet ("supplier_order_download_printfile");
					}
					
					$desc = htmlspecialchars(urldecode($item['item']['item_desc']));
					$f['DESCRIPTION'] = $desc;
					
					$mfdesc = "(see above for matting/framing)";
					$f['MATTEFRAMESDESC'] = $mfdesc;
					
					$lineitem = Substitutions ($lineitem, $item['item']);	// $item['item'] is an array containing the order
	
					$lineitem = Substitutions ($lineitem, $image);	// $image is an array containing the image record
					$lineitem = Substitutions ($lineitem, $item['artist']);	// $item['artist'] is an array containing the artist record
					
					// Match Print (Test print) line
					if ($item['item']['MatchprintRequired']) {
						$f["MATCHPRINT"] = $requireMatchprint;
						$mpcost = SupplierMatchprintCostBySaleID($item['id']);
						if (!$mpcost) {
							$f["MATCHPRINT"] = $existsMatchprint;
						}
					} else {
						$f["MATCHPRINT"] = $noMatchprint;
						$mpcost = 0;
					}
					$f["MATCHPRINTCOST"] = sprintf("%01.2f", $mpcost);
					
					$f["ITEMCOST"] = sprintf("%01.2f", $item['item']['cost']);
					
					$itemtotalcost = $item['item']['cost'] + $mpcost;
					$f["ITEMTOTALCOST"] = sprintf("%01.2f", $itemtotalcost);

					// Editioning
					// Number of picture in the edition, e.g. #5 of 10
					if ($item['item']['quantity'] > 1) {
						$lan = $item['item']['PrintNumber'] + $item['item']['quantity'] - 1;
						$f['EDITIONING_INFO'] = Substitutions($editionInfoMulti, array (
							'LAST_EDITION_NUMBER'	=> $lan,
							'EDITION_NUMBER_RANGE'	=> $item['item']['PrintNumber'] . " - " . $lan
							));
					} else {
						$f['EDITIONING_INFO'] = $editionInfoSingle;
					}
					$f['NUMBER_IN_EDITION'] = $item['item']['PrintNumber'];
					$f['EDITION_SIZE'] = $item['EDITION_SIZE'];

					$f['ROW_NUMBER'] = $rownum++;

					// shipping/handling are given to the printer
					$f['SHIPPING'] = $item['item']['mc_shipping'];
					$f['HANDLING'] = $item['item']['mc_handling'];
					
					$fields['SHIPPING'] += $item['item']['mc_shipping'];
					$fields['HANDLING'] += $item['item']['mc_handling'];

					$fields['SHTOTAL'] += $item['item']['mc_shipping'] + $item['item']['mc_handling'];

					// The tax the supplier charges is based on his sale, NOT what the customer paid the artist!
					// Does supplier charge tax?
					if ($chargeSalesTax) {
						$itemTax = $itemtotalcost * $salesTaxRate;
						$f['ITEMTAX'] = sprintf("%01.2f", $itemTax);
						$fields['TAX'] += $itemtotalcost * $salesTaxRate;
					} else {
						$f['ITEMTAX'] =  "---";
						$itemTax = 0;
					}
					
					$f["ITEM_TOTAL_MSHT"] = sprintf("%01.2f", $itemtotalcost + $mpcost + $item['item']['mc_shipping'] + $item['item']['mc_handling'] + $itemTax);


					$fields['SHIPPINGMETHOD'] = $item['item']['shipping_method'];
					if ($fields['SHIPPINGMETHOD'] == "pickup") {
						$fields['ORDER_SHIPPING_METHOD_TO_SHOW'] = FP_CUSTOMER_PICKUP_TEXT;
						$fields['ORDER_SHIPTO'] = FP_CUSTOMER_PICKUP_TEXT;
					}

					// ---- Send Picture
					// If the picture is local, send it. If it is held by the supplier
					// then notify the supplier that he has it.

					
					// SUBSTITUTE ROW INFO INTO THE ROW TEMPLATE
					$lineitem = Substitutions ($lineitem, $f);
					$orderitems .= $lineitem;

					$itemstotal += $itemtotalcost;


					if ($picIsOffsite)
						{
						// Send the image via FTP to the supplier
						// Do this by adding it to the queue. The sender should be called with a CRON
						WriteImageSendOrder ($image['ID'], $supplierID);
						CopyImageToSendQueue ($image['URL'], FP_DIR_ORIGINAL_IMAGES);
		
						}
					}
				else
				{
					// ORIGINAL/ART ITEMS
					$DEBUG = true;
					$DEBUG && fp_error_log(__FUNCTION__.":".__LINE__, 3, FP_ORDER_LOG);
					$DEBUG && fp_error_log(__FUNCTION__.":".__LINE__.": Original Artwork Sale", 3, FP_ORDER_LOG);
					$DEBUG && fp_error_log(__FUNCTION__.":".__LINE__.":Build item ID = ".$item['item']['id']."<BR>", 3, FP_ORDER_LOG);
					
					$desc = htmlspecialchars(urldecode($item['item']['item_desc']));
					
					$itemtype = GetParam ($params, FP_PARAM_IMAGE_ARTWORK_NAME);
					$itemtype 
					? $f['ARTWORK_NAME'] = '{fp:Image}'
					: $f['ARTWORK_NAME'] = $itemtype;
					
					$desc = htmlspecialchars(urldecode($item['item']['item_desc']));
					$f['DESCRIPTION'] = $desc;
					
					$mfdesc = "(see above for matting/framing)";
					$f['MATTEFRAMESDESC'] = $mfdesc;
					
					$lineitem = Substitutions ($lineitem, $item['item']);	// $item['item'] is an array containing the order
	
					$lineitem = Substitutions ($lineitem, $image);		// $image is an array containing the image record
					$lineitem = Substitutions ($lineitem, $item['artist']);	// $item['artist'] is an array containing the artist record
					
					$mpcost = 0;

					$f["ITEMCOST"] = sprintf("%01.2f", $item['item']['cost']);
					
					$itemtotalcost = $item['item']['cost'] + $mpcost;
					$f["ITEMTOTALCOST"] = sprintf("%01.2f", $itemtotalcost);
					
					$lineitem = Substitutions ($lineitem, $f);
					$orderitems .= $lineitem;
					
					$itemstotal += $itemtotalcost;
	
					// shipping/handling/tax handled separately from items
					$fields['SHIPPING'] += $item['item']['mc_shipping'];
					$fields['HANDLING'] += $item['item']['mc_handling'];
					$fields['TAX'] += $item['item']['tax'];
					$fields['SHTOTAL'] += $fields['SHIPPING'] + $fields['HANDLING'];
					
					$fields['SHIPPINGMETHOD'] = $item['item']['shipping_method'];
					if ($fields['SHIPPINGMETHOD'] == "pickup") {
						$fields['ORDER_SHIPPING_METHOD_TO_SHOW'] = FP_CUSTOMER_PICKUP_TEXT;
						$fields['ORDER_SHIPTO'] = FP_CUSTOMER_PICKUP_TEXT;
					}
					
				}
			
			}
			$message = Substitutions ($message, array ("ITEMS"=>$orderitems));
			
			$shtotal = $fields['SHTOTAL'];
			$total = $itemstotal + $fields['TAX'] + $shtotal;
			
			// number formatting
			$fields['ORDER_TAX_RATE'] = sprintf("%1.2f", $supplierTaxRate);
			$fields['ORDER_TAX'] = AddCurrencySymbol(sprintf("%01.2f", $fields['TAX']));
			$fields['ORDER_SHIPPING'] = AddCurrencySymbol(sprintf("%01.2f", $fields['SHIPPING']));
			$fields['ORDER_HANDLING'] = AddCurrencySymbol(sprintf("%01.2f", $fields['HANDLING']));
			$fields['ORDER_SHTOTAL'] = AddCurrencySymbol(sprintf("%01.2f", $fields['SHTOTAL']));
			$fields['ORDER_ITEMSTOTAL'] = AddCurrencySymbol(sprintf("%01.2f", $itemstotal));
			$fields['ORDER_TOTAL'] = AddCurrencySymbol(sprintf("%01.2f", $total));

			!empty($supplier['Address2']) ? $addr = $supplier['Address1']."<br>\n".$supplier['Address2'] : $addr = $supplier['Address1'];
			$fields['ORDER_TO'] = 	$supplier['Name']."<br>\n".
								$addr."<br>\n".
								$supplier['City'].", ".$supplier['State']." ".$supplier['Zip']."<br>\n".
								$supplier['Country'];
			$message = Substitutions ($message, $fields);
			$message = Substitutions ($message, $paypal);
			$message = Substitutions ($message, $artist);
			$message = Substitutions ($message, $image);
			
			$message = ReplaceAllSnippets ($message);
			$message = ReplaceSysVars ($message);
			$message = DeleteUnusedSnippets ($message);


			$supplierOrderTitle = FetchSnippet ("supplier_order_title");

			$logmsg = "Order sent to ".$supplier['Name']." (Email: {$supplier['Email']}, ID #{$supplierID}) for $total {$item['item']['mc_currency']}";

			fp_error_log($logmsg, 3, FP_ORDER_LOG );

			
			$DEBUG && fp_error_log(preg_replace ('/(\n(\t*))+/', "\n", "Message sent:\n\n".preg_replace ('/<.*?>/','', $message)), 3, FP_ORDER_LOG);

			$DEBUG && print __FUNCTION__.":".__LINE__."<HR>".$message;

			// Headers for HTML
			$headers = 'MIME-Version: 1.0' . "\r\n";
			$headers .= 'Content-type: text/html; charset=utf-8' . "\r\n";
			
			// Additional headers
			$headers .= "From: ". SYSTEMNAME . "<".FP_COMMERCE_EMAIL.">" . "\r\n";
			$headers .= "Reply-To: ". SYSTEMNAME . "<".$artist['artist_Email'].">" . "\r\n";
			$headers .= 'Cc: ' . $artist['artist_Email'] ."\r\n";
			$headers .= 'Bcc: ' . FP_SYSTEM_EMAIL ."\r\n";
	
			if (!$NOSEND) {
				// Don't send an order if the total is zero!!! That would indicate a mistake!
				if ($total > 0) {
					$result = mail($supplier['Email'], "Pre-Paid Print Order from ".SYSTEMNAME." [$date]", $message, $headers);
					$err_message = "Failed trying to send email to supplier (".$supplier['Name']." at ".$supplier['Email'].")\n\n".$message;
					$err_subject = "ERROR: [$date] FP could not send email to supplier.";
					$result || mail($this->error_email, $err_subject, $err_message, FP_EMAIL_HEADERS);
				} else {
					$err_message = "No order sent to supplier (".$supplier['Name']." at ".$supplier['Email'].") because no money was allocated for this order. Check the Price Set to correct this. We don't send orders for no money. (".__FUNCTION__.":".__LINE__.")";
					$err_subject = "ERROR: [$date] FP could not send email to supplier.";
					$result || mail($this->error_email, $err_subject, $err_message, FP_EMAIL_HEADERS);
				}
			} else {
				$result = mail(PAYPAL_ERROR_EMAIL, "Pre-Paid Print Order from ".SYSTEMNAME." [$date]", $message, $headers);
			}
		}
				

	}
	
	
	/* 	Given a successful payment, distribute the money
		to the artist, the supplier, and the system owner.
		
		Do this for each item in the sale. Don't group
		the payments because it's harder to know what
		the breakdown is if it's grouped. I think...
	*/
	
	function DistributeClientPayment ()
	{
		global $BASEDIR, $LOGS;
		
		$DEBUG = 0;
		$NOSEND = FP_NO_ORDERS;	// if true, don't actually send payments but print them out.
		
		$emailsubject = "Payment Notice from ".SYSTEMNAME;
		$date = date ("F j, Y, g:i a");
		$datetime = date ("Y-m-d h:m:s");
		
		$txn_id = $this->paypal_post_vars['txn_id'];
		if (!$txn_id)
			return false;
			
		$sales = FetchSalesByTxnID ($txn_id);
		
		$paymentorders = array ();

		foreach ($sales as $item) {
		
			$DEBUG && print "<HR>".__FUNCTION__."<BR><BR>";
			//$DEBUG && print_r ($item);
		
			$cost = 0;
			$net = 0;
			$commission = 0;
			
			$imageID = $item["item_id"];	// get picture ID
			$image = FetchImage ($imageID);
			$artist = FetchArtist ($image['ArtistID']);
			
			$spec = UnpackSpec ($item["item_spec"]);
			$salesinfo = ImageSalesInfo ($imageID);
			$supplierID = $salesinfo['supplierID'];
			$supplier = FetchSupplier ($supplierID);
			
			// 	A payment order consists of the amount, the paypal email, 
			//	the ID of the order in the DB_ORDERS table, and the full description
			$desc = "PAYMENT FOR: " . $item["item_name"] . " (" . "#".$item['item_id'] . ") : \n".htmlspecialchars(urldecode($item['item_desc']));

			// How do we divide our payment?
			$gross = $item['mc_gross'] - $item['mc_fee'];
			$cost = $item['cost'] + $item['mc_shipping'] + $item['mc_handling'] + $item['tax'];
			
			// Matchprint?
			$mpcost = SupplierMatchprintCostBySaleID ($saleID);
			$cost += $mpcost;
			
			
			if ($cost > 0 && $cost < $gross) {
				// System owner gets a percentage the NET
				($artist['Commission'] > 0) 
					? $commissionRate = round($artist['Commission'] / 100, 2) 
					:  $commissionRate = round(FP_DEFAULT_COMMISSION / 100, 2);
				($commissionRate > 1) && ($commissionRate = 1);
				($commissionRate < 0) && ($commissionRate = 0);
				
				// Possible second commission (e.g. for another cause)
				($artist['Commission2'] > 0) 
					? $commissionRate2 = round($artist['Commission2'] / 100, 2) 
					:  $commissionRate2 = 0;
				
				($commissionRate2 > 1) && ($commissionRate2 = 1);
				($commissionRate2 < 0) && ($commissionRate2 = 0);

				if ($cr1+$commissionRate2 > 1)
					$commissionRate2 = 1 - $cr1;
				
				// if we're getting small amounts, where rounding will screw it all up...for testing, for example...we don't round:
				if ($gross < 2) {
					$commission = ceil( ($gross - $cost) * $commissionRate * 100)/100;
					$commission2 = ceil( ($gross - $cost) * $commissionRate2 * 100)/100;
				} else {
					$commission = ceil( ($gross - $cost) * $commissionRate);
					$commission2 = ceil( ($gross - $cost) * $commissionRate2);
				}
				
				// Commission 2 paypal address
				$commission2addr = trim($artist['PayPalBusiness2']);
	
				// Artist gets the rest.
				$net = $gross - $commission - $commission2 - $cost;
								
				$DEBUG && print "SALE AMOUNT: ".$item['mc_gross']."<BR>";
				$DEBUG && print "TRANS FEE: ".$item['mc_fee']."<BR>";
				$DEBUG && print "GROSS: $gross<BR>";
				$DEBUG && print "Artist payment: $net<BR>";
				$DEBUG && print "Supplier payment: $cost<BR>";
				$DEBUG && print "Commission (".($commissionRate * 100)."%): $commission<BR>";
				$DEBUG && print "TOTAL: ". $gross + $commission + $cost;
				
				$suppliernote = FetchSnippet('masspay_supplier_purpose');
				$mpcost > 0 && $suppliernote .= FetchSnippet('masspay_matchprint_note');
				$suppliernote = ReplaceSysVars($suppliernote);
				
				// Order for artist
				if ($net > 0)
					{
					$purpose = ReplaceSysVars (FetchSnippet('masspay_artist_purpose'));
					$paymentorders[] = array (
						"L_AMT"			=> number_format ($net, 2,".",""),
						"L_EMAIL"		=> $artist['PayPalBusiness'],
						"L_UNIQUEID"		=> $item['item_id'],
						"L_NOTE"		=> $desc,
						"Email"			=> $artist['Email'],
						"SaleID"		=> $item['id'],
						"Payee"			=> "artist",
						"DateTime"		=> $datetime,
						"Purpose"		=> $purpose
					);
					}
	
				//order for supplier
				if ($cost > 0)
					{
					$paymentorders[] = array (
						"L_AMT"			=> number_format ($cost, 2,".",""),
						"L_EMAIL"		=> $supplier['PayPalBusiness'],
						"L_UNIQUEID"		=> $item['item_id'],
						"L_NOTE"		=> $desc,
						"Email"			=> $supplier['Email'],
						"SaleID"		=> $item['id'],
						"Payee"			=> "supplier",
						"DateTime"		=> $datetime,
						"Purpose"		=> $suppliernote
					);
					}
				
				// order for system owner
				if ($commission > 0 && PAYPAL_COMMISSIONS_EMAIL) 
					{
					$purpose = Substitutions (FetchSnippet('masspay_commission_purpose'), array ("COMMISSION_RATE"=>number_format ($commissionRate, 2,".","")));
					$purpose = ReplaceSysVars ($purpose);
					$paymentorders[] = array (	
						"L_AMT"			=> number_format ($commission, 2,".",""),
						"L_EMAIL"		=> PAYPAL_COMMISSIONS_EMAIL,
						"L_UNIQUEID"		=> $item['item_id'],
						"L_NOTE"		=> $desc,
						"Email"			=> $supplier['Email'],
						"SaleID"		=> $item['id'],
						"Payee"			=> "gallery",
						"DateTime"		=> $datetime,
						"Purpose"		=> $purpose						
					);
					}
				// order for second commission (e.g. for a gallery owner or a cause)
				if ($commission2 > 0 && $commission2addr) 
					{
					$purpose = Substitutions (FetchSnippet('masspay_commission2_purpose'), array ("COMMISSION_RATE"=>number_format ($commissionRate2, 2,".","")));
					$purpose = ReplaceSysVars ($purpose);
					$paymentorders[] = array (
						"L_AMT"			=> number_format ($commission2, 2,".",""),
						"L_EMAIL"		=> $commission2addr,
						"L_UNIQUEID"	=> $item['item_id'],
						"L_NOTE"		=> $desc,
						"Email"			=> $supplier['Email'],
						"SaleID"		=> $item['id'],
						"Payee"			=> "optional",
						"DateTime"		=> $datetime,
						"Purpose"		=> $purpose						
					);
					}
			} else {
				$grossf = sprintf("%01.2f", $gross);
				$costf = sprintf("%01.2f", $cost);

				($cost > 0)
				? $costmsg = "The price of the item ($grossf) is not high enough to cover the cost ($costf)."
				: $costmsg = "The cost of the item is set to zero (0.00).";
				
				$message = "-----\PAYMENT ERROR: {$costmsg}\nSale Information:\n ID: {$item['item_id']}\nDesc : $desc\n";
				fp_error_log($message, 3, FP_PAYMENTS_LOG );
				$err_message = "PAYMENT ERROR: [$date] ".SYSTEMNAME." could not pay the artist/supplier/gallerist because not enough money was allocated to the supplier (".$supplier['Name']." at ".$supplier['Email'].").\n- Be sure the selling price is high enough to cover the extra cost of a test print!\n- Check the Price Set for this item.\n\n(".__FUNCTION__.":".__LINE__.")\n\n".$message;
				$err_subject = "PAYMENT ERROR: [$date] ".SYSTEMNAME." could not pay the artist/supplier/gallerist!";
				mail($this->error_email, $err_subject, $err_message, FP_EMAIL_HEADERS);
				$DEBUG && print __FUNCTION__.":ERROR MSG SENT: $err_subject, $err_message<BR>";
			}
		}
		
		if ($DEBUG) {
			print "<HR>".__FUNCTION__."<HR>";
			foreach ($paymentorders as $po) {
				print ArrayToTable ($po) . "<BR><BR>";
			}
		}
		
		$NOSEND || $response = MassPay ($paymentorders, $emailsubject);
		return $response;
	}
	
	
	// *** UNUSED ***
	// If there's a sale, we lock the pricing/edition sizes
	// To do this, we make a new price set exclusively for this image
	// by copying the current one and assigning it to the image.
	// We lock the pricing on that set, don't let it be based on markup 
	// of costs.
	// Also, note the supplier ID in the priceset...even if the artist
	// changes suppliers, the set stays with the supplier!
	function CreateImagePriceSets ()
	{
		global $PAYPAL, $SALES;

		$DEBUG = false;
		$DEBUG && print "<hr>";
		
		$num_cart_items = $this->paypal_post_vars['num_cart_items'];
		for ($i=1;$i<=$num_cart_items;$i++) {

			if ($this->paypal_post_vars['cmd'] == "_cart") {
				$x = "_$i";
				$xx = "$i";	// some vars don't have an underscore...paypal is weird!
			} else {
				$x = "";
				$xx = "";	// some vars don't have an underscore...paypal is weird!
			}

			$saleID = $this->paypal_post_vars["item_number$xx"];
			$cart = GetRecord (DB_SALES, $saleID);


			$imageID = $cart['item_number'];
			$image = FetchImage ($imageID);
			
			// Get price set. If a general one (not exclusive to the image)
			// then create new price set for image based on current set.
			// Set the supplier ID

			$pricesetID = FetchPriceSetID ($imageID);
			$DEBUG && print __FUNCTION__.": imageID = $imageID, pricesetID = $pricesetID<BR>";
			$priceset = FetchPriceSetByID ($pricesetID);

			$supplierID = $priceset['SupplierID'];
			if ($supplierID < 1) {
				$artist = FetchArtist ($image["ArtistID"]);
				$supplierID = $artist["Vendor"];
			}

			$DEBUG && var_dump ($priceset);
			if ($priceset and $priceset['ImageID'] != $imageID) {
			
				/*
				use the other version of this, commerce.inc : CreateUniqueImagePriceSet ($imageID, $saleID=null)
			
			
				unset ($priceset['ID']);	// or we just recreate an existing record!
				$priceset['SupplierID'] = $supplierID;
				$priceset['Title'] = CatalogNumber($imageID);
				$priceset['ImageID'] = $imageID;
				
				// Write calculated prices to fixed prices
				$pricing = GetPricingByID ($imageID, $supplierID, $pricesetID);
				$priceset['PrintCost'] = $pricing['PrintCosts'];
				$priceset['PrintPrice'] = $pricing['PrintPrices'];
				$priceset['MatteCost'] = $pricing['MatteCosts'];
				$priceset['MattePrice'] = $pricing['MattePrices'];
				$priceset['FrameToPrintCost'] = $pricing['FrameToPrintCosts'];
				$priceset['FrameToPrintPrice'] = $pricing['FrameToPrintPrices'];
				$priceset['FrameMatteCost'] = $pricing['FrameMatteCosts'];
				$priceset['FrameMattePrice'] = $pricing['FrameMattePrices'];

				if ($DEBUG) {
					$r = join("\t", $priceset);
					fp_error_log(__FUNCTION__." : write to new price set: $r", 3, FP_ERROR_LOG );
				}


				// Record the paper and inkset used for this print
				$spec = UnpackSpec ($cart["item_spec"]);
				$priceset['PaperCode'] = $spec[FP_ORDER_PAPERCODE];
				$priceset['InksetCode'] = $spec[FP_ORDER_INKSETCODE];
				
				$papername = PaperName ($supplierID, $priceset['PaperCode']);
				$inksetname = InkName ($supplierID, $priceset['InksetCode']);

				if (!$DEBUG)
					{
					$priceset = SortAndCleanPriceSetRows($priceset);
					$priceset = CompactArraysToStrings ($priceset);
					$pricesetID = EditTable ("insert", DB_PRICESETS, '', $priceset);
					
					$DEBUG && print __FUNCTION__.": Created new price set for image $imageID : pricesetID = $pricesetID<BR>";
	
					// Set the image to use the new price set
					UpdateRecord( DB_IMAGES, $imageID, array ("PriceSetID" => $pricesetID) );
					$DEBUG && print __FUNCTION__.": Updated image to point to new price set $pricesetID<BR>";
					}	
				*/
			}
			
		}


		
	}
	
		
	// If there's a sale, we add the quantity sold
	// to the PriceSet record for that image
	// This function assumes there is a custom PriceSet record
	// for the image!
	function UpdateSalesCount ()
	{
		global $PAYPAL, $SALES;
		global $error, $msg;
		
		$DEBUG = 0;
		
		$num_cart_items = $this->paypal_post_vars['num_cart_items'];
		for ($i=1;$i<=$num_cart_items;$i++)
			{
			if ($this->paypal_post_vars['cmd'] == "_cart") {
				$x = "_$i";
				$xx = "$i";	// some vars don't have an underscore...paypal is weird!
			} else {
				$x = "";
				$xx = "";	// some vars don't have an underscore...paypal is weird!
			}
		
			// Get order from the sales db
			$saleID = $this->paypal_post_vars["item_number$xx"];
			RecordImageSaleBySale ($saleID);
			}
	}

	function update_all_paypal_vars () 
	{
		global $PAYPAL, $SALES;
		$col = "txn_id";
		$val = $this->paypal_post_vars['txn_id'];
		UpdateRow ( $PAYPAL, $this->paypal_post_vars, $col, $val );
		return $result;
	}

	function update_paypal_vars ($set) 
	{
		global $PAYPAL, $SALES;

		$where = "WHERE txn_id='" . $this->paypal_post_vars['txn_id'] . "'";
		$query = "UPDATE $PAYPAL SET $set $where";
		return mysqli_query ($query);
	}

	// Check that the order exists in our Sales db.
	// We recorded the order before sending it off, mostly
	// to be able to store extra ording info that we can't pass
	// through Paypal. Now we are making sure there is an order
	// corresponding to a Paypal transaction.
	function OrderExistsMultiItem ($invoice) {
		global $PAYPAL, $SALES;
		global $BASEDIR, $LOGS;
		
		$DEBUG = false;
		
		$invoice || $item_number=$this->paypal_post_vars['invoice'];
		
		$where = "WHERE invoice = '$invoice' AND txn_id = '' ";
		$query = "SELECT * from $SALES $where";
		
		$result = mysqli_query ($query);
		if ($result && mysqli_num_rows($result)) {
			$result = mysqli_fetch_array($result, MYSQLI_ASSOC);
			fp_error_log("OK (".__FUNCTION__.") : confirmed invoice = $invoice.", 3, FP_ORDER_LOG );
			
		} else {
			$result = 0;
			fp_error_log("ERROR (".__FUNCTION__.") : could not confirm order with invoice #$invoice. There is no matching order in $SALES.", 3, FP_ORDER_LOG );
		}
		return $result;
	}	


	// Check that the order exists in our Sales db.
	// We recorded the order before sending it off, mostly
	// to be able to store extra ording info that we can't pass
	// through Paypal. Now we are making sure there is an order
	// corresponding to a Paypal transaction.
	//
	// Single Item orders: item_number is the order number in the Paypal transaction, it is NOT the image number!
	// Cart orders: "invoice" is the key for items in a single cart
	function OrderExists() {
		global $PAYPAL, $SALES;
		global $BASEDIR, $LOGS;

		if ($this->paypal_post_vars['txn_type'] == "cart") {
			// MULTI-ITEM ORDER
			$query = "SELECT * from ".DB_SALES." WHERE `invoice` = '".$this->paypal_post_vars['invoice']."'";
			$result = mysqli_query ($query);
			if ($result && mysqli_num_rows($result)) {
				$result = mysqli_fetch_array($result, MYSQLI_ASSOC);
				fp_error_log("OK (" . __FUNCTION__ . ") : confirmed order invoice #{$invoice}.", 3, FP_ORDER_LOG);
			} else {
				$result = 0;
				fp_error_log("ERROR (" . __FUNCTION__ . ") : could not confirm order #{$invoice}. There is no matching order in ".DB_SALES, 3, FP_ORDER_LOG);
			}

		} else {
			// SINGLE ITEM ORDER
			$item_number || $item_number = $this->paypal_post_vars['item_number'];

			$where = "WHERE id = '$item_number' ";
			$query = "SELECT * from $SALES $where";

			$result = mysqli_query ($query);
			if ($result && mysqli_num_rows($result)) {
				$result = mysqli_fetch_array($result, MYSQLI_ASSOC);
				fp_error_log("OK (" . __FUNCTION__ . ") : confirmed order ID=$item_number.", 3, FP_ORDER_LOG);
			} else {
				$result = 0;
				fp_error_log("ERROR (" . __FUNCTION__ . ") : could not confirm order supposedly made on $item_number. There is no matching order in $SALES.", 3, FP_ORDER_LOG);
			}
		}
		return $result;
	}


	// Multi-item cart: Compare the amounts of the order with the paypal order, to be sure they are the same.
	// Single item order: Confirm Secret code passed with order
	// 
	function OrderIsAuthentic() {
		global $PAYPAL, $SALES;
		global $BASEDIR, $LOGS;

		if ($this->paypal_post_vars['txn_type'] == "cart") {

			// Compare order amount to our recorded amount for proper security.
			// Most important, is the AMOUNT of the order is correct?
			fp_error_log("OK (" . __FUNCTION__ . ") : NOT CHECKING AMOUNT IS CORRECT FOR AUTHENTICITY FOR NOW!", 3, FP_ORDER_LOG);

			$result = true;
		} else {

			$secret = "";

			$secret = $this->paypal_post_vars['secret'];

			$where = "WHERE secret = '$secret'";
			$query = "SELECT * from $SALES $where limit 1";

			$result = mysqli_query ($query);
			if ($result && mysqli_num_rows($result)) {
				$result = mysqli_fetch_array($result, MYSQLI_ASSOC);
				fp_error_log("OK (" . __FUNCTION__ . ") : confirmed order made where secret = $secret.\n query=$query", 3, FP_ORDER_LOG);
			} else {
				$result = 0;
				fp_error_log("ERROR (" . __FUNCTION__ . ") : could not confirm order with secret = $secret. There is no matching order in $SALES.", 3, FP_ORDER_LOG);
			}
		}
		return $result;
	}


	// RecordNewPaypalVars ()
	// Keeps a record of paypal vars sent. Not really necessary.
	function RecordNewPaypalVars () {

		//return true;
		//reset ($this->paypal_post_vars);
//		ksort ($this->paypal_post_vars);
//		foreach ($this->paypal_post_vars as $key=>$value) {
//			if (getType($key)=="string") {
//				$$key = $value;
//			}
//		}
		
		$id = AddRecord( DB_PAYPAL, $this->paypal_post_vars);
		$id || fp_error_log("ERROR (" . __FUNCTION__ . ") : Failed to write Paypal vars to the ".DB_PAYPAL." database.", 3, FP_PAYMENTS_LOG);
		$this->paypal_post_vars['id'] = $id;
		return $id;
	}

	

	// Write cart items to the Sales db
	// Use the same record as the pre-order...don't make a new one
	function RecordCartItems () {
		global $PAYPAL, $SALES;
		global $BASEDIR, $LOGS;
		global $DEBUG;
		$txn_id = $this->paypal_post_vars['txn_id'];

		fp_error_log(__FUNCTION__.": Recording values for txn_id = $txn_id", 3, FP_PAYMENTS_LOG );

		$tax = $this->paypal_post_vars['tax'];	// tax for entire sale, not just items

		// Convert the Paypal payment_date into a MYSQL and PHP  compatible value.
		$paymentDateTmp = strtotime($this->paypal_post_vars['payment_date']);
		$paymentDate = strftime('%Y-%m-%d %H:%M:%S',$paymentDateTmp);
			
		
		// if cart is used, then names have appended numbers, e.g. _1 or 1
		// if one-click is used, no appended numbers
		
		$success = true;

		for ($i=1;$i<=$this->paypal_post_vars['num_cart_items'];$i++) {

			// extended names depend on whether the purchase was made with a cart or 'buy now' button

			if ($this->paypal_post_vars['cmd'] == "_cart") {
				$x = "_$i";
				$xx = "$i";	// some vars don't have an underscore...paypal is weird!
			} else {
				$x = "";
				$xx = "";	// some vars don't have an underscore...paypal is weird!
			}

			// Get ID of entry in Sales DB, and get sales DB record.
			// The trick is, the item_number Paypal gets is the ID of the sales record.
			// Once we have that ID number, we replace item_number with the image ID.
			$itemRowID = mysql_real_escape_string($this->paypal_post_vars["item_number$xx"]);
			$invoice = mysql_real_escape_string($this->paypal_post_vars["invoice"]);
			$query = "select * from ".DB_SALES." where invoice = '$invoice' AND item_number = '$itemRowID'";
			$result = mysqli_query ($query);
			if ($result) {
				$saleRecord = mysqli_fetch_array($result, MYSQLI_ASSOC);
				$saleID = $saleRecord['id'];
			} else {
				return false;
			}

			/*
			// notice these don't have an underscore
			$item_name = $this->paypal_post_vars["item_name$xx"];
			
			// these vars do
			$quantity = $this->paypal_post_vars["quantity$xx"];	// no underscore...not sure why
			$weight = $this->paypal_post_vars["weight$x"];
			$weight_unit = $this->paypal_post_vars["weight_unit$x"];
			$mc_gross = $this->paypal_post_vars["mc_gross$x"];
						
			// If set in PayPal, fine, but don't let it overwrite what we recorded ourselves
			// if there's nothing to write.
			$this->paypal_post_vars["shipping_method"] && $shipping_method = $this->paypal_post_vars["shipping_method"];

			$option_name1 = $this->paypal_post_vars["option_name1$x"];
			$option_name2 = $this->paypal_post_vars["option_name2$x"];
			$option_selection1 = urldecode($this->paypal_post_vars["option_selection1$x"]);
			$option_selection2 = urldecode($this->paypal_post_vars["option_selection2$x"]);

			$saleID = $this->paypal_post_vars["item_number$xx"];
			$cart = GetRecord (DB_SALES, $saleID);
			$item_desc = urldecode($cart['item_desc']);
			$item_spec = urldecode($cart['item_spec']);
			$item_number = $cart['item_number'];
			*/

			// Shipping could be one amount for the whole transaction, or broken up.
			if ($this->paypal_post_vars["mc_shipping$xx"]) {
				// shipping by item
				$mc_shipping = $this->paypal_post_vars["mc_shipping$xx"];
				$mc_handling = $this->paypal_post_vars["mc_handling$xx"];
			} else {
				// shipping for the whole entry (should be a single item buy, e.g. "buy now")
				$mc_shipping = $this->paypal_post_vars["shipping"];
				$mc_handling = $this->paypal_post_vars["handling_amount"];
			}

			// Paypal tells us the fee for this transaction.
			$mc_fee = $this->paypal_post_vars["mc_fee"];
			
			$item_spec = urldecode($saleRecord['item_spec']);
			$spec = UnpackSpec ($item_spec);

			// Does this order include a matchprint?
			// If so, record the amount in the sale record (it could change in the price set, later).
			// This cost is subtracted from the payment to the artist
			// and paid to the supplier.
			$pricesetID = FetchPriceSetID ($spec[FP_ORDER_IMAGEID]);
			$priceset = GetPriceSet ($pricesetID);
			if ($spec[FP_ORDER_MATCHPRINT] && (array_sum ($priceset["Amount"]) < 1) ) {
				// How much does a matchprint cost?
				// If the picture has no sales, then we assume no matchprint
				// was made.
				$mpRequired = 1;
				$supplier = FetchSupplier($priceset['supplierID']);
				$mpcost = $supplier["MatchPrintPrice"];
			} else {
				$mpcost = 0;
				$mpRequired = 0;
			}
/*
"address_name"		=> $this->paypal_post_vars["address_name"];
"address_state"		=> $this->paypal_post_vars["address_state"];
"address_street"		=> $this->paypal_post_vars["address_street"];
"address_zip"		=> $this->paypal_post_vars["address_zip"];
"address_city"		=> $this->paypal_post_vars["address_city"];
"address_country"		=> $this->paypal_post_vars["address_country"];
"address_country_code"		=> $this->paypal_post_vars["address_country_code"];

"contact_phone"		=> $this->paypal_post_vars["contact_phone"];
"first_name"		=> $this->paypal_post_vars["first_name"];
"last_name"		=> $this->paypal_post_vars["last_name"];
"payer_business_name"		=> $this->paypal_post_vars["payer_business_name"];
"payer_email"		=> $this->paypal_post_vars["payer_email"];
"payer_id"		=> $this->paypal_post_vars["payer_id"];
*/

			// This won't work: If two items are the same image, one a print, the other framed,
			// then our count won't be right, because we don't record that counting here.
			// Next print number
			// $printNumber = $priceset['count'] + 1;

			$pairs = 	array (
						"txn_id"				=> $txn_id,
 						"MatchPrintPrice"		=> $mpcost,
						"tax"					=> $tax,
						"MatchprintRequired"	=> $mpRequired,
// 						"PrintNumber"			=> $printNumber,
				
// 						"item_name"				=> $item_name,
// 						"item_number"			=> $spec[FP_ORDER_IMAGEID],
// 						"quantity"				=> $quantity,
// 						"weight"				=> $weight,
// 						"weight_unit"			=> $weight_unit,
// 						"mc_gross"				=> $mc_gross,
 						"mc_fee"				=> $mc_fee,
 						"mc_handling"			=> $mc_handling,
 						"mc_shipping"			=> $mc_shipping,
// 						"option_name1"			=> $option_name1,
// 						"option_name2"			=> $option_name2,
// 						"option_selection1"		=> $option_selection1,
// 						"option_selection2"		=> $option_selection2,
// 						"item_desc"				=> $item_desc,
// 						"item_spec"				=> $item_spec,
// 						"shipping_method"		=> $shipping_method,
 						"payment_date"			=> $paymentDate
						);
						
			// Our values overwrite values in paypal_post_vars
			//$pairs = array_merge($this->paypal_post_vars, $pairs);
			
			// Update the correct sales DB entry
			$result = UpdateRecord( DB_SALES, $saleID, $pairs );
			$result || fp_error_log("Failed to update ID=".$saleID, 3, FP_PAYMENTS_LOG );
			$result || $success = false;
		}
		return $success;
	}
	
	function DeleteCartItems ($cartID) {
		global $SALES;
		DeleteRow( $SALES, "paypal_id = '$cartID'" );
	}


// Is a print sold out, and should not have been offered? Also prevents fraud.
// Two ways of deciding this:
// Two versions of edition info:
// - based on total size of edition and total sales
// - based on edition sizes for each print size
	function soldout() {
		global $BASEDIR, $LOGS;

		$DEBUG = false;
		
		$soldout = false;
		
		$num_cart_items = $this->paypal_post_vars['num_cart_items'];
		for ($i=1;$i<=$num_cart_items;$i++) {

			if ($this->paypal_post_vars['cmd'] == "_cart") {
				$x = "_$i";
				$xx = "$i";	// some vars don't have an underscore...paypal is weird!
			} else {
				$x = "";
				$xx = "";	// some vars don't have an underscore...paypal is weird!
			}

			$saleID = $this->paypal_post_vars["item_number$xx"];
			$cart = GetRecord (DB_SALES, $saleID);
			$imageID = $cart['item_number'];
			$quantity = $cart["quantity"];
			$image = FetchImage ($imageID);
			$pricesetID = FetchPriceSetID($imageID);
			$priceset = GetPriceSet ($pricesetID);
			$spec = UnpackSpec ($cart["item_spec"]);
			
			$whichrow = $spec[FP_ORDER_ROW];

			$priceset = GetPriceSet ($pricesetID);
			// Be sure prints are available in this size!
			$editionsize = $priceset['EditionSize'][$whichrow];

			
			if (FP_EDITION_CALC_METHOD == "total") {
				// total prints reached?
				$sold = array_sum ($priceset["Amount"]);
				$totaleditionsize = array_sum ($priceset["EditionSize"]);
				if ($editionsize > 0) {
					($totaleditionsize - $sold - $quantity >= 0) || $soldout = true;
				}
			} else {
				// max for this size reached?
				$sold = $priceset["Amount"][$whichrow];
				if ($editionsize > 0) {
					($editionsize - $sold - $quantity >= 0) || $soldout = true;
				}
			}

			
			$DEBUG && print "<hr>";
			$DEBUG && print __FUNCTION__.": row=$whichrow, sold=$sold, editionsize=$editionsize<BR>"; 
			
		}
		$DEBUG && ($soldout && print "SOLD OUT<BR>");
		$DEBUG && fp_error_log(__FUNCTION__.": soldout says: $soldout", 3, FP_PAYMENTS_LOG );
		return $soldout;
	}
	
	
}


// ===== FUNCTIONS

?>