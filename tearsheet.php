<?php

// This shows a single picture with information about that picture.
// It's for making tear-sheets.


include "_config/sysconfig.inc";
include "_config/fpconfig.inc";
include "_config/config.inc";
include "includes/functions.inc";
include "includes/project_management.inc";
include "includes/image_management.inc";
include "includes/commerce.inc";

$error = "";
$msg = "";

$LINK = StartDatabase(MYSQLDB);
Setup ();

session_name("fp_gallery_session");
session_start();

isset($_REQUEST['GroupID']) && $_SESSION['GroupID'] = $_REQUEST['GroupID'];
isset($_SESSION['GroupID']) || $_SESSION['GroupID'] = PUBLIC_GROUP_ID;

// isset($_REQUEST['theme']) && $_SESSION['theme'] = $_REQUEST['theme'];
isset($_SESSION['theme']) || $_SESSION['theme'] = DEFAULT_THEME;

$groupID = $_SESSION['GroupID'];
$myGroup = new FPGroup ($groupID);

$GroupBannerURL = $myGroup->LogoFilename();
$GroupBanner = $myGroup->LogoHTML("style='border:1px solid black;margin-right:10px;'");
$GroupIcon = $myGroup->IconHTML();

$myTheme = CurrentThemeID ();

$results = GetFormInput();

$actions = $results['actions'];
$action = $results['actions']['action'];
$vars = $results['vars'];
	
// Get user info, if it exists. It would come because the admin passed a user ID variable
$vars['fp_user'] && $_SESSION['fp_user'] = $vars['fp_user'];

$imageID = $results['vars']['ImageID'];
$image = FetchImage ($imageID);
$image['Date'] = strftime("%B %e, %G", mysqldate_to_timestamp($image['CreatedDate']));
$image["Place"] = ($image['City'] and $image['Country']) ? $image['City'].", ".$image['Country'] : $image['Country'];
$image["Artist"] = join (" ", StripBlankFields (array ($artist["Firstname"] ,  $artist['Lastname'])));
$image['FullCredit'] = join (" : ", StripBlankFields (array ($image["Place"], $image['Date'], $image["Artist"])));

$fields['ITEM_NAME'] = htmlspecialchars($image['Title'], ENT_QUOTES);


$artistID = $image['ArtistID'];
$artist = FetchArtist ($artistID);
$artist || $artist = array();

// rename keys to avoid possible bad substitutions
while (list ($k,$v) = each ($artist)) {
	$artistinfo[DB_ARTISTS."_$k"] = $v;
}


$projectID = $_REQUEST['ProjectID'];
$project = FetchProject ($projectID);

// Get sales settings
$salesinfo = ImageSalesInfo ($imageID, $projectID);
$printsizes = $salesinfo['printsizes'];
$editionsizes = $salesinfo['editionsizes'];
$amounts = $salesinfo['amount'];	// will only appear if there's a dedicated PriceSet entry
$prices = $salesinfo['PrintPrices'];
$pricesframed = $salesinfo['FrameMattePrices'];
$sizes = $salesinfo['size'];
$pricesetID = $salesinfo['pricesetID'];
$dims = $salesinfo['dims'];
$rows = $salesinfo['rows'];

$sizesToShow = $printsizes;

// NOTE that the index for sizes starts at "0" not "1"

// Get matte/frame/glazing info for the selected supplier. We're going to have to resolve this
// "vendor" vs. "supplier" name, dammit. Same thing, but confusing!
// $supplierID = $artist['Vendor'];	<-- wrong...artist could change vendors, but an image stays with original printer
$supplierID = $salesinfo['supplierID'];

$supplier = GetRecord( DB_SUPPLIERS, $supplierID);

$framestyles = array_merge (array(0=>"No Frame"), explode ("\n", $supplier['Frames']));
$framecodes = array_merge (array(0=>""), explode ("\n", $supplier['FrameCodes']));
$supplierHasFrames = trim($supplier['FrameCodes']);

$mattenames = array_merge (array(0=>"No Matte"), explode ("\n", $supplier['Mattes']));
$mattecolors = array_merge (array(0=>"No Matte"), explode ("\n", $supplier['MatteColors']));
$mattecodes = array_merge (array(0=>"NA"), explode ("\n", $supplier['MatteCodes']));
$supplierHasMattes = trim($supplier['MatteColors']);

