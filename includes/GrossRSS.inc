<?php

// FPRSS
// Creates an RSS feed for the FP system
//
// BASED ON:
// $Id: rss10.inc,v 1.3 2001/05/20 17:58:02 edmundd Exp $
// A convenience class to make it easy to write RSS classes
// Edd Dumbill <mailto:edd+rsswriter@usefulinc.com>

class RSSWriter {

function RSSWriter($uri, $title, $description, $meta=array()) {
	$this->chaninfo=array();
	$this->website=$uri;
	$this->chaninfo["link"]=$uri;
	$this->chaninfo["description"]=$description;
	$this->chaninfo["title"]=$title;
	$this->items=array();
	// We're not going to use modules, esp. since I don't understand them
	//$this->modules=array("dc" => "https://purl.org/dc/elements/1.1/");
	// thanks James Mills for bugfix to this line
	$this->channelURI=str_replace("&", "&amp;", "https://" . $_SERVER["SERVER_NAME"] . $_SERVER["REQUEST_URI"]);
	foreach ($meta as $key => $value) {
		$this->chaninfo[$key]=$value;
	}
}

// Create a channel
function setChannel ($title, $link, $description, $meta = array ()) {
	$this->channel = array (
							'title'	=>	$title,
							'link'	=>	$link,
							'description'	=>	$description
							);
	foreach ($meta as $key => $value) {
		$channel[$key] = $value;
	}
}

function setImage($title, $URL, $link, $description, $w, $h) {
	$this->image = array(	"URL" 	=> $URL, 
						"title"	=> $title,
						"link"	=> $link,
						"description"	=>	$description,
						"width"	=>	$w,
						"height"	=>	$h
						);
}

function addItem($item = array('title'=>'missing info')) {
	$this->items[]=$item;
}

function serialize() {
	$this->preamble();
	$this->channelinfo();
	$this->items();
	$this->postamble();
}

function deTag($in) {
  while(ereg('<[^>]+>', $in)) {
	$in=ereg_replace('<[^>]+>', '', $in);
  }
  return $in;
}

function preamble() {
	header("Content-type: text/xml");
	$pre =  "<?xml version=\"1.0\" encoding=\"iso-8859-1\" ?" . ">";
	$pre .= '<rss version="2.0"';
//	$pre .= "xmlns=\"";
//	$pre .= str_replace("&", "&amp;", "https://" . $_SERVER["SERVER_NAME"] . $_SERVER["REQUEST_URI"]);
//	$pre .= str_replace("&", "&amp;", "https://www.frontline-photos.com/");
//	$pre .= "\";
	$pre .= ">\n\n";
	print $pre;
}

function channelinfo() {
	print "  <channel>\n";
	$i=$this->chaninfo;
	foreach (array("title", "link", "source", "description", "language", "publisher",
		"creator", "rights") as $f) {
		if (isset($i[$f])) {
			print "    <${f}>" . htmlspecialchars($i[$f]) . "</${f}>\n";
		}
	}
	if (isset($this->image)) {
			print $this->image();
	}
}

function image() {
	if (isset($this->image)) {
	print "  <image>\n";
    print "     <title>" . htmlspecialchars($this->image["title"]) . "</title>\n";
    print "     <url>" . htmlspecialchars($this->image["URL"]) . "</url>\n";
    print "     <link>" . htmlspecialchars($this->image["link"]) . "</link>\n";
    isset($this->image["description"]) && print "     <description>" . htmlspecialchars($this->image["description"]) . "</description>\n";
    isset($this->image["height"]) && print "     <height>" . htmlspecialchars($this->image["height"]) . "</height>\n";
    isset($this->image["width"]) && print "     <width>" . htmlspecialchars($this->image["width"]) . "</width>\n";
	print "  </image>\n\n";
	}
}

function postamble() {
	print "  </channel>\n\n";
	print '</rss>
';
}

function items() {
	foreach ($this->items as $item) {
		print "  <item>\n";
		foreach ($item as $key => $value) {
			if ($key!="uri") {
				if (is_array($value)) {
					foreach ($value as $v1) {
						print "    <${key}>" . htmlspecialchars($v1) . "</${key}>\n";
					}
				} else {
					print "    <${key}>" . htmlspecialchars($value) . "</${key}>\n";
				}
			}
		}
		print "  </item>\n\n";
	}
}

}

?>
