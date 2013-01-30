<?php

// CONFIG FILE FOR FP PICTURE GALLERY SYSTEM
// User, interface, and local config settings
// This will look for my_config.inc, which can override values here.
// version 7/11/2009

// FP SYSTEM VARIABLES
// Used for naming the site, system functions

// *** NOTE, some are variables and some are constants. Why? Because I didn't know which was better.
// Since they never change, however, they should all be constants. They're not. So, eventually,
// the code should be rewritten to change them to constants. I think....

// ============================================================


//User password settings
$FP_PASSWORD_MIN_LENGTH = 8;
$FP_PASSWORD_MAX_LENGTH = 32;

// Seconds allowed for a user to confirm/activate his username/password for the system. 
// He gets an email and must reply in this amount of time.
// A user who doesn't confirm will be deleted, so we don't clutter up with non-users.
$MAXCONFIRMTIME = 24 * 3600 ;	// 24 hours

if ($MAXCONFIRMTIME < (47 * 3600)) {
	$FP_SHOW_MAXCONFIRMTIME = floor($MAXCONFIRMTIME / 3600) . " {fp:hours}";
	
	
} else {
	$FP_SHOW_MAXCONFIRMTIME = floor($MAXCONFIRMTIME / (24 * 3600)) . " {fp:days}";
	}

// Maximum number of pictures in a gallery
$MAX_PROJECT_PICTURES	=	80;

// *** NOT IN USE:
// Maximum number of days after last update that a gallery (not a group) remains featured
// NOTE: If you enter "0" here, all values are acceptable because "0" means infinite. 
// $FP_MAX_DAYS_FEATURED = 0;

// *** NOT IN USE:
// Maximum number of days after last update that a gallery (not a group) remains ACTIVE...after which it is deleted.
// NOTE: If you enter "0" here, all values are acceptable because "0" means infinite. 
$FP_MAX_DAYS_ACTIVE = 0;

// Show the "Public" checkbox in the admin (for projects, galleries, etc.)
// that allows something (like a project or gallery, such as the "public" gallery
// to be accessible by everyone.
$FP_SHOW_PUBLICPRIVATE_FOR_GROUPS = false;
$FP_SHOW_PUBLICPRIVATE_FOR_PROJECTS = true;

// Flag: Should a user get a new, personal gallery
$FP_USERS_GET_GALLERY = true;

// Names for database tables to show in Admin section, as in "Edit a record in {TABLENAME}"
$TABLE_ALIASES = array (	
			$IMAGES			=> "Pictures",
			$PRICES			=> "Prices",
			$PRICESETS		=> "Price Sets",
			$ARTISTS		=> "Photographers",
			$TOPICS			=> "TOPICS",
			$KEYWORDS		=> "KEYWORDS",
			$COMMENTS		=> "COMMENTS",
			$SETS			=> "SETS",
			$RATINGS			=> "RATINGS",
			$GROUPS			=> "Galleries",
			$PROJECTS		=> "Exhibitions",
			$PARTS			=> "PARTS",
			$STORIES			=> "Stories",
			$SUPPLIERS		=> "Suppliers",
			$PAYPAL			=> "PAYPAL",
			$SALES			=> "Sales",
			$PAYMENTS		=> "PAYMENTS"
	);

// Names for database tables to show in Admin section, as in "Edit a record in {TABLENAME}"
$TABLE_OBJECTS = array (	
			$IMAGES			=> "Picture",
			$PRICES			=> "Price",
			$PRICESETS		=> "Price Set",
			$ARTISTS		=> "Photographer",
			$TOPICS			=> "Topic",
			$KEYWORDS		=> "Keyword",
			$COMMENTS		=> "Comment",
			$SETS			=> "Set",
			$RATINGS			=> "Rating",
			$GROUPS			=> "Gallery",
			$PROJECTS		=> "Exhibition",
			$PARTS			=> "Part",
			$STORIES			=> "Story",
			$SUPPLIERS		=> "Supplier",
			$PAYPAL			=> "PAYPAL",
			$SALES			=> "Sale",
			$PAYMENTS		=> "PAYMENTS"
	);

//-------------------
// "Gallery of" naming
// This is the name of a default gallery for an artist.
//$default_gallery_name = "Gallery of {NAME}";
$default_gallery_name = "Portfolio of {NAME}";

// Should system hide the name of the project's owner in an exhibition,
// in a single-user system? In general, such systems use the owner's name
// as the name of the Group (the gallery), so it ends up being repeated
// at the top of the screen in most themes, and in the exhibition.
$FP_HIDE_NAME_SOLO_GALLERY = 0;

// For a theme which uses a fixed size background picture, should we resize the
// screen to fit it? This could be disturbing and annoying.
$FP_RESIZE_TO_BKGD_IMAGE = 0;

// ------------------
// Group settings:
$FP_MAX_GROUP_SIZE = 25;

$groups_display_cols	= 1;
$groups_items_per_page	= 6;
$groups_desc_len	=	100;	// num chars to get to show in listing of groups

