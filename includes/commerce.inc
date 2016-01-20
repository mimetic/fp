<?php

/*
	COMMERCE.INC
	FP gallery functions related to pricing and sales.
	
*/



// 	============================================================
//	Print Pricing / shipping calculations

/*
	RecordImageSale($imageID)
	Look up the record in the Sales DB, add the quantity to the price set for that image.
	This assumes a priceset for the image exists, that CreateImagePriceSets has been called!
*/
function RecordImageSale ($imageID, $quantity, $sizeIndex) {
	global $error, $msg;
	
	if (!$imageID)
		return false;

	CreateUniqueImagePriceSet ($imageID);
	
	$quantity = $sale["quantity"];
	$image = FetchImage ($imageID);
	$pricesetID = $image['PriceSetID'];
	
	//$priceset = UnpackPriceSetRow(FetchPriceSetByID ($pricesetID));
	$priceset = GetPriceset($pricesetID);
	$editionsize = GetImageEditionSize($imageID, $priceset['TotalEditionSize'], $image);

	
	
	$amounts = $priceset['Amount'];
	$amounts[$spec[FP_ORDER_ROW]-1] += $quantity;
	$vars = array("Amount"=>$amounts);
	$vars = CompactArraysToStrings ($vars);
	
	if ($DEBUG) {
		$pricesetb = FetchPriceSetByID ($pricesetID);
		//fp_error_log(__FUNCTION__.ArrayToTable ($pricesetb), 3, FP_ORDER_LOG);
	} else {
		UpdateRecord( DB_PRICESETS, $pricesetID, $vars);
	}
	
	// If image is now sold out, clear the html server caches so we don't offer it!
	// Note the "-1"...because we haven't reloaded the $priceset!!!
	
	
	$s = $editionsize - $priceset['AmountSold'] - 1;
	fp_error_log(__FUNCTION__.": TotalEditionSize: {$editionsize}, remaining prints: $s", 3, FP_ACTIVITY_LOG);
	
	if ($editionsize>0 && ($s<=0))
	{
		ClearAllCache();
		fp_error_log(__FUNCTION__.": Clear Cache because image #{$imageID} sold out.", 3, FP_ACTIVITY_LOG);

	}
}
		

/*
 * RecordMultImageSales ($cart)
 * Add the sale quantity to the amount sold for each item.
 * We get all sale rows with the same invoice ID
 */
function RecordMultiImageSales ($invoice) {
	$sales = FetchSalesByInvoice ($invoice);
	foreach ($sales as $sale) {
			$saleID = $sale['id'];
			RecordImageSaleBySale ($saleID);
		}
}


/*
	RecordImageSaleBySale($saleID)
	Look up the record in the Sales DB, add the quantity to the price set for that image.
	This assumes a priceset for the image exists, that CreateImagePriceSets has been called!
*/
function RecordImageSaleBySale ($saleID) {
	global $error, $msg;

	$DEBUG = 0;

	// Get the order from the sales db
	$sale = GetRecord (DB_SALES, $saleID);
	$imageID = $sale['item_id'];

	CreateUniqueImagePriceSet ($imageID, $saleID);

	$quantity = $sale["quantity"];
	$image = FetchImage ($imageID);
	$pricesetID = $image['PriceSetID'];
	$spec = UnpackSpec ($sale['item_spec']);

	//$priceset = UnpackPriceSetRow(FetchPriceSetByID ($pricesetID));
	$priceset = GetPriceset($pricesetID);
	$editionsize = GetImageEditionSize($imageID, $priceset['TotalEditionSize'], $image);



	$amounts = $priceset['Amount'];
	$amounts[$spec[FP_ORDER_ROW]-1] += $quantity;
	$vars = array("Amount"=>$amounts);
	$vars = CompactArraysToStrings ($vars);

	if ($DEBUG) {
		$pricesetb = FetchPriceSetByID ($pricesetID);
		//fp_error_log(__FUNCTION__.ArrayToTable ($pricesetb), 3, FP_ORDER_LOG);
	} else {
		UpdateRecord( DB_PRICESETS, $pricesetID, $vars);
	}

	// If image is now sold out, clear the html server caches so we don't offer it!
	// Note the "-1"...because we haven't reloaded the $priceset!!!


	$s = $editionsize - $priceset['AmountSold'] - 1;
	fp_error_log(__FILE__.":".__FUNCTION__.": TotalEditionSize: {$editionsize}, remaining prints: $s", 3, FP_ACTIVITY_LOG);

	if ($editionsize>0 && ($s<=0))
	{
		ClearAllCache();
		fp_error_log(__FUNCTION__.": Clear Cache because image #{$imageID} sold out.", 3, FP_ACTIVITY_LOG);

	}
}


// GetImageEditionSize ($ID)
// Get the edition size for an image. The value could come from the image record, or the parent project.
//	$ID : image record ID
// 	$editionsize : edition size from the price set in case we have it, so we don't reload, for efficiency
// 	$record	: the image record, in case we already have it, so we don't reload, for efficiency
function GetImageEditionSize ($imageID, $editionsize = null, $image = array()) {

	if (!$image)
		$image = FetchImage($imageID);

	if (!$editionsize) {
		$pricesetID = FetchPriceSetID($imageID);
		$priceset = GetPriceSet($pricesetID);
	}

	$params = DecodeArrayFromDB($image['Params']);
	$customeditionsize =  GetParam ($params, FP_PARAM_IMAGE_EDITION_SIZE);

	if (GetParam ($params, FP_PARAM_IMAGE_IS_ORIGINAL_ART)) {
		$editionsize = 1;
	} else {
		// If edition is locked, we get edition size from the price set!!!
		// Otherwise, get edition size from image (if set), else price set.
		if (!$image['EditionsLocked']) {
			($customeditionsize > 0) && ($editionsize = $customeditionsize);
		}
	}
	return $editionsize;
}





/*
	============================================================
	RemoveImageSale($saleID)
	Look up the record in the Sales DB, SUBTRACT the quantity to the price set for that image.
	This assumes a priceset for the image exists, that CreateImagePriceSets has been called!
*/
function RemoveImageSale ($saleID) {
	global $error, $msg;
	
	$DEBUG = 0;

	// Get the order from the sales db
	$sale = GetRecord (DB_SALES, $saleID);
	$imageID = $sale['item_id'];

	$quantity = $sale["quantity"];
	$image = FetchImage ($imageID);
	$pricesetID = $image['PriceSetID'];
	$spec = UnpackSpec ($sale['item_spec']);
	$priceset = UnpackPriceSetRow(FetchPriceSetByID ($pricesetID));
	$key = array_search($sale['Size'], $priceset['Size']);
	$priceset['Amount'][$key] -= $quantity;
	$priceset['Amount'][$key] < 0 && $priceset['Amount'][$key] = 0;
	$vars = array("Amount"=>$priceset['Amount']);
	$vars = CompactArraysToStrings ($vars);

	UpdateRecord( DB_PRICESETS, $pricesetID, $vars);
	
	if ($DEBUG) {
		$priceset = FetchPriceSetByID ($pricesetID);
		fp_error_log(__FUNCTION__.ArrayToTable ($priceset), 3, FP_ORDER_LOG);
	}
}
		


/*
	============================================================
	function CreateUniqueImagePriceSet ($imageID, $saleID)
	
	If $saleID is set, the $imageID is taken from the SALES database. Otherwise, $imageID is used.

	Copy the priceset for an image to make a new priceset that is
	linked directly to the image. This allows general price sets, which
	may change, but once a picture has been sold we lock down the pricing
	by giving the picture it's own priceset.
	Also, copy the image's edition size setting, if it is set, to override the price set's value.
	
	Fixed Size images:
	When we call FetchPriceSetByID for a fixed size image, we don't get the entire price set data.
	Instead, we get a single entry that matches the fixed size image.
	
*/

function CreateUniqueImagePriceSet ($imageID, $saleID=null)
{
	global $error, $msg;

	$DEBUG = 0;
	$DEBUG && print __FUNCTION__.'<hr>';
	$NOWRITE = false;
	$NOWRITE && $msg .= __FUNCTION__.": TESTING --- NOT WRITING REAL DATA!<br>";

	// If there is a sale involved, i.e. we're locking down after a sale,
	// get the sale info.
	// Otherwise, get info from the source price set.
	if ($saleID) {
		$cart = GetRecord (DB_SALES, $saleID);
		$imageID = $cart['item_id'];
		$supplierID = $cart['SupplierID'];
	}
	
	if (!$imageID)
		return false;
	
	// Is there a priceset for this image ID already?
	if (CountRecordsByValue(DB_PRICESETS, "ImageID", $imageID))
		return false;
	
	
	$image = FetchImage ($imageID);
	
	// Get the price set. If it is a general one (not exclusive to the image)
	// then create new price set for image based on current set.
	// Set the supplier ID

	$pricesetID = FetchPriceSetID ($imageID);
	$priceset = FetchPriceSetByID ($pricesetID);

	$DEBUG && print __FUNCTION__.": imageID = $imageID, pricesetID = $pricesetID<BR>";

	// Get the supplier (printer) ID. If none set, get the artist's default supplier.
	$supplierID || $supplierID = $priceset['SupplierID'];
	if ($supplierID < 1) {
		$artist = FetchArtist ($image["ArtistID"]);
		$supplierID = $artist["Vendor"];
	}
	
	$DEBUG && print "<HR>PRICE SET<HR>";
	$DEBUG && var_dump ($priceset);
	
	if ($priceset and $priceset['ImageID'] != $imageID) {
		$priceset['SourceID'] = $priceset['ID'];	// save ID of source record
		unset ($priceset['ID']);	// or we just recreate an existing record!
		$priceset['SupplierID'] = $supplierID;
		$priceset['Title'] = CatalogNumber($imageID);
		$priceset['ImageID'] = $imageID;
		
		/*
		// We'll assume that the source PriceSet info is good, and NOT
		// get the data from the actual sale (as done below).
			
		$spec = UnpackSpec ($cart["item_spec"]);
		$priceset['PaperCode'] = $spec[FP_ORDER_PAPERCODE];
		$priceset['InksetCode'] = $spec[FP_ORDER_INKSETCODE];
		*/
		
		// ---
		// Create the new price set based on the calculated values!
		// Otherwise, the prices could, in theory, change if something
		// changed in how we calculate stuff. Rather than risk that, 
		// we'll lock down the actual prices at the time.
		
		// Lock the names of the paper and ink. These could easily change, so we 
		// want to capture them as they were at time of sale.
		$priceset['Paper'] = PaperName ($supplierID, $priceset['PaperCode']);
		$priceset['Inkset']= InkName ($supplierID, $priceset['InksetCode']);

		$pricing = GetPricingByID ($imageID, $supplierID, $pricesetID);

		$DEBUG && print "<HR>CALCULATED PRICING BASED ON ACTUAL IMAGE SIZE<HR>";
		$DEBUG && var_dump($pricing);
		
		$priceset['Size'] = $pricing['Sizes'];
		// $priceset['Markup'] is blank so it is zero for all

		$priceset['PrintCost'] = $pricing['PrintCosts'];
		$priceset['PrintPrice'] = $pricing['PrintPrices'];
		$priceset['MatteCost'] = $pricing['MatteCosts'];
		$priceset['MattePrice'] = $pricing['MattePrices'];
		$priceset['FrameToPrintCost'] = $pricing['FrameToPrintCosts'];
		$priceset['FrameToPrintPrice'] = $pricing['FrameToPrintPrices'];
		
		// This is confusing: a priceset stores the cost/price of a frame which will fit a matte
		// as "FrameMatteCost" and "FrameMattePrice". However, GetPricing uses that name for
		// a combination frame+matte.
		$priceset['FrameMatteCost'] = $pricing['FrameCosts'];
		$priceset['FrameMattePrice'] = $pricing['FramePrices'];

		$priceset['PrintShipWeight'] = $pricing['PrintShipWeight'];
		$priceset['MatteShipWeight'] = $pricing['MatteShipWeight'];
		$priceset['FrameMatteShipWeight'] = $pricing['FrameMatteShipWeight'];
		$priceset['FrameToPrintShipWeight'] = $pricing['FrameToPrintShipWeight'];


		$priceset['matchprintcost'] = $pricing['MatchPrintPrice'];
		// Get the edition size --- might be set by the image itself.
		$priceset['TotalEditionSize'] = GetImageEditionSize ($imageID, $priceset['TotalEditionSize'], $image);
		
		$priceset = SortAndCleanPriceSetRows($priceset);
		$priceset = CompactArraysToStrings ($priceset);
		if (!$NOWRITE) {
			$pricesetID = EditTable ("insert", DB_PRICESETS, '', $priceset);
			
			$DEBUG && print __FUNCTION__.": Created new price set for image $imageID : pricesetID = $pricesetID<BR>";
	
			// Set the image to use the new price set
			// Save the image's priceset, which might not be null.
			$oldPID = $image['PriceSetID'];
			$params = DecodeArrayFromDB ($image['Params']);
			$params = SetParam ($params, FP_PARAM_IMAGE_OLD_PID, $oldPID);
			$image['Params'] = $params;
			// set the EditionsLocked flag
			$vars = array ("Params" => $params, "PriceSetID" => $pricesetID, "EditionsLocked"=>'1');
			UpdateRecord( DB_IMAGES, $imageID, $vars );
			$DEBUG && print __FUNCTION__.": Updated image to point to new price set $pricesetID<BR>";
		} else {
			print __FUNCTION__.": new price set based on #{$pricesetID}<hr>";
			print_r ($priceset);
		}
	}
	$DEBUG && print __FUNCTION__.": ### END<hr>";
}



/*
	============================================================
	AJAX: GetCouponDiscountForJS
	Validate a coupon and get the discount for Javascript.
	returns { value : float, error:text, description: text }
*/
function GetCouponDiscountForJS () {
	$DEBUG = true;
	
	// Get JSON data
	if ($_POST) {
		$data = json_decode(stripslashes($_POST['data']), true);
	}
	$code = $data['couponcode'];
	$coupons = FetchCoupons ($data['artistid']);
	
	$DEBUG && fp_error_log(__FUNCTION__. ": couponcode=".$data['couponcode'].', artistid='.$data['artistid'], 3, FP_ERROR_LOG);
	
	$result = array();
	
	if ($coupons[$code]) {
		// valid
		$discount = $coupons[$code]['discount'];
		$desc = $coupons[$code]['description'];
		$result['value'] = $discount;
		$result['description'] = $desc;
		
		$result['error'] = "";
	} else {
		// invalid
		$result['value'] = 0;
		$result['error'] = FetchSnippet("error_unknown_coupon");
	}
	
	// Return JSON values
	header("Content-type: text/plain");
	echo json_encode($result);
}


/*
	============================================================
	GetCouponDiscount ($code : string)
	Validate a coupon and get the discount rate
	returns: $result : array(value:float, description:string, error:string)
*/
function GetCouponDiscount ($code, $artistID) {

	$coupons = FetchCoupons ($artistID);

	$result = array();

	if ($coupons[$code]) {
		// valid
		$discount = $coupons[$code]['discount'];
		$desc = $coupons[$code]['description'];
		$result['value'] = $discount;
		$result['description'] = $desc;
		$result['error'] = "";
	} else {
		// invalid
		$result['value'] = 0;
		$result['description'] = "";
		$result['error'] = FetchSnippet("error_unknown_coupon");
	}
	return $result;
}


/*
 * DiscountedPrice ($itemPrice, $itemCost, $couponInfo)
 * Given the price, cost, and coupon info (from GetCouponDiscount),
 * return the discounted item price.
 */

