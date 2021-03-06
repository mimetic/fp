-- phpMyAdmin SQL Dump
-- version 4.4.10
-- http://www.phpmyadmin.net
--
-- Host: localhost:3306
-- Generation Time: May 23, 2016 at 07:37 AM
-- Server version: 5.5.42
-- PHP Version: 7.0.0

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;

--
-- Database: `dgross_fp`
--

-- --------------------------------------------------------

--
-- Table structure for table `Artists`
--

DROP TABLE IF EXISTS `Artists`;
CREATE TABLE `Artists` (
  `ID` bigint(20) unsigned NOT NULL,
  `Commission` int(2) DEFAULT NULL,
  `Commission2` int(2) DEFAULT '0',
  `Username` varchar(64) DEFAULT NULL,
  `Password` varchar(64) DEFAULT NULL,
  `AccessLevel` int(3) DEFAULT NULL COMMENT '1=Admin,2=gallerist,3=personal,4=supplier',
  `UserLevel` int(2) DEFAULT '2' COMMENT '1=FP_SINGLE_GALLERY_SINGLE_USER 2=FP_SINGLE_GALLERY_MULTI_USER 3=FP_MULTI_GALLERY_SINGLE_USER 4=FP_MULTI_GALLERY_MULTI_USER ',
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
  `Params` text
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `Comments`
--

DROP TABLE IF EXISTS `Comments`;
CREATE TABLE `Comments` (
  `ID` bigint(20) unsigned NOT NULL,
  `ImageID` bigint(20) unsigned DEFAULT NULL,
  `Comment` varchar(255) DEFAULT NULL,
  `IP` varchar(96) DEFAULT NULL,
  `ParentTopicID` bigint(20) unsigned DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `Groups`
--

DROP TABLE IF EXISTS `Groups`;
CREATE TABLE `Groups` (
  `ID` bigint(20) unsigned NOT NULL,
  `Title` varchar(64) DEFAULT NULL,
  `Description` text,
  `ArtistID` bigint(20) DEFAULT NULL,
  `Public` tinyint(1) DEFAULT '0',
  `Icon` varchar(64) DEFAULT NULL,
  `URL` varchar(64) DEFAULT NULL,
  `Statement` text,
  `Theme` text,
  `Params` text
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `Images`
--

DROP TABLE IF EXISTS `Images`;
CREATE TABLE `Images` (
  `ID` bigint(20) unsigned NOT NULL,
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
  `Params` text
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `Keywords`
--

DROP TABLE IF EXISTS `Keywords`;
CREATE TABLE `Keywords` (
  `ID` bigint(20) unsigned NOT NULL,
  `Keyword` varchar(255) DEFAULT NULL,
  `ImageID` bigint(20) unsigned DEFAULT '0',
  `ParentTopicID` bigint(20) unsigned DEFAULT NULL,
  `Description` varchar(255) DEFAULT NULL,
  `Views` int(11) DEFAULT '0',
  `Rating` float DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `Parts`
--

DROP TABLE IF EXISTS `Parts`;
CREATE TABLE `Parts` (
  `ID` bigint(20) unsigned NOT NULL,
  `ProjectID` bigint(20) unsigned DEFAULT '0',
  `ArtistID` bigint(20) unsigned DEFAULT '0',
  `PartTable` varchar(32) DEFAULT 'Artists',
  `PartID` bigint(20) unsigned DEFAULT '0',
  `OrderInGallery` int(2) unsigned DEFAULT NULL,
  `OrderInProject` int(2) unsigned DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `Payments`
--

DROP TABLE IF EXISTS `Payments`;
CREATE TABLE `Payments` (
  `ID` bigint(20) NOT NULL,
  `Amount` decimal(9,2) DEFAULT NULL,
  `Email` varchar(60) DEFAULT NULL,
  `UniqueID` varchar(30) DEFAULT NULL,
  `Note` text,
  `SaleID` bigint(20) DEFAULT NULL,
  `Payee` varchar(50) DEFAULT NULL,
  `DateTime` datetime DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `Paypal`
--

DROP TABLE IF EXISTS `Paypal`;
CREATE TABLE `Paypal` (
  `id` int(11) NOT NULL,
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
  `amount` float DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `PriceSets`
--

DROP TABLE IF EXISTS `PriceSets`;
CREATE TABLE `PriceSets` (
  `ID` bigint(20) NOT NULL,
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
  `Params` text
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `Projects`
--

DROP TABLE IF EXISTS `Projects`;
CREATE TABLE `Projects` (
  `ID` bigint(20) unsigned NOT NULL,
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
  `client_list` text
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `Ratings`
--

DROP TABLE IF EXISTS `Ratings`;
CREATE TABLE `Ratings` (
  `ID` bigint(20) unsigned NOT NULL DEFAULT '0',
  `SetID` bigint(20) unsigned DEFAULT '0',
  `Rating` int(11) DEFAULT NULL,
  `IP` varchar(96) DEFAULT NULL,
  `RateTime` int(11) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `Sales`
--

DROP TABLE IF EXISTS `Sales`;
CREATE TABLE `Sales` (
  `id` int(11) NOT NULL,
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
  `payer_id` varchar(13) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `Sets`
--

DROP TABLE IF EXISTS `Sets`;
CREATE TABLE `Sets` (
  `ID` bigint(20) unsigned NOT NULL,
  `Description` varchar(255) DEFAULT NULL,
  `Active` set('yes','no') DEFAULT 'yes',
  `Featured` set('yes','no') DEFAULT 'no'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `Stories`
--

DROP TABLE IF EXISTS `Stories`;
CREATE TABLE `Stories` (
  `ID` bigint(20) unsigned NOT NULL,
  `ArtistID` bigint(20) unsigned DEFAULT '0',
  `ProjectID` bigint(20) unsigned DEFAULT '0',
  `Title` varchar(255) DEFAULT NULL,
  `Filename` varchar(64) DEFAULT NULL,
  `Story` text
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `Suppliers`
--

DROP TABLE IF EXISTS `Suppliers`;
CREATE TABLE `Suppliers` (
  `ID` bigint(20) NOT NULL,
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
  `Params` text NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `Topics`
--

DROP TABLE IF EXISTS `Topics`;
CREATE TABLE `Topics` (
  `ID` bigint(20) unsigned NOT NULL,
  `Description` varchar(255) DEFAULT NULL,
  `City` varchar(64) DEFAULT NULL,
  `Country` varchar(64) DEFAULT NULL,
  `Views` int(11) DEFAULT '0',
  `Rating` float DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Indexes for dumped tables
--

--
-- Indexes for table `Artists`
--
ALTER TABLE `Artists`
  ADD PRIMARY KEY (`ID`);

--
-- Indexes for table `Comments`
--
ALTER TABLE `Comments`
  ADD PRIMARY KEY (`ID`);

--
-- Indexes for table `Groups`
--
ALTER TABLE `Groups`
  ADD PRIMARY KEY (`ID`);

--
-- Indexes for table `Images`
--
ALTER TABLE `Images`
  ADD PRIMARY KEY (`ID`);

--
-- Indexes for table `Keywords`
--
ALTER TABLE `Keywords`
  ADD PRIMARY KEY (`ID`);

--
-- Indexes for table `Parts`
--
ALTER TABLE `Parts`
  ADD PRIMARY KEY (`ID`),
  ADD KEY `OrderInProject` (`OrderInProject`),
  ADD KEY `ProjectID` (`ProjectID`),
  ADD KEY `PartID` (`PartID`),
  ADD KEY `OrderInGallery` (`OrderInGallery`);

--
-- Indexes for table `Payments`
--
ALTER TABLE `Payments`
  ADD PRIMARY KEY (`ID`),
  ADD KEY `SaleID` (`SaleID`);

--
-- Indexes for table `Paypal`
--
ALTER TABLE `Paypal`
  ADD PRIMARY KEY (`id`),
  ADD UNIQUE KEY `txn_id` (`txn_id`),
  ADD KEY `item_number` (`item_number`);

--
-- Indexes for table `PriceSets`
--
ALTER TABLE `PriceSets`
  ADD PRIMARY KEY (`ID`);

--
-- Indexes for table `Projects`
--
ALTER TABLE `Projects`
  ADD PRIMARY KEY (`ID`);

--
-- Indexes for table `Ratings`
--
ALTER TABLE `Ratings`
  ADD PRIMARY KEY (`ID`);

--
-- Indexes for table `Sales`
--
ALTER TABLE `Sales`
  ADD PRIMARY KEY (`id`),
  ADD KEY `item_number` (`item_number`),
  ADD KEY `txn_id` (`txn_id`);

--
-- Indexes for table `Sets`
--
ALTER TABLE `Sets`
  ADD PRIMARY KEY (`ID`);

--
-- Indexes for table `Stories`
--
ALTER TABLE `Stories`
  ADD PRIMARY KEY (`ID`);

--
-- Indexes for table `Suppliers`
--
ALTER TABLE `Suppliers`
  ADD UNIQUE KEY `ID` (`ID`);

--
-- Indexes for table `Topics`
--
ALTER TABLE `Topics`
  ADD PRIMARY KEY (`ID`);

--
-- AUTO_INCREMENT for dumped tables
--

--
-- AUTO_INCREMENT for table `Artists`
--
ALTER TABLE `Artists`
  MODIFY `ID` bigint(20) unsigned NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `Comments`
--
ALTER TABLE `Comments`
  MODIFY `ID` bigint(20) unsigned NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `Groups`
--
ALTER TABLE `Groups`
  MODIFY `ID` bigint(20) unsigned NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `Images`
--
ALTER TABLE `Images`
  MODIFY `ID` bigint(20) unsigned NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `Keywords`
--
ALTER TABLE `Keywords`
  MODIFY `ID` bigint(20) unsigned NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `Parts`
--
ALTER TABLE `Parts`
  MODIFY `ID` bigint(20) unsigned NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `Payments`
--
ALTER TABLE `Payments`
  MODIFY `ID` bigint(20) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `Paypal`
--
ALTER TABLE `Paypal`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `PriceSets`
--
ALTER TABLE `PriceSets`
  MODIFY `ID` bigint(20) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `Projects`
--
ALTER TABLE `Projects`
  MODIFY `ID` bigint(20) unsigned NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `Sales`
--
ALTER TABLE `Sales`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `Sets`
--
ALTER TABLE `Sets`
  MODIFY `ID` bigint(20) unsigned NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `Stories`
--
ALTER TABLE `Stories`
  MODIFY `ID` bigint(20) unsigned NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `Suppliers`
--
ALTER TABLE `Suppliers`
  MODIFY `ID` bigint(20) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `Topics`
--
ALTER TABLE `Topics`
  MODIFY `ID` bigint(20) unsigned NOT NULL AUTO_INCREMENT;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