// ------------------
// light table settings
$LIGHTTABLECOLS = 5;
// HTML option list of artists, projects, etc.
$FP_OPTION_LIST_MAXSIZE = 10;

// ImageTool can either be "gd" or "im" (ImageMagick) 
// ImageMagick does better quality scaling but may be 
// slower on some machines.  Take your pick
$FP_IMAGE_TOOL = "im";

// ImageMagick JPG quality setting (0-100, 100=best)
// 85 looks great. 80 looks OK. 75 is OK. Let's look great.
$FP_IMAGEMAGICK_QUALITY = 85;

// ImageMagick converter
if ($DEVELOPING) {
	$FP_IM_CONVERT = "/opt/local/bin/convert";
	$FP_IM_COMPOSITE = "/opt/local/bin/composite";
} else {
	$FP_IM_CONVERT = "/usr/local/bin/convert";
	$FP_IM_COMPOSITE = "/usr/local/bin/composite";
}


// COLOR SETTINGS

// Matting:
// We start with a final frame size, then reduce the picture to fit inside.
// These values determine how to resize the picture. 
// E.g. $mattescale = 0.75 reduces the picture by 75%, giving us a 25% matte
$mattescale		= 0.75;	// = 75% : multiplier to reduce an image to fit inside matte. Do not set to zero!
$mattebottom		= 0.00;	// = 5% : multiplier to find ADDITIONAL space added to the bottom of the matte (0 = none)

$FP_MATTECOLOR = "#FDFDFD";

// Amount to multiply an enlarged image in a gallery (when popped-up with jquery.lightbox.js
// 1.0 means no change; .98 is safe as a maximum.
// This setting reduces the space for a picture, but in proportion to the window. SO, it's not exactly the
// same as the padding setting that follows it.
$FP_LIGHTBOX_DISPLAY_REDUCTION = 0.98;
// Extra spacing around the entire picture. This is a minimum space around the picture.
$FP_LIGHTBOX_DISPLAY_PADDING = 50;

// Size of an icon in the gallery listing
$FP_GALLERY_ICON_WIDTH = 45;
$FP_GALLERY_ICON_HEIGHT = 30;

// Max size of a logo. The logo might be a banner, and very wide or tall!
$FP_GALLERY_LOGO_WIDTH = 1000;
$FP_GALLERY_LOGO_HEIGHT = 1000;

// Size of an icon at the top of the screen (in a title)
$FP_GALLERY_ICON_TITLE_WIDTH = 50;
$FP_GALLERY_ICON_TITLE_HEIGHT = 30;

// If there is no icon in the gallery listing, should we show a blank space
// or shift the title left?
$FP_SHOW_BLANK_GALLERY_ICON = false;

// Should we show gallery icons at all? Block them here:
$FP_SHOW_GALLERY_ICON = true;

// Break lines in captions in gallery, if there's a line break in the caption text?
$FP_CAPTION_SHOW_LINEBREAKS = true;


// Shadowing of pictures (using jquery.shadowTilt)
$FP_SHADOWS = true;

// Fade value when showing close-ups of pictures
// 0=BLACK, 100=no fade
$FP_GALLERYFADEDARKNESS = 0;

// SHOW SLIDESHOW LINKS IN GALLERY?
// Show the link for the postcard viewer slideshow?
$FP_SLIDESHOW_PC = true;
$FP_SLIDESHOW_PC_COLUMNS = 5;

// Show the link for the tilt viewer slideshow?
$FP_SLIDESHOW_TV = false;

// Show the link for the simpleviewer slideshow?
$FP_SLIDESHOW_SV = true;

$jpgquality		=	90;		//JPG setting for resizes

// Opacity of watermark on a picture (between 0=opaque and 127=transparent)
$FP_WATERMARK_OPACITY = 90;

// Size of font used for watermarking, for built-in PHP fonts. 1 is small, 5 is large
$FP_WATERMARK_FONT_SIZE = 12.0;
// Font directory for TrueType fonts
$FP_WATERMARK_FONT_DIR = "fonts";
// Font must be a .ttf font (TrueType) and must be in "fonts" folder
$FP_WATERMARK_FONT_NAME = "Vera.ttf";
// Should we watermark small pictures, as in the gallery = true
// or only the big pictures (slide show) = false
$FP_WATERMARK_GALLERY_PIX = false;

// Administration page of exhibitions, when displaying a project.
// When showing a grid of pictures, how many to show?
// Current setup fits 4 wide, so we should use numbers divisible by four.
$items_per_page	= 12;
$items_per_page_arr = array (12=>12,20=>20,28=>28,36=>36,9999=>"all");

// Columns in the grid in table-based slide shower (I'll probably dump this)
$display_cols	= 3;

// sizes are (max horizontal, max vertical) and depend on orientation of images
// Note: you may wish to set the TFU uploader config resize values to match $slide_size

// Matte sizing: pixel width of matte can be calculated, based on the Slide matte sizing
// to keep mattes proportional throughout the system, OR you can enter fixed values.
// IF there is not value for the mattes, then MATTESCALE is used. 
// If the width is too narrow, the height will not match across the pictures. So, if you have extra wide pix,
// it could look funny.