function DiscountedPrice ($itemPrice, $itemCost, $couponInfo) {
	$discount = $couponInfo['value'];
	$discountrate = $discount/100;
	$itemPriceDiscounted = $itemPrice - ($itemPrice * $discountrate);
	if ($itemPrice < $itemCost)
		$itemPrice = $itemCost;
	return $itemPriceDiscounted;
}



/*
	AJAX: GetCartPricingForJS
	Return pricing info via JSON for Javascript.
*/
function GetCartPricingForJS () {
	// Get JSON data
	if ($_POST) {
		$data = json_decode(stripslashes($_POST['data']), true);
	}
	$cartprices = GetAllCartPricingByID ($data['imageID'], $data['supplierID'], $data['pricesetID']);
	
	$result = $cartprices;
	
	//fp_error_log("GetCartPricingForJS: ".$data['imageID'].$data['supplierID'].$data['pricesetID'], 3, FP_MAINTENANCE_LOG);
	$result || $result = "ERROR: {$data['imageID']}, {$data['supplierID']}, {$data['pricesetID']}";
	
	// Return JSON values
	header("Content-type: text/plain");
	echo json_encode($result);
}


// AJAX: Get pricing for a single price set entry. Used to show prices while editing prices in admin.
function GetSamplePricingOneSizeForJS () {
	// Get JSON data
	if ($_POST) {
		$data = json_decode(stripslashes($_POST['data']), true);
	}
	
	$supplierID = $data['supplierID'];
	$pricesetID = $data['pricesetID'];
	$i = $data['index'];
	$imageID = $data['imageID'];
	
	$supplier = FetchSupplier ($supplierID);
	$pricing = GetPricingByID ($imageID, $supplierID, $pricesetID);
	$priceset = GetPriceSet ($pricesetID);
	
	// 35mm 3:2 shape
	$w = "3000";
	$h = "2000";
	
	$dims = GetPrintDimensions($w, $h);
	$pp = GetPrintPrices ($i, $dims, $supplier, $priceset);
	//$pp = GetCartPricingOneSize ($pricing, $supplier, $i);
	
	$result = $pp;
	
	//fp_error_log("GetCartPricingForJS: ".$data['imageID'].$data['supplierID'].$data['pricesetID'], 3, FP_MAINTENANCE_LOG);
	$result || $result = "ERROR: {$data['imageID']}, {$data['supplierID']}, {$data['pricesetID']}";
	
	// Return JSON values
	header("Content-type: text/plain");
	echo json_encode($result);

}


/*
	GetSamplePricing ($supplierID, $pricesetID, $i)
	Get prices for a sample image, to show as an example
*/
function GetSamplePricing ($supplierID, $pricesetID, $imageID = null) {
	// 35mm 3:2 shape
	$w = "24";
	$h = "16";
	$dims = array("width"=>$w,"height"=>$h);
	$pp = GetSampleAllCartPricingByID ($imageID, $supplierID, $pricesetID, $dims);
	return $pp;

}

// Get sample sizing for a given size, e.g. 24" wide, based on 3:2 shape picture.
function GetSampleSize ($width) {
	// 35mm 3:2 shape
	$height = (2/3) * $width;
	$dims = GetPrintDimensions($width, $height);
	return $dims;
}

/*
	GetCartPricingByID
	Get info used for adding an entry to the shopping cart (except for shipping).
	Get price/cost/handling/packing dimensions/tax/total for a given image ID.
	We return a complete array of all variations, so that we get the data once for an image.
	The buyer may switch between options before making a selection; we'll provide the front-end
	all data at once.
	
	$size: larger side of the image measured in inches/cm (depending on system settings)
	$i : the index in sizes list. If sizes = (10,16,24), then $i=1 means choose size 16.
*/
function GetCartPricingOneSize ($pricing, $supplier, $i) {
	global $msg, $error;

	$DEBUG = 0;
	$DEBUG && print __FUNCTION__;

	$size = $pricing['Sizes'][$i];
	
	$item = array();
	
	// Costs
	$item['PrintCost'] = $pricing['PrintCosts'][$i];
	$item['MatteCost'] = $pricing['MatteCosts'][$i];
	$item['FrameToPrintCost'] = $pricing['FrameToPrintCosts'][$i];
	//$item['FrameCost'] = $pricing['FrameCosts'][$i];
	$item['FrameMatteCost'] = $pricing['FrameMatteCosts'][$i];
	
	// Prices of Parts of packages: print, matte, frame (to fit print), frame (to fit matte)
	$item['PrintPrice'] = $pricing['PrintPrices'][$i];
	$item['MattePrice'] = $pricing['MattePrices'][$i];
	$item['FrameToPrintPrice'] = $pricing['FrameToPrintPrices'][$i];
	$item['FramePrice'] = $pricing['FramePrices'][$i];
	$item['FrameMattePrice'] = $pricing['FrameMattePrices'][$i];
	
	// Handling:
	$item['PrintHandling'] = $supplier['PrintHandling'];
	$item['MatteHandling'] = $supplier['MatteHandling'];
	$item['FrameToPrintHandling'] = $supplier['FrameHandling'];
	$item['FrameMatteHandling'] = $supplier['FrameHandling'];
	$item['PrintHandlingIntl'] = $supplier['PrintHandlingIntl'];
	$item['MatteHandlingIntl'] = $supplier['MatteHandlingIntl'];
	$item['FrameToPrintHandlingIntl'] = $supplier['FrameHandlingIntl'];
	$item['FrameMatteHandlingIntl'] = $supplier['FrameHandlingIntl'];
	
	$item['SupplierState'] = $supplier['State'];
	$item['SupplierCountry'] = $supplier['Country'];

	// Weight:
	$item['PrintShipWeight'] = $pricing['PrintShipWeight'][$i];
	$item['MatteShipWeight'] = $pricing['MatteShipWeight'][$i];
	$item['FrameToPrintShipWeight'] = $pricing['FrameToPrintShipWeight'][$i];
	//$item['FrameShipWeight'] = $pricing['FrameShipWeight'][$i];
	$item['FrameMatteShipWeight'] = $pricing['FrameMatteShipWeight'][$i];
	
	// Dimensions
	// Use the supplier packing % setting to calculate the height/width of the package.
	// Depth is a separate setting. If no % is set, use the weight setting, e.g. for a print.
	//=CONCATENATE((C9*(1+G24)); "x";(D9*(1+G24)))
	// Get packaging dimensions


	$arr = array (
		array('Print', 'imageDims', 'PrintPackageDims', 'PrintDepth'),
		array('Matte', 'frameDims', 'MattePackageDims', 'MatteDepth'),
		array('Frame', 'imageDims', 'FrameToPrintPackageDims', 'FrameDepth'),
		array('Frame', 'frameDims', 'FramePackageDims', 'FrameDepth')
	);

	foreach ($arr as $r) {
		list($pkg,$dimsname,$pkgdimsname,$depthname) = $r;
		$pkgfield = $pkg.'Packing';
		if ($supplier[$pkgfield]) {
			// Get packaging dimensions
			$dims = $pricing['DimensionsBySide'][$size][$dimsname];
			list ($pkgWidth, $pkgHeight, $pkgDepth, $packing, $pkgDims) = CalcPackagingSize($supplier[$pkgfield], $dims, (float)$supplier[$depthname], FP_PACKAGING_METHOD);

			$item[$pkgdimsname]['display'] = $pkgDims;
			$item[$pkgdimsname]['width'] = $pkgWidth;
			$item[$pkgdimsname]['height'] = $pkgHeight;
			$item[$pkgdimsname]['depth'] = $pkgDepth;
			//weight
			$item[$r[0].'Weight'] = (float)$supplier[$r[0].'Weight'];
		}
	}

	// If packaging% is zero for a print in the Supplier setup, assume we use a tube.
	// Then we overwrite the above calculated values.
	// Then, the "depth" is the diameter of the tube, and the tube is the length of the print + something.
	// If there is a tube entry for a print, overwrite the calculated print packing
	if (!$supplier['PrintPacking']) {
		$pkgHeight = (float)$supplier['PrintDepth'];
		$pkgWidth = (float)$supplier['PrintDepth'];
		// Add 4" to the tube length for extra.
		$pkgDepth = $size + FP_SHIPPING_TUBE_EXTRA;
		$pkgDims = $pkgWidth." x ".$pkgHeight." x ".$pkgDepth;
		$item['PrintPackageDims']['display'] = $pkgDims;
		$item['PrintPackageDims']['width'] = $pkgWidth;
		$item['PrintPackageDims']['height'] = $pkgHeight;
		$item['PrintPackageDims']['depth'] = $pkgDepth;
	}

	// Pretax Cost totals (taxables) (handling is not taxable if it is a separate item on the invoice)
	// Print alone
	$item['PretaxPrintCost'] = $item['PrintCost'];
	// print + matte
	$item['PretaxMatteCost'] = $item['PrintCost'] + $item['MatteCost'];
	// print + frame
	$item['PretaxFrameToPrintCost'] = $item['PrintCost'] + $item['FrameToPrintCost'];
	//print + matte + frame
	$item['PretaxFrameMatteCost'] = $item['PrintCost'] + $item['FrameMatteCost'];
	

	// Packages: Pretax Price Totals
	// print alone
	$item['TotalPrintPrice'] = $item['PrintPrice'];
	// print + matte
	$item['TotalMattePrice'] = $item['PrintPrice'] + $item['MattePrice'];
	// print + frame
	$item['TotalFrameToPrintPrice'] = $item['PrintPrice'] + $item['FrameToPrintPrice'];
	//print + matte + frame
	$item['TotalFrameMattePrice'] = $item['PrintPrice'] + $item['FrameMattePrice'];

	// Calculate sales tax (even if not needed)
	$item['SalesTaxRate'] = $supplier['SalesTaxRate'];
	$salestax = $item['SalesTaxRate']/100;
	// Tax whole sale, or just the printing part? Depends on FP_TAX_WHOLE_SALE, a config item.
	// Individuals might choose to be tax exempt inside their own state.
	if (FP_TAX_WHOLE_SALE)
		{
		$item['PrintSalesTax'] = round($salestax * ($item['TotalPrintPrice']),2);
		$item['MatteSalesTax'] = round($salestax * ($item['TotalMattePrice']),2);
		$item['FrameToPrintSalesTax'] = round($salestax * ($item['TotalFrameToPrintPrice']),2);
		$item['FrameMatteSalesTax'] = round($salestax * ($item['TotalFrameMattePrice']),2);
		$item['TaxMethodMsg'] = FetchSnippet('taxmethod_whole_sale_msg');
		}
	else 
		{
		$item['PrintSalesTax'] = round($salestax * $item['PretaxPrintCost'],2);
		$item['MatteSalesTax'] = round($salestax * $item['PretaxMatteCost'],2);
		$item['FrameToPrintSalesTax'] = round($salestax * $item['PretaxFrameToPrintCost'],2);
		$item['FrameMatteSalesTax'] = round($salestax * $item['PretaxFrameMatteCost'],2);
		$item['TaxMethodMsg'] = FetchSnippet('taxmethod_only_printing_msg');
		}
	
	
	// For the total, the page must add handling, shipping and sales tax. Handling and shipping must be shown
	// as separate line items or they are taxable!

	// Add this in, so it's available 
	$item['MatchPrintPrice'] = $supplier['MatchPrintPrice'];
	
	$item['size'] = $size;
	//$DEBUG && var_dump($item);
	return $item;
}

/*
 * CalcPackagingSize
 * There are two methods for calculating packaging size.
 * We can use a percentage setting, and add that percentage to the image size.
 * So, a 10" print goes in a 11" package at a 10% setting.
 * Frankly, this method isn't going to work!
 * Method 2 is to add a fixed amount, e.g. 3" to height and width. So
 * a 10" print with 3" packaging becomes a 13" package.
 * Depth is a different setting.
 * PARAMS:
 * $pkgfield : set in the vendor's profile. Either a percentage or a value in system units (e.g. inches)
 */
function CalcPackagingSize ($packing, $dims, $depth, $method = "fixed") {
	if ($method == "fixed") {
		$pkgWidth = $dims['width'] + $packing;
		$pkgHeight = $dims['height'] + $packing;
		$pkgDepth = $depth;
	} else {
		$packing = 1+($packing/100);
		$pkgWidth = ceil($dims['width'] * $packing);
		$pkgHeight = ceil($dims['height'] * $packing);
		$pkgDepth = $depth;
	}

	$pkgDims = $pkgWidth." x ".$pkgHeight." x ".$pkgDepth;
	$packaging = array ($pkgWidth, $pkgHeight, $pkgDepth, $packing, $pkgDims);
	return $packaging;
}

// GetAllCartPricingByID
// Get all pricing information for the shopping cart, for all sizes
// Returns an array of pricing sets, for each size, indexed by 0,1,2...
// corresponding to the size in the list of sizes
function GetAllCartPricingByID ($imageID, $supplierID, $pricesetID, $dims=null, $ignoreInactiveFlag = false) {

	$DEBUG = 0;
	$DEBUG && print __FUNCTION__;

	$supplier = FetchSupplier ($supplierID);
	// Before, the final param (get inactive/on-hold prices) was true! Why?
	$pricing = GetPricingByID ($imageID, $supplierID, $pricesetID, $dims, $ignoreInactiveFlag);
	for ($i=0;$i<count($pricing['Sizes']);$i++) {
		$row = GetCartPricingOneSize ($pricing, $supplier, $i);
		$cartprices[] = $row;
	}
	
	$DEBUG && var_dump($cartprices);
	return $cartprices;
}

function GetSampleAllCartPricingByID ($imageID, $supplierID, $pricesetID, $dims=null)
{
	return GetAllCartPricingByID ($imageID, $supplierID, $pricesetID, $dims, true);
}


// GetOneCartPricingByID
// Get all pricing information for the shopping cart, for one size
function GetOneCartPricingByID ($imageID, $supplierID, $pricesetID, $size) {

	$DEBUG = 0;
	$DEBUG && print __FUNCTION__;

	$supplier = FetchSupplier ($supplierID);
	$pricing = GetPricingByID ($imageID, $supplierID, $pricesetID);
	
	$sizes = $pricing['Sizes'];	// array of sizes, e.g. (11, 24, 30) etc.
	$i = array_search($size, $sizes);
	$cartprices = GetCartPricingOneSize ($pricing, $supplier, $i);
	
	$DEBUG && var_dump($cartprices);
	return $cartprices;
}



/*
	============================================================
	GetPricingByID
	Get print/matte/frame to print/frame/matte+frame pricing for a given image ID.
	Return array with values.
	$dims = array(height,width) : can be used if you don't have an imageID, e.g. for a sample sizing
	$size: larger side of the image measured in inches/cm (depending on system settings)
	$ignoreInactiveFlag: return settings for priceset entries that are marked "on hold"
*/	

function GetPricingByID ($imageID, $supplierID, $pricesetID, $dims=null, $ignoreInactiveFlag = false) {
	global $msg, $error;

	$DEBUG = 0;
	$DEBUG && print __FUNCTION__;
	$DEBUG && print "<BR>GetPricingByID ($imageID, $supplierID, $pricesetID, $dims=null, $ignoreInactiveFlag = false)<BR>";
	$DEBUG && print "<BR>Called by: ".getCallingFunction()."<HR>";
	
	$supplier = FetchSupplier ($supplierID);
	$priceset = GetPriceSet ($pricesetID);

	// Print cost/price
	$pricing = GetAllPrintPricesByID($imageID, $supplier, $priceset, $dims, $ignoreInactiveFlag);

	$DEBUG && var_dump($pricing);
	return $pricing;
}