$papers = explode ("\n", $supplier['Papers']);
$papercodes = explode ("\n", $supplier['PaperCodes']);

$inksets = explode ("\n", $supplier['Inksets']);
$inksetcodes = explode ("\n", $supplier['InksetCodes']);

$glazing = explode ("\n", $supplier['Glazing']);
$glazingcodes = explode ("\n", $supplier['GlazingCodes']);
$supplierHasGlazing = trim($supplier['GlazingCodes']);

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

$fields['FRAMES'] = join (",", $framestyles );
$fields['FRAMEWIDTHS'] = join (",", $FRAMESTYLESWIDTHS);
$fields['FRAMECODES'] = join(",",$framecodes);

$fields['PAPERS'] = join(",",$papers);
$fields['PAPERCODES'] = join(",",$papercodes);

$fields['INKSETS'] = join(",",$inksets);
$fields['INKSETCODES'] = join(",",$inksetcodes);

$fields['GLAZING'] = join(",",$glazing);
$fields['GLAZINGCODES'] = join(",",$glazingcodes);

// Javascript for changing values
$popupjs = FetchSnippet ("tearsheet_popupjs");



// -------
// Picture sizing, matte sizing

$URL = $image["URL"];
// What size pictures?
$WHICHPIX = $SLIDES;
$fields['WHICHPIX'] = $WHICHPIX;
$fields['URL'] = $URL;

isset ($_REQUEST['currentsize']) && $currentsize = $_REQUEST['currentsize'];
$currentsize || $currentsize = $sizes[count($sizes)-1];
$mainfields['SizesList'] = OptionListFromArray ($sizesToShow, "currentsize", $currentsize, false, OPTION_LIST_IS_POPUP, $popupjs." ID=\"currentsize\"");



// WALL COLOR: allow user to choose the color of the wall (#FFFFFF is white)
$mainfields['WallColorList'] = OptionListFromArray ($WALLCOLORS, "wallcolor", "#FFFFFF", false, OPTION_LIST_IS_POPUP,$popupjs." ID=\"wallcolor\"");

$matted = ($currentmatte > 0);	//currentmatte == 0 means no matte.


// PAPER
if ( FP_CUSTOMER_CHOOSES_PAPER ) {
	$currentpaper = $project['PaperCode'];
	isset ($_REQUEST['currentpaper']) && $currentpaper = $_REQUEST['currentpaper'];
	$mainfields['PaperList'] = OptionListFromArray ($papers, "currentpaper", $currentpaper, false, OPTION_LIST_IS_POPUP,$popupjs." ID=\"currentpaper\"");
} else {
	$currentpaper = $project['PaperCode'];
	empty($currentpaper) && $currentpaper = 0;
	$currentpapername = $papers[$currentpaper];
	$mainfields['PaperList'] = $currentpapername . "<input type=\"hidden\" id=\"currentpaper\" value=\"$currentpaper\">";
}


// INK
if ( FP_CUSTOMER_CHOOSES_INKSET ) {
	$currentinkset = $project['InksetCode'];
	isset ($_REQUEST['currentinkset']) && $currentinkset = $_REQUEST['currentinkset'];
	$mainfields['InksetList'] = OptionListFromArray ($inksets, "currentinkset", $currentinkset, false, OPTION_LIST_IS_POPUP,$popupjs." ID=\"currentinkset\"");
} else {
	$currentinkset = $project['InksetCode'];
	$currentinkset || $currentinkset = 0;
	$currentinksetname = $inksets[$currentinkset];
	$mainfields['InksetList'] = $currentinksetname . "<input type=\"hidden\" id=\"currentinkset\" value=\"$currentinkset\">";
}


// CALC SCREEN SIZES
// Set sizes for screen display
// Draw with or without a matte
$imgsize = GetLocalImageSize("$BASEDIR/$WHICHPIX/$URL");
$w = $imgsize[0] * $FRAMESHOP_SIZE_ADJUSTMENT;
$h = $imgsize[1] * $FRAMESHOP_SIZE_ADJUSTMENT;

$fields['IMG_Width'] = $w;
$fields['IMG_Height'] = $h;

// Get prices for unframed, framed prints
if ($currentframe > 0) {		// FRAMED
	$unitPrice = $salesinfo['priceframed'][$currentsize];
} else {							// UNFRAMED
	$unitPrice = $salesinfo['PrintPrices'][$currentsize];
}