// * NOTE: slide sizing is dynamically resized based on screen size in Javascript.
// These sizes do affect the size created when a picture is imported into the system
$slide_size		= array (1500, 800);	// works well for 1900x1200 and larger
//$slide_size		= array (900, 600);	 // works well for most screens
//$slide_size		= array (1200, 700);	 // Georgie

// This value is the percent, e.g. .80 is 80%, of the height the 
// current window to resize a popup slide, and possibly other sizes, 
// such as gallery pictures.
// We use the slide size settings above to limit the maximum size, so we don't pixelate.
$FP_SLIDE_ADJUSTMENT = .80;
// pixel width of matte for a slide picture
$slide_matte_size	= ($slide_size[0] * (1-$FP_SLIDE_ADJUSTMENT)) / 2;

$default_size	= array (900, 300);

$default_matte_size	= ceil ($slide_matte_size * ($default_size[1] / $slide_size[1])) ;
//$default_matte_size	= 20;

$thumb_size		= array (200,100);
$thumb_border		= 0;
$default_border		= 0;	//black border around the picture

$PRINTDPI		= 150;	//acceptable minimum dpi for printing. We find that 100 dpi is OK for very large prints.

// ** Unused:
$PRINTSIZES		= array (10=>"8x10", 14=>"11x14", 20=>"16x20", 30=>"20x30", 36=>"24x36", 40=>"30x40");

// Whether to add a frame to a matted picture
$FP_FRAMED_MATTED_PIX = "true";

//-------------------
// ARTIST DEFAULTS

// default commission if the artist' commission isn't set.
// value is a percent, e.g. 20 is 20%
$FP_ARTIST_DEFAULT_COMMISSION = "20";
$FP_ARTIST_DEFAULT_USERLEVEL = FP_SINGLE_GALLERY_SINGLE_USER;
$FP_ARTIST_DEFAULT_ACCESSLEVEL = FP_PERSONAL_LEVEL;
// Default storage space in MB
// Note: 0 means unlimited up to the Admin user's allowed maximum
$FP_ARTIST_DEFAULT_STORAGE = 5;

// ------------------
// SLIDE SHOW SETTINGS
// Always end an album with a black slide. This is good for making sure there's a blank between shows.
$EndWidthBlackSlide = false;
// use absolute paths, meaning each XML image is encoded with an absolute HTTP path, instead of a relative. Might be slower? (true seems to break the slide shower, don't know why)
$SlideShowAbsPaths = false;
// true= show a flash slide show on the front page, false=show a random still picture.
$FrontPageSlideShow = true;

$FP_DEFAULT_SLIDESHOW_PAUSE = 6;

// This value is set in the SWF file that SSP creates. I don't know if you can alter it in the XML...
$FP_SSP_TRANSITION_LENGTH = 2;



// FRAMESHOP AND COMMERCE SETTINGS
// ------------------
// Tax can be on printing only, if the artist is exempt. This could be true for a "hobbyist" or
// someone who doesn't make most of their money from prints.
// An individual might choose to be tax exempt.
$FP_TAX_WHOLE_SALE = false;


// FRAMESHOP_PICTURE_SCALING controls the javascript in the frameshop
// which scales a picture when you change the print size.
// If it is not zero, the frameshop will scale the picture.
$FP_FRAMESHOP_PICTURE_SCALING = 0;


// Only Admin can see Artist's sales settings?
$FP_ADMIN_CONTROLS_ARTIST_SALES = false;


// Only Admin can set "Printer has file" settings if set to 1
$FP_ADMIN_CONTROLS_PRINT_FILE = false;

// Does everyone have access to the image metadata editor?
$FP_ALL_METADATAEDIT_ACCESS = true;

// Edition sales count
// There are two ways of calculating available prints:
// - Available in each print size, e.g. there are 5 more 8x10 prints available
// - Available out of the total size of the edition, e.g. 5 more of 100 prints available
$FP_EDITION_CALC_METHOD = "total";
// Really, this other method shouldn't be used. It might not work by now, because image edition sizing might not overrule it.
//$FP_EDITION_CALC_METHOD = "size";

// Do we allow a customer to buy a matted picture w/o a frame in the Print Shop?
$FP_SELL_MATTE_NO_FRAME = true;


$FP_CURRENCY = "&#36;";	// &#36; = "$" i.e. dollar
$FP_CURRENCY_NAME = "USD";
$FP_DECIMAL = ".";
$FP_THOUSANDS = ",";
$FP_CURRENCY_POSITION = "before";	// "before" or "after"
$FP_CURRENCY_AFTER = "USD";
$FP_UNIT = "inch";	// "inch" or "cm"
$FP_UNITS = "inches";
$FP_LABEL_INCHES = "in";
$FP_LABEL_CM = "cm";

// Text for "Customer Pickup". Used on order forms to suppliers, to indicate a customer will
// pick up the item and they should not ship it.
$FP_CUSTOMER_PICKUP_TEXT = "Customer Pickup";

// allowed values for PayPal are "lbs" or "kgs"
$FP_WEIGHT_UNIT = "lbs";

