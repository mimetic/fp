<?php
/*
 * An autoupdater for the FP system.
 * This PHP script is run by the index.php
 */
require_once "_config/sysconfig.inc";
require_once "_config/fpconfig.inc";
require_once "_config/config.inc";
require_once "includes/functions.inc";
require_once "includes/project_management.inc";

$LINK = StartDatabase(MYSQLDB);
Setup ();
/*
 *
 * UPDATER CODE HERE
 */

$updaterdescription = "Update 1/22/2010: add new mysql fields";
//-----------------


$query = "ALTER TABLE `PriceSets` ADD COLUMN `a_EditionSize` TEXT,
ADD COLUMN `a_PrintCost` TEXT,
ADD COLUMN `a_PrintPrice` TEXT,
ADD COLUMN `a_Markup` TEXT,
ADD COLUMN `a_MatteCost` TEXT,
ADD COLUMN `a_MattePrice` TEXT,
ADD COLUMN `a_FrameToPrintCost` TEXT,
ADD COLUMN `a_FrameToPrintPrice` TEXT,
ADD COLUMN `a_FrameMatteCost` TEXT,
ADD COLUMN `a_FrameMattePrice` TEXT,
ADD COLUMN `a_Inactive` TEXT,
ADD COLUMN `a_PrintShipWeight` TEXT,
ADD COLUMN `a_MatteShipWeight` TEXT,
ADD COLUMN `a_FrameToPrintShipWeight` TEXT,
ADD COLUMN `a_FrameMatteShipWeight` TEXT,
ADD COLUMN `a_Amount` TEXT;";

$result = mysql_query($query);

$query = "ALTER TABLE `Suppliers` 
ADD COLUMN `PrintMinPrice` FLOAT,
ADD COLUMN `PrintAreaPrice` FLOAT,
ADD COLUMN `PrintHandling` FLOAT,
ADD COLUMN `PrintHandlingIntl` FLOAT,
ADD COLUMN `PrintPacking` FLOAT,
ADD COLUMN `PrintDepth` FLOAT,
ADD COLUMN `PrintWeight` FLOAT,
ADD COLUMN `MatteMinPrice` FLOAT,
ADD COLUMN `MatteAreaPrice` FLOAT,
ADD COLUMN `MatteHandling` FLOAT,
ADD COLUMN `MatteHandlingIntl` FLOAT,
ADD COLUMN `MattePacking` FLOAT,
ADD COLUMN `MatteDepth` FLOAT,
ADD COLUMN `MatteWeight` FLOAT,
ADD COLUMN `FrameMinPrice` FLOAT,
ADD COLUMN `FrameAreaPrice` FLOAT,
ADD COLUMN `FrameHandling` FLOAT,
ADD COLUMN `FrameHandlingIntl` FLOAT,
ADD COLUMN `FramePacking` FLOAT,
ADD COLUMN `FrameDepth` FLOAT,
ADD COLUMN `FrameWeight` FLOAT,
ADD COLUMN `SalesTaxRate` FLOAT;";

$result = mysql_query($query);

$query = "ALTER TABLE  `Sales` ADD  `tax` FLOAT NOT NULL AFTER `shipping_method`;";
			
$result = mysql_query($query);

$query = "ALTER TABLE `Artists` ADD COLUMN `Commission2` int(2) DEFAULT 0 AFTER `Commission`,
ADD COLUMN `PayPalBusiness2` varchar(64) AFTER `PayPalBusiness`;";

$result = mysql_query($query);

$query = "ALTER TABLE `PriceSets`
DROP COLUMN `priceframed1`,
DROP COLUMN `priceframed2`,
DROP COLUMN `priceframed3`,
DROP COLUMN `priceframed4`,
DROP COLUMN `priceframed5`,
DROP COLUMN `priceframed6`,
DROP COLUMN `price1`,
DROP COLUMN `price2`,
DROP COLUMN `price3`,
DROP COLUMN `price4`,
DROP COLUMN `price5`,
DROP COLUMN `price6`,
DROP COLUMN `size1`,
DROP COLUMN `size2`,
DROP COLUMN `size3`,
DROP COLUMN `size4`,
DROP COLUMN `size5`,
DROP COLUMN `size6`,
DROP COLUMN `weight1`,
DROP COLUMN `weight2`,
DROP COLUMN `weight3`,
DROP COLUMN `weight4`,
DROP COLUMN `weight5`,
DROP COLUMN `weight6`,
DROP COLUMN `weightframed1`,
DROP COLUMN `weightframed2`,
DROP COLUMN `weightframed3`,
DROP COLUMN `weightframed4`,
DROP COLUMN `weightframed5`,
DROP COLUMN `weightframed6`,
DROP COLUMN `editionsize1`,
DROP COLUMN `editionsize2`,
DROP COLUMN `editionsize3`,
DROP COLUMN `editionsize4`,
DROP COLUMN `editionsize5`,
DROP COLUMN `editionsize6`,
DROP COLUMN `amount1`,
DROP COLUMN `amount2`,
DROP COLUMN `amount3`,
DROP COLUMN `amount4`,
DROP COLUMN `amount5`,
DROP COLUMN `amount6`,
DROP COLUMN `extrashipping1`,
DROP COLUMN `extrashipping2`,
DROP COLUMN `extrashipping3`,
DROP COLUMN `extrashipping4`,
DROP COLUMN `extrashipping5`,
DROP COLUMN `extrashipping6`,
DROP COLUMN `cost1`,
DROP COLUMN `cost2`,
DROP COLUMN `cost3`,
DROP COLUMN `cost4`,
DROP COLUMN `cost5`,
DROP COLUMN `cost6`,
DROP COLUMN `framecost1`,
DROP COLUMN `framecost2`,
DROP COLUMN `framecost3`,
DROP COLUMN `framecost4`,
DROP COLUMN `framecost5`,
DROP COLUMN `framecost6`
DROP COLUMN `pricematted1`,
DROP COLUMN `pricematted2`,
DROP COLUMN `pricematted3`,
DROP COLUMN `pricematted4`,
DROP COLUMN `pricematted5`,
DROP COLUMN `pricematted6`,
DROP COLUMN `mattecost1`,
DROP COLUMN `mattecost2`,
DROP COLUMN `mattecost3`,
DROP COLUMN `mattecost4`,
DROP COLUMN `mattecost5`,
DROP COLUMN `mattecost6`;";

$result = mysql_query($query);




//----------------- WRITE LOG -----------------
fp_error_log("AUTOUPDATER: $updaterdescription", 3, FP_MAINTENANCE_LOG);

/*
 * END UPDATER CODE
 */
mysql_close($LINK);

/*
 * RENAMER:
 * This part of the script renames the file after running, so it won't be run again.
 */
$BASEDIR = dirname(__FILE__);

if (!$DEVELOPING) {

	// legacy fix
	$x = "$BASEDIR/_user/_audio/slideshow/.htaccess";
	if (file_exists($x)) {
		unlink ($x);
		fp_error_log("Fix access to slide show audio files.", 3, FP_MAINTENANCE_LOG);
	}
	$f = __FILE__;
	$k = 1;
	$ff = $f;
	while (file_exists($ff)) {
		$ff = $f.".completed.".$k;
		$k++;
	}
	rename(__FILE__, $ff);
}


?>