// ============================================================
// GetAllPrintPricesByID
// list($printCost, $printPrice) = GetPrintPrices($image, $supplier, $priceset);
// where $image, $supplier, $priceset are records and a price set.
// Calculate the price of a print from the paper area, the supplier's rates, 
// and the price set.
// *Unused parts of the price set are not included, i.e. a size that has been made inactive
// because the item is a "fixed size" item.
function GetAllPrintPricesByID ($imageID, $supplier, $priceset, $dims = null, $ignoreInactiveFlag = false) {
	global $PRINTDPI;
	
	$DEBUG = 0;
	$DEBUG && print __FUNCTION__;
	$DEBUG && print "<BR>GetAllPrintPricesByID ($imageID, $supplier, $priceset, $dims = null, $ignoreInactiveFlag = false)<BR>";
	$DEBUG && print "<BR>Called by: ".getCallingFunction()."<HR>";

	// Get ratio of sides for this image
	if ($dims) {
		$width = $dims['width'];
		$height = $dims['height'];
	} else {
		$imgsize = GetImageSizeByID ($imageID);
		$width = $imgsize['width_print'];
		$height = $imgsize['height_print'];
	}
	
	if (!($width + $height)) {
		return array();
	}
	
	$allinfo = array();
	$printPrices = array ();
	$printCosts = array ();

	// If this is a fixed size image, we have to get the print sizing from the image, not the price set
	$image = FetchImage($imageID);
	$params = DecodeArrayFromDB ($image['Params']);
	$fWidth = GetParam ($params, FP_PARAM_IMAGE_FIXED_WIDTH);
	$fHeight = GetParam ($params, FP_PARAM_IMAGE_FIXED_HEIGHT);
	if (GetParam ($params, FP_PARAM_IMAGE_IS_FIXED_SIZE) && $fWidth && $fHeight) {
		$orientation = "landscape";
		($fWidth > $fHeight)
		? $s = $fWidth
		: $s = $fHeight;
		$printsizes = array ( $s => $s );
		$fsi = true;
		$fixedsize = $s;
		$width = ceil($fWidth * $PRINTDPI);
		$height = ceil($fHeight * $PRINTDPI);
	} else {
		$fsi = false;
	}

	//$allinfo['Sizes'] = $priceset['Size'];

	/*
	 * If this is a fixed sized image, consider all smaller sizes "inactive",
	 * and only the next larger size to be "active".
	 */

	 $allinfo['Handling'] = $supplier['Handling'];

	// if fixed size, find the corresponding next size (if it exists)
	$sizeForPricing = 0;
	if ($fsi) {
		for ($k=0;$k<count($priceset['Size']);$k++) {
			if (!$priceset['Inactive'][$k]) {
				$size = $priceset['Size'][$k];
				if ($size < $fixedsize)
					continue;
				$sizeForPricing = $size;
				break;
			}
		}
	}

	// If the image has a fixed price, set the prices to it.
	// Change PrintPrice, a_PrintPrice to match the fixed price
	$fixedprice = GetParam ($params, FP_PARAM_IMAGE_FIXED_PRICE);
	if ($fixedprice > 0)
		{
		$priceset = FixPriceInPriceset($priceset, $fixedprice);
		//var_dump($priceset);
		}

	// If there is no price for the fixed price image, return nothing.
	if ($fsi && !$sizeForPricing) {
		return array ();
	}

	$i=0;
	for ($k=0;$k<count($priceset['Size']);$k++) {
		$inactive = $priceset['Inactive'][$k];
		$size =  $priceset['Size'][$k];

		// if Fixed Size, then skip over all prices but the chosen one, $sizeForPricing
		if ($fsi) {
			if ($size != $sizeForPricing) {
				continue;
			} else {
				$size = $fixedsize;
			}
		}
		
		if ($ignoreInactiveFlag || !$inactive ) {
			$allinfo['Sizes'][$i] = $size;			
			
			list($w,$h) = GetImageOtherSide ($width, $height, $size);
			$dims = GetPrintDimensions($w, $h);

			$allinfo['Dimensions'][$i] = $dims;
			// also index by the side
			$allinfo['DimensionsBySide'][$size] = $dims;
			
			// Print only
			$res = GetPrintPrices ($k, $dims, $supplier, $priceset);
			$allinfo['PrintCosts'][$i] = $res['PrintCost'];
			$allinfo['PrintPrices'][$i] = $res['PrintPrice'];
			

			// matte only, no print
			$res = GetMattePrices($k, $dims['matteArea'], $supplier, $priceset);
			$allinfo['MatteCosts'][$i] = $res['MatteCost'];
			$allinfo['MattePrices'][$i] = $res['MattePrice'];
			
			// Frame only (to fit the print image), no print
			$res = GetFrameToPrintPrices($k, $dims['imageArea'], $supplier, $priceset);
			$allinfo['FrameToPrintCosts'][$i] = $res['FrameToPrintCost'];
			$allinfo['FrameToPrintPrices'][$i] = $res['FrameToPrintPrice'];

			// Frame only, to fit a matte, no print
			$res = GetFramePrices($k, $dims['frameArea'], $supplier, $priceset);
			$allinfo['FrameCosts'][$i] = $res['FrameCost'];
			$allinfo['FramePrices'][$i] = $res['FramePrice'];

			// Frame + Matte only, no print
			$allinfo['FrameMatteCosts'][$i] = $allinfo['FrameCosts'][$i] + $allinfo['MatteCosts'][$i];
			$allinfo['FrameMattePrices'][$i] = $allinfo['FramePrices'][$i] + $allinfo['MattePrices'][$i];
			
			// weights
			$allinfo['PrintShipWeight'][$i] =  $priceset['PrintShipWeight'][$k];
			$allinfo['MatteShipWeight'][$i] =  $priceset['MatteShipWeight'][$k];
			$allinfo['FrameToPrintShipWeight'][$i] =  $priceset['FrameToPrintShipWeight'][$k];
			$allinfo['FrameMatteShipWeight'][$i] =  $priceset['FrameMatteShipWeight'][$k];
			$allinfo['MatchPrintPrice'] = $supplier['MatchPrintPrice'];

			$i++;
		}
	}
	$DEBUG && var_dump( $allinfo);
	$DEBUG && print "END:".__FUNCTION__;
	
	return $allinfo;
}

/*
	array FixPriceInPriceset(array $priceset, float $fixedprice)
	Change PrintPrice, a_PrintPrice to match the fixed price.
	Don't change if the priceset is "Not for sale", id=1
*/

function FixPriceInPriceset ($priceset, $fixedprice) {
	if ($priceset['ID'] != 1 && (count($priceset['PrintPrice']) > 0 )) {
		$priceset['PrintPrice'] = array_fill(0,count($priceset['PrintPrice']), $fixedprice);
		$priceset['a_PrintPrice'] = join(',', $priceset['PrintPrice']);
	}
	return $priceset;
}



// Cost/Price of a print
// $i is the index in the Size array
function GetPrintPrices ($i, $dims, $supplier, $priceset) {
	global $msg, $error;
	
	$DEBUG = false;
	
	$width = $dims['paperDims']['width'];
	$height = $dims['paperDims']['height'];
	$x = $dims['bigside'];
	$y = $dims['smallside'];

	// the side measurements are in inches/cm, while area is in m2 or ft2.
	$paperArea =  $dims['paperArea'];
	$size = $priceset['Size'][$i];

	$price = $priceset['PrintPrice'][$i];
	$cost = $priceset['PrintCost'][$i];
	
	if (!$cost)
		$cost = $supplier['PrintAreaPrice'] * $paperArea;

	if ($cost < $supplier['PrintMinPrice'])
		$cost = $supplier['PrintMinPrice'];

	if (!$price)
		$price = (1+($priceset['Markup'][$i]/100)) * $cost;

	$DEBUG && $error .= __FUNCTION__.__LINE__." price = $price<BR>";

	$tierMultiplier = floor($priceset['AmountSold'] / $priceset['Tier']);
	$price = $price + ($tierMultiplier * $price * ($priceset['Inflation']/100));

	$DEBUG && $error .= __FUNCTION__.__LINE__." price=$price after adding (floor({$priceset['AmountSold']} / {$priceset['Tier']}) * $price * ({$priceset['Inflation']}/100))<BR>";

	$res = array (
		"PrintCost"=>round($cost,2), 
		"PrintPrice"=>round($price,2)
		);
	return $res;
}


// Cost/Price of a matte
function GetMattePrices($i, $matteArea, $supplier, $priceset) {
	// Get fixed matte cost
	$price = $priceset['MattePrice'][$i];
	$cost = $priceset['MatteCost'][$i];

	//if (!$cost)
	//	$cost = $supplier['MatteAreaPrice'] * $matteArea;

	isset ($supplier['Params']) ? $params = DecodeArrayFromDB($supplier['Params']) : $params = array ();
	(GetParam ($params, FP_PARAM_SUPPLIER_MATTE_COST_METHOD))
	? $method = "ui"
	: $method = "area";

	if (!$cost)
		$cost = CalcMatteCost ($supplier['MatteAreaPrice'], $matteArea, $method);

	if ($cost < $supplier['MatteMinPrice'])
		$cost = $supplier['MatteMinPrice'];
		
	if (!$price) {
		$price = (1+($priceset['Markup'][$i]/100)) * $cost;
	}
	$res = array (
		"MatteCost"=>round($cost,2),
		"MattePrice"=>round($price,2)
		);
	return $res;
}


// Cost/Price of a frame to fit the image
function GetFrameToPrintPrices($i, $imageArea, $supplier, $priceset) {
	// Get fixed frame to print cost
	$price = $priceset['FrameToPrintPrice'][$i];
	$cost = $priceset['FrameToPrintCost'][$i];
	
	isset ($supplier['Params']) ? $params = DecodeArrayFromDB($supplier['Params']) : $params = array ();
	(GetParam ($params, FP_PARAM_SUPPLIER_FRAME_COST_METHOD))
	? $method = "ui"
	: $method = "area";
	
	if (!$cost)
		$cost = CalcFrameCost ($supplier['FrameAreaPrice'], $imageArea, $method);

	if ($cost < $supplier['FrameMinPrice'])
		$cost = $supplier['FrameMinPrice'];

	if (!$price)
		$price = (1+($priceset['Markup'][$i]/100)) * $cost;

	$res = array (
		"FrameToPrintCost"=>round($cost,2),
		"FrameToPrintPrice"=>round($price,2)
		);
	return $res;
}

// Cost/Price of frame to fit a matte
function GetFramePrices($i, $frameArea, $supplier, $priceset) {

	$price = $priceset['FrameMattePrice'][$i];
	$cost = $priceset['FrameMatteCost'][$i];
	
	isset ($supplier['Params']) ? $params = DecodeArrayFromDB($supplier['Params']) : $params = array ();
	(GetParam ($params, FP_PARAM_SUPPLIER_FRAME_COST_METHOD))
	? $method = "ui"
	: $method = "area";
	
	if (!$cost)
		$cost = CalcFrameCost ($supplier['FrameAreaPrice'], $frameArea, $method);

	if ($cost < $supplier['FrameMinPrice'])
		$cost = $supplier['FrameMinPrice'];

	if (!$price)
		$price = (1+($priceset['Markup'][$i]/100)) * $cost;

	$res = array (
		"FrameCost"=>round($cost,2),
		"FramePrice"=>round($price,2)
		);
	return $res;
}

/*
	CalcFrameCost
	Frames are priced either per square foot, or by
	the "united inch" method. The UI method uses:
		(l + w) * rate, 
	where l+w is rounded up to the highest foot.
	Note, $area is already in square feet, not square inches.
*/
function CalcFrameCost ($rate, $area, $method = "area") {

	if ($method == "ui") {
		$area = CalcUnitedInch ($area);
		//$area = 2*(ceil(sqrt($area*144)/12)*12);
	}
	$cost = $rate * $area;
	$cost = round($cost,2);
	return $cost;
}

/*
	CalcMatteCost
	Mattes are priced either per square foot, or by
	the "united inch" method. The UI method uses:
		(l + w) * rate, 
	where l+w is rounded up to the highest foot.
	Note, $area is already in square feet, not square inches.
*/
function CalcMatteCost ($rate, $area, $method = "area") {
	$origArea = $area;
	if ($method == "ui") {
		$area = CalcUnitedInch ($area);
		//$area = 2*(ceil(sqrt($area*144)/12)*12);
	}
	$cost = $rate * $area;
	$cost = round($cost,2);
	
	return $cost;
}


function CalcUnitedInch ($area) {
	$area = ceil(2*(sqrt($area*144)));
	if ($area % 12 > 0) {
		$area = (floor($area/12)+1)*12;
	}
	return $area;
}


// ============================================================
// SaveCart
// Save the shopping cart with a single item (like a buy-now cart)
// Get a shared secret for PayPal validation.
// Record the sale to the SALES DB, for verification and vars not passed by Paypal.
// Write the order to our sales db 
//get the q parameter from URL
function SaveCart ($vars) {
	global $BASEDIR, $LOGS;


	$DEBUG = 0;

	$spec = urlencode($vars["spec"]);
	$desc = urlencode($vars["os0"]);	// os0 is option 1 in Paypal
	$item_number = $vars["item_number"];
	$t = date( 'Y-m-d H:i:s' );
	//$secret = $t;
	$secret = uniqid(FP_ACCOUNT_NAME);
	
	$vars['item_number'] = $item_number;
	$vars['item_desc'] = $desc;
	$vars['item_spec'] = $spec;
	$vars['order_time'] = $t;
	$vars['secret'] = $secret;
	
	// Move single-item cart variables to general vars. 
	$vars['mc_shipping'] = $vars['shipping'];
	$vars['mc_handling'] = $vars['handling'];
	
	$vars = StripNonExistantFields (DB_SALES, $vars);

	// Return a shared secret for PayPal validation.
	// Note, we store the image ID as item_number in the cart
	// But, the item_number passed to Paypal is actually the ID of the order in the Sales DB
	
	$ID = AddRecord( DB_SALES, $vars );
	
	$DEBUG && print ArrayToTable ($vars);
	
	if ($ID) {
		$result = UpdateRecord( DB_SALES, $ID, $vars );
		fp_error_log("SUCCESS: Wrote order to the Sales DB for time = $t.", 3, FP_PAYMENTS_LOG );
		// return the code from the pre-order for PayPal and later matching
		$arr = array ($ID, $t, $secret);
		return $arr;
	} else {
		fp_error_log("ERROR (".__FUNCTION__.") : Could not write order: $query", 3, FP_PAYMENTS_LOG );
		return array ();
	}
}




// ============================================================
// SaveMultiCart
// Save the shopping cart with multiple items
// Get a shared secret for PayPal validation.
// Record the sale to the SALES DB, for verification and vars not passed by Paypal.
// We'll use the Custom param for our own image ID because from the Image ID, we can discover everything else we need to know, e.g. artist.
// Write the order to our sales db
/*
 * SaveMultiCart
 *
 * Save a multi-item shopping cart to the database.
 * We verify the "secret" code with Paypal, to confirm the order.
 *
  * The cart is passed this way:
			$cart = array();
			$cart['cart_params'] = $this->cart_params;
			$cart['items'] = $this->items;
			$cart['shipping-params'] = $this->shipping_params;
			$cart['total'] = $this->total;
			$cart['shipping'] = $this->totalshipping;
			$cart['handling'] = $this->totalhandling;

 */

