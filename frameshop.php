<?php

/*
Frameshop page
Choose options on a print to buy
- Frame and Matte
- Size
- Limited edition or unlimited
- paper?
- other options?

*/
	
include "_config/sysconfig.inc";
include "_config/fpconfig.inc";
include "_config/config.inc";
include "includes/functions.inc";
include "includes/project_management.inc";
include "includes/image_management.inc";
include "includes/commerce.inc";

$DEBUG = 0;

$error = "";
$msg = "";
$pagevars = array ();

session_name("fp_gallery_session");
session_start();

$LINK = StartDatabase(MYSQLDB);
Setup ();

$results = GetFormInput();
$actions = $results['actions'];
$action = $results['actions']['action'];
$vars = $results['vars'];
isset($vars['GroupID']) && $_SESSION['GroupID'] = $vars['GroupID'];
isset($_SESSION['GroupID']) || $_SESSION['GroupID'] = PUBLIC_GROUP_ID;

// isset($vars['theme']) && $_SESSION['theme'] = $vars['theme'];
isset($_SESSION['theme']) || $_SESSION['theme'] = DEFAULT_THEME;

$groupID = $_SESSION['GroupID'];
$myGroup = new FPGroup ($LINK, $groupID);

$GroupBannerURL = $myGroup->LogoFilename();
$GroupBanner = $myGroup->LogoHTML("style='border:1px solid black;margin-right:10px;'");
$GroupIcon = $myGroup->IconHTML();

$myTheme = CurrentThemeID ();

// Get user info, if it exists. It would come because the admin passed a user ID variable
(isset($vars['fp_user']) && $vars['fp_user']) && $_SESSION['fp_user'] = $vars['fp_user'];

$imageID = $results['vars']['ImageID'];
$image = FetchImage ($imageID);
$artistID = $image['ArtistID'];
$artist = FetchArtist ($artistID);
$image['Date'] = strftime("%B %e, %G", mysqldate_to_timestamp($image['CreatedDate']));
$image["Place"] = ($image['City'] and $image['Country']) ? $image['City'].", ".$image['Country'] : $image['Country'];
$image["Artist"] = join (" ", StripBlankFields (array ($artist["Firstname"] ,  $artist['Lastname'])));
$image['FullCredit'] = join (" : ", StripBlankFields (array ($image["Place"], $image['Date'], $image["Artist"])));
isset ($image["Params"]) 
? $imageParams = DecodeArrayFromDB($image["Params"])
: $imageParams = array ();

$imageParams[FP_PARAM_IMAGE_ARTWORK_NAME]
? $artworkname = $imageParams[FP_PARAM_IMAGE_ARTWORK_NAME]
: $artworkname = '{fp:Image}';

$fields['ITEM_NAME'] = htmlspecialchars($image['Title'], ENT_QUOTES);


// GET SALES INFO
// Full original offering of the entire edition
$allPriceSet = ImageSalesInfo ($imageID, '', true);
$allPrintsizes = $allPriceSet['printsizesformatted'];
$allprices = $allPriceSet['PrintPrices'];
$allavailable = $allPriceSet['available'];
$alleditionsizes = $allPriceSet['editionsizes'];
$editionType = $allPriceSet['edition_type'];
$maxFramedSize = $allPriceSet['maxframedsize'];

$available = array_sum ($allavailable);
$artistinfo = array();

// No prints available, you arrive here in error!
if (($editionType != "unlimited") && !$available)
	{
	$error .= "ERROR: There are no prints remaining to sell! You should not have arrived at this page!<br>";
	}
