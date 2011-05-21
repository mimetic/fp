<?php
class NewsletterRSS
{
	public $dropbox = "newsletter_dropbox";
	public $message;
	
	public function NewsletterRSS ($dropbox)
	{
		$dropbox && $this->dropbox = $dropbox;
	}
	
	public function SetDropbox ($dropbox)
	{
		$dropbox && $this->dropbox = $dropbox;
	}
	
	public function GetFeed($filename = null)
	{
		if ($filename)
		{
			$description = file_get_contents ("{$this->dropbox}/$filename");
		} else {
			$description = $this->message;
		}
		if (empty($description))
			return false;
		
		$rss = '<?xml version="1.0" encoding="ISO-8859-1" ?>
			<rss version="2.0">
				<channel>
					<title>'. 'title' .'</title>
					<link>'. 'link' .'</link>
					<description>'. 'description' .'</description>
					<language>'. 'language' .'</language>';
					
		$rss .= '<item>
			<title>'. 'Newsletter' .'</title>
			<link>'. 'link' .'</link>
			<description><![CDATA['. $description .']]></description>
		</item>';
		$rss .= '</channel>
				</rss>';
		return $rss;
	}
}
	
?>