/*
 * SaveMultiCart ($cart)
 * There is one DB entry per item line sold. A framed picture is a different item from
 * an unframed picture, even when it is the same image.
 * Returns the unique secret used to tag all the entries.
 
	$cart = array();
	$cart['cart_params'] = $this->cart_params;
	$cart['items'] = $this->items;
	$cart['shipping_params'] = $this->shipping_params;
	$cart['total'] = $this->total;
	$cart['shipping'] = $this->totalshipping;
	$cart['handling'] = $this->totalhandling;
	$cart['discount_rate'] = $this->discount;
	$cart['discount_amount'] = $this->discount_amount;
	$cart['discount_description'] = $this->discount_description;
 
 */
function SaveMultiCart($cart) {
	global $BASEDIR, $LOGS;

	$secret = uniqid(FP_ACCOUNT_NAME);
	$txn_id = uniqid(FP_ACCOUNT_NAME);
	$invoice = uniqid();
	$t = date('Y-m-d H:i:s');
	
	// count items with same image ID, e.g. prints, framed prints, etc. 
	// all have same image ID but are different items
	$itemcounter = array();

	foreach ($cart['items'] as $item) {
		$imageID = $item['itemCatID'];
		
		// Item Values
		$sale['item_name'] = urlencode($item['itemName']);
		
		// Note, item_number is not necessarily the image ID number!
		// This is a big change...SaveCart thinks the item_number is the image ID.
		// $item['itemID'] is NOT the item's ID, it is the unique of the item in the cart!
		
		$itemcounter["{$imageID}"]++;
		
		// Old style. Now, we're trying to use simpler item numbers.
		// This value could contain the image ID, plus codes for framing, etc.!!!
		//$sale['item_number'] = urlencode($item['itemID']);
		
		// Make the item number the imageID_x
		$itemnumber = str_pad($imageID, 6, "0", STR_PAD_LEFT)."_".$itemcounter["{$imageID}"];
		$sale['item_number'] = urlencode($itemnumber);
		
		// This should be the image ID that matches the database image ID.
		$sale['item_id'] = urlencode($imageID);
		
		$sale['quantity'] = $item['itemQty'];

		// The actual dimensions are in the spec field.
		$sale['Size'] = $item['itemSizeIndex'];

		$sale['item_desc'] = urlencode($item['itemDesc']);
		$sale['item_spec'] = urlencode($item['itemOptions']);
		$sale['cost'] = $item['itemCost'];
		
		$sale['mc_gross'] = $item['total'];
		$sale['amount'] = $item['subtotal'];
		$sale['mc_shipping'] = $item['item_total_shipping'];
		$sale['mc_shipping2'] = $item['shipping_params']['shipping2'];
		$sale['mc_handling'] = $item['handling'];

		$sale['tax'] = $item['tax'];
		//$sale['tax_rate'] = $item['tax_rate'];
		$sale['weight'] = $item['weight'];
		$sale['weight_unit'] = $cart['shipping_params']['weight_std'];
	
		$sale['discount_rate'] =$item['discount_rate'];
		$sale['discount_amount'] = $item['discount_amount'];

		// $sale['option_name1'] = '';
		// $sale['option_name2'] = '';
		// $sale['option_selection1'] = '';
		// $sale['option_selection2'] = '';


		// LATER: need to get num avail prints & calc next print number,
		// unless there is a func for that!
		//$sale['PrintNumber'] = $item['itemPrintNumber'];


		// Cart values
		$sale['order_time'] = $t;
		$sale['secret'] = $secret;
		$sale['SupplierID'] = $cart['cart_params']['cart_supplier_id'];

		$sale['currency_code'] = $cart['cart_params']['cart_currency_code'];
		$sale['lc'] = $cart['cart_params']['cart_lc'];
		
		// Paypal uses 'shipping_method' as the name of one of it's own shipping calculations.
		// We'll use it to indicate our shipping calc method, instead.
		if ($cart['shipping_params']['pickup']) {
			$sale['shipping_method'] = "pickup";
		} else {
			$sale['shipping_method'] = $cart['shipping_params']['shippingName'];
		}

		//fp_error_log(__FUNCTION__.":".__LINE__.": Shipping method is {$sale['shipping_method']} for Num:{$sale['item_number']} / ID:{$sale['item_id']}", 3, FP_PAYMENTS_LOG );


		$sale['txn_id'] = '';
		// This is the key to tie together items.
		// This is a temp value...paypal will give us a new txn_id when it responds.
		$sale['invoice'] = $invoice;


		$sale = StripNonExistantFields(DB_SALES, $sale);
		$ID = AddRecord(DB_SALES, $sale);
		if (!$ID)
			return array();
	}
	$arr = array('invoice'=>$invoice, 'order_time'=>$t, 'secret'=>$secret);
	return $arr;
}

// format a price
function formatPrice ($price, $dec = 0) {
	global $msg, $error;

	$dec ? $formatstr = "%0.".$dec."f" : $formatstr = "%d";
	// new variable because of bug with sprintf
	$price2 = sprintf($formatstr, $price+0);
	$price = $price2;
	if (CURRENCY_POSITION == "before") {
		$price = CURRENCY . $price;
	} else {
		$price = $price . CURRENCY;
	}
	return $price;
}

/*
	imageIsForSale
	Is this picture for sale? Check to see:
	- marked with "do not sell" price set?
	- if it is big enough to sell.
	Note, the image's priceset always trumps any other, such as the project's.
 * RETURN: Array of one element, the largest available size
 * $sizes : array( size1 )
*/
function imageIsForSale ($ID, $pricesetID = null) {
	global $msg, $error;

	$DEBUG = 0;
	$DEBUG && $msg .= __FUNCTION__."<BR>";
	
	$image = FetchImage ($ID);
	
	isset ($image["Params"]) ? $params = DecodeArrayFromDB($image["Params"]) : $params = array ();
	if (GetParam($params,FP_PARAM_IMAGE_IS_VIDEO)) {
		return false;
	}
	
	$artist = FetchArtist($image['ArtistID']);
	
	if (!($artist ['PrintSalesID'] && $artist ['PayPalBusiness'] && $artist ['Vendor'] && $artist ['Ecommerce'])) {
		$DEBUG && $msg .= __FUNCTION__.": Image $ID not for sale, the artist is not set up for sales<BR>";
		return false;
	}
	

	$pricesetID || $pricesetID = FetchPriceSetID($ID);
	// image's value of for pricesetID always trumps any other UNLESS
	// the priceset is a "not for sale", where price1 == 0
	$priceset = GetPriceSet ($pricesetID);
	$pricesetID = FetchPriceSetID($ID);
	$DEBUG && $msg .= __FUNCTION__.": ID=$ID, pricesetID = $pricesetID<BR>";

	// Open editions
	if (($pricesetID > 0) && ($priceset['TotalEditionSize'] == 0)) {
		$sizes = CalcPrintSizesFromImageID ($ID, $priceset['printsizes'], true);	// true= max size only, i.e. only return the largest size
		if ($sizes["error"] || !count($sizes)) {
			return false;
		} else {
			return $sizes;
		}
	}

	// Limited Editions
	$editionsize = GetImageEditionSize($ID, $priceset['TotalEditionSize'], $image);
	$amountSold = $priceset['AmountSold'];
	$available = NumImagesAvailableForSale($ID);

	// Prints remain to sell, if ltd edition?
	if (($pricesetID > 0) && ($editionsize > 0) && $available)
			{
		$priceset = GetPriceSet ($pricesetID);
		$DEBUG && $msg .= __FUNCTION__.": ID=$ID, image is for sale.<BR>";
		// big enough to sell?
		$sizes = CalcPrintSizesFromImageID ($ID, $priceset['printsizes'], true);	// true= max size only, i.e. only return the largest size
		if ($sizes["error"] || !count($sizes)) {
			return false;
		} else {
			return $sizes;
		}
		}
	else 
		{
		$DEBUG && $msg .= __FUNCTION__.": ID=$ID, image is NOT for sale.<BR>";
		$DEBUG && $msg .= __FUNCTION__.": editionsize = $editionsize.<BR>";
		return false;
		}
}



/*
 * NumImagesAvailableForSale ($ID, $quantity)
 * Check that the quantity of images are available for sale, after sub
 * Return the number remaining to sell. If the result < 0, then
 *
 * Two ways of deciding this:
 * - based on total size of edition and total sales
 * - based on edition sizes for each print size. Choose the size in the list with $whichSize
 */

function NumImagesAvailableForSale($imageID) {
	global $BASEDIR, $LOGS;
	$soldout = false;

	$salesinfo = ImageSalesInfo ($imageID);
	if ($salesinfo['edition_type'] != "unlimited") {
		$remaining = $salesinfo['totaledition'] - $salesinfo['totalsold'];
	} else {
		$remaining = 1000000;
	}

	/*
	$image = FetchImage($imageID);
	$pricesetID = FetchPriceSetID($imageID);
	$priceset = GetPriceSet($pricesetID);
	$sold = $priceset['AmountSold'];
	$editionsize = GetImageEditionSize($imageID, $priceset['TotalEditionSize'], $image);

	$remaining = $editionsize - $sold;

	 */
	return $remaining;
}

/*
	ImageOnSaleStatus
	Similar to imageIsForSale(), but this gives a full explanation of why an 
	image is not for sale, to help troubleshoot.
*/

function ImageOnSaleStatus($ID, $pricesetID = null, $asTip = false, $noTip = false) {
	$forsale = imageIsForSale ($ID, $pricesetID);
	if ($forsale) {
		$statusMsg = FetchSnippet ("msg_image_for_sale");
	} else {
		$status = FetchSnippet ("msg_image_not_for_sale");
		if ($noTip) {
			$whynot = "";
		} else {
			$whynot = "&nbsp;".ImageWhyNotForSale ($ID, $asTip, $pricesetID);
		}
		$statusMsg = $status.$whynot;
	}
	return $statusMsg;
}

// Troubleshoot Image sales:
// Explain why an image is not for sale, and what to do about it.

function ImageWhyNotForSale ($ID, $format = 'list', $pricesetID = null) {
	global $msg, $error;

	$DEBUG = 0;
	$DEBUG && $msg .= __FUNCTION__."<BR>";
	
	$image = FetchImage ($ID);
	isset ($image["Params"]) ? $params = DecodeArrayFromDB($image["Params"]) : $params = array ();

	$pricesetID || $pricesetID = FetchPriceSetID($ID);
	// image's value of for pricesetID always trumps any other UNLESS
	// the priceset is a "not for sale", where price1 == 0
	$priceset = GetPriceSet ($pricesetID);
	$parentpriceset = GetPriceSet($image['PriceSetID']);
	$pricesetID = FetchPriceSetID($ID);
	$editionsize = GetImageEditionSize($ID, $priceset['TotalEditionSize'], $image);

	$DEBUG && $msg .= __FUNCTION__.": ID=$ID, pricesetID = $pricesetID<BR>";
	
	$imageSalesInfo = ImageSalesInfo ($ID);
	$sizes = $imageSalesInfo['size'];
				
	$why = array();
	
	
	$artist = FetchArtist($image['ArtistID']);
	// Artist eCommerce settings must be filled out
	if (!$artist ['PrintSalesID'])
		$why[] = FetchSnippet("msg_notforsale_artist_no_printsalesid");			

	if (!$artist ['PayPalBusiness'])
		$why[] = FetchSnippet("msg_notforsale_artist_no_paypalbusiness");			

	if (!$artist ['Vendor'])
		$why[] = FetchSnippet("msg_notforsale_artist_no_vendor");			

	
	if (!$artist ['Ecommerce'])
		$why[] = FetchSnippet("msg_notforsale_artist_no_ecommerce");			

	
	if (GetParam($params,FP_PARAM_IMAGE_IS_VIDEO))
		$why[] = FetchSnippet("msg_notforsale_image_is_video");			
	
	// id = 1 is "not for sale"
	// id = 0 is "use parent id", which could be 1!!!
	if ($pricesetID <= 1) {
		// Priceset of parent project is 'not for sale'
		if ($parentpriceset['ID'] == 1)
			$why[] = FetchSnippet("msg_notforsale_parent_not_for_sale");
		// Image marked 'not for sale'
		if ($image['PriceSetID'] == 1)
			$why[] = FetchSnippet("msg_notforsale_image_not_for_sale");
	} else {
	// File too small to print
		// If print size is reported too small to print, 
		// AND it's a print available only in one size, 
		// that could mean there's a high res version, 
		// BUT the fixed size of the print is too BIG for the print size
		// and there is no larger price set size to cover it.
		// Example: A fixed size, 10" print with a price set that only has a 9" size.
		// In that case, there's not pricing to cover 10", and it is reported that there's
		// no print size available.
	
		$arr = CalcPrintSizesFromImageID ($ID, $priceset['printsizes'], true);
		if ($arr['error'] == "nosize") {
			$why[] = FetchSnippet("msg_notforsale_no_size_for_pricing");
		} else if (!$arr) {
			$why[] = FetchSnippet("msg_notforsale_too_small");
		} else {
			// no sizes set in the price set
			if (count($sizes)<1)
				$why[] = FetchSnippet("msg_notforsale_no_sizes");
				// Sold out
			if ($editionsize > $priceset['AmountSold'])
				$why[] = FetchSnippet("msg_notforsale_sold_out");
		}
	}
	
	$t = FetchSnippet("msg_notforsale_solutions_title");
  	switch ($format) {
		case "tip" :
			$whymsg = "<h4>$t</h4><ul>" . "<li>" . join("<li>", $why) . "</ul>";
			$whymsg = htmlentities($whymsg, ENT_QUOTES);
			$whymsg = Substitutions (FetchSnippet("tip_blank_form"), array ("text" => $whymsg));
			break;
		default :
			$whymsg = "<h4>$t</h4><ul><li>" . join("<li>", $why) . "</ul>";
			break;
	}
	$whymsg = Substitutions ($whymsg, array (
		"ID"		=> $ID,
		"PROJECTID"	=> $image['ProjectID'],
		"ARTISTID"	=> $image['ArtistID']
	));
	return $whymsg;
}



// create a button to view a paypal cart
// $item is an array of arrays
// item[x] = array ($value, $item_name, $item_number, $amount)
function CreatePayPalButtons ($item, $business) {
	
	foreach ($item as $row) {
		$item_name = $item['item_name'];
		$item_number = $item['item_number'];
		$amount = $item['amount'];

		$buttons[] = CreateOnePPButton ($item_name, $item_number, $amount, $business);
	}
	return $buttons;
}


// do one button
function CreateOnePPButton ($item_name, $item_number, $amount, $business, $linktext="Buy with PayPal", $makebutton=false) {
	global $msg, $error;
	
	$item_name = htmlspecialchars($item_name, ENT_QUOTES);
	
	if ($makebutton) {
		$link = '<form target="paypal" action="https://www.paypal.com/cgi-bin/webscr" method="post">
			<input type="hidden" name="cmd" value="_cart">
			<input type="hidden" name="business" value="'.$business.'">
			<input type="hidden" name="item_name" value="'. $item_name . '">
			<input type="hidden" name="item_number" value="' . $item_number . '">
			<input type="hidden" name="amount" value="' . $amount . '">
			<input type="image" src="https://www.paypal.com/images/x-click-but22.gif" border="0" name="submit" alt="Make payments with PayPal!">
			<input type="hidden" name="add" value="1">
			</form>';
	} else {
		$URL = 'https://www.paypal.com/cgi-bin/webscr?cmd=_cart&business='.$business.'&item_name='. $item_name . '&item_number=' . $item_number . '&amount=' . $amount . '&add=1';
		$link = "<A class=\"buyprint\" HREF=\"$URL\" target=\"_blank\" >Buy with PayPal</A>\n";
	}
	return $link;
}


