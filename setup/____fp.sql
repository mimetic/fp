-- MySQL dump 10.13  Distrib 5.1.51, for apple-darwin10.3.0 (i386)
--
-- Host: localhost    Database: dgross_fp
-- ------------------------------------------------------
-- Server version	5.1.51

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `Artists`
--

DROP TABLE IF EXISTS `Artists`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `Artists` (
  `ID` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `Commission` int(2) DEFAULT NULL,
  `Commission2` int(2) DEFAULT '0',
  `Username` varchar(64) DEFAULT NULL,
  `Password` varchar(64) DEFAULT NULL,
  `AccessLevel` int(3) DEFAULT NULL,
  `UserLevel` int(2) DEFAULT '2',
  `Storage` int(11) DEFAULT NULL,
  `Password_Reminder` varchar(64) DEFAULT NULL,
  `Firstname` varchar(64) DEFAULT NULL,
  `Middlename` varchar(64) DEFAULT NULL,
  `Lastname` varchar(64) DEFAULT NULL,
  `Agency` varchar(64) DEFAULT NULL,
  `Tel1` varchar(32) DEFAULT NULL,
  `Tel2` varchar(32) DEFAULT NULL,
  `Tel3` varchar(32) DEFAULT NULL,
  `Tel4` varchar(32) DEFAULT NULL,
  `Address1` varchar(64) DEFAULT NULL,
  `Address2` varchar(64) DEFAULT NULL,
  `City` varchar(64) DEFAULT NULL,
  `State` varchar(64) DEFAULT NULL,
  `Zip` varchar(64) DEFAULT NULL,
  `Country` varchar(64) DEFAULT NULL,
  `Email` varchar(64) DEFAULT NULL,
  `Email2` varchar(64) DEFAULT NULL,
  `Website` varchar(64) DEFAULT NULL,
  `Biography` text,
  `Statement` text,
  `DefaultPriceID` bigint(20) unsigned DEFAULT NULL,
  `DefaultPriceSetID` bigint(20) DEFAULT '1',
  `DefaultEditionSize` int(11) DEFAULT NULL,
  `DefaultMatted` set('yes','no') DEFAULT NULL,
  `DefaultCopyrightNotice` varchar(255) DEFAULT NULL,
  `DefaultCredit` varchar(255) DEFAULT NULL,
  `DefaultLifespan` int(11) DEFAULT '14',
  `DefaultActiveLifespan` int(11) DEFAULT '30',
  `Active` set('yes','no') DEFAULT 'yes',
  `Featured` set('yes','no') DEFAULT 'yes',
  `Timestamp` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `PortraitImageID` bigint(20) unsigned DEFAULT NULL,
  `ProjectID` bigint(20) unsigned DEFAULT NULL,
  `OutsourceID1` varchar(50) DEFAULT NULL,
  `OutsourceID2` varchar(50) DEFAULT NULL,
  `OutsourceID3` varchar(50) DEFAULT NULL,
  `OutsourceID4` varchar(50) DEFAULT NULL,
  `OutsourceID5` varchar(50) DEFAULT NULL,
  `ftp_server` varchar(50) DEFAULT NULL,
  `ftp_directory` varchar(64) DEFAULT NULL,
  `ftp_user_name` varchar(50) DEFAULT NULL,
  `ftp_user_pass` varchar(50) DEFAULT NULL,
  `ftp_proj_is_dir` int(1) DEFAULT '1',
  `Confirmed` varchar(50) DEFAULT NULL,
  `SubscriptionID` int(11) DEFAULT NULL,
  `SubscriptionDesc` text,
  `ActivationHash` varchar(150) DEFAULT '',
  `CreationDate` datetime DEFAULT '0000-00-00 00:00:00',
  `ShortName` varchar(32) DEFAULT '',
  `PrintSaleCode` text,
  `PayPalBusiness` varchar(64) DEFAULT NULL,
  `PayPalBusiness2` varchar(64) DEFAULT NULL,
  `PrintSalesID` varchar(50) DEFAULT NULL,
  `GroupID` bigint(20) DEFAULT '1',
  `PictureFrameWidth` int(1) DEFAULT '12',
  `PictureFrameColor` varchar(7) DEFAULT '000000',
  `Vendor` int(3) DEFAULT '1',
  `Ecommerce` int(3) DEFAULT NULL,
  `NotesToVendor` text,
  `Awards` text,
  `FullBiography` text,
  `Exhibitions` text,
  `Publications` text,
  `Params` text,
  PRIMARY KEY (`ID`)
) ENGINE=MyISAM AUTO_INCREMENT=31 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `Comments`
--