// Most of these don't work right now, only #5. 
// $ECOMMERCE = array (
// 	1	=>	"$SYSTEMNAME-Popup",
// 	2	=>	"Pictopia",
// 	3	=>	"PayPal",
// 	4	=>	"E-Junkie",
// 	5	=>	"$SYSTEMNAME-Frameshop"
// );

$ECOMMERCE = array (
	5	=>		FP_SYSTEM_DISPLAY_NAME . " {fp:Print Shop}"
);

// You could allow a customer to choose the paper/ink he wants. 
// I think that's an abomination, a sin against the artist, but hey,
// I had the programming, so here it is. Folks, leave these 'false'.
$FP_CUSTOMER_CHOOSES_PAPER = false;
$FP_CUSTOMER_CHOOSES_INKSET = false;

// Shopping Cart code snippet name
$FP_SHOW_CART_NAME = "showcart_paypal";
$FP_NAVBAR_SHOW_CART_NAME = "navbar_showcart_paypal";

// Email Headers for messages about payments, orders,etc.
$em_headers  = "From: " . SYSTEMNAME . " Payments <" . FP_COMMERCE_EMAIL . ">\n";		
$em_headers .= "Reply-To: " . FP_COMMERCE_EMAIL . "\n";
$em_headers .= "Return-Path: " . FP_COMMERCE_EMAIL . "\n";
$em_headers .= "Organization: " . SYSTEMNAME . "\n";
$em_headers .= "X-Priority: 3";

// Email Headers for messages about login, bugs, etc.
$em_headers_sys  = "From: " . SYSTEMNAME . " Gallery <" . FP_SYSTEM_EMAIL . ">\n";		
$em_headers_sys .= "Reply-To: " . FP_SYSTEM_EMAIL . "\n";
$em_headers_sys .= "Return-Path: " . FP_SYSTEM_EMAIL . "\n";
$em_headers_sys .= "Organization: " . SYSTEMNAME . "\n";
$em_headers_sys .= "X-Priority: 3";

// SALES PAGE SETTINGS
$FP_PAYPAL_CURRENCY_CODE = "USD";
$FP_FRAMESHOP_TITLE = "{FP:The Print Shop}";

// TEAR SHEET
$FP_TEARSHEET_TITLE = "Tear Sheet";

// CERTIFICATE OF Authenticity (COA)
$FP_COA_TITLE = "{FP:Certificate of Authenticity}";
// Max image size in pixels to show in the COA
$COA_IMAGE_W = 200;
$COA_IMAGE_H = 300;

// Packaging calculation method. Options are "fixed" or "percentage"
$FP_PACKAGING_METHOD = "fixed";

// Wall colors for the frame shop
// default
$FP_FRAMESHOP_WALLCOLOR = "#e6e6e6";

// other colors
$WALLCOLORS	= array (
			"#222222" => "Wall Color: Dark Grey",
			"#cccccc" => "Wall Color: Light Grey",
			"#e6e6e6" => "Wall Color: Very Light Grey",
			"#FDFDFD" => "Wall Color: White",
			"#FFFFFF" => "Wall Color: Bright White",
			"#fdf5e6" => "Wall Color: Ivory" ,
			"#faebd7" => "Wall Color: Antique White",
			"#fafad2" => "Wall Color: Linen",
			"#eee8aa" => "Wall Color: Pale Yellow",
			"#f5deb3" => "Wall Color: Wheat",
			"#d2b48c" => "Wall Color: Light Brown",
			"#e6e6fa" => "Wall Color: Lavender",
			"#bc8f8f" => "Wall Color: Rose",
			"#cd5c5c" => "Wall Color: Pale Red",
			"#808080" => "Wall Color: Gray",
			"#f5fffa" => "Wall Color: Mint Cream",
			"#f0f8ff" => "Wall Color: Light Blue",
			"#add8e6" => "Wall Color: Sky Blue",
			"#5f9ea0" => "Wall Color: Sea Green",
			"#6699cc" => "Wall Color: Azure",
			"#666699" => "Wall Color: Slate Blue",
			"#993333" => "Wall Color: Crimson Red",
			"#336633" => "Wall Color: Dark Green",
			"#111111" => "Wall Color: Black"
			);

// Maximum number of prints to sell, in the quantity setting in the frame shop
$FP_MAX_PRINTS_TO_SELL = 1;

$MATTES 	= array (	
	"0"		=> "None",
	"#FDFDFD"	=> "White", 
	"#FFFFAA"	=> "Cream", 
	"#111111"	=> "Black");

// Framing using HTML and graphics
$SYSTEM_FRAMESTYLES  = array (
	0=>" No Frame",
	1=>"Black Metal",
	2=>"Wood", 
	3=>"Gold Leaf", 
	4=>"Baroque Gold", 
	5=>"Black Wood"
);

// These correspond to $SYSTEM_FRAMESTYLES. These are allowed widths.
$FRAMESTYLESWIDTHS 	= array (0=>"0", 1=>"10", 2=>"10", 3=>"15", 4=>"20", 5=>"10");