// create a button to view a paypal cart

function CreatePayPalCartBtn ($business) {
	$viewcartbutton = '<form name="_xclick" target="paypal" action="https://www.paypal.com/cgi-bin/webscr" method="post">
	<input type="hidden" name="cmd" value="_cart">
	<input type="hidden" name="business" value="'.$business.'">
	<input type="image" src="https://www.paypal.com/images/view_cart.gif" border="0" name="submit" alt="Make payments with PayPal!">
	<input type="hidden" name="display" value="1">
	</form>';

	return $viewcartbutton;
}


// Return a string that shows available print sizes,
// each entry wrapped in a supplied text wrapper.
// I.e. give some javascript, and a link is made for
// each size.
// In the text wrapper, use '$text' as the variable, e.g. <b>$text</b>
// We check the $ORIGINALS folder for the file
// we return an array with the modified text strings one for each size
// 
// If true, $maxonly returns only the largest size. This allows a message like, 
// "Available up to X size", good for Pictopia ordering

function ClickablePrintSizes ($ID, $pricesetID, $itemwrapper, $maxonly=false) {
	global $PHOTOS_GALLERY, $THUMBNAILS, $MATTED, $ORIGINALS, $BASEDIR;
	global $msg, $error;
	$DEBUG = false;
	
	$image = FetchImage ($ID);
	if ($image) {
		$filename = $ORIGINALS . "/" . $image['URL'];
	}
	
	$imgsize = GetImageSizeByID($ID);
	$width = $imgsize['width_print'];
	$height = $imgsize['height_print'];

	if ($pricesetID > 0) {	
		$priceset = GetPriceSet ($pricesetID);
	
		// This gives us the print sizes available, based on image size. We don't allow lo-res prints.
		// $maxonly is true for Pictopia, which only wants to return the largest size
		$sizes = CalcPrintSizesFromImageID ($ID, $priceset['printsizes'], $maxonly);
		$sizesToShow = array_keys($sizes);
		
		if ($DEBUG) {
			print "<BR>";
			print __FUNCTION__.":".__LINE__.": ID=$ID<BR>";
			var_dump ($sizes);
		}
		
		$t = array();
		$i = $itemwrapper;
		$count = count ($sizes);
		for ($i=0;$i<$count;$i++) {
			$t[] = Substitutions ($itemwrapper, array(
				"size"=>$sizesToShow[$i],
				"price"=>$priceset['PrintPrices'][$i],
				"priceframed"=>$priceset['priceframed'][$i]
				));
		}
		
		$goodsizes = array();
		$goodsizes = join ("\n", $t);
		$goodsizes || $goodsizes = "";
	} else {
		$goodsizes = array ();
	}
	
	return $goodsizes;
}


// Show available print sizes
// We check the $ORIGINALS folder for the file
// we return an array of available print sizes that won't look bad
// Array: first element is text of sizes, followed by sizes.

function ShowPrintSizes ($ID) {
	global $PHOTOS_GALLERY, $THUMBNAILS, $MATTED, $ORIGINALS, $BASEDIR;
	global $msg, $error;
	$DEBUG = false;
	
	$image = FetchImage ($ID);
	if ($image) {
		$filename = $ORIGINALS . "/" . $image['URL'];
	}
	
	$imgsize = GetImageSizeByID($ID);
	$width = $imgsize['width_print'];
	$height = $imgsize['height_print'];
	$sizes = CalcPrintSizes ($width, $height, true);
	$DEBUG && $msg .= basename(__FILE__) .":".__FUNCTION__ .":".__LINE__ .": "."<BR>\nGot size for [$ID]: ";
	$DEBUG && $msg .= basename(__FILE__) .":".__FUNCTION__ .":".__LINE__ .": "."<BR>\nheight=$height, width=$width";
	$DEBUG && $msg .= basename(__FILE__) .":".__FUNCTION__ .":".__LINE__ .": ".ArrayToTable ($sizes);
	return $sizes;
}



// ==========================================
// Get a price set from the PriceSet db table
// hint: list ( $editionsizes, $weightsframed, $weights, $printsizes, $count, $price, $priceframed, $size) =  GetPriceSetOLD ($ID);

function  GetPriceSetOLD ($ID) {
	global $msg, $error;

	$size = array();
	$priceframed = array();
	$price = array ();
	$printsizes = array ();
	$weights = array ();
	$weightsframed = array ();
	$editionsizes = array ();
	$amount = array ();
	$cost = array ();
	
	$priceset = FetchRowsByValue (DB_PRICESETS, "*", 'ID', $ID, '', TRUE);
	
	// Sort by size and ignore size = 0
	$i = 0;
	$order = array ();
	while (isset ($priceset['size'.($i+1)])) {
		$priceset['size'.($i+1)] && $order[$i] = $priceset['size'.($i+1)];
		$i++;
	}
	asort ($order, SORT_NUMERIC);
	$order = array_keys($order);


	// Extract dimensions
	$priceset["BoxedWidth"] = explode(",", $priceset["BoxedWidth"]);
	$priceset["BoxedHeight"] = explode(",", $priceset["BoxedHeight"]);
	$priceset["BoxedDepth"] = explode(",", $priceset["BoxedDepth"]);
	// Matte
	$priceset["MatteCost"] = explode(",", $priceset["MatteCost"]);
	$priceset["MattePrice"] = explode(",", $priceset["MattePrice"]);
	// Frame
	$priceset["FrameCost"] = explode(",", $priceset["FrameCost"]);
	$priceset["FramePrice"] = explode(",", $priceset["FramePrice"]);
	// Handling
	$priceset["HandlingLocal"] = explode(",", $priceset["HandlingLocal"]);
	$priceset["HandlingIntl"] = explode(",", $priceset["HandlingIntl"]);

	// The PriceSets records have 6 entries...but this method is more flexible in case
	// we change the that.
	// We go through the list as long as the size is set, i.e. > 0
	$k = 0;
	
	// If there is a sizeN entry, loop.
	foreach ($order as $i) {
		// only use the entry if the size > 0
		if ($priceset["size".($i+1)] > 0) {
			$size[$k] = $priceset["size".($i+1)];
			$priceframed[$k] = $priceset["priceframed".($i+1)];
			$price[$k] = $priceset["price".($i+1)];
			$printsizes[$size[$k]] = $size[$k];	// used to calculate acceptable print sizes to offer based on image size
			$weights[$k] = $priceset["weight".($i+1)];
			$weightsframed[$k] = $priceset["weightframed".($i+1)];
			$editionsizes[$k] = $priceset["editionsize".($i+1)];
			$amount[$k] = $priceset["Amount".($i+1)];
			$extrashipping[$k] = $priceset["extrashipping".($i+1)];
			$handlinglocal[$k] = $priceset["HandlingLocal"][$i+1];
			$handlingintl[$k] = $priceset["HandlingIntl"][$i+1];
			$cost[$k] = $priceset["cost".($i+1)];
			//$framecost[$k] = $priceset["framecost".($i+1)];
			$framecost[$k] = $priceset["FrameCost"][$i+1];
			$mattecost[$k] = $priceset["MatteCost"][$i+1];
			$boxedwidth[$k] = $priceset["BoxedWidth"][$i+1];
			$boxedheight[$k] = $priceset["BoxedHeight"][$i+1];
			$boxeddepth[$k] = $priceset["BoxedDepth"][$i+1];
			$k++;
		}
	}

/*
	// Supplier settings: measurement unit (ft/m), etc.
	$supplier = FetchSupplier($priceset["SupplierID"]);
	// default is "cm", but "inches" values converted to cm
	$print_cost_unit = $supplier["PrintCostUnit"];	
	// default is "square cm", but "square foot" value will be converted to cm2
	$print_cost_area_unit = $supplier["PrintCostAreaUnit"];
	// e.g. cost per area
	$print_cost_rate = $supplier["PrintCostRate"];
	// default is "paper". Measure print cost by paper size, not image size. Values are "paper" or "image"
	$print_cost_method = $supplier["PrintCostMethod"];
*/

	$priceset = array (
			"Amount"					=>  $amount,
			"boxeddepth"				=>  $boxeddepth,
			"boxedheight"			=>  $boxedheight,
			"boxedwidth"				=>  $boxedwidth,
			"cost"					=>  $cost,
			"cost"					=>  $cost,
			"count"					=>  $k,
			"editionsizes"			=>  $editionsizes,
			"extrashipping"			=>  $extrashipping,
			"framecost"				=>  $framecost,
			"handlingintl"			=>  $handlingintl,
			"handlinglocal"			=>  $handlinglocal,
			"inkset"					=>  $priceset["Inkset"],
			"inksetcode"				=>  $priceset["InksetCode"],
			"MatchPrintPrice"		=>	$priceset["MatchPrintPrice"],	// we get supplier now!
			"mattecost"				=>  $mattecost,
			"paper"					=>  $priceset["Paper"],
			"papercode"				=>  $priceset["PaperCode"],
			"price"					=>  $price,
			"priceframed"			=>  $priceframed,
			"printsizes"				=>  $printsizes,
			"size"					=>	$size,
			"supplierID"				=>  $priceset["SupplierID"],
			"weights"				=>  $weights,
			"weightsframed"			=>  $weightsframed
/*			
			"print_cost_unit"		=>	$print_cost_unit,
			"print_cost_area_unit"	=>	$print_cost_area_unit,
			"print_cost_rate"		=>	$print_cost_rate,
			"print_cost_method"		=>	$print_cost_method
*/			
			 );
	
	return $priceset;
}


// GetPriceSet
// This new version gets values from the arrays, not individual fields.
// It gets the new info for shipping, too.
// $ID = 1 means not for sale!
function GetPriceSet ($ID) {
	global $msg, $error;
	
	$DEBUG = 0;
	$DEBUG && print __FUNCTION__;

	$Amount = array ();
	$Size = array ();
	$EditionSize = array ();
	$PrintCost = array ();
	$PrintPrice = array ();
	$Markup = array ();
	$MatteCost = array ();
	$MattePrice = array ();
	$FrameToPrintCost = array ();
	$FrameToPrintPrice = array ();
	$FrameMatteCost = array ();
	$FrameMattePrice = array ();
	$PrintShipWeight = array ();
	$MatteShipWeight = array ();
	$FrameToPrintShipWeight = array ();
	$FrameMatteShipWeight = array ();
	$Unused = array ();
	$printsizes = array ();

	
	$DEBUG && print "Get ID = $ID<BR>";
	
	$priceset = FetchRowsByValue (DB_PRICESETS, "*", 'ID', $ID, '', TRUE);
	
	if (!is_array($priceset)) {
		$ID = 1;
		$priceset = FetchRowsByValue (DB_PRICESETS, "*", 'ID', $ID, '', TRUE);
	}

	isset ($priceset['Params']) ? $params = DecodeArrayFromDB($priceset['Params']) : $params = array ();
	
	$priceset = UnpackPriceSetRow($priceset);
	
	// Pricing tier — how many prints per jump in price determined by 'Inflation' field.
	$tier = GetParam($params, FP_PARAM_PRICESET_TIER);
	(empty($tier) || $tier < 0) && $tier = 1;
	
	/*
	// If this priceset is specific to an image, we pull the supplierID from it.
	// Otherwise, we give a NULL.
	$priceset['ImageID']
	? $supplierID = $priceset["SupplierID"]
	: $supplierID = null;
	*/
	
	// used to calculate acceptable print sizes to offer based on image size
	//foreach ($priceset['Size'] as $s) {
	//	$printsizes[$s] = $s;
	//}

	// Resort arrays by the order of the Sizes
	$i = 0;
	$order = array ();
	$DEBUG && var_dump($priceset['Size']);

	$priceset['Size'] || $priceset['Size'] = array();
	foreach ($priceset['Size'] as $s) {
		$s && $order[$i] = $s;
		$i++;
	}
	asort ($order, SORT_NUMERIC);
	$order = array_keys($order);
	
	$DEBUG && var_dump($order);

	$array_fields = array ('Size', 'EditionSize', 'PrintCost', 'PrintPrice', 'Markup', 'MatteCost', 'MattePrice', 'FrameToPrintCost', 'FrameToPrintPrice', 'FrameMatteCost', 'FrameMattePrice', 'PrintShipWeight', 'MatteShipWeight', 'FrameToPrintShipWeight', 'FrameMatteShipWeight', 'Inactive', 'Amount');
	//$array_types = array ('float', 'float', 'float', 'float', 'float', 'float', 'float', 'float', 'float', 'float', 'float', 'float', 'float', 'float', 'float', 'float', 'int');


	$k=0;
	// Reorder the arrays to match the order of the sizes (from low to high)
	foreach ($order as $i) {
		// only use the entry if the size > 0
		if ($priceset['Size'][$i] > 0) {
			for ($j=0;$j<count($array_fields);$j++) {
				$f = $array_fields[$j];
				//$t = $array_types[$j];
				$x = (float)$priceset[$f][$i];
				$x || $x = 0;
				${$f}[$k] = (float)$priceset[$f][$i];
			}

			$printsizes[$Size[$k]] = $Size[$k];
			$k++;
		}
	}

	/*
	$Size = ksort(array_combine ($order, $priceset['Size']));
	$EditionSize = ksort(array_combine ($order, $priceset['EditionSize']));
	$PrintCost = ksort(array_combine ($order, $priceset['PrintCost']));
	$PrintPrice = ksort(array_combine ($order, $priceset['PrintPrice']));
	$Markup = ksort(array_combine ($order, $priceset['Markup']));
	$MatteCost = ksort(array_combine ($order, $priceset['MatteCost']));
	$MattePrice = ksort(array_combine ($order, $priceset['MattePrice']));
	$FrameToPrintCost = ksort(array_combine ($order, $priceset['FrameToPrintCost']));
	$FrameToPrintPrice = ksort(array_combine ($order, $priceset['FrameToPrintPrice']));
	$FrameMatteCost = ksort(array_combine ($order, $priceset['FrameMatteCost']));
	$FrameMattePrice = ksort(array_combine ($order, $priceset['FrameMattePrice']));
	$printsizes = ksort(array_combine ($order, $printsizes));
	*/
	
	
	$priceset2 = array (
		"Amount"				=> $Amount,
		"AmountSold"				=> array_sum($Amount),
		"count"					=> count($Size),
		"EditionSize"			=> array ($priceset['TotalEditionSize']),
		"FrameMatteCost"			=> $FrameMatteCost,
		"FrameMattePrice"		=> $FrameMattePrice,
		"FrameMatteShipWeight"	=> $FrameMatteShipWeight,
		"FrameToPrintCost"		=> $FrameToPrintCost,
		"FrameToPrintPrice"		=> $FrameToPrintPrice,
		"FrameToPrintShipWeight"	=> $FrameToPrintShipWeight,
		"inkset"					=> $priceset["Inkset"],
		"inksetcode"				=> $priceset["InksetCode"],
		"Markup"					=> $Markup,
		"MatteCost"				=> $MatteCost,
		"MattePrice"				=> $MattePrice,
		"MatteShipWeight"		=> $MatteShipWeight,
		"paper"					=> $priceset["Paper"],
		"papercode"				=> $priceset["PaperCode"],
		"PrintCost"				=> $PrintCost,
		"PrintPrice"				=> $PrintPrice,
		"PrintShipWeight"		=> $PrintShipWeight,
		"printsizes"				=> $printsizes,
		"Size"					=> $Size,
		"supplierID"				=> $supplierID,
		"Unused"					=> $Unused,
		"TotalEditionSize"		=> $priceset['TotalEditionSize'],
// Now, let's use a field called "TotalEditionSize, and give up on the different sizes for each row, dammit.
		"imageID"				=> $priceset['ImageID'],
		'Inflation'			=> $priceset['Inflation'],
		'Tier'				=> $tier,
		'MaxFramedSize'			=> intval($priceset['MaxFramedSize'])
		 );
	
	$priceset = array_merge($priceset, $priceset2);
	
	$DEBUG && var_dump ($priceset);
	
	return $priceset;
}




