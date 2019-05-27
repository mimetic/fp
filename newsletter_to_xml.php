<?php
include("includes/GrossRSS.inc");

$rss=new RSSWriter("https://localhost/fp/GrossRss.php", "Example Site", 
	"The best examples out there.", 
		array("publisher" => "David Gross", 
				"creator" => "David Gross"));
		
$rss->setChannel ("My Feed", "localhost/fp", "This is my feed.");

$text = file_get_contents ("newsletter_dropbox/newsletter.html");

$item = array (	'title'		=>	"My second title",
				'link'		=>	"images/photos/processed/thumbnails/031227-135102-567.jpg",
				'URL'		=>	"images/photos/processed/slides/031227-135102-567.jpg",
				'description'	=>	'<![CDATA[' . $text . "]]>",
				'subject'	=>	"Newsletter",
				'creator'	=>	"David Gross"
				);

$rss->addItem($item);

$rss->serialize();
?>
