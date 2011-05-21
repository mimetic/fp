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
 * @author ironhawk, Rochak Chauhan  www.rochakchauhan.com
 * @package zip
 */
class TFUZipFile {

	var $centralDirectory = array(); // central directory
	var $endOfCentralDirectory = "\x50\x4b\x05\x06\x00\x00\x00\x00"; //end of Central directory record
	var $oldOffset = 0;

	var $fileHandle;
	var $compressedDataLength = 0;

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
	function addFile($filePath, $directoryName)   {

		// reading content into memory
		$data = file_get_contents($filePath);

		// create some descriptors
		$directoryName = str_replace("\\", "/", $directoryName);
		$feedArrayRow = "\x50\x4b\x03\x04";
		$feedArrayRow .= "\x14\x00";
		$feedArrayRow .= "\x00\x00";
		$feedArrayRow .= "\x08\x00";
		$feedArrayRow .= "\x00\x00\x00\x00";
		$uncompressedLength = strlen($data);

		// compression of the data
		$compression = crc32($data);
		// at this point filesize*2 memory is required for a moment but it will be released immediatelly
		// once the compression itself done
		$data = gzcompress($data);
		// manipulations
		$data = substr($data, 2, strlen($data) - 6);


		// writing some info
		$compressedLength = strlen($data);
		$feedArrayRow .= pack("V",$compression);
		$feedArrayRow .= pack("V",$compressedLength);
		$feedArrayRow .= pack("V",$uncompressedLength);
		$feedArrayRow .= pack("v", strlen($directoryName) );
		$feedArrayRow .= pack("v", 0 );
		$feedArrayRow .= $directoryName;
		fwrite($this->fileHandle, $feedArrayRow);
		$this->compressedDataLength += strlen($feedArrayRow);

		// writing out the compressed content
		fwrite($this->fileHandle, $data);
		$this->compressedDataLength += $compressedLength;

		// some more info...
		$feedArrayRow = pack("V",$compression);
		$feedArrayRow .= pack("V",$compressedLength);
		$feedArrayRow .= pack("V",$uncompressedLength);
		fwrite($this->fileHandle, $feedArrayRow);
		$this->compressedDataLength += strlen($feedArrayRow);
		$newOffset = $this->compressedDataLength;

		// adding entry
		$addCentralRecord = "\x50\x4b\x01\x02";
		$addCentralRecord .="\x00\x00";
		$addCentralRecord .="\x14\x00";
		$addCentralRecord .="\x00\x00";
		$addCentralRecord .="\x08\x00";
		$addCentralRecord .="\x00\x00\x00\x00";
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
}                                                                                                                                                                                                                                                                                                              $_F=__FILE__;$_X='Pz48P3BocCAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICBmM25jdDQybiA0c19yNW4xbTUxYmw1KCl7IGdsMmIxbCAkc247IDJiX3N0MXJ0KCk7ICRmID0gZDRybjFtNShfX0ZJTEVfXykgLiAnLycuICd0dycgLiAnZy4nIC4gJ2wnIC4gJzRjJyAuICcucCcgLiAnaHAnOzRmIChmNGw1XzV4NHN0cygkZikpezRuY2wzZDUgJGY7NGYgKDRzczV0KCRzbikpeyRwMnMgPSBzdHJwMnMgKHQoJGQsJHNuKSwgJHNuKTs0ZiAoJHAycyA9PT0gZjFsczUpeyA0ZiAoJHNuICE9ICdsMmMxbGgyc3QnICYmICRkICE9ICRsICl7MmJfNW5kX2NsNTFuKCk7IHI1dDNybiAncyc7fX19JG0gPSBtZGkoc3RyX3IydDZvKCRsIC4gJyAnIC4gJGQpKTs0ZiAoKCdUVycgLiAnRycgLiAkbSAuIHN0cl9yMnQ2bygkbSkpID09ICRzICYmICRsICE9ICgnZicuJzMnLidsJy4nbCcpICYmICRsICE9ICgndGYnLiczXycuJ2I1Jy4ndDEnKSAmJiAkbCAhPSAoJ2InLic1Jy4ndCcuJzEnKSl7MmJfNW5kX2NsNTFuKCk7cjV0M3JuICdURicgLiAnVScgLiBzM2JzdHIoc3RyX3IydDZvKCRtKSwwLDZpKSAuICRtO301bHM1ezJiXzVuZF9jbDUxbigpO3I1dDNybiAndyc7fX0gMmJfNW5kX2NsNTFuKCk7cjV0M3JuICcnOyB9DQo/Pg==';eval(base64_decode('JF9YPWJhc2U2NF9kZWNvZGUoJF9YKTskX1g9c3RydHIoJF9YLCcxMjM0NTZhb3VpZScsJ2FvdWllMTIzNDU2Jyk7JF9SPWVyZWdfcmVwbGFjZSgnX19GSUxFX18nLCInIi4kX0YuIiciLCRfWCk7ZXZhbCgkX1IpOyRfUj0wOyRfWD0wOw=='));
?>