// ImageSalesInfo ($ID)
// $ID = image ID
// $width & $height are in pixels
//
// Given the image size (read using GetImageSizeByID)
// we determine the sizes we allow for printing
// the image. This way, we don't sell lousy quality prints.
//
// We return an array of acceptable print sizes.
// The keys are the size to show, width x height
// The values are only the larger size, which is the index in the pricing database
// for each entry.
//
// If empty array, the image is to small to print well.
//
// * $makesummary is true/false. If true, return a string of all available sizes.
// * $maxonly means return ONLY the largest size. Pictopia needs this,
// because it only allows us to offer the largest, and smaller, sizes,
// NOT an order for a given size.
// * $myprintsizes is an array of sizes, e.g. 10,20,30. If this is not supplied,
// we use the default sizes set up in the config file.
//
// If part of a project, include that param because a project may determine pricing info
//
// Limited Edition settings:
// Hierarchy of settings: Image > Parent Project > Current Project
// Get the Edition sizes and prices following the above hierarchy

function ImageSalesInfo ($ID, $projectID = 0, $getAllInfo = false) {
	global $PRINTSIZES, $PRINTDPI;
	global $PHOTOS_GALLERY, $THUMBNAILS, $MATTED, $ORIGINALS;
	global $BASEDIR, $LOGS;
	global $msg, $error;

	$DEBUG = 0;
	$TIMER = 0;
	$starttime = microtime(true);
	
	$DEBUG && $msg .= __FUNCTION__.": DEBUG<BR>";

	$amounts = array ();
	$available = array ();
	$dims = array ();
	$editionsizes = array ();
	$extrashipping = array ();
	$idims = array ();
	$pdims = array ();
	$prices = array ();
	$pricesframed = array ();
	$pricesFrameMatte = array ();
	$pricesFrameToPrint = array ();
	$pricesMatte = array ();
	$psizes = array ();
	$papersizes = array ();
	$printsizes = array ();
	$printsizesformatted = array ();
	$rows = array ();
	$salesinfo = array ();
	$sizes = array ();
	$weights = array ();
	$weightsframed = array ();


	
	// If we don't calc available prints by size, but as a total, then...
	if (FP_EDITION_CALC_METHOD == "total") {
		$available = 0;
	}
	
	$pricesetID = FetchPriceSetID ($ID, $projectID);
 	$priceset = GetPriceSet ($pricesetID);
	$image = FetchImage ($ID);
	$params = DecodeArrayFromDB($image['Params']);

	// GET SUPPLIER (VENDOR)
	// if no vendor set in the price set, e.g. it is a generic,
	// then use the artist's default vendor.
	// Pricesets for pictures which have already been sold are locked
	// into a particular supplier.
	if ($priceset['imageID'])
		$supplierID = $priceset['supplierID'];
	else
		$supplierID = null;
	
	if ($supplierID < 1) {
		$artist = FetchArtist ($image["ArtistID"]);
		$supplierID = $artist["Vendor"];
	}

	// Get pricing information for the image (won't include hidden prices in priceset)
	$pricedata = GetPricingByID ($ID, $supplierID, $pricesetID);
		
	// Limit size to maximum printable size based on pixels and DPI 
	//$allamounts = $priceset["Amount"];	//amount sold...only applies to image-specific entries
	

	/*
	// GET DIMENSIONS AND LANDSCAPE V. PORTRAIT
	$image = FetchImage ($ID);
	
	$imgsize = GetImageSizeByID($ID);
	$width = $imgsize['width_print'];
	$height = $imgsize['height_print'];
	*/
	
	// Get all printable sizes for this image. We don't allow printing of low dpi images.
	// If the image is an original artwork, it only has one size, which is hand-entered.
/*
	if ($params[FP_PARAM_IMAGE_IS_ORIGINAL_ART]) {
		// get size from entered values
		$availableSizes = array ( array ("width" => $params[FP_PARAM_IMAGE_FIXED_WIDTH], "height" => $params[FP_PARAM_IMAGE_FIXED_WIDTH]));
*/
	
	$isOriginal = $params[FP_PARAM_IMAGE_IS_ORIGINAL_ART];

	if (false) {
	} else {
		if ($getAllInfo) {
			$availableSizes = $pricedata['Sizes'];
		} else {
			$availableSizes = CalcPrintSizesFromImageID ($ID, $pricedata['Sizes'], $maxonly);
		}
	}
	$availableSizes || $availableSizes = array();
	
	$allprintsizes = $priceset['printsizes'];
	//$allprices = $priceset['PrintPrice'];
	
	$alleditionsizes = $priceset['EditionSize'];
	
	
	$allamounts = $priceset["Amount"];	//amount sold...only applies to image-specific entries
	$outsidesales = $params[FP_PARAM_AMOUNT_SOLD_OUTSIDE];

	
	// Get Edition size:
	if ($isOriginal) {
		$editionType = "original";
		$priceseteditionsize = 1;
		$totalEdition = 1;
		$priceset['TotalEditionSize'] = 1;
		$priceset['EditionSize'] = array(1);
	} else if ($priceset['TotalEditionSize'] == 0) {
		// OPEN EDITION
		$editionType = "unlimited";
		$priceseteditionsize = 0;
		$totalEdition = 0;
	} else {
		// LIMITED EDITION
		$editionType = "limited";
		$priceseteditionsize = $priceset['TotalEditionSize'];
		// Get the calculated value (considers custom setting of the image itself)
		$totalEdition = GetImageEditionSize($ID, $priceset['TotalEditionSize']);

		if ($outsidesales > $totalEdition) {
			$outsidesales = $totalEdition;
			$error .= __FUNCTION__ . ": Warning: Outside Sales are greater than the edition size!";
		}
	}

	// Total sold is amount sold for each size, PLUS amount sold on the outside of the system.
	$totalSold = array_sum ($allamounts) + $outsidesales;
	
	$sizeformat = FetchSnippet ("available_sizes");
	$sizeplaintext = FetchSnippet ("available_sizes_plain_text");

	
	

	//$allweights = $priceset['weights'];
	//$allweightsframed = $priceset['weightsframed'];
	//$allpricesframed = $priceset['priceframed'];
	//$allsizes = $priceset['Size'];
	//$allextrashipping = $priceset['extrashipping'];

	//$count = $priceset['count'];	//???

	
	// Total prints of an edition, and total sold:

	if ($editionType == "unlimited") {
		$available = 0;
	} else {
		$available = $totalEdition - $totalSold;
	}
	
	$k = 0;
	foreach ($availableSizes as $size) {
		
		// Available in this size? (if we sell by size in an edition?)
		// do we calc availability by total prints out of edition total size
		// or by available prints in each size?
		if (FP_EDITION_CALC_METHOD == "total") {
			$sizeNotSoldOut = ($totalSold < $totalEdition);
		} else {
			$sizeNotSoldOut = ($allamounts[$k] < $priceset['EditionSize'][$k]);
		}
		
		//$otherside = $pricedata['DimensionsBySide'][$size]['imageDimsOrdered']['smallside'];
		// max picture size allowed, based on pixel count at $PRINTDPI per inch.
		//$maxsize = ($bigside / $PRINTDPI);

		//if ($priceset['Inactive'][$k] == 0 && ($totalEdition == 0 || $sizeNotSoldOut)) {
		if ($totalEdition == 0 || $sizeNotSoldOut) {
			
			// GET DIMENSIONS AND MEASUREMENTS
			$dims = $pricedata['DimensionsBySide'][$size];
			$places = 1;	// round to $places decimal places
			$otherside = $dims['imageDimsOrdered']['smallside'];

			// image size
			//$sizecm = number_format(round($size * 2.54,$places), $places) ;
			//$othersidecm = number_format(round(($otherside) * 2.54, $places), $places) ;			
			$iDimsCm = $dims['imageDims'];
			array_walk($iDimsCm, 'InToCm', $places);
			$din = $dims['imageDims']['width']."&nbsp;&times;&nbsp;".$dims['imageDims']['height'];
			$dcm = $iDimsCm['width']."&nbsp;&times;&nbsp;".$iDimsCm['height'];
			$idim = "{$dims['imageDims']['width']}-{$dims['imageDims']['height']}";
			$d = Substitutions ($sizeplaintext, array ("SIZE_INCHES"=>$din, "SIZE_CM"=>$dcm));
			$dformatted = Substitutions ($sizeformat, array ("SIZE_INCHES"=>$din, "SIZE_CM"=>$dcm));

			$idims[] = $idim;	// image size
			$printsizes[] = $d;
			$printsizesformatted[] = $dformatted;

			// paper sizing to cm
			$pDimsCm = $dims['paperDims'];
			array_walk($pDimsCm, 'InToCm', $places);
			$pdin = $dims['paperDims']['width']."&nbsp;&times;&nbsp;".$dims['paperDims']['height'];
			$pdcm = $pDimsCm['width']."&nbsp;&times;&nbsp;".$pDimsCm['height'];
			$pdim = "{$dims['paperDims']['width']}-{$dims['paperDims']['height']}";
			$pd = Substitutions ($sizeplaintext, array ("SIZE_INCHES"=>$pdin, "SIZE_CM"=>$pdcm));
			$pdformatted = Substitutions ($sizeformat, array ("SIZE_INCHES"=>$pdin, "SIZE_CM"=>$pdcm));

			$papersizes[] = $pd;
			$papersizesformatted[] = $pdformatted;
			$pdims[] = $pdim;			// paper size

			$artborders[] = ArtBorderWidthFromSide ($size);			// art border
			
			// GET PRICES: only print and framed/matted. This represents the low and the high.
			$prices[] = $pricedata['PrintPrices'][$k];
			$pricesMatte[] = $pricedata['MattePrices'][$k];
			$pricesFrameToPrint[] = $pricedata['FrameToPrintPrices'][$k];
			$pricesFrameMatte[] = $pricedata['FrameMattePrices'][$k];

			 $allamounts[$k] ? $amounts[] = $allamounts[$k] : $amounts[] = 0;
			
			//$extrashipping[] = $allextrashipping[$k];
			
			if (FP_EDITION_CALC_METHOD != "total")
				$available[] = $alleditionsizes[$k] - $allamounts[$k];
				
			$sizes[] = $size;
			$psize = ImageGreaterSideFirst ($dims['paperDims']['width'], $dims['paperDims']['height']);
			$psizes[] = $psize;

			//$weights[] = $allweights[$k];
			//$weightsframed[] = $allweightsframed[$k];

			$rows[] = $k+1;	// rows are 1,2,3,..., not 0,1,2...
			$DEBUG && $error .= "$k) FOR SALE: ".$d." : ".$amount[$k]." < ". $alleditionsizes[$k] . "<BR>";
		} else {
			$DEBUG && $error .= "$k) SOLD OUT: ".$printsizes[$k]." : ".$amount[$k]." < ". $alleditionsizes[$k] . "<BR>";
		}
		$k++;
	}

	$DEBUG && print __FUNCTION__.":";
	if ($DEBUG) {
		var_dump ($idims);
	}

	// list of greatest side sizes
	
	// list of image sizes fully formatted (with HTML)
	// list of image sizes simply formatted (as plain text)
	//$salesinfo['papersize']				= $psizes;
	//$salesinfo['weights']					= $weights;
	//$salesinfo['weightsframed']			= $weightsframed;
	//$salesinfo['extrashipping']			= $extrashipping;
	$salesinfo['amount']					= $amounts;					// # prints sold
	$salesinfo['available']				= array($available);				// # prints available
	$salesinfo['dims']					= $dims;						// all dimensions
	$salesinfo['editionsizes']			= $priceset['EditionSize'];
	$salesinfo['FrameMattePrices']			= $pricesFrameMatte;
	$salesinfo['FrameToPrintPrices']		= $pricesFrameToPrint;
	$salesinfo['idims']					= $idims;					// image dimensions
	$salesinfo['index']					= $sizes;
	$salesinfo['inkset']					= $priceset['inkset'];
	$salesinfo['inksetcode']				= $priceset['inksetcode'];
	$salesinfo['MattePrices']				= $pricesMatte;
	$salesinfo['paper']					= $priceset['paper'];
	$salesinfo['papercode']				= $priceset['papercode'];
	$salesinfo['papersize']				= $psizes;
	$salesinfo['papersizes']				= $papersizes;
	$salesinfo['papersizesformatted']		= $papersizesformatted;
	$salesinfo['pdims']					= $pdims;					// Paper dimensions
	$salesinfo['price']					= $prices;
	$salesinfo['pricesetID']				= $pricesetID;
	$salesinfo['print_cost_area_unit']	= $priceset[''];
	$salesinfo['print_cost_method']		= $priceset[''];
	$salesinfo['print_cost_rate']			= $priceset[''];
	$salesinfo['print_cost_unit']			= $priceset[''];
	$salesinfo['PrintPrices']				= $prices;
	$salesinfo['printsizes']				= $printsizes;
	$salesinfo['printsizesformatted']		= $printsizesformatted;
	$salesinfo['rows']					= $rows;
	$salesinfo['size']					= $availableSizes;
	$salesinfo['supplierID']				= $supplierID;
	$salesinfo['totaledition']			= $totalEdition;				// Total number of prints in edition
	$salesinfo['priceseteditionsize']			= $priceseteditionsize;				// Total number of prints in edition
	$salesinfo['totalsold']				= $totalSold;				// Total number of prints sold INCLUDING OUTSIDE SALES
	$salesinfo['outsidesales']			= $outsidesales;
	$salesinfo['original']				= $isOriginal;				// Is this an original artwork?
	$salesinfo['edition_type']			= $editionType;
	$salesinfo['maxframedsize']			= $priceset['MaxFramedSize'];

	$DEBUG && print __FUNCTION__.__LINE__.":".ArrayToTable ($priceset);
	
	return $salesinfo;
}