// picture frame for matted pix
$frameRGB		=	array (0,0,0);
$framehiliteRGB		=	array (80,80,60);
$framewidth		=	7;
$hilitepercent 		= 	1;


// In the frameshop, you can have the print appear smaller or larger depending on 
// the size the customer chooses. 

// FRAMESHOP_REF_SIZE is the "normal" size (in inches or cm) of a picture
// shown in the preview. "20" means the largest side is 20 inches or cm, for example.
// When the frameshop shows resized pictures to give a sense of scale, it shrinks the picture
// if its smaller than FRAMESHOP_REF_SIZE, or grows the picture if it is larger than FRAMESHOP_REF_SIZE.
// Again, we don't care about the units (inches/cm).
$FP_FRAMESHOP_REF_SIZE = 40;

// Multiplier to adjust preview image to a "real" size. That is, we can reduce/enlarge the default picture
// the frameshop shows. 
$FRAMESHOP_SIZE_ADJUSTMENT		= 1.0;

// See MATTESCALE, above (which is 80%)
// Therefore, this will %20 (or .2)
// This is the width of the matte border we use
$FP_MATTE_BORDER_WIDTH = (1 - $mattescale);


// Art border (if no matte or frame), measured in pixels
// Note, this is a fixed pixel amount, not a percentage!
// It should look around 2" on the screen
//$FP_FRAMESHOP_ART_BORDER = 20;
// Art border, using percentage method, e.g. 0.1 means the border width is 10% of the greater size of the image.
// We determine whether we're using a pixel or percentage by checking whether the amount is < 2.
// If it is less than 2, it must be a percentage.
$FP_FRAMESHOP_ART_BORDER = 0.1;

// If we show a frame w/o a matte, do we have an art border, or does the print meet the frame?
// 0=meet the frame, no art border; 1=show art border
$FRAMESHOP_ART_BORDER_NOFRAME = 0;

// Default paper color, which should be white.
$FP_FRAMESHOP_ART_BORDER_COLOR = "#FDFDFD";




//-------------------
// NEWSLETTER

// Time between issues in days, e.g. 2 weeks
$FP_NEWSLETTER_PERIOD = 14;

$FP_MAILER_AUTHOR = $SYSTEMNAME;
$FP_MAILER_SUBJECT = "$SYSTEMNAME Newsletter of ". date("F j, Y");
// ID of email list in the newsletter mailer system (phpEmailList)
$FP_MAILER_LIST = 2;

// This is here so we don't have to change FPConfig
// CSS code to show/hide (display:none or default). True=show, false=hide
switch ($FP_GALLERY_TYPE) {
	case 1	:	$FP_INSTALLATION_SHOW_HIDE = true;
				break;
	case 2	:	$FP_INSTALLATION_SHOW_HIDE = false;
				break;
	default	:	$FP_INSTALLATION_SHOW_HIDE = true;
}


//-------------------
// MENU BAR

// In which menu should the FAQ be shown? The value matches the suffix of the snippet, i.e. 'navbar' or'navbar-2'
// ''=top , '2'=bottom
$FP_NAVBAR_FAQ = '2';

// show By Location in the menubar?
$FP_NAVBAR_SHOW_LOCATION = true;

// Make objects solid if they have "solid" style applied to them.
// This is mostly for nav bars over graphics
$FP_SOLIDIFY = 0;

//-------------------
// Tolerance between desired shape ratio and picture shape ratio, of a random image to display (e.g. 35mm shape).
// So, if you call for a random picture shaped 35mm (constant is FP_35MM_SHAPE), and your picture is not 3:2 but
// is 3:2.1 (a ratio of 1.4), then if the tolerance is .1, you're OK, because 1.5 - 1.4 <= .1
$FP_RANDOM_SHAPE_TOLERANCE = .2;

// Force the top menu to show a link returning to the main gallery.
// This is an option for artists, and this flag will over ride them.
// Useful if you're running a gallery, not a coop.
$FP_SHOW_RETURN_TO_GROUP = false;

//=============  DEFAULT PARAMS SETTINGS
// These are arrays of param defaults for each table.

// GALLERY PARAMS

$FP_PARAMS_DEFAULTS = array ();

$FP_PARAMS_DEFAULTS[DB_PROJECTS] = array (		
	FP_PARAM_GALLERY_GROUPING			=>	'0',
	FP_PARAM_GALLERY_SPACING			=>	'80',
	FP_PARAM_GALLERY_SHOWCAPTIONS		=>	'1',
	FP_PARAM_GALLERY_THEME			=>	'',
	FP_PARAM_GALLERY_GROUP_SPACING	=>	'0',
	FP_PARAM_SLIDES_PROJECT			=>	'0',
	FP_PARAM_GALLERY_SCALING			=>	'1.0',
	FP_PARAM_GALLERY_COMMENTS		=>	'0'
);
// ARTIST PARAMS

$FP_PARAMS_DEFAULTS[DB_ARTISTS] = array (	
	FP_PARAM_ARTIST_BLOG			=>	'',
	FP_PARAM_ARTIST_LINKS			=>	'',
	FP_PARAM_ARTIST_LINKNAMES		=>	'',
);
// GROUPS PARAMS