DROP TABLE IF EXISTS `Comments`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `Comments` (
  `ID` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `ImageID` bigint(20) unsigned DEFAULT NULL,
  `Comment` varchar(255) DEFAULT NULL,
  `IP` varchar(96) DEFAULT NULL,
  `ParentTopicID` bigint(20) unsigned DEFAULT NULL,
  PRIMARY KEY (`ID`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `Groups`
--

DROP TABLE IF EXISTS `Groups`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `Groups` (
  `ID` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `Title` varchar(64) DEFAULT NULL,
  `Description` text,
  `ArtistID` bigint(20) DEFAULT NULL,
  `Public` tinyint(1) DEFAULT '0',
  `Icon` varchar(64) DEFAULT NULL,
  `URL` varchar(64) DEFAULT NULL,
  `Statement` text,
  `Theme` text,
  `Params` text,
  PRIMARY KEY (`ID`)
) ENGINE=MyISAM AUTO_INCREMENT=34 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `Images`
--

DROP TABLE IF EXISTS `Images`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `Images` (
  `ID` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `Title` varchar(255) DEFAULT 'Untitled',
  `Type` set('normal','sample','thumbnail') DEFAULT 'normal',
  `RollID` varchar(32) DEFAULT '0',
  `FrameID` varchar(32) DEFAULT '0',
  `ArtistID` bigint(20) unsigned DEFAULT '0',
  `SetID` bigint(20) unsigned DEFAULT '0',
  `ProjectID` bigint(20) unsigned DEFAULT '0',
  `PriceID` bigint(20) unsigned DEFAULT '0',
  `PriceSetID` bigint(20) DEFAULT NULL,
  `URL` varchar(255) DEFAULT NULL,
  `Status` set('hold','current','archive') DEFAULT 'current',
  `Lifespan` int(11) DEFAULT NULL,
  `Medium` varchar(64) DEFAULT NULL,
  `Delivery` int(11) DEFAULT NULL,
  `Caption` text,
  `Headline` varchar(255) DEFAULT NULL,
  `SpecialInstructions` varchar(255) DEFAULT NULL,
  `Byline` varchar(64) DEFAULT NULL,
  `BylineTitle` varchar(64) DEFAULT NULL,
  `Credit` varchar(64) DEFAULT NULL,
  `Source` varchar(64) DEFAULT NULL,
  `ObjectName` varchar(64) DEFAULT NULL,
  `CreatedDate` date DEFAULT NULL,
  `City` varchar(64) DEFAULT NULL,
  `State` varchar(64) DEFAULT NULL,
  `Country` varchar(64) DEFAULT NULL,
  `Copyrighted` set('yes','no') DEFAULT NULL,
  `CopyrightNotice` varchar(255) DEFAULT NULL,
  `Featured` set('yes','no') DEFAULT 'no',
  `Active` set('yes','no') DEFAULT 'yes',
  `Timestamp` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `IPTCSubjectCode` varchar(12) DEFAULT NULL,
  `Keywords` text,
  `editionsize1` int(5) DEFAULT NULL,
  `editionsize2` int(5) DEFAULT NULL,
  `editionsize3` int(5) DEFAULT NULL,
  `editionsize4` int(5) DEFAULT NULL,
  `editionsize5` int(5) DEFAULT NULL,
  `editionsize6` int(5) DEFAULT NULL,
  `amount1` int(5) DEFAULT '0',
  `amount2` int(5) DEFAULT '0',
  `amount3` int(5) DEFAULT '0',
  `amount4` int(5) DEFAULT '0',
  `amount5` int(5) DEFAULT '0',
  `amount6` int(5) DEFAULT '0',
  `size1` float DEFAULT NULL,
  `size2` float DEFAULT NULL,
  `size3` float DEFAULT NULL,
  `size4` float DEFAULT NULL,
  `size5` float DEFAULT NULL,
  `size6` float DEFAULT NULL,
  `EditionsLocked` tinyint(4) DEFAULT '0',
  `Params` text,
  PRIMARY KEY (`ID`)
) ENGINE=MyISAM AUTO_INCREMENT=1467 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `Keywords`
--

DROP TABLE IF EXISTS `Keywords`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `Keywords` (
  `ID` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `Keyword` varchar(255) DEFAULT NULL,
  `ImageID` bigint(20) unsigned DEFAULT '0',
  `ParentTopicID` bigint(20) unsigned DEFAULT NULL,
  `Description` varchar(255) DEFAULT NULL,
  `Views` int(11) DEFAULT '0',
  `Rating` float DEFAULT '0',
  PRIMARY KEY (`ID`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `Parts`
--

DROP TABLE IF EXISTS `Parts`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `Parts` (
  `ID` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `ProjectID` bigint(20) unsigned DEFAULT '0',
  `ArtistID` bigint(20) unsigned DEFAULT '0',
  `PartTable` varchar(32) DEFAULT 'Artists',
  `PartID` bigint(20) unsigned DEFAULT '0',
  `OrderInGallery` int(2) unsigned DEFAULT NULL,
  `OrderInProject` int(2) unsigned DEFAULT NULL,
  PRIMARY KEY (`ID`),
  KEY `OrderInProject` (`OrderInProject`),
  KEY `ProjectID` (`ProjectID`),
  KEY `PartID` (`PartID`),
  KEY `OrderInGallery` (`OrderInGallery`)
) ENGINE=MyISAM AUTO_INCREMENT=3824 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `Payments`
--

DROP TABLE IF EXISTS `Payments`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `Payments` (
  `ID` bigint(20) NOT NULL AUTO_INCREMENT,
  `Amount` decimal(9,2) DEFAULT NULL,
  `Email` varchar(60) DEFAULT NULL,
  `UniqueID` varchar(30) DEFAULT NULL,
  `Note` text,
  `SaleID` bigint(20) DEFAULT NULL,
  `Payee` varchar(50) DEFAULT NULL,
  `DateTime` datetime DEFAULT NULL,
  PRIMARY KEY (`ID`),
  KEY `SaleID` (`SaleID`)
) ENGINE=MyISAM AUTO_INCREMENT=149 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `Paypal`
--

DROP TABLE IF EXISTS `Paypal`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `Paypal` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `pp_comments` text,
  `pp_user_id` bigint(20) DEFAULT NULL,
  `pp_service_id` bigint(20) DEFAULT NULL,
  `pp_package_id` int(11) DEFAULT NULL,
  `pp_status` text,
  `address_city` varchar(255) DEFAULT '',
  `address_country` varchar(255) DEFAULT '',
  `address_name` varchar(255) DEFAULT '',
  `address_state` varchar(255) DEFAULT '',
  `address_status` varchar(255) DEFAULT '',
  `address_street` varchar(255) DEFAULT '',
  `address_zip` varchar(255) DEFAULT '',
  `amount1` varchar(127) DEFAULT '0',
  `amount2` varchar(127) DEFAULT '0',
  `amount3` varchar(127) DEFAULT '0',
  `business` varchar(127) DEFAULT NULL,
  `comment` varchar(255) DEFAULT NULL,
  `coupon_code` varchar(15) DEFAULT 'none',
  `custom` varchar(255) DEFAULT NULL,
  `exchange_rate` varchar(127) DEFAULT '0',
  `first_name` varchar(127) DEFAULT NULL,
  `init_pass` varchar(8) DEFAULT 'UNK',
  `invoice` varchar(127) DEFAULT '0',
  `IP` varchar(15) DEFAULT '000.000.000.000',
  `item_name` varchar(127) DEFAULT 'UNK',
  `item_number` varchar(127) DEFAULT '',
  `last_change` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `last_name` varchar(127) DEFAULT NULL,
  `mc_amount1` varchar(127) DEFAULT '0',
  `mc_amount2` varchar(127) DEFAULT '0',
  `mc_amount3` varchar(127) DEFAULT '0',
  `mc_currency` varchar(127) DEFAULT 'USD',
  `mc_fee` varchar(127) DEFAULT '0',
  `mc_gross` varchar(127) DEFAULT '0',
  `mc_handling` decimal(9,2) DEFAULT NULL,
  `mc_shipping` decimal(9,2) DEFAULT NULL,
  `memo` tinytext,
  `notify_version` varchar(127) DEFAULT NULL,
  `num_cart_items` int(2) DEFAULT NULL,
  `option_name1` varchar(60) DEFAULT NULL,
  `option_name2` varchar(60) DEFAULT NULL,
  `option_selection1` varchar(200) DEFAULT 'UNK',
  `option_selection2` varchar(200) DEFAULT 'UNK',
  `parent_txn_id` varchar(127) DEFAULT NULL,
  `password` varchar(127) DEFAULT '0',
  `payer_email` varchar(75) DEFAULT NULL,
  `payer_id` varchar(60) DEFAULT NULL,
  `payer_status` varchar(50) DEFAULT NULL,
  `payment_date` varchar(50) DEFAULT NULL,
  `payment_fee` varchar(127) DEFAULT '0',
  `payment_gross` varchar(127) DEFAULT '0',
  `payment_status` varchar(127) DEFAULT NULL,
  `payment_type` varchar(50) DEFAULT NULL,
  `pending_reason` varchar(255) DEFAULT '',
  `period1` varchar(127) DEFAULT 'UNK',
  `period2` varchar(127) DEFAULT 'UNK',
  `period3` varchar(127) DEFAULT 'UNK',
  `quantity` int(11) DEFAULT '0',
  `reason_code` varchar(127) DEFAULT NULL,
  `reattempt` varchar(127) DEFAULT '1',
  `receiver_email` varchar(127) DEFAULT NULL,
  `recur_times` varchar(127) DEFAULT '0',
  `recurring` varchar(127) DEFAULT '1',
  `retry_at` varchar(127) DEFAULT NULL,
  `settle_amount` varchar(127) DEFAULT '0',
  `settle_currency` varchar(127) DEFAULT 'USD',
  `shipping` decimal(9,2) DEFAULT NULL,
  `handling` float DEFAULT NULL,
  `discount_amount` float DEFAULT NULL,
  `discount_rate` float DEFAULT NULL,
  `weight` float DEFAULT NULL,
  `weight_unit` int(11) DEFAULT NULL,
  `on0` text,
  `on1` text,
  `os0` text,
  `os1` text,
  `newcolumn` int(11) DEFAULT NULL,
  `shipping_method` varchar(50) DEFAULT NULL,
  `subscr_date` varchar(127) DEFAULT '0',
  `subscr_effective` varchar(127) DEFAULT '0',
  `subscr_id` varchar(127) DEFAULT NULL,
  `tax` decimal(9,2) DEFAULT NULL,
  `txn_id` varchar(50) DEFAULT NULL,
  `txn_type` varchar(255) DEFAULT '',
  `username` varchar(127) DEFAULT '0',
  `verify_sign` varchar(127) DEFAULT NULL,
  `whm_name` varchar(127) DEFAULT 'UNK',
  `amount` float DEFAULT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `txn_id` (`txn_id`),
  KEY `item_number` (`item_number`)
) ENGINE=MyISAM AUTO_INCREMENT=745 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `PriceSets`
--

DROP TABLE IF EXISTS `PriceSets`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `PriceSets` (
  `ID` bigint(20) NOT NULL AUTO_INCREMENT,
  `SourceID` int(11) NOT NULL,
  `ArtistID` bigint(20) DEFAULT NULL,
  `ImageID` bigint(20) DEFAULT '0',
  `SupplierID` bigint(20) DEFAULT NULL,
  `TotalEditionSize` int(11) NOT NULL,
  `Title` varchar(50) DEFAULT NULL,
  `matchprintcost` float DEFAULT NULL,
  `MaxFramedSize` int(11) DEFAULT NULL,
  `Paper` varchar(100) DEFAULT NULL,
  `Inkset` varchar(100) DEFAULT NULL,
  `PaperCode` varchar(32) DEFAULT NULL,
  `InksetCode` varchar(32) DEFAULT NULL,
  `a_MatteCost` text,
  `a_MattePrice` text,
  `a_EditionSize` text,
  `a_PrintCost` text,
  `a_PrintPrice` text,
  `a_Markup` text,
  `a_FrameToPrintCost` text,
  `a_FrameToPrintPrice` text,
  `a_FrameMatteCost` text,
  `a_FrameMattePrice` text,
  `a_Size` text,
  `a_Inactive` text,
  `a_PrintShipWeight` text,
  `a_MatteShipWeight` text,
  `a_FrameToPrintShipWeight` text,
  `a_FrameMatteShipWeight` text,
  `a_Amount` text,
  `Inflation` int(11) DEFAULT NULL,
  `Params` text,
  PRIMARY KEY (`ID`)
) ENGINE=MyISAM AUTO_INCREMENT=106 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `Projects`
--

DROP TABLE IF EXISTS `Projects`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `Projects` (
  `ID` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `ArtistID` bigint(20) unsigned NOT NULL DEFAULT '0',
  `GroupID` bigint(20) unsigned DEFAULT '1',
  `OwnerAccessOnly` tinyint(1) DEFAULT '0',
  `Title` varchar(255) DEFAULT NULL,
  `ProjectDate` date DEFAULT NULL,
  `LastUpdate` date DEFAULT NULL,
  `Lifespan` int(4) unsigned DEFAULT '30',
  `ActiveLifespan` int(4) unsigned DEFAULT '30',
  `Statement` text,
  `FeaturedX` int(1) DEFAULT NULL,
  `ActiveX` int(1) DEFAULT NULL,
  `Description` text,
  `City` varchar(64) DEFAULT NULL,
  `Country` varchar(64) DEFAULT NULL,
  `Views` int(11) unsigned DEFAULT '0',
  `Rating` float DEFAULT '0',
  `Timestamp` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `Nickname` varchar(64) DEFAULT NULL,
  `Matted` int(1) unsigned DEFAULT '0',
  `MaxPix` int(2) unsigned DEFAULT '15',
  `ProjectPassword` varchar(64) DEFAULT NULL,
  `pps` varchar(50) DEFAULT 'default',
  `PriceSetID` bigint(20) DEFAULT '0',
  `RSS` varchar(80) DEFAULT NULL,
  `Framewidth` smallint(5) unsigned DEFAULT '0',
  `Framestyle` smallint(5) unsigned DEFAULT '1',
  `Public` tinyint(1) DEFAULT '0',
  `PaperCode` varchar(32) DEFAULT NULL,
  `InksetCode` varchar(32) DEFAULT NULL,
  `Matchprint` int(1) DEFAULT NULL,
  `Audio` varchar(64) DEFAULT NULL,
  `SlideShowDuration` int(11) DEFAULT NULL,
  `Params` text,
  `Slides` tinyint(1) DEFAULT '0',
  PRIMARY KEY (`ID`)
) ENGINE=MyISAM AUTO_INCREMENT=231 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `Ratings`
--

DROP TABLE IF EXISTS `Ratings`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `Ratings` (
  `ID` bigint(20) unsigned NOT NULL DEFAULT '0',
  `SetID` bigint(20) unsigned DEFAULT '0',
  `Rating` int(11) DEFAULT NULL,
  `IP` varchar(96) DEFAULT NULL,
  `RateTime` int(11) DEFAULT NULL,
  PRIMARY KEY (`ID`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `Sales`
--

DROP TABLE IF EXISTS `Sales`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `Sales` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `txn_id` varchar(127) DEFAULT NULL,
  `item_name` varchar(127) DEFAULT 'UNK',
  `item_number` varchar(127) DEFAULT '',
  `item_id` bigint(20) DEFAULT NULL,
  `Size` int(11) NOT NULL,
  `SupplierID` int(11) NOT NULL,
  `quantity` int(11) DEFAULT '0',
  `weight` decimal(5,2) DEFAULT '0.00',
  `weight_unit` varchar(3) DEFAULT 'lbs',
  `cost` float NOT NULL,
  `amount` float DEFAULT NULL,
  `mc_gross` decimal(9,2) DEFAULT NULL,
  `mc_handling` decimal(9,2) DEFAULT NULL,
  `mc_shipping` decimal(9,2) DEFAULT NULL,
  `mc_fee` decimal(9,2) DEFAULT NULL,
  `option_name1` varchar(60) DEFAULT NULL,
  `option_name2` varchar(60) DEFAULT NULL,
  `option_selection1` varchar(200) DEFAULT 'UNK',
  `option_selection2` varchar(200) DEFAULT 'UNK',
  `item_desc` text,
  `item_spec` text,
  `order_time` datetime DEFAULT NULL,
  `secret` varchar(50) DEFAULT NULL,
  `shipping_method` varchar(50) DEFAULT NULL,
  `tax` float NOT NULL,
  `MatchprintRequired` tinyint(4) DEFAULT NULL,
  `payment_date` datetime DEFAULT NULL,
  `PrintNumber` int(11) DEFAULT NULL,
  `discount_amount` float DEFAULT NULL,
  `discount_rate` float DEFAULT NULL,
  `invoice` varchar(127) DEFAULT NULL,
  `currency_code` varchar(3) DEFAULT NULL,
  `address_name` varchar(128) DEFAULT NULL,
  `address_state` varchar(2) DEFAULT NULL,
  `address_street` varchar(200) DEFAULT NULL,
  `address_zip` varchar(20) DEFAULT NULL,
  `address_city` varchar(40) DEFAULT NULL,
  `address_country` varchar(64) DEFAULT NULL,
  `address_country_code` varchar(2) DEFAULT NULL,
  `contact_phone` varchar(20) DEFAULT NULL,
  `first_name` varchar(64) DEFAULT NULL,
  `last_name` varchar(64) DEFAULT NULL,
  `payer_business_name` varchar(127) DEFAULT NULL,
  `payer_email` varchar(127) DEFAULT NULL,
  `payer_id` varchar(13) DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `item_number` (`item_number`),
  KEY `txn_id` (`txn_id`)
) ENGINE=MyISAM AUTO_INCREMENT=809 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `Sets`
--

DROP TABLE IF EXISTS `Sets`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `Sets` (
  `ID` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `Description` varchar(255) DEFAULT NULL,
  `Active` set('yes','no') DEFAULT 'yes',
  `Featured` set('yes','no') DEFAULT 'no',
  PRIMARY KEY (`ID`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `Stories`
--

DROP TABLE IF EXISTS `Stories`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `Stories` (
  `ID` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `ArtistID` bigint(20) unsigned DEFAULT '0',
  `ProjectID` bigint(20) unsigned DEFAULT '0',
  `Title` varchar(255) DEFAULT NULL,
  `Filename` varchar(64) DEFAULT NULL,
  `Story` text,
  PRIMARY KEY (`ID`)
) ENGINE=MyISAM AUTO_INCREMENT=8 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `Suppliers`
--

DROP TABLE IF EXISTS `Suppliers`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `Suppliers` (
  `ID` bigint(20) NOT NULL AUTO_INCREMENT,
  `ArtistID` bigint(20) DEFAULT NULL,
  `Name` varchar(64) DEFAULT NULL,
  `Firstname` varchar(127) DEFAULT NULL,
  `Middlename` varchar(50) DEFAULT NULL,
  `Lastname` varchar(50) DEFAULT NULL,
  `Email` varchar(127) DEFAULT NULL,
  `PayPalBusiness` varchar(64) DEFAULT NULL,
  `Contact` varchar(127) DEFAULT NULL,
  `EmailDelivery` varchar(127) DEFAULT NULL,
  `ftp_server` varchar(127) DEFAULT NULL,
  `ftp_user_name` varchar(50) DEFAULT NULL,
  `ftp_user_pass` varchar(50) DEFAULT NULL,
  `ftp_directory` varchar(127) DEFAULT NULL,
  `URL` varchar(127) DEFAULT NULL,
  `OrdersURL` varchar(127) DEFAULT NULL,
  `Papers` text,
  `Inksets` text,
  `Frames` text,
  `Mattes` text,
  `MatteColors` text,
  `Glazing` text,
  `PaperCodes` text,
  `InksetCodes` text,
  `FrameCodes` text,
  `MatteCodes` text,
  `GlazingCodes` text,
  `Tel1` varchar(32) DEFAULT NULL,
  `Tel2` varchar(32) DEFAULT NULL,
  `Fax` varchar(32) DEFAULT NULL,
  `Website` varchar(127) DEFAULT NULL,
  `Address1` varchar(64) DEFAULT NULL,
  `Address2` varchar(64) DEFAULT NULL,
  `City` varchar(50) DEFAULT NULL,
  `State` varchar(50) DEFAULT NULL,
  `Zip` varchar(18) DEFAULT NULL,
  `Country` varchar(50) DEFAULT NULL,
  `Description` text,
  `MatchPrintPrice` float NOT NULL,
  `PrintMinPrice` float DEFAULT NULL,
  `PrintAreaPrice` float DEFAULT NULL,
  `PrintHandling` float DEFAULT NULL,
  `PrintHandlingIntl` float DEFAULT NULL,
  `PrintPacking` float DEFAULT NULL,
  `PrintDepth` float DEFAULT NULL,
  `PrintWeight` float DEFAULT NULL,
  `MatteMinPrice` float DEFAULT NULL,
  `MatteAreaPrice` float DEFAULT NULL,
  `MatteHandling` float DEFAULT NULL,
  `MatteHandlingIntl` float DEFAULT NULL,
  `MattePacking` float DEFAULT NULL,
  `MatteDepth` float DEFAULT NULL,
  `MatteWeight` float DEFAULT NULL,
  `FrameMinPrice` float DEFAULT NULL,
  `FrameAreaPrice` float DEFAULT NULL,
  `FrameHandling` float DEFAULT NULL,
  `FrameHandlingIntl` float DEFAULT NULL,
  `FramePacking` float DEFAULT NULL,
  `FrameDepth` float DEFAULT NULL,
  `FrameWeight` float DEFAULT NULL,
  `SalesTaxRate` float DEFAULT NULL,
  `PrintCostUnit` int(11) DEFAULT NULL,
  `PrintCostMethod` int(11) DEFAULT NULL,
  `Params` text NOT NULL,
  UNIQUE KEY `ID` (`ID`)
) ENGINE=MyISAM AUTO_INCREMENT=3 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `Topics`
--

DROP TABLE IF EXISTS `Topics`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `Topics` (
  `ID` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `Description` varchar(255) DEFAULT NULL,
  `City` varchar(64) DEFAULT NULL,
  `Country` varchar(64) DEFAULT NULL,
  `Views` int(11) DEFAULT '0',
  `Rating` float DEFAULT '0',
  PRIMARY KEY (`ID`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2011-05-20 14:23:32