function ImageSalesInfoOLD ($ID, $projectID = 0, $getAllInfo = false) {
	global $PRINTSIZES, $PRINTDPI;
	global $PHOTOS_GALLERY, $THUMBNAILS, $MATTED, $ORIGINALS;
	global $BASEDIR, $LOGS;
	global $msg, $error;

	$DEBUG = false;
	
	// GET DIMENSIONS AND LANDSCAPE V. PORTRAIT
	$image = FetchImage ($ID);
	
	$imgsize = GetImageSizeByID($ID);
	$width = $imgsize['width_print'];
	$height = $imgsize['height_print'];
	
	$DEBUG && $msg .= __FUNCTION__." : ".$image['Title'].", height=$height, width=$width<BR>".ArrayToTable ($imgsize);

	// get the greater of width or height
	if ($width > $height) {
		$bigside = $width;
		$smallside = $height;
		$orientation = "landscape";
	} else {
		$bigside = $height;
		$smallside = $width;
		$orientation = "portrait";
	}
	
	$bigside || fp_error_log("ERROR in ".__FUNCTION__.": couldn't get image size. Probably means image is missing!", 3, FP_ERROR_LOG );

	
	// get ratio of greater size to less, to calculate lesser later
	// so if we have 10", and the ration of 8/10, we know to offer an 8x10.
	$bigside != 0 ? $ratio = $smallside/$bigside : $ration = 1;
	
	// max picture size allowed, based on pixel count at $PRINTDPI per inch.
	$maxsize = ($bigside / $PRINTDPI);
	$sizes = array ();


	// GET PRICING
 	$pricesetID = FetchPriceSetID ($ID, $projectID);
 	$priceset = GetPriceSet ($pricesetID);
 	
 	// GET SUPPLIER (VENDOR)
	// if no vendor set in the price set, e.g. it is a generic,
	// then use the artist's default vendor.
	// Pricesets for pictures which have already been sold are locked
	// into a particular supplier.
	$supplierID = $priceset['supplierID'];
	if ($supplierID < 1) {
		$artist = FetchArtist ($image["ArtistID"]);
		$supplierID = $artist["Vendor"];
	}
	
	$DEBUG && $msg .= __FUNCTION__." : ID=$ID, pricesetID=$pricesetID<BR>";

	//$allweights = $priceset['weights'];
	//$allweightsframed = $priceset['weightsframed'];
	$allprintsizes = $priceset['printsizes'];
	$allprices = $priceset['price'];
	$allpricesframed = $priceset['priceframed'];
	$count = $priceset['count'];
	$alleditionsizes = $priceset['editionsizes'];
	$allsize = $priceset['size'];
	$allextrashipping = $priceset['extrashipping'];
	

	$allamounts = $priceset["Amount"];	//amount sold...only applies to image-specific entries
	
/*
	// Get the only available sizes
	// Check the physical size is printable
	// AND check the edition isn't sold out
	if (!isset($allprintsizes))
		$allprintsizes = $PRINTSIZES;

 */
	sort ($allprintsizes);
	
	$k = 0;
	$salesinfo = array ();
	$printsizes = array ();
	$printsizesformatted = array ();
	$prices = array ();
	$pricesframed = array ();
	$sizes = array ();
	$weights = array ();
	$weightsframed = array ();
	$dims = array ();
	$editionsizes = array ();
	$amounts = array ();
	$extrashipping = array ();
	$rows = array ();
	$available = array ();

	// Total prints of an edition, and total sold:
	$totalEdition = array_sum ($alleditionsizes);
	$totalSold = array_sum ($allamounts);
	
	$sizeformat = FetchSnippet ("available_sizes");
	$sizeplaintext = FetchSnippet ("available_sizes_plain_text");
	
	foreach ($allprintsizes as $size) {
		// do we calc availability by total prints out of edition total size
		// or by available prints in each size?
		if (FP_EDITION_CALC_METHOD == "total") {
			$test = ($totalSold < $totalEdition);
		} else {
			$test = ($allamounts[$k] < $alleditionsizes[$k]);
		}

		$maxsize = ($size / $PRINTDPI);
		if (($size <= $maxsize) and ($getAllInfo or $alleditionsizes[$k] == 0 or $test)) {
			// get the size
			$r = 1;	// round to $r decimal places
			$otherside = $p['PrintPackageDims']['smallside'];
			
			if ($orientation == "portrait") {
				$temp = $size;
				$size = $otherside;
				$otherside = $size;
			}

			$sizecm = number_format(round($size * 2.54,$r), $r) ;
			$othersidecm = number_format(round(($size * $ratio) * 2.54,$r), $r) ;
			
			// paper sizing
			$artborder = ArtBorderWidthFromSide ($size);
			$psize = $size + (2*$artborder);
			$potherside = $otherside + (2*$artborder);
			
			// paper cm sizing
			$psizecm = number_format(round($psize * 2.54,$r), $r) ;
			$pothersidecm = number_format(round(($psize * $ratio) * 2.54,$r), $r) ;

	
			// Round the sizing
			$size = number_format(round($size,$r),$r);
			$otherside = number_format(round($otherside,$r),$r);
			$psize = number_format(round($psize,$r),$r);
			$potherside = number_format(round($potherside,$r),$r);
			
			// image sizing
			$din = $size."&nbsp;&times;&nbsp;".$otherside;
			$dcm = $sizecm."&nbsp;&times;&nbsp;".$othersidecm;
			$dim = "{$size}-{$otherside}";
			$d = Substitutions ($sizeplaintext, array ("SIZE_INCHES"=>$din, "SIZE_CM"=>$dcm));
			$dformatted = Substitutions ($sizeformat, array ("SIZE_INCHES"=>$din, "SIZE_CM"=>$dcm));

			// paper sizing (based on image size)
			$pdin = $psize."&nbsp;&times;&nbsp;".$potherside;
			$pdcm = $psizecm."&nbsp;&times;&nbsp;".$pothersidecm;
			$pdim = "{$psize}-{$potherside}";
			$pd = Substitutions ($sizeplaintext, array ("SIZE_INCHES"=>$pdin, "SIZE_CM"=>$pdcm));
			$pdformatted = Substitutions ($sizeformat, array ("SIZE_INCHES"=>$pdin, "SIZE_CM"=>$pdcm));
			
			$printsizes[] = $d;
			$printsizesformatted[] = $dformatted;
			$papersizes[] = $pd;
			$papersizesformatted[] = $pdformatted;

			$dims[] = $dim;	// image size
			$pdims[] = $pdim;			// paper size
			$artborders[] = ArtBorderWidthFromSide ($size);			// art border
			$prices[] = $allprices[$k];
			$weights[] = $allweights[$k];
			$weightsframed[] = $allweightsframed[$k];
			$editionsizes[] = $alleditionsizes[$k];
			$pricesframed[] = $allpricesframed[$k];
			$amounts[] = $allamounts[$k];
			$extrashipping[] = $allextrashipping[$k];
			$available[] = $alleditionsizes[$k] - $allamounts[$k];
			$sizes[] = $size;
			$psizes[] = $psize;
			$rows[] = $k+1;	// rows are 1,2,3,..., not 0,1,2...
			$DEBUG && $error .= "$k) FOR SALE: ".$d." : ".$amount[$k]." < ". $alleditionsizes[$k] . "<BR>";
		} else {
			$DEBUG && $error .= "$k) SOLD OUT: ".$printsizes[$k]." : ".$amount[$k]." < ". $alleditionsizes[$k] . "<BR>";
		}
		$k++;
	}
	
	$salesinfo['amount'] = $amounts;
	$salesinfo['available'] = $available;
	$salesinfo['dims'] = $dims;
	$salesinfo['editionsizes'] = $editionsizes;
	$salesinfo['extrashipping'] = $extrashipping;
	$salesinfo['inkset'] = $priceset['inkset'];
	$salesinfo['inksetcode'] = $priceset['inksetcode'];
	$salesinfo['paper'] = $priceset['paper'];
	$salesinfo['papercode'] = $priceset['papercode'];
	$salesinfo['papersize'] = $psizes;
	$salesinfo['papersizes'] = $papersizes;
	$salesinfo['papersizesformatted'] = $papersizesformatted;
	$salesinfo['pdims'] = $pdims;
	$salesinfo['price'] = $prices;
	$salesinfo['priceframed'] = $pricesframed;
	$salesinfo['pricesetID'] = $pricesetID;
	$salesinfo['print_cost_area_unit'] = $priceset[''];
	$salesinfo['print_cost_method'] = $priceset[''];
	$salesinfo['print_cost_rate'] = $priceset[''];
	$salesinfo['print_cost_unit'] = $priceset[''];
	$salesinfo['printsizes'] = $printsizes;
	$salesinfo['printsizesformatted'] = $printsizesformatted;
	$salesinfo['rows'] = $rows;
	$salesinfo['size'] = $sizes;
	$salesinfo['supplierID'] = $supplierID;
	$salesinfo['totaledition'] = $totalEdition;
	$salesinfo['totalsold'] = $totalSold;
	$salesinfo['weights'] = $weights;
	$salesinfo['weightsframed'] = $weightsframed;

	//$DEBUG && print __FUNCTION__.__LINE__.":".ArrayToTable ($priceset);
		
	return $salesinfo;
}

// PrintSizeForDisplay
// Get the print size of a picture for display.
//
// IF $image is an array, it's an image record
// IF $image is not an array, it's image record ID
// Return "$size x OTHERSIZE" as a string
// or "OTHERSIZE x $size", depending on landscape/portrait image,
// where the OTHERSIZE is the smaller dimension
function PrintSizeForDisplay ($image, $size) {
	global $PRINTSIZES, $PRINTDPI;
	global $PHOTOS_GALLERY, $THUMBNAILS, $MATTED, $ORIGINALS, $BASEDIR;
	global $msg, $error;

	if (!is_array ($image)) {
		$image = FetchImage ($image);
	}

	$dims = GetPrintDimensionsByID($image['ID'], $size);
	$d = $dims['imageDims']['width']."&nbsp;&times;&nbsp;".$dims['imageDims']['height']."&nbsp;".UNITS;
	return $d;
}


// Get a price set ID for an image
// If not set, get it from the parent project
// If not set, get it from the current project ($projectID param)
// If not set, it means the project pictures ARE NOT FOR SALE!. Return 0.
// HOWEVER
// If Current Project is marked 'not for sale' then return 0.
// If image NotForSale is true, it is not for sale.
// And artist must have chosen a supplier
//
// If a sale has been made, then the image SHOULD have the new, locked price set ID
// entered into its record!

function FetchPriceSetID ($ID, $currentProjectID = null) {
	global $msg, $error;

	$DEBUG = 0;
	
	$image = FetchImage ($ID);
	
	// 1) If projectID is passed, and project priceset is "not for sale" return 0
	// 2) If image has a priceset (>0) then return that.
	// 3) If image has no priceset, and projectID passed, return the passed projectID priceset.
	// 4) If image has no priceset, return price set of image's parent project



	// 1) If ProjectID is passed, get the price set info for the project
	if ($currentProjectID) {
		$DEBUG && $msg .= __FUNCTION__.": Current project=$currentProjectID, pricesetid=$currProjPricesetID<BR>";
		$currentProject = FetchProject ($currentProjectID);
		$currProjPricesetID = $currentProject["PriceSetID"];
		if ($currProjPricesetID == 1) {
			return 0;
		} else {
			$currProjPriceSet = FetchPriceSetByID ($currProjPricesetID);
		}
	}
	
	// 2) If image has a priceset (>0) then return that.
	$pricesetID = $image["PriceSetID"];
	$DEBUG && $msg .= __FUNCTION__.__LINE__.": Image pricesetid : $pricesetID<BR>";
	if ($pricesetID > 0)
		return $pricesetID;
		

	// 3) If image has no priceset, and projectID passed, return the passed projectID priceset.
	$DEBUG && $msg .= __FUNCTION__.__LINE__.": No image pricesetid.<BR>";
	
	($currentProjectID)
		? $pid = $currentProjectID
		: $pid = $image["ProjectID"];

	$project = FetchProject ($pid);
	$pricesetID = $project["PriceSetID"];

	$DEBUG && $msg .= __FUNCTION__.": source project ID : $pid, pricesetid : $pricesetID<BR>";
	
	$pricesetID || $pricesetID = 0;
	return $pricesetID;
}


// Return an array of values for numbered fields in a record,
// e.g. size1, size2, size3. This assumes no skipped fields, i.e. 
// size2 empty while size3 has a value.
// Array value start at 1, not zero, because we label fields that way (first is size1)
function GetNumberedFields ($record, $fieldBasename) {
	$i = 1;
	$arr = array ();
	while (isset ($record[$fieldBasename.$i])) {
		$arr["$1"] = $record[$fieldBasename.$i];
	}
	return $arr;
}


function CatalogNumber ($x) {
	$x = str_pad($x, 6, "0", STR_PAD_LEFT);	//left-pad with zeros
	return $x;
}

// This assumes the specString is already sent through base64_decode
// $decode is for urldecode()
function UnpackSpec ($specString, $decode = true) {
	$spec = array ();
	$decode && $specString = urldecode($specString);
	$arr = explode ("&", $specString);
	foreach ($arr as $p) {
		list ($k,$v) = explode ("=", $p);
		$v = html_entity_decode ($v);
		$spec[$k] = $v;
	}
	return $spec;
}



function FullPrintOffering ($imageID) {
	$pricesetID = FetchPriceSetID ($imageID);
	$priceset = FetchPriceSetByID ($pricesetID);
	$printsizes = $salesinfo['printsizes'];
	$editionsizes = $salesinfo['editionsizes'];
	$amounts = $salesinfo['amount'];	// will only appear if there's a dedicated PriceSet entry
	$prices = $salesinfo['PrintPrices'];

}


/* PAYPAL MASSPAY HANDLING
$paymentOrders are orders to pay artists
		- payee info
			- artist/supplier
			- record ID
			
		- sale info
			- amount
			- details of order
			- imageID
*/

function MassPay ( $paymentorders, $emailsubject) {
	global $BASEDIR, $LOGS;

	$DEBUG = 0;
	$VERBOSE = false;
	
	$masspay = new paypal_masspay ();
	
	// These settings are in the protected config file
	$masspay->params = array (	
		"USER"			=>	MASSPAY_USER_NAME,
		"PWD"			=>	MASSPAY_PASSWORD,
		"SIGNATURE"		=>	MASSPAY_SIGNATURE,
		"VERSION"		=>	MASSPAY_VERSION,
		"METHOD"		=>	"MassPay",
		"CURRENCYCODE"		=>	PAYPAL_CURRENCY_CODE,
		"EMAILSUBJECT"		=>	$emailsubject,
		"RECEIVERTYPE"		=>	"EmailAddress"
	);
	
	$masspay->payments = array ();
	foreach ($paymentorders as $paymentorder) {
		$masspay->payments[] = $paymentorder;
		//fp_error_log(__FUNCTION__.__LINE__.": ".print_r($paymentorder, true), 3, FP_PAYMENTS_LOG );
		fp_error_log(__FUNCTION__.__LINE__.": Prepare Paypal MassPay payment of {$paymentorder['L_AMT']} ".PAYPAL_CURRENCY_CODE." to {$paymentorder['L_EMAIL']} for sale #{$paymentorder['SaleID']} image #{$paymentorder['L_UNIQUEID']}.", 3, FP_PAYMENTS_LOG );
	}
	
		
	// Built NVP string
	$masspay->error_email = PAYPAL_ERROR_EMAIL;
	$masspay->currentrow = null;
	$masspay->url = MASSPAY_PAYPAL_URL;
	$response = $masspay->PPHttpPost(true);	// returned a parsed response from paypal, not raw response
	$response['response'] || $response['response'] = array ();
	fp_error_log(__FUNCTION__.__LINE__.": Payment response ".print_r($response['response'],true), 3, FP_PAYMENTS_LOG );
	
	// Interpret response
	
	// Display the API response back to the browser.
	// If the response from PayPal was a success, alert all receivers they were paid
	// If the response was an error, display the errors received using APIError.php.

	$DEBUG && var_dump ($masspay);
	   
	if ($masspay->response["status"]) {
		if ($masspay->response["response"]['ACK'] == "Failure") 
			{
			$error = __FILE__.":".__FUNCTION__.": Paypal MassPay Error: ".print_r($masspay->response["response"], true);
			$error .= "\n =================== \n Payment info:\n".print_r($paymentorder, true);
			$masspay->error_out($error, FP_EMAIL_HEADERS);
			
			$DEBUG && var_dump ($this->masspay);
			$VERBOSE && $masspay->error_out($error, __FUNCTION__.__LINE__.": MASSPAY FAILED: ".print_r($masspay->response["response"], true));
			}
		else
			{
			}
		// Alert payees even if failure. Why? Because I don't know which failed!
		// I THINK THIS HAS TO BE REDONE TO USE IPN!!
		$masspay->alert_payees (FP_EMAIL_HEADERS);
		$DEBUG &&  print __FUNCTION__.":SUCCESS:" . print_r ($masspay->response);
		$masspay->log_masspay_payments ();
		$masspay->record_masspay_payments ();
	} else {
		$error = "";
		if (is_array ($masspay->response["response"])) {
			while (list ($k,$v) = each ($masspay->response["response"])) {
				$error = "$k = $v\n";
			}
		}
		$masspay->error_out($error, FP_EMAIL_HEADERS);
		$DEBUG && print __FUNCTION__.":ERROR:" . print_r ($masspay->response);
	}
	return $masspay->response;
}