$FP_PARAMS_DEFAULTS[DB_GROUPS] = array (	
	// Don't know!
	FP_PARAM_GROUP_MENU_1		=>	'',
	FP_PARAM_GROUP_MENU_2		=>	'',
	
	// Menu items in custom menus
	FP_PARAM_GROUP_MENU_1_1	=>	'new',
	FP_PARAM_GROUP_MENU_1_2	=>	'place',
	FP_PARAM_GROUP_MENU_1_3	=>	'all',
	FP_PARAM_GROUP_MENU_1_4	=>	'blog',
	FP_PARAM_GROUP_MENU_1_5	=>	'about',
	FP_PARAM_GROUP_MENU_1_6	=>	'home',
	FP_PARAM_GROUP_MENU_1_7	=>	'',
	FP_PARAM_GROUP_MENU_1_8	=>	'',

	FP_PARAM_GROUP_MENU_2_1	=>	'terms',
	FP_PARAM_GROUP_MENU_2_2	=>	'privacy',
	FP_PARAM_GROUP_MENU_2_3	=>	'contact',
	FP_PARAM_GROUP_MENU_2_4	=>	'fp',
	FP_PARAM_GROUP_MENU_2_5	=>	'admin',
	FP_PARAM_GROUP_MENU_2_6	=>	'',
	FP_PARAM_GROUP_MENU_2_7	=>	'',
	FP_PARAM_GROUP_MENU_2_8	=>	'',

	// checkbox: using custom or built-in menus?
	FP_PARAM_GROUP_CUSTOM_MENU	=>	'0',

	// custom menu items
	FP_PARAM_GROUP_MENU_CUSTOM_1	=>	'<a href="http://www.facebook.com/" target="_blank">Find me on FaceBook</a>',
	FP_PARAM_GROUP_MENU_CUSTOM_2	=>	'',
	FP_PARAM_GROUP_MENU_CUSTOM_3	=>	'',
	FP_PARAM_GROUP_MENU_CUSTOM_4	=>	'',
	FP_PARAM_GROUP_MENU_CUSTOM_5	=>	'',

	FP_PARAM_GROUP_ICON	=>	'',
	FP_PARAM_GROUP_LOGO	=>	''
);


//============= SHIPPING
// Add 4" to the print size to get the length shipping tube
$FP_SHIPPING_TUBE_EXTRA = 4;
// if cm, convert to measurement to cm.
if ($FP_UNIT != "inch")
	$FP_SHIPPING_TUBE_EXTRA = $FP_SHIPPING_TUBE_EXTRA * 2.54;

//============= RSS
// Choose a project as updated for RSS feeds if there any kinds of changes,
// not just pictures, in the last 15 minutes (15*60 sec)
$FP_PROJECT_UPDATED_INTERVAL = 15 * 60;


//=============  CALCULATED VALUES
$FP_COMMERCE_EMAIL_OBFUSCATED = "<span class=\"obfuscated\">".str_replace ("@", "(put the 'at' sign here)", FP_COMMERCE_EMAIL) . "</span>";


//============= COMMENTS WEBSITE
// Really, the admin should be able to override this!
$FP_COMMENTS_URL = "http://mimetic.ning.com";

// ================================================
// ================================================
// NO USER CHANGEABLE SETTINGS BELOW
// ================================================
// ================================================


// ============ user overrides to general config file
file_exists ("_config/my_config.inc") && include "_config/my_config.inc";



// ================================================
// ================================================
// Override the above setting in single-galley systems...there is no main gallery to go to!
if (FP_GALLERY_TYPE == FP_SINGLE_GALLERY_SINGLE_USER || FP_GALLERY_TYPE == FP_SINGLE_GALLERY_MULTI_USER)
	$FP_SHOW_RETURN_TO_GROUP = false;