else
	{
	
	// rename keys to avoid possible bad substitutions
	foreach ($artist as $k => $v) {
		$artistinfo[DB_ARTISTS."_$k"] = $v;
	}
	
	$projectID = $vars['ProjectID'];
	$project = FetchProject ($projectID);
	
	// Get sales settings available (not full offering of entire original)
	$salesinfo = ImageSalesInfo ($imageID, $projectID);

	$printsizesPlain = $salesinfo['printsizes'];	// not HTML in the lines, for the popup
	$printsizes = $salesinfo['printsizesformatted'];
	
	$papersizesPlain = $salesinfo['papersizes'];	// not HTML in the lines, for the popup
	$papersizes = $salesinfo['papersizesformatted'];
	
	$editionsizes = $salesinfo['editionsizes'];
	$amounts = $salesinfo['amount'];	// will only appear if there's a dedicated PriceSet entry
	$prices = $salesinfo['PrintPrices'];
	$pricesframed = $salesinfo['priceframed'];
	$sizes = $salesinfo['size'];
	$psizes = $salesinfo['papersize'];
	$pricesetID = $salesinfo['pricesetID'];
	$weights = $salesinfo['weights'];
	$weightsframed = $salesinfo['weightsframed'];
	$dims = $salesinfo['dims'];
	$rows = $salesinfo['rows'];
	
	// Printer costing values
	$fields["print_cost_unit"] = $salesinfo['print_cost_unit'];
	$fields["print_cost_area_unit"] = $salesinfo['print_cost_area_unit'];
	$fields["PRINT_COST_RATE"] = $salesinfo['print_cost_rate'];
	$fields["PRINT_COST_METHOD"] = $salesinfo['print_cost_method'];

	// Build popup of available sizes
	$sizesToShow = array ();
	
	// mark unlimited (unsigned) editions so we don't confuse the customer
	$k = 0;
	foreach ($printsizesPlain as $s) {
		/*
		$totaleditionsize 
			? $sizesToShow[$k] = $s . " (limited edition)" 
			: $sizesToShow[$k] = $s . " (unsigned, open edition)";
		*/
		$sizesToShow[$k] = $s;
		$k++;
	}
	
	// NOTE that the index for sizes starts at "0" not "1"
	
	// Get matte/frame/glazing info for the selected supplier. We're going to have to resolve this
	// "vendor" vs. "supplier" name, dammit. Same thing, but confusing!
	// $supplierID = $artist['Vendor'];	<-- wrong...artist could change vendors, but an image stays with original printer
	$supplierID = $salesinfo['supplierID'];
	
	//$supplier = GetRecord( DB_SUPPLIERS, $supplierID);
	$supplier = FetchSupplier($supplierID);
	
	// Supplier description
	$supplierinfo = $supplier['Description'];
	$supplierinfo = LimitHTLM ($supplierinfo);
	$supplierinfo = htmlentities ($supplierinfo, ENT_QUOTES , "UTF-8");
	$supplierinfo = preg_replace ("/\n|\r/", " ", nl2br($supplierinfo));
	$supplierinfo = str_replace(array("&gt;", "&lt;", "&amp;"), array(">", "<", "&"), $supplierinfo);

	$allframestyles = array_merge (array(0=>"No Frame"), explode ("\n", $supplier['Frames']));
	$allframecodes = array_merge (array(0=>""), explode ("\n", $supplier['FrameCodes']));
	$framestyles = $supplier['FrameNamesList'];
	$framecodes = $supplier['FrameCodesList'];
	
	$noSellFrame = $imageParams[FP_PARAM_IMAGE_NO_SELL_FRAME];
	$noSellFrame
		? $supplierHasFrames = 0
		: $supplierHasFrames = countAvailableItems($supplier['Frames']);

	$mattenames = array_merge (array(0=>"No Matte"), explode ("\n", $supplier['Mattes']));
	$mattecolors = array_merge (array(0=>"No Matte"), explode ("\n", $supplier['MatteColors']));
	$mattecodes = array_merge (array(0=>"NA"), explode ("\n", $supplier['MatteCodes']));

	$noSellMatte = $imageParams[FP_PARAM_IMAGE_NO_SELL_MATTE];
	$noSellMatte
		? $supplierHasMattes = 0
		: $supplierHasMattes = countAvailableItems($supplier['Mattes']);
	
	
	
	
	$papers = explode ("\n", $supplier['Papers']);
	$papercodes = explode ("\n", $supplier['PaperCodes']);
	
	$inksets = explode ("\n", $supplier['Inksets']);
	$inksetcodes = explode ("\n", $supplier['InksetCodes']);
	
	$glazing = explode ("\n", $supplier['Glazing']);
	$glazingcodes = explode ("\n", $supplier['GlazingCodes']);
	$supplierHasGlazing = trim($supplier['GlazingCodes']);
	
	array_walk ( $allframestyles, "trimMe" );
	array_walk ( $allframecodes, "trimMe" );
	array_walk ( $framestyles, "trimMe" );
	array_walk ( $framecodes, "trimMe" );
	
	array_walk ( $mattenames, "trimMe" );
	array_walk ( $mattecolors, "trimMe" );
	array_walk ( $mattecodes, "trimMe" );
	
	array_walk ( $papers, "trimMe" );
	array_walk ( $papercodes, "trimMe" );
	
	array_walk ( $inksets, "trimMe" );
	array_walk ( $inksetcodes, "trimMe" );
	
	array_walk ( $glazing, "trimMe" );
	array_walk ( $glazingcodes, "trimMe" );
	
	$fields['MATTES'] = join(",",$mattecolors);
	$fields['MATTENAMES'] = join(",", $mattenames );
	$fields['MATTECODES'] = join(",",$mattecodes);
	
	// These need to list ALL system items, because we index from the list of all items
	// not from the list of existing items
	$fields['FRAMES'] = join (",", $allframestyles );
	$fields['FRAMEWIDTHS'] = join (",", $FRAMESTYLESWIDTHS);	// $FRAMESTYLESWIDTHS is a global
	$fields['FRAMECODES'] = join(",",$allframecodes);
	
	$fields['PAPERS'] = join(",",$papers);
	$fields['PAPERCODES'] = join(",",$papercodes);
	
	$fields['INKSETS'] = join(",",$inksets);
	$fields['INKSETCODES'] = join(",",$inksetcodes);
	
	$fields['GLAZING'] = join(",",$glazing);
	$fields['GLAZINGCODES'] = join(",",$glazingcodes);
	
	// Javascript for changing values
	//$popupjs = FetchSnippet ("frameshop_popupjs");
	$popupjs = "";
	
	
	
	// -------
	// Picture sizing, matte sizing
	
	$URL = $image["URL"];
	// What size pictures?
	$WHICHPIX = $PHOTOS_GALLERY;
	$fields['WHICHPIX'] = $WHICHPIX;
	$fields['URL'] = $URL;
	$sizelabel = FetchSnippet ("frameshop_label_size");
	
	isset ($vars['currentsize']) && $currentsize = $vars['currentsize'];
	$currentsize || $currentsize = $sizes[count($sizes)-1];
	
	// If only one size is available, don't show a menu
	if (count($sizesToShow) == 1) {
		$sizelist = "<input type='hidden' id='currentsize' value='0'>";
		$mainfields['SizesList'] = "<div id=\"frameshop-sizelist\">$sizelist".$sizelabel.$sizesToShow[0]."</div>";
		
	} else {
		$mainfields['SizesList'] = "<div id=\"frameshop-sizelist\">".$sizelabel.OptionListFromArray ($sizesToShow, "currentsize", $currentsize, false, OPTION_LIST_IS_POPUP, " ID=\"currentsize\"")."</div>";
	}
	
	// Frame settings
	$projectframewidth = $project['Framewidth'];
	$currentframe = null;	// no frame
	isset($vars['currentframe']) && $currentframe = $vars['currentframe'];
	
	if (isset($currentframe)) {
		$currentframe = $vars['currentframe'];
		// is the requested frame valid?
		if ($allframestyles[$currentframe][0] == "/") {
			$currentframe = null;
		}
		$currentframewidth = $FRAMESTYLESWIDTHS[$currentframe];
	} else {
		$currentframe = $project['Framestyle'];
		$currentframewidth = $project['Framewidth'];
	}
	
	// Just in case...I've found missing params for this before!
	$currentframewidth || $currentframewidth = 10;
	
	$fields["currentframe"] = $currentframe;
	$fields["CURRENTFRAMEWIDTH"] = $currentframewidth;
	$framelabel = FetchSnippet ("frameshop_label_frame");
	if ($supplierHasFrames) {
		$mainfields['FrameStyleList'] = $framelabel . "<span id='frameshop-framelist-hidden' style='display:none;'>{FP_NOT_AVAILABLE}</span>" . "<span id='frameshop-framelist' style='display:none;'>". OptionListFromArray ($framestyles, "currentframe", $currentframe, false, OPTION_LIST_IS_POPUP, " ID='currentframe'")."</span><br>";
	} else {
		$mainfields['FrameStyleList'] = "<input type='hidden' id='currentframe' value='0'>";
	}
	
	// MATTE
	$currentmattecolor = MATTECOLOR;	// Default from config
	$currentmatte = 0;	// default
	isset ($vars['currentmatte']) && $currentmatte = $vars['currentmatte'];
	$fields['CURRENTMATTECOLOR'] = $mattecolors[$currentmatte];
	$fields['CURRENTMATTENAME'] = $mattenames[$currentmatte];
	
	// If we don't allow a frame w/o a matte, then set matte if not set.
	if ($currentframe && !$currentmatte)
		$currentmatte = 1;
	
	// Or, if no frame set, remove the matte
	if (!$currentframe)
		$currentmatte = 0;
	
	$mattelabel = FetchSnippet ("frameshop_label_matte");
	if ($supplierHasMattes) {
		$mainfields['MatteList'] = $mattelabel . "<span id='frameshop-mattelist-hidden' style='display:none;'>{FP_NOT_AVAILABLE}</span>" . "<span id='frameshop-mattelist' style='display:none;'>". OptionListFromArray ($mattenames, "currentmatte", $currentmatte, false, OPTION_LIST_IS_POPUP," ID='currentmatte'")."</span><br>";
	} else {
		$mainfields['MatteList'] = "<input type='hidden' id='currentmatte' value='0'>";
	}
	
	$matted = ($currentmatte > 0);	//currentmatte == 0 means no matte.
	
	
	// GLAZING
	$currentglazing = 1;	// default
	isset ($vars['currentglazing']) && $currentglazing = $vars['currentglazing'];
	$glazinglabel = FetchSnippet ("frameshop_label_glazing");
	if ($supplierHasGlazing) {
		$mainfields['GlazingList'] = $glazinglabel . "<span id='frameshop-glazinglist-hidden' style='display:none;'>{FP_NOT_AVAILABLE}</span>" . "<span id='frameshop-glazinglist' style='display:none;'>". OptionListFromArray ($glazing, "currentglazing", $currentglazing, false, OPTION_LIST_IS_POPUP," ID='currentglazing'</span><br>");
	} else {
		$mainfields['GlazingList'] = "<input type='hidden' id='currentglazing' value='0'>";
	}
	
	
	
	// WALL COLOR: allow user to choose the color of the wall
	$mainfields['WallColorList'] = OptionListFromArray ($WALLCOLORS, "wallcolor", FP_FRAMESHOP_WALLCOLOR, false, OPTION_LIST_IS_POPUP," ID='wallcolor'");
	
	$matted = ($currentmatte > 0);	//currentmatte == 0 means no matte.
	
	
	// PAPER

	$paperlabel = FetchSnippet ("frameshop_label_paper");
	if ( FP_CUSTOMER_CHOOSES_PAPER ) {
		$currentpaper = $project['papercode'];
		$mainfields['PaperList'] = $paperlabel . "<span id='frameshop-paperlist-hidden' style='display:none;'>{FP_NOT_AVAILABLE}</span>" . "<span id='frameshop-paperlist' style='display:none;'>" . OptionListFromArray ($papers, "currentpaper", $currentpaper, false, OPTION_LIST_IS_POPUP," ID='currentpaper'")."</span><br>";
	} else {
		$currentpapercode = $salesinfo['papercode'];
		empty($currentpapercode) && $currentpapercode = 0;
		$currentpapername = $papers[$currentpapercode];

		/*
		if ($salesinfo['papercode']) {
			$currentpapername = $salesinfo['paper'];
			$currentpaper = 0;
		} else {
			$currentpaper = $project['PaperCode'];
			empty($currentpaper) && $currentpaper = 0;
			$currentpapername = $papers[$currentpaper];
		}
		*/
		//$mainfields['PaperList'] = $currentpapername . "<input type='hidden' id='currentpaper' value='$currentpaper'>";
		$mainfields['PaperList'] = "<input type='hidden' id='currentpaper' value='$currentpaper'>";
	}
	
	// INK
	$paperlabel = FetchSnippet ("frameshop_label_ink");
	if ( FP_CUSTOMER_CHOOSES_INKSET ) {
		$currentinkset = $project['inksetcode'];
		isset ($vars['currentinkset']) && $currentinkset = $vars['currentinkset'];
		$mainfields['InksetList'] = $inklabel . "<span id='frameshop-inksetlist-hidden' style='display:none;'>{FP_NOT_AVAILABLE}</span>" . "<span id='frameshop-inklist' style='display:none;'>" . OptionListFromArray ($inksets, "currentinkset", $currentinkset, false, OPTION_LIST_IS_POPUP,$popupjs." ID='currentinkset'")."</span><br>";
	} else {
		$currentinkset = $salesinfo['inksetcode'];
		empty($currentinkset) && $currentinkset = 0;
		$currentinksetname = $inksets[$currentinkset];
	/*
		if ($salesinfo['inkset']) {
			$currentinksetname = $salesinfo['inkset'];
			
		} else {
			$currentinkset = $project['InksetCode'];
			
			$currentinksetname = $inksets[$currentinkset];
		}
	*/
		//$mainfields['InksetList'] = $currentinksetname . "<input type='hidden' id='currentinkset' value='$currentinkset'>";
		$mainfields['InksetList'] = "<input type='hidden' id='currentinkset' value='$currentinkset'>";
	}
	
	
	
	// QUANTITY
	// The quantity available depends on the number of prints already sold!
	// default is 1
	// $salesinfo['available'] is the array of amount available.
	// If the value is zero, then it means there are an unlimited amount! (not zero...those sizes aren't returned as values)
	$quantitylist = array();
	if ($editionType == "original")
	{
		$ql = "<input id='currentquantity' name='quantity' type='hidden' value='1'>1";
	} else if ($editionType == "unlimited") {
		$ql = "<input type='text' ID='currentquantity' name='quantity' maxsize='3' value='1' $popupjs style='text-align:right;width:3.0em;'>";
	} else {
		for ($i=1;$i<=$available;$i++)
		{
			$quantitylist[$i] = $i;
		}
		$ql = OptionListFromArray ($quantitylist, "quantity", "1", false, OPTION_LIST_IS_POPUP, $popupjs." ID='currentquantity'");
	}
	$qlabel = FetchSnippet("frameshop_label_quantity");
	$mainfields['Quantity'] = $qlabel . $ql;

	// CALC SCREEN SIZES
	// Set sizes for screen display
	// Draw with or without a matte
	$imgsize = GetLocalImageSize("$BASEDIR/$WHICHPIX/$URL");
	$w = $imgsize[0] * $FRAMESHOP_SIZE_ADJUSTMENT;
	$h = $imgsize[1] * $FRAMESHOP_SIZE_ADJUSTMENT;
	
	// Resize to fit show area
	// Not right now...
	
	$fields['IMG_Width'] = $w;
	$fields['IMG_Height'] = $h;
	
	// Get prices for unframed, framed prints
	if ($currentframe > 0) {		// FRAMED
		$unitPrice = $salesinfo['priceframed'][$currentsize];
	} else {							// UNFRAMED
		$unitPrice = $salesinfo['PrintPrices'][$currentsize];
	}
	
	$fields['PAPERCODE'] = $salesinfo['papercode'];
	$fields['INKSETCODE'] = $salesinfo['inksetcode'];
	$fields['PAPERNAME'] = $currentpapername;
	$fields['INKSETNAME'] = $currentinksetname;
	
	
	$fields['MAXPRINTSIZE'] = $maxprintsize;
	
	$fields['PRINTERNAME'] = $supplier["Name"];
	
	
	$fields['ROWS'] = join(",",$rows);
	
	$fields['UNITPRICE'] = $unitPrice;
	
	$fields['MAXPRINTSIZE'] = $maxprintsize;
	$fields['MAXDIMS'] = join(",",$sizes);
	$fields['MAXPDIMS'] = join(",",$psizes);
	
	
	$fields['PRINTPRICELIST'] = join(",",$salesinfo['PrintPrices']);
	$fields['FRAMEMATTEPRICELIST'] = join(",",$salesinfo['FrameMattePrices']);
	
	
	// CATALOG NUMBER USES IMAGE ID
	$artistInitials = $artist["Firstname"][0] . $artist["Lastname"][0];
	$catalogNum = $artistInitials . "-". str_pad($imageID, 6, "0", STR_PAD_LEFT);	//left-pad with zeros
	$fields['CATALOGNUMBER'] = $catalogNum;
	
	//$salesinfo = ImageSalesInfo ($imageID, $projectID);
	//$fields['WEIGHTS'] = join (",",$salesinfo['weights']);
	//$fields['WEIGHTSFRAMED'] = join (",",$salesinfo['weightsframed']);
	
	$fields['SIZES'] = join(",", $printsizesPlain);
	$fields['PAPERSIZES'] = join(",", $papersizesPlain);
	
	
	$fields['DIMENSIONS'] = join(",",$salesinfo['idims']);
	$fields['PDIMENSIONS'] = join(",",$salesinfo['pdims']);
	
	
	if ($editionType == "original") {
		$printDetailsWrapper = FetchSnippet('frameshop_printing_details_original');
	} else {
		$printDetailsWrapper = FetchSnippet('frameshop_printing_details');
	}
	
	$fields['PRINTING_DETAILS'] = Substitutions ($printDetailsWrapper, $fields);

	// =================

	
	// SHIPPING
	//$fields['EXTRASHIPPING'] = join(",",$salesinfo['extrashipping']);
	$shipping = explode(",", FetchSnippet ("frameshop_shipping_popup"));
	$mainfields['ShippingList'] = OptionListFromArray ($shipping, "currentshipping", "0", false, OPTION_LIST_IS_POPUP, $popupjs." ID='currentshipping'");


	// HANDLING
	$fields["SHIP_HANDLING_RATES"] = join(",", $supplier['Handling']);



	
	/*
		$rate = new UPS;
		$rate->upsProduct("2DA"); // See upsProduct() function for codes
		$rate->origin("32825", "US"); // Use ISO country codes!
		$rate->dest("87540", "US"); // Use ISO country codes!
		$rate->rate("CC"); // See the rate() function for codes
		$rate->container("CP"); // See the container() function for codes
		$rate->weight("3");
		$rate->rescom("COM"); // See the rescom() function for codes
		$quote = $rate->getQuote();
		
		$rate->setSelectRate('2DA', $quote);
		$rate->upsProduct("3DS");
		$quote = $rate->getQuote();
		$rate->setSelectRate('3DS', $quote);
		$rate->upsProduct("GND");
		$quote = $rate->getQuote();
		$rate->setSelectRate('GND', $quote);
		
		echo $rate->displayRatesHtml("data[shipping-type]", "radio");
	*/
	
	
	
	// MATCHPRINT REQUIRED
	$fields['MATCHPRINT'] = $project['Matchprint'];
	
	// SUPPLIER ID : WHICH SUPPLIER IS THE ARTIST USING?
	$fields['SUPPLIERID'] = $salesinfo['supplierID'];
	
	$invoice = date("Ymd-G:i:s");			// e.g. 20080214-15:16:08
	$fields['INVOICE'] = $invoice;
	
	
	$k=0;
	$editionInfo = "";
	
	// Two versions of edition info:
	// - based on a single entry for the total size of edition and total sales
	// - based on adding up edition sizes for each print size
	if (FP_EDITION_CALC_METHOD == "total") {
		$totalav = 0;
		$totaleditionsize = $allPriceSet['totaledition'];
		$totalsold = $allPriceSet['totalsold'];
		$orwrapper = FetchSnippet ("frameshop_edition_original_wrapper");
		$liwrapper = FetchSnippet ("frameshop_edition_limited_wrapper");
		$ulwrapper = FetchSnippet ("frameshop_edition_unlimited_wrapper");
		$item = FetchSnippet ("frameshop_edition_info_edition_item");
		$delim = FetchSnippet ("frameshop_edition_info_edition_delim");
		$ulrow = FetchSnippet ("frameshop_edition_info_totals");
		$lirow = FetchSnippet ("frameshop_edition_info_list");
		$eiwrapper = FetchSnippet ("frameshop_edition_info_totals_box");
		
		// Gather print sizes for display
		foreach ($allPrintsizes as $ps) {
			$av = 0;
			$es = 0;
			if ($totaleditionsize > 0) {
				if ($editionType == "original") {
					// Original artwork
					$orInfo = Substitutions ($item, array (
						"PRINTSIZE"	=> $ps,
						"PRICE"		=> $allprices[$k],
						"AVAILABLE"	=> '',
						"EDITIONSIZE"	=> ''
						));
				} else {
					// Ltd edition
					$liInfo[] = Substitutions ($item, array (
						"PRINTSIZE"	=> $ps,
						"PRICE"		=> $allprices[$k],
						"AVAILABLE"	=> '',
						"EDITIONSIZE"	=> ''
						));
				}
			} else {
				// Open edition
				$es = "unlimited";
				$av = "";
				$r = $ulrow;
				$ulInfo[] = Substitutions ($item, array (
					"PRINTSIZE"	=> $ps,
					"PRICE"		=> $allprices[$k],
					"AVAILABLE"	=> '',
					"EDITIONSIZE"	=> ''
					));
			}
			
			$k++;
		}
		$orInfo ? $orList = $orInfo : $orList = "";
		$liInfo ? $liList = join ($delim, $liInfo) : $liList = "";
		$ulInfo ? $ulList = join ($delim, $ulInfo): $ulList = "";
		
		$orList || $orwrapper = "<!--no original artwork info-->";
		$liList || $liwrapper = "<!--no limited edition info-->";
		$ulList || $ulwrapper = "<!--no open edition info-->";
		
		// fix display of plurals if only one print, e.g. prints => print
		if ($available != 1) {
			
			$available > 0 ? $availableText = $available : $availableText = "no";	// "0" becomes "no"
			$avmsg = str_replace("{AVAILABLE}", $availableText, FetchSnippet ("are_blank_prints"));
		} else {
			$avmsg = FetchSnippet ("is_one_print");
		}
		
		$orInfo =  Substitutions ($orwrapper, array (
			"list"			=> $orList, 
			"ARE_BLANK_PRINTS"	=> $avmsg,
			"EDITIONSIZE"		=> $totaleditionsize,
			"ARTWORK_NAME"		=> $artworkname
			)
		);

		$liInfo =  Substitutions ($liwrapper, array (
			"list"			=> $liList, 
			"ARE_BLANK_PRINTS"	=> $avmsg,
			"EDITIONSIZE"		=> $totaleditionsize)
		);
		$ulInfo =  Substitutions ($ulwrapper, array ("list"=>$ulList));
								
									
		$arr = array (
			"ORIGINALEDITIONINFO" => $orInfo,
			"LIMITEDEDITIONINFO" => $liInfo,
			"UNLIMITEDEDITIONINFO" => $ulInfo
		);
		
		$fields['EDITIONINFO'] =  Substitutions ($eiwrapper, $arr);
		
	} else {
		// THIS IS GENERALLY NOT USED (A CONFIG SETTING ALLOWS IT)
		// PROBABLY DOESN'T WORK!
		$r = FetchSnippet ("frameshop_edition_info");
		foreach ($allPrintsizes as $ps) {
			if ($alleditionsizes[$k]+0) {
				$es = $alleditionsizes[$k];
				$allavailable[$k] > 0 ? $av = ", ".$allavailable[$k]." for sale" : $av = ", SOLD OUT";
			} else {
				 $es = "unlimited";
				 $av = "";
			}
			
			$editionInfo .= Substitutions ($r, array (
				"PRINTSIZE"	=> $ps,
				"PRICE"		=> $allprices[$k],
				"AVAILABLE"	=> $av,
				"EDITIONSIZE"	=> $es
				));
			$k++;
		}
		$fields['EDITIONINFO'] = Substitutions (FetchSnippet ("frameshop_edition_info_box"), array ("EDITIONINFO" => $editionInfo));
	}
	
	// Message: delay due to test print
	if ($totalsold < 1 && $project['Matchprint']) {
		$delaymsg = FetchSnippet("frameshop_matchprint_delay_message");
	} else {
		$delaymsg = "";
	}
	
	// ----- DEBUG -----
	if ($DEBUG) {
		
		$msg .= __FILE__.":".__LINE__.":<BR>";
		print_r ($unframed);
	}
	// -----
	
	
	// page to return to after PayPal purchase is gallery we just came from AND CLEAR THE CACHE!
	//$returnURL = urlencode("http://".$SYSTEMURL."gallery.php?ProjectID=".$projectID);
	if ($vars['FP_PREVIOUS_URL']) {
		$returnURL = $vars['FP_PREVIOUS_URL']."&clearcache=1";
	} else {
		$returnURL = "http://".$SYSTEMURL."gallery.php?ProjectID=".$projectID."&clearcache=1";
	}
	
	// DO WE NEED TO ENCODE THE '&'????
	if ($vars['FP_CURRENT_IMAGE_INDEX'])
		$returnURL .= "&FP_CURRENT_IMAGE_INDEX=".($vars['FP_CURRENT_IMAGE_INDEX'] + 0);
	
	// Frameshop picture
	$picture = Substitutions (FetchSnippet ("frameshop_picture"), $image);
	$picture = Substitutions ($picture, $project);
	$picture = Substitutions ($picture, $mainfields);
	$picture = Substitutions ($picture, $fields);
	
	// CONTROLS AND PRINT INFO
	$controls = FetchSnippet ("frameshop_controls");
	// image priceset settings may override
	$image['pricesetID'] || $image['pricesetID'] = $salesinfo['pricesetID'];
	$controls = Substitutions ($controls, $image);
	$controls = Substitutions ($controls, $project);
	$controls = Substitutions ($controls, $mainfields);
	$controls = Substitutions ($controls, $fields);
	
	// Get all pricing and costs
	//$cartjsvars = array (
	//);
	//$cartjsvars = ArrayToJavascriptVars($cartjsvars, "FP_CART_");
	
	$x = GetAllCartPricingByID ($imageID, $supplierID, $pricesetID);
	//var_dump ($x);
	//$cart_vars_js = ArrayToJavascriptVars($x, "FP_CART_");
	
	// Supplier address is ship-from location. This must FOLLOW above code to fill it out properly.
	
	$page = FetchSnippet ("master_page");
	$header = FetchSnippet ("master_page_header");
	
	$sampleImage = "";
	$bkgd = "";
	
	$page = Substitutions ($page, array(
		'list'		=> $picture
		));
	
	// Load JS shipping calculator
	$extra_libraries = "<script src=\"{JAVASCRIPT_PATH}/dig.upsshipping.js\" type=\"text/javascript\" language=\"javascript\"></script>\n";

	}
	
//$navbar = FetchSnippet ('navbar-goback');	// we use of limited navbar on this page
$navbar2 = NavBar ($groupID, '', 2);	// 1 means navbar item 1 is selected, 2 is bottom navbar
$pagevars = array_merge($pagevars, $artistinfo, array(
	'META_INDEX'				=> FetchSnippet ('meta_robots_noindex'),
	'EXTRA_JS_LIBRARIES'		=> $extra_libraries,
	'NAVBAR'					=> '',
	'GOBACKNAV'				=> FetchSnippet ('navbar-goback'),
	'NAVBAR_2'				=> $navbar2,
	'header'					=> $header,
	'pagetitle' 				=> FRAMESHOP_TITLE,
	'title' 					=> FRAMESHOP_TITLE,
	'subtitle'				=> FP_SYSTEM_DISPLAY_NAME,
	'ProjectID'				=> $projectID,
	'grouptitle'				=> $myGroup->title,
	'grouplogo'				=> $myGroup->icon,
	'BACKGROUND_IMG_STYLE'		=> $bkgd,
	'GROUPICON'				=> $GroupIcon,
	'GROUPBANNER'			=> $GroupBanner,
	'GROUPBANNERURL'			=> $GroupBannerURL,
	'sectionclass'			=> "frameshop",	//class name for some objects
	'message'	 			=> $msg,
	'error' 					=> $error,
	'pps' 					=> $project['pps'],
	'slideshowpopup'			=> $sspop,
	'master_page_popups'		=> FetchSnippet ("master_page_popups"),
	'frameshop_controls'		=> $controls,
	'SUPPLIERINFO'			=> $supplierinfo,
	'DELAY_MESSAGE'			=> $delaymsg,
	'ImageID'			=> $imageID,
	'RETURNURL'			=> $returnURL,
	'RETURNURL_ENCODED'		=> urlencode($returnURL),
	'FP_PREVIOUS_URL'		=> $returnURL,	// don't let ReplaceSysVars do this, we want to clear cache
	'SHIPPING_ENTRY_FORM'		=> FetchSnippet(FP_SHIPPER_CODE."_entry_form"),
	'SUPPLIER_COUNTRY'		=> $supplier['Country'],
	'SUPPLIER_ZIP'			=> $supplier['Zip'],
	'SUPPLIER_STATE'		=> $supplier['State'],
	'SUPPLIER_TAX_RATE'		=> $supplier['SalesTaxRate'],
	'MALS_SD'			=> "",
	"ARTWORK_NAME"			=> $artworkname,
	"EDITION_TYPE"			=> $editionType,
	"AVAILABLE"				=> $available,
	"FP_MAX_FRAMED_SIZE"	=> $maxFramedSize,
	"FP_NOT_AVAILABLE"		=> "{fp:notAvailable}",
	"google_analytics_js"		=> "<!-- No Google Analytics Tracking -->"

));

$page = Substitutions ($page, $pagevars);
$page = Substitutions ($page, $artistinfo);

$page = ReplaceAllSnippets ($page);
$page = ReplaceSysVars ($page);
$page = DeleteUnusedSnippets ($page);

print $page;

mysqli_close($LINK);
//$FP_MYSQL_LINK->close();


?>