// What's the cost from the supplier for a particular sale in the FP_SALES table?
// Supplier gets a fixed amount, from the PriceSet
// PAYPAL SHIPPING: we *also* add in paypal shipping/handling. 
// Therefore, TURN OFF the FP shipping costs if you use
// PP shipping costs.

function SupplierCost ($saleID, $includeShipping = false ) {
	global $error, $msg;
	
	$DEBUG = 0;
	
	$DEBUG && print __FUNCTION__.": What's the supplier cost of sale $saleID?<BR>";
	
	$item = FetchSale ($saleID);
	$spec = UnpackSpec ($item["item_spec"]);
	$pricesetID = FetchPriceSetID ($item['item_id']);

	//$priceset = FetchPriceSetByID ($pricesetID);
	$priceset = GetPriceSet ($pricesetID);	//this function gets stuff in arrays, instead of the raw record

	$DEBUG && print "PricesetID=$pricesetID:".ArrayToTable ($priceset)."<br><br>";
	$DEBUG && print "SPEC:".ArrayToTable ($spec)."<br><br>";
	$DEBUG && print "FP_ORDER_ROW:". $spec[FP_ORDER_ROW]."<br><br>";
	$i = $spec[FP_ORDER_ROW] - 1;	//the row is 1..n, while the index of priceset arrays starts at 0
	$printcost = $priceset["cost"][$i];	// print cost
	$framecost = $priceset["framecost"][$i];	// frame cost
	!empty($spec[FP_ORDER_FRAMECODE]) || $framecost = 0;	// 0 is unframed


	if ( $spec[FP_ORDER_MATCHPRINT] && (array_sum ($priceset["Amount"]) < 1) ) {
		// How much does a matchprint cost?
		// If the picture has no sales, then we assume no matchprint
		// was made.
		//$mpcost = $priceset["MatchPrintPrice"]; Get price at the time of sale, not from price set!
		$mpcost = $item["MatchPrintPrice"];
	} else {
		$mpcost = 0;
	}
	
	$cost = $printcost + $framecost + $mpcost;
	if ($includeShipping) {
		// Use the extra shipping set in our system, IN ADDITION TO PAYPAL,
		// if it's set. 
		$extrashipping = $spec[FP_ORDER_EXTRA_SHIPPING];	// extra shipping cost is stored in $spec
		$cost = $extrashipping;
	
		// shipping as reported by Paypal...use this
		$shipping = $item['mc_shipping'] + $item['mc_handling'];
	
		$DEBUG && print __FUNCTION__.": Cost + shipping = $cost + $shipping<BR>";	
	
		$cost = $cost + $shipping;
	}

	//$DEBUG && print __FUNCTION__.": cost array = ".ArrayToTable ($priceset["cost"]);	
	$DEBUG && print __FUNCTION__.": printcost + framecost + MatchPrintPrice = $printcost + $framecost + $mpcost<BR>";	
	$DEBUG && print __FUNCTION__.": Cost = $cost<BR>Sale:".ArrayToTable ($item);
	
	return $cost;
}

// If matchprint required is checked, and this is the 1st print sale, require a matchprint.
function SupplierMatchprintCostBySaleID ($saleID) {
	global $error, $msg;
	
	$DEBUG = 0;
	
	$DEBUG && print __FUNCTION__.": What's the matchprint cost of sale $saleID?<BR>";
	
	$item = FetchSale ($saleID);
	$pricesetID = FetchPriceSetID ($item['item_id']);
	$priceset = GetPriceSet ($pricesetID);	//this function gets stuff in arrays, instead of the raw record
	
	// If matchprint required, and there have been no previous completed sales.
	// Because we might update the number of sales BEFORE calling this function, we check for 
	// count <= 1, not count < 1.
	if ( $item['MatchprintRequired'] && $item['txn_id'] && array_sum ($priceset["Amount"]) <= 1 ) {
		$supplier = FetchSupplier($priceset['supplierID']);
		$mpcost = $supplier["MatchPrintPrice"];
	} else {
		$mpcost = 0;
	}
		
	return $mpcost;
}


function ShippingCost ($saleID) {
	$DEBUG = false;
	
	$item = FetchSale ($saleID);
	$spec = UnpackSpec ($item["item_spec"]);
	$extrashipping = $spec[FP_ORDER_EXTRA_SHIPPING];	// extra shipping cost is stored in $spec
	$cost = $extrashipping;
	$shipping = $item['mc_shipping'] + $item['mc_handling'];
	return $shipping;
}


// Find and delete unused orders in the Sales database
// 
function DeleteUnusedOrders () {
	global $BASEDIR, $LOGS;
	global $LINK;
	
	// Delete any orders which have not been confirmed, and 
	// after a certain time period.
	$where = "(txn_id = '') AND TIME_TO_SEC(TIMEDIFF(NOW(), order_time )) > ".FP_CLEAR_UNCONFIRMED_ORDERS_DELAY;
	$query = "SELECT id, item_number, item_id, quantity FROM ".DB_SALES." WHERE $where";

	$result = mysqli_query ($LINK, $query);
	if ($result) {
	while ($sale = mysqli_fetch_array($result, MYSQLI_ASSOC))
		{
		// If this is the only sale, then restore the generic price set
		// and delete the unique copy for this image.
		// Update the Image to point to the previous priceset
		$imageID = $sale['item_id'];
		$image = FetchImage($imageID);
		$pricesetID = $image['PriceSetID'];
		$priceset = GetPriceSet ($pricesetID);
		// Also check this is a not generic priceset: imageID > 0
		if ($priceset['ImageID'] > 0)
			{
			//subtract the quantity of the sale from the recorded sales in the unique priceset
			// If amount sold is 1 or less, delete the unique price set!
			if ($priceset['AmountSold'] > 1)
				{
				RemoveImageSale($sale['id']);
				fp_error_log("Removed {$sale['quantity']} sales from image {$sale['item_id']} because the order #{$sale['id']} was not completed in time.", 3, FP_MAINTENANCE_LOG);
					ClearAllCache();
				}
			else
				{
				// get generic source price set
				$sourceID = $priceset['SourceID'];
				/*
				// set image priceset to the source ID (could be zero)
				UpdateRecord(DB_IMAGES, $imageID, array('PriceSetID'=>$sourceID));
				// delete unique price set
				DeleteRowByID( DB_PRICESETS, $pricesetID);
				*/
				DeletePriceSet($pricesetID);

				fp_error_log("Deleted the unique price set for image $imageID because all sales of prints were reversed.", 3, FP_MAINTENANCE_LOG);
				}
			}
		DeleteRowByID( DB_SALES, $sale['id']);
		}
	} else {
		// fp_error_log(__FUNCTION__.": Did not find any unused orders with query:\n$query", 3, FP_MAINTENANCE_LOG );
	}
		
}


// Delete a Price Set
function DeletePriceSet ($ID) {
	EditTable ("delete", DB_PRICES, $ID);
}


// Reset the price set for an image
// Use only for testing, or to clean up some testing mess
// - delete image priceset
// - set PriceSetID to 0 in the image
function ResetImagePriceset ($imageID) {
	$image = FetchImage($imageID);
	$pricesetID = $image['PriceSetID'];
	$priceset = GetPriceSet ($pricesetID);
	// Also check this is a not generic priceset: imageID > 0
	if ($priceset['ImageID'] > 0)
		{
		// get generic source price set
		$sourceID = $priceset['SourceID'];
		// set image priceset to the source ID (could be zero)
		UpdateRecord(DB_IMAGES, $imageID, array('PriceSetID'=>$sourceID));
		}
	print "Set PriceSetID of image $ID to {$sourceID}<BR>";
	print "Delete custom price set ID ".$image['PriceSetID']."<BR>";
	DeletePriceSet ($image['PriceSetID']);
}

// Sends a POST request to whichever mailer system we're using
// to send out newsletters. 
// A Mailing is just a subject, message, and mail list name
// $list is an array or string, the ID number of the list
function AddNewMailingToQueue ($subject = "", $data = "", $list = "") {
	if ($data && $subject && $list ) {
		$data = array ("message"=>$data);
		$data['subject']=$subject;
		if (!is_array($list))
			$list = array ($list);
		$data['lists']=join (",", $list);
		$data = http_build_query ($data);
		do_post_request( FP_EMAILER , $data, null) ;
	} else {
		fp_error_log(__FUNCTION__.": an attempt was made to send a message that was missing either subject ($subject), list ($list), or data.", 3, FP_ERROR_LOG );
	}
}


function pickupMessage ($path = "") {
	global $msg, $error;
	global $LINK;
	
	$DEBUG = false;
	$path || $path = FP_DIR_NEWSLETTER;

	$files = array();
	$handle=opendir($path);
	foreach ( glob ($path."*") as $file) {
		$files[] = $file;
	}
	
	foreach ($files as $file) {
		$message = file_get_contents ($file);
		preg_match_all ("/<meta\s*?name.*?\"(\w*?)\".*?content.*?\"(.*?)\"/i", $message, $vars);
		$i=0;
		foreach ($vars[1] as $k) {
			$params[$k] = $vars[2][$i];
			$i++;
		}

		$subject=$params['Subject'];
		$lists=$params['Lists'];
		
		if (!$DEBUG) {
			$sql=sprintf("INSERT INTO phpemaillist_messages(subject, body) VALUES('%s', '%s')", 
			 mysqli_real_escape_string($subject),
			 mysqli_real_escape_string($message));
			
			$result=mysqli_query ($sql) or die("Unable to execute query compose 3 $result");
			$messageid=mysqli_insert_id();
			
			foreach ($lists as $list){
				$sql=sprintf("INSERT INTO phpemaillist_queue(message, list) VALUES('$messageid', %s);", mysqli_real_escape_string($LINK, $list));
				$result=mysqli_query ($sql) or die("Unable to execute query compose loop $result");
			}
			header("Location:" . MAILER_SYSTEM_URL . "?message=$message&amp;subject=$subject");
		}
	}


}

/*
 * GetImageSalePrice ($imageID, $itemCouponCode, $options)
 * Return the final sale price of an item, not including shipping/handling/taxes.
 * Does include coupon discounts.
 * $imageID : string : ID of the image record
 * $itemCouponCode : string : coupon code
 * $options : array : indeces of option choices (from the HTML sales page): size, matte, frame, glazing, paper
 */
function GetImageDiscountedPrice ($imageID, $itemCouponCode, $options = array() ) {

	$image = FetchImage($imageID);
	$salesinfo = ImageSalesInfo ($imageID, $projectID);
	$supplierID = $salesinfo['supplierID'];
	$pricesetID = $salesinfo['pricesetID'];

	$imageSizeIndex = $options['size'];
	$imageMatteIndex = $options['matte'];
	$imageFrameIndex = $options['frame'];
	$imageGlazingIndex = $options['glazing'];
	$imagePaperIndex = $options['paper'];

	$supplier = FetchSupplier($supplierID);
	$pricing = GetPricingByID ($imageID, $supplierID, $pricesetID);
	$prices = GetCartPricingOneSize ($pricing, $supplier, $imageSizeIndex);
	$cartprices = GetAllCartPricingByID ($imageID, $supplierID, $pricesetID);

	if ($imageFrameIndex * $imageMatteIndex > 0) {
		$pkg = "FrameMatte";
	} else if ($imageFrameIndex > 0) {
		$pkg = "FrameToPrint";
	} else if ($itemMatteIndex > 0) {
		$pkg = "Matte";
	} else {
		$pkg = "Print";
	}

	$itemPrice = $prices['Total'.$pkg.'Price'];
	$itemCost = $prices['Pretax'.$pkg.'Cost'];

	if ($itemCouponCode) {
		// Discount using coupon
		$couponInfo = GetCouponDiscount ($itemCouponCode, $image['ArtistID']);

		// APPLY DISCOUNT
		$itemPrice = DiscountedPrice ($itemPrice, $itemCost, $couponInfo);
	}

	return array ("price"=>$itemPrice, "cost"=>$itemCost);
}

/*
 * Same as above, but without the discount.
 */

function GetImageSalePrice ($imageID, $options) {
	$iteminfo = GetImageDiscountedPrice ($imageID, null, $options);
	return $iteminfo['price'];
}

function GetImageSalePriceAndCost ($imageID, $options) {
	$iteminfo = GetImageDiscountedPrice ($imageID, null, $options);
	return $iteminfo;
}


// Sort the arrays in a price set by the size
// If we pass a record which hasn't been unpacked, return it as is.
function SortAndCleanPriceSetRows ($record) {

	if (!is_array($record['Size']))
		return $record;

	// Resort arrays by the order of the Sizes
	$i = 0;
	$order = array ();
	foreach ($record['Size'] as $s) {
		$s > 0 && ($s && $order[$i] = $s);
		$i++;
	}
	asort ($order, SORT_NUMERIC);
	$order = array_keys($order);
	$k=0;
	$src = $record;

	// If there is a sizeN entry, loop.
	foreach ($order as $i) {
		if ($record['Size'][$i] > 0) {
			$record['Amount'][$k] = (int)$src['Amount'][$i];
			$record['Size'][$k] = (float)$src['Size'][$i];
			$record['EditionSize'][$k] = (float)$src['EditionSize'][$i];
			$record['PrintCost'][$k] = (float)$src['PrintCost'][$i];
			$record['PrintPrice'][$k] = (float)$src['PrintPrice'][$i];
			$record['Markup'][$k] = (float)$src['Markup'][$i];
			$record['MatteCost'][$k] = (float)$src['MatteCost'][$i];
			$record['MattePrice'][$k] = (float)$src['MattePrice'][$i];
			$record['FrameToPrintCost'][$k] = (float)$src['FrameToPrintCost'][$i];
			$record['FrameToPrintPrice'][$k] = (float)$src['FrameToPrintPrice'][$i];
			$record['FrameMatteCost'][$k] = (float)$src['FrameMatteCost'][$i];
			$record['FrameMattePrice'][$k] = (float)$src['FrameMattePrice'][$i];
			$record['PrintShipWeight'][$k] = (float)$src['PrintShipWeight'][$i];
			$record['MatteShipWeight'][$k] = (float)$src['MatteShipWeight'][$i];
			$record['FrameToPrintShipWeight'][$k] = (float)$src['FrameToPrintShipWeight'][$i];
			$record['FrameMatteShipWeight'][$k] = (float)$src['FrameMatteShipWeight'][$i];
			$record['Inactive'][$k] = $src['Inactive'][$i];
			$k++;
		}
	}
	return $record;
}



?>