// ================================================
// ================================================
// Make constants from variables. Must do AFTER the above my_config inclusion
define ('FP_PASSWORD_MIN_LENGTH', $FP_PASSWORD_MIN_LENGTH);
define ('FP_PASSWORD_MAX_LENGTH', $FP_PASSWORD_MAX_LENGTH);
define ('FP_SHOW_MAXCONFIRMTIME', $FP_SHOW_MAXCONFIRMTIME);
define ("FP_MAXCONFIRMTIME", $MAXCONFIRMTIME);
define ("MAX_PROJECT_PICTURES", $MAX_PROJECT_PICTURES);
define ("FP_DEFAULT_GALLERY_NAME", $default_gallery_name);
define ("FP_HIDE_NAME_SOLO_GALLERY", $FP_HIDE_NAME_SOLO_GALLERY);
define ("FP_RESIZE_TO_BKGD_IMAGE", $FP_RESIZE_TO_BKGD_IMAGE);
define ("MATTESCALE", $mattescale);
define ("MATTEBOTTOM", $mattebottom);
define ("FP_SLIDE_WIDTH", $slide_size[0]);
define ("FP_SLIDE_HEIGHT", $slide_size[1]);
define ("FP_SLIDE_ADJUSTMENT", $FP_SLIDE_ADJUSTMENT);
define ("FP_DEFAULT_WIDTH", $default_size[0]);
define ("FP_DEFAULT_HEIGHT", $default_size[1]);
define ('FP_DEFAULT_SLIDESHOW_PAUSE', $FP_DEFAULT_SLIDESHOW_PAUSE);
define ('FP_SSP_TRANSITION_LENGTH', $FP_SSP_TRANSITION_LENGTH);
define ("FP_EMAIL_HEADERS", $em_headers);
define ("FP_EMAIL_HEADERS_SYSTEM", $em_headers_sys);
define ('FP_MAILER_AUTHOR', $FP_MAILER_AUTHOR);
define ('FP_MAILER_SUBJECT', $FP_MAILER_SUBJECT);
define ("FP_MAILER_LIST", $FP_MAILER_LIST);
define ("FP_INSTALLATION_SHOW_HIDE", $FP_INSTALLATION_SHOW_HIDE);
//define ('MAX_DAYS_FEATURED', $FP_MAX_DAYS_FEATURED);
define ('MAX_DAYS_ACTIVE', $FP_MAX_DAYS_ACTIVE);
define ('SHOW_PUBLICPRIVATE_FOR_GROUPS', $FP_SHOW_PUBLICPRIVATE_FOR_GROUPS);
define ('SHOW_PUBLICPRIVATE_FOR_PROJECTS', $FP_SHOW_PUBLICPRIVATE_FOR_PROJECTS);
define ('FP_USERS_GET_GALLERY', $FP_USERS_GET_GALLERY);
define ('MAX_GROUP_SIZE', $FP_MAX_GROUP_SIZE);
define ('OPTION_LIST_MAXSIZE', $FP_OPTION_LIST_MAXSIZE);	
define ('FP_IMAGE_TOOL', $FP_IMAGE_TOOL);
define ('FP_IMAGEMAGICK_QUALITY', $FP_IMAGEMAGICK_QUALITY);
define ('FP_IM_CONVERT', $FP_IM_CONVERT);
define ('FP_IM_COMPOSITE', $FP_IM_COMPOSITE);
define ('MATTECOLOR', $FP_MATTECOLOR);
define ('FP_LIGHTBOX_DISPLAY_REDUCTION', $FP_LIGHTBOX_DISPLAY_REDUCTION);
define ('FP_LIGHTBOX_DISPLAY_PADDING', $FP_LIGHTBOX_DISPLAY_PADDING);
define ('FP_GALLERY_ICON_WIDTH', $FP_GALLERY_ICON_WIDTH);
define ('FP_GALLERY_ICON_HEIGHT', $FP_GALLERY_ICON_HEIGHT);
define ('FP_GALLERY_LOGO_WIDTH', $FP_GALLERY_LOGO_WIDTH);
define ('FP_GALLERY_LOGO_HEIGHT', $FP_GALLERY_LOGO_HEIGHT);
define ('FP_GALLERY_ICON_TITLE_WIDTH', $FP_GALLERY_ICON_TITLE_WIDTH);
define ('FP_GALLERY_ICON_TITLE_HEIGHT', $FP_GALLERY_ICON_TITLE_HEIGHT);
define ('FP_SHOW_BLANK_GALLERY_ICON', $FP_SHOW_BLANK_GALLERY_ICON);
define ('FP_SHOW_GALLERY_ICON', $FP_SHOW_GALLERY_ICON);
define ('FP_SHADOWS', $FP_SHADOWS);
define ('GALLERYFADEDARKNESS', $FP_GALLERYFADEDARKNESS);
define ('FP_SLIDESHOW_PC', $FP_SLIDESHOW_PC);
define ('FP_SLIDESHOW_PC_COLUMNS', $FP_SLIDESHOW_PC_COLUMNS);
define ('FP_SLIDESHOW_TV', $FP_SLIDESHOW_TV);
define ('FP_SLIDESHOW_SV', $FP_SLIDESHOW_SV);
define ('FP_WATERMARK_OPACITY', $FP_WATERMARK_OPACITY);
define ('FP_WATERMARK_FONT_SIZE', $FP_WATERMARK_FONT_SIZE);
define ('FP_WATERMARK_FONT_DIR', $FP_WATERMARK_FONT_DIR);
define ('FP_WATERMARK_FONT_NAME', $FP_WATERMARK_FONT_NAME);
define ('FP_WATERMARK_GALLERY_PIX', $FP_WATERMARK_GALLERY_PIX);
define ('FP_PRINTDPI', $PRINTDPI);
define ('FRAMED_MATTED_PIX', $FP_FRAMED_MATTED_PIX);
define ('FP_ARTIST_DEFAULT_COMMISSION', $FP_ARTIST_DEFAULT_COMMISSION);	
define ('FP_ARTIST_DEFAULT_USERLEVEL', $FP_ARTIST_DEFAULT_USERLEVEL);	
define ('FP_ARTIST_DEFAULT_ACCESSLEVEL', $FP_ARTIST_DEFAULT_ACCESSLEVEL);
define ('FP_ARTIST_DEFAULT_STORAGE', $FP_ARTIST_DEFAULT_STORAGE);
define ('FRAMESHOP_PICTURE_SCALING', $FP_FRAMESHOP_PICTURE_SCALING);
define ('FP_TAX_WHOLE_SALE', $FP_TAX_WHOLE_SALE);
define ('FP_ADMIN_CONTROLS_ARTIST_SALES', $FP_ADMIN_CONTROLS_ARTIST_SALES);
define ('FP_ADMIN_CONTROLS_PRINT_FILE', $FP_ADMIN_CONTROLS_PRINT_FILE);
define ('FP_ALL_METADATAEDIT_ACCESS', $FP_ALL_METADATAEDIT_ACCESS);
define ('FP_EDITION_CALC_METHOD', $FP_EDITION_CALC_METHOD);
define ('FP_SELL_MATTE_NO_FRAME', $FP_SELL_MATTE_NO_FRAME);
//define ('FP_EDITION_CALC_METHOD', $FP_EDITION_CALC_METHOD);
define ('CURRENCY', $FP_CURRENCY);	// span is necessary to avoid PHP substitutions!
define ('FP_CURRENCY_NAME', $FP_CURRENCY_NAME);
define ('FP_DECIMAL', $FP_DECIMAL);		// decimal point 
define ('FP_THOUSANDS', $FP_THOUSANDS);	// thousands separator (e.g. 1,000,000)
define ('CURRENCY_POSITION', $FP_CURRENCY_POSITION);	// can be 'before' or 'after', e.g. $100 or 100 euro
define ('CURRENCY_AFTER', $FP_CURRENCY_AFTER); // name of currency is if used after, e.g. USD
define ('UNIT', $FP_UNIT);
define ('UNITS', $FP_UNITS);
define ('FP_LABEL_INCHES', $FP_LABEL_INCHES);
define ('FP_LABEL_CM', $FP_LABEL_CM);
define ('WEIGHT_UNIT', $FP_WEIGHT_UNIT);
define ('FP_CUSTOMER_PICKUP_TEXT', $FP_CUSTOMER_PICKUP_TEXT);
define ('FP_CUSTOMER_CHOOSES_PAPER', $FP_CUSTOMER_CHOOSES_PAPER);
define ('FP_CUSTOMER_CHOOSES_INKSET', $FP_CUSTOMER_CHOOSES_INKSET);
define ('FP_SHOW_CART_NAME', $FP_SHOW_CART_NAME);
define ('FP_NAVBAR_SHOW_CART_NAME', $FP_NAVBAR_SHOW_CART_NAME);
define ('PAYPAL_CURRENCY_CODE', $FP_PAYPAL_CURRENCY_CODE);
define ('FRAMESHOP_TITLE', $FP_FRAMESHOP_TITLE);
define ('FP_TEARSHEET_TITLE', $FP_TEARSHEET_TITLE);
define ('FP_FRAMESHOP_WALLCOLOR', $FP_FRAMESHOP_WALLCOLOR);
define ('FP_PACKAGING_METHOD', $FP_PACKAGING_METHOD);
define ('FP_COA_TITLE', $FP_COA_TITLE);
define ('COA_IMAGE_W', $COA_IMAGE_W);
define ('COA_IMAGE_H', $COA_IMAGE_H);
define ("MAX_PRINTS_TO_SELL", $FP_MAX_PRINTS_TO_SELL);
define ("FRAMESHOP_REF_SIZE", $FP_FRAMESHOP_REF_SIZE);
define ('MATTE_BORDER_WIDTH', $FP_MATTE_BORDER_WIDTH);
define ('FRAMESHOP_ART_BORDER', $FP_FRAMESHOP_ART_BORDER);
define ('FRAMESHOP_ART_BORDER_COLOR', $FP_FRAMESHOP_ART_BORDER_COLOR);
define ('FRAMESHOP_ART_BORDER_NOFRAME', $FRAMESHOP_ART_BORDER_NOFRAME);
define ('FP_NEWSLETTER_PERIOD', $FP_NEWSLETTER_PERIOD);
define ('FP_NAVBAR_FAQ', $FP_NAVBAR_FAQ);
define ('FP_NAVBAR_SHOW_LOCATION', $FP_NAVBAR_SHOW_LOCATION);
define ('FP_SOLIDIFY', $FP_SOLIDIFY);
define ('FP_SHOW_RETURN_TO_GROUP', $FP_SHOW_RETURN_TO_GROUP);
define ('FP_RANDOM_SHAPE_TOLERANCE', $FP_RANDOM_SHAPE_TOLERANCE);
define ("FP_COMMERCE_EMAIL_OBFUSCATED", $FP_COMMERCE_EMAIL_OBFUSCATED);
define ('FP_PROJECT_UPDATED_INTERVAL', $FP_PROJECT_UPDATED_INTERVAL);
define ('FP_SHIPPING_TUBE_EXTRA', $FP_SHIPPING_TUBE_EXTRA);

?>