$fields['PAPERCODE'] = $currentpaper;
$fields['INKSETCODE'] = $currentinkset;
$fields['MAXPRINTSIZE'] = $maxprintsize;

$fields['PRINTERNAME'] = $supplier["Name"];


$fields['ROWS'] = join(",",$rows);

$fields['UNITPRICE'] = $unitPrice;

$fields['MAXPRINTSIZE'] = $maxprintsize;
$fields['MAXDIMS'] = join(",",$sizes);

$fields['FRAMEMATTEPRICELIST'] = join(",",$salesinfo['FrameMattePrices']);
$fields['PRINTPRICELIST'] = join(",",$salesinfo['PrintPrices']);


// CATALOG NUMBER USES IMAGE ID
$artistInitials = $artist["Firstname"][0] . $artist["Lastname"][0];
$catalogNum = $artistInitials . "-". str_pad($imageID, 6, "0", STR_PAD_LEFT);	//left-pad with zeros
$fields['CATALOGNUMBER'] = $catalogNum;


$salesinfo = ImageSalesInfo ($imageID, $projectID);

$fields['SIZES'] = join(",",$salesinfo['printsizes']);

$fields['DIMENSIONS'] = join(",",$salesinfo['dims']);

// SHIPPING
$shipping = explode(",", FetchSnippet ("frameshop_shipping_popup"));
$mainfields['ShippingList'] = OptionListFromArray ($shipping, "currentshipping", "0", false, OPTION_LIST_IS_POPUP, $popupjs." ID=\"currentshipping\"");

// MATCHPRINT REQUIRED
$fields['MATCHPRINT'] = $project['Matchprint'];

// SUPPLIER ID : WHICH SUPPLIER IS THE ARTIST USING?
$fields['SUPPLIERID'] = $salesinfo['supplierID'];

$invoice = date("Ymd-G:i:s");			// e.g. 20080214-15:16:08
$fields['INVOICE'] = $invoice;

// Full original offering of the entire edition
$allPriceSet = ImageSalesInfo ($imageID, '', true);
$allPrintsizes = $allPriceSet['printsizes'];
is_array($allPrintsizes) || $allPrintsizes = array ($allPrintsizes);
$allprices = $allPriceSet['PrintPrices'];
$allavailable = $allPriceSet['available'];
$alleditionsizes = $allPriceSet['editionsizes'];

$totaledition = $allPriceSet['totaledition'];
$totalsold = $allPriceSet['totalsold'];
if ($totaledition > 0) {
	$available = $totaledition - $totalsold;
} else {
	$available = "unlimited";
}

$k=0;
$editionInfo = "";
// Two versions of edition info:
// - based on total size of edition and total sales
// - based on edition sizes for each print size
if (FP_EDITION_CALC_METHOD == "total") {
	$totalav = 0;
	$totaleditionsize =0;
	$liwrapper = FetchSnippet ("frameshop_edition_limited_wrapper");
	$ulwrapper = FetchSnippet ("frameshop_edition_unlimited_wrapper");
	$item = FetchSnippet ("frameshop_edition_info_edition_item");
	$delim = FetchSnippet ("frameshop_edition_info_edition_delim");
	$ulrow = FetchSnippet ("frameshop_edition_info_totals");
	$lirow = FetchSnippet ("frameshop_edition_info_list");
	$eiwrapper = FetchSnippet ("frameshop_edition_info_totals_box");
	foreach ($allPrintsizes as $ps) {
		$av = 0;
		$es = 0;
		if ($totaledition > 0) {
			$es += $alleditionsizes[$k];
			$totaleditionsize += $alleditionsizes[$k];
			$allavailable[$k] > 0 && $totalav += $allavailable[$k];
			$liInfo[] = Substitutions ($item, array ("PRINTSIZE"		=> $ps,
													"PRICE"			=> $allprices[$k],
													"AVAILABLE"		=> $av,
													"EDITIONSIZE"	=> $es
													));
		} else {
			 $es = "unlimited";
			 $av = "(no limit)";
			 $r = $ulrow;
			$ulInfo[] = Substitutions ($item, array ("PRINTSIZE"		=> $ps,
													"PRICE"			=> $allprices[$k],
													"AVAILABLE"		=> $av,
													"EDITIONSIZE"	=> $es
													));
		}
		
		$k++;
	}
	$liInfo ? $liList = join ($delim, $liInfo) : $liList = "";
	$ulInfo ? $ulList = join ($delim, $ulInfo): $ulList = "";
	
	$liList || $liwrapper = "<!--no limited edition info-->";
	$ulList || $ulwrapper = "<!--no limited edition info-->";
	
	if ($available != 1) {
		$available || $available = "no";	// "0" becomes "no"
		$avmsg = str_replace("{AVAILABLE}", $available, FetchSnippet ("are_blank_prints"));
	} else {
		$avmsg = FetchSnippet ("is_one_print");
	}

	$liInfo =  Substitutions ($liwrapper, array ("list"				=> $liList, 
												"ARE_BLANK_PRINTS"	=> $avmsg,
												"EDITIONSIZE"		=> $totaledition
												));
	$ulInfo =  Substitutions ($ulwrapper, array ("list"=>$ulList));

	
	$arr = array (	"LIMITEDEDITIONINFO" => $liInfo,
					"UNLIMITEDEDITIONINFO" => $ulInfo
			);

	$fields['EDITIONINFO'] =  Substitutions ($eiwrapper, $arr);
} else {
	$r = FetchSnippet ("frameshop_edition_info");
	foreach ($allPrintsizes as $ps) {
		if ($alleditionsizes[$k]+0) {
			$es = $alleditionsizes[$k];
			$allavailable[$k] > 0 ? $av = ", ".$allavailable[$k]." for sale" : $av = ", SOLD OUT";
		} else {
			 $es = "unlimited";
			 $av = "";
		}
		
		$editionInfo .= Substitutions ($r, array (	"PRINTSIZE"		=> $ps,
													"PRICE"			=> $allprices[$k],
													"AVAILABLE"		=> $av,
													"EDITIONSIZE"	=> $es
											));
		$k++;
	}
	$fields['EDITIONINFO'] = Substitutions (FetchSnippet ("frameshop_edition_info_box"), array ("EDITIONINFO" => $editionInfo));
}

