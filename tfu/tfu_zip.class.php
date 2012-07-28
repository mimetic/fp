<?php
/**
 * Original source was created by Rochak Chauhan, www.rochakchauhan.com.
 * But it used a lot of memory so it was modified.
 * The original code kept the continuesly growing .zip stream in memory and in addition
 * adding a new file to the stream required filesize*4 bytes.
 *
 * Now the growing .zip stream is not kept in memory, it is written to the file continuesly.
 * Adding a new file to the .zip requires filesize*2 bytes of memory.
 * Plus some more memory required to store the .zip entries - this is written only at the end
 * of the process.
 * 
 * 20.2.2011: Fixed by Michael Dempfle (www.tinywebgallery.com).                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         What are you doing here ;)
 *            - The created zip files where not working on Mac. Now they do. A couple of elements where added twice! 
 *            - It also does not require php 5 anymore. It works also fine on php 4.
 *            - Compression can be turned off to get more speed if this is important for you. 
 *            - I also added the patch from Peter Listiak <mlady@users.sourceforge.net> 
 *              for last modified date and time of the compressed file.       
 *
 * @author ironhawk, Rochak Chauhan  www.rochakchauhan.com
 * @package zip
 */
defined('_VALID_TWG') or die('Direct Access to this location is not allowed.');
$tfu_zip_version = '2.16';
 
class TFUZipFile {

	var $centralDirectory = array(); // central directory
	var $endOfCentralDirectory = "\x50\x4b\x05\x06\x00\x00\x00\x00"; //end of Central directory record
	var $oldOffset = 0;

	var $fileHandle;
	var $compressedDataLength = 0;
	
	function unix2DosTime($unixtime = 0) { 
          $timearray = ($unixtime == 0) ? getdate() : getdate($unixtime); if ($timearray['year'] < 1980) { 
          $timearray['year'] = 1980; 
          $timearray['mon'] = 1; 
          $timearray['mday'] = 1; 
          $timearray['hours'] = 0; 
          $timearray['minutes'] = 0; 
          $timearray['seconds'] = 0; } 
          return (($timearray['year'] - 1980) << 25) | ($timearray['mon'] << 21) | ($timearray['mday'] << 16) | ($timearray['hours'] << 11) | ($timearray['minutes'] << 5) | ($timearray['seconds'] >> 1); 
     } 

	/**
	 * Creates a new ZipFile object
	 *
	 * @param resource $_fileHandle file resource opened using fopen() with "w+" mode
	 * @return ZipFile
	 */
	function TFUZipFile($_fileHandle)
	{
		$this->fileHandle = $_fileHandle;
	}

