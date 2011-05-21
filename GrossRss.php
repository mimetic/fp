<?php
include("includes/GrossRSS.inc");

$rss=new RSSWriter("http://www.frontline-photos.com/GrossRss.php", "Example Site", 
	"The best examples out there.", 
		array("publisher" => "David Gross", 
				"creator" => "David Gross"));
		
$rss->setChannel ("My Feed", "www.frontline-photos.com", "This is my feed.");

$rss->setImage("My Image", "http://www.frontline-photos.com/images/logotype.gif", "http://www.frontline-photos.com/",	"This is image description", 300, 45); 

$item = array (	'title'		=>	"My first title",
				'link'		=>	"images/photos/processed/thumbnails/031227-135102-567.jpg",
				'URL'		=>	"images/photos/processed/slides/031227-135102-567.jpg"
				);


$rss->addItem($item);

$item = array (	'title'		=>	"My second title",
				'link'		=>	"images/photos/processed/thumbnails/031227-135102-567.jpg",
				'URL'		=>	"images/photos/processed/slides/031227-135102-567.jpg",
				'description'	=>	'Here is the description:<BR> <img src="images/photos/processed/thumbnails/031227-135102-567.jpg">',
				'subject'	=>	"Pictures",
				'creator'	=>	"David Gross"
				);

$rss->addItem($item);

$rss->serialize();
?>