// page to return to is gallery we just came from
//$returnURL = urlencode("http://".$SYSTEMURL."gallery.php?ProjectID=".$projectID);
$returnURL = "http://".$SYSTEMURL."gallery.php?ProjectID=".$projectID;

$page = FetchSnippet ("tearsheet_master_page");

// Picture
$picture = Substitutions (FetchSnippet ("tearsheet_picture"), $image);
$picture = Substitutions ($picture, $project);
$picture = Substitutions ($picture, $mainfields);
$picture = Substitutions ($picture, $fields);

// Info
$text = Substitutions (FetchSnippet ("tearsheet_text"), $image);
$text = Substitutions ($text, $project);
$text = Substitutions ($text, $mainfields);
$text = Substitutions ($text, $fields);

$navbar = FetchSnippet ('navbar-goback');	// we use of limited navbar on this page

$page = Substitutions ($page, $mainfields);
$page = Substitutions ($page, array(
	'META_INDEX'					=> FetchSnippet ('meta_robots_noindex'),
	'NAVBAR'					=> $navbar,
	'NAVBAR_2'					=> $navbar2,
	'GALLERY_STYLESHEET'	=>		"",
	'header'					=> $header,
	'text'				=>	$text,
	'image'				=> $picture,
	'pagetitle' 		=> FRAMESHOP_TITLE,
	'imagetitle' 		=> $image['Title'],
	'title' 			=> FRAMESHOP_TITLE,
	'projectTitle' 		=> $project['Title'],
	'ProjectID'			=> $projectID,
	'grouptitle'		=>	$myGroup->title,
	'grouplogo'			=>	$myGroup->icon,
	'GROUPICON'			=> $GroupIcon,
	'GROUPBANNER'		=> $GroupBanner,
	'GROUPBANNERURL'		=> $GroupBannerURL,
	'sectionclass'		=>	"",	//class name for some objects
	'message'	 		=> $msg,
	'error' 			=> $error,
	'master_page_popups'	=> "",
	'pps' 				=> $project['pps'],
	'slideshowpopup'	=>	$sspop,
	'master_page_popups'=>	"",
	'ImageID'			=>	$imageID,
	'RETURNURL'			=>	$returnURL,
	'RETURNURL_ENCODED'		=> urlencode($returnURL)
));
$page = Substitutions ($page, $artistinfo);
$page = ReplaceAllSnippets ($page);
$page = ReplaceSysVars ($page);
$page = DeleteUnusedSnippets ($page);

print $page;

mysql_close($LINK);
$FP_MYSQL_LINK->close();
?>