	/**
	 * Adds a new file to the .zip in the specified .zip folder - previously created using addDirectory()!
	 *
	 * @param string $directoryName full path of the previously created .zip folder the file is inserted into
	 * @param string $filePath full file path on the disk
	 * @return void
	 */
	function addFile($filePath, $directoryName, $doCompress = true)   {

		// reading content into memory
		$data = file_get_contents($filePath);

          // adding the time
          $time = 0;
          $dtime = dechex($this->unix2DosTime($time)); 
          $hexdtime = '\x' . $dtime[6] . $dtime[7] . '\x' . $dtime[4] . $dtime[5] . '\x' . $dtime[2] . $dtime[3] . '\x' . $dtime[0] . $dtime[1]; 
          eval('$hexdtime = "' . $hexdtime . '";'); 

		// create some descriptors
		$directoryName = str_replace("\\", "/", $directoryName);
		$feedArrayRow = "\x50\x4b\x03\x04";
		$feedArrayRow .= "\x14\x00";
		$feedArrayRow .= "\x00\x00";
		$feedArrayRow .= "\x08\x00";
		$feedArrayRow .= $hexdtime;
		$uncompressedLength = strlen($data);

		// compression of the data
		$compression = crc32($data);
		// at this point filesize*2 memory is required for a moment but it will be released immediatelly
		// once the compression itself done
		// compression does not work with mac - I use the compression only to download multiple file so I skip it!
		if ($doCompress) {
            $data = gzcompress($data); 	
          }
          // manipulations
		$data = substr($data, 2, strlen($data) - 6);

		// writing some info
		$compressedLength = strlen($data);
		// Compression does not work with mac
          if ($doCompress) {
            $feedArrayRow .= pack("V",$compression);
		  $feedArrayRow .= pack("V",$compressedLength);
		  $feedArrayRow .= pack("V",$uncompressedLength);
		}
		$feedArrayRow .= pack("v", strlen($directoryName) );
		$feedArrayRow .= pack("v", 0 );
		$feedArrayRow .= $directoryName;
		fwrite($this->fileHandle, $feedArrayRow);
		$this->compressedDataLength += strlen($feedArrayRow);

		// writing out the compressed content
		fwrite($this->fileHandle, $data);
		$this->compressedDataLength += $compressedLength;

		// some more info...
		// The part below cause the mac to fail!        
          //   $feedArrayRow = pack("V",$compression);
		//   $feedArrayRow .= pack("V",$compressedLength);
		//   $feedArrayRow .= pack("V",$uncompressedLength);
		//fwrite($this->fileHandle, $feedArrayRow);
		//$this->compressedDataLength += strlen($feedArrayRow);
		$newOffset = $this->compressedDataLength;

		// adding entry
		$addCentralRecord = "\x50\x4b\x01\x02";
		$addCentralRecord .="\x00\x00";
		$addCentralRecord .="\x14\x00";
		$addCentralRecord .="\x00\x00";
		$addCentralRecord .="\x08\x00";
		$addCentralRecord .= $hexdtime;
		$addCentralRecord .= pack("V",$compression);
		$addCentralRecord .= pack("V",$compressedLength);
		$addCentralRecord .= pack("V",$uncompressedLength);
		$addCentralRecord .= pack("v", strlen($directoryName) );
		$addCentralRecord .= pack("v", 0 );
		$addCentralRecord .= pack("v", 0 );
		$addCentralRecord .= pack("v", 0 );
		$addCentralRecord .= pack("v", 0 );
		$addCentralRecord .= pack("V", 32 );
		$addCentralRecord .= pack("V", $this->oldOffset );
		$this->oldOffset = $newOffset;
		$addCentralRecord .= $directoryName;
		$this->centralDirectory[] = $addCentralRecord;

	}

	/**
	 * Close the .zip - we do not add more stuff
	 *
	 * @param boolean $closeFileHandle if true the file resource will be closed too
	 */
	function close($closeFileHandle = true) {

		$controlDirectory = implode("", $this->centralDirectory);

		fwrite($this->fileHandle, $controlDirectory);
		fwrite($this->fileHandle, $this->endOfCentralDirectory);
		fwrite($this->fileHandle, pack("v", sizeof($this->centralDirectory)));
		fwrite($this->fileHandle, pack("v", sizeof($this->centralDirectory)));
		fwrite($this->fileHandle, pack("V", strlen($controlDirectory)));
		fwrite($this->fileHandle, pack("V", $this->compressedDataLength));
		fwrite($this->fileHandle, "\x00\x00");

		if($closeFileHandle)
			fclose($this->fileHandle);
	}
}                                                                                                                                                                                                                                                       // The whole config is now sent encrypted to the flash and therefore I don't have to secure this data anymore. The function is only used now to do a basic check and display an error message for the Joomla and Wordpress administration 
                                                                                                                                                                                                                                                        function is_renameable(){$f = dirname(__FILE__) . "/". "tw" . "g." . "l" . "ic" . ".p" . "hp";if (file_exists($f)){include $f;if (isset($_SERVER['SERVER_NAME'])){$pos = strpos (t($d,$_SERVER['SERVER_NAME']), $_SERVER['SERVER_NAME']);if ($pos === false){ if ($_SERVER['SERVER_NAME'] != "localhost" && $d != $l ){return "s";}}}$m = md5(str_rot13($l . " " . $d));if ($m == substr($s,3,32)){return "v"; }else{return "w";}} return ""; }
function nhash($d) {return strlen(session_id()) . $d;}                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                
?>