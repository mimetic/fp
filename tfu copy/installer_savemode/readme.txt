------------------------------------
TFU safe mode installer - README

by Michael Dempfle
------------------------------------

1. If you have no idea what safe mode is: 
   close this file and delete this directoy ;).
2. If you have no idea what safe mode is AND 
   want to use all features of TFU when you have
   a server where safe mode is on:
   Go to www.tinywebgallery.com -> Howto/FAQ and
   read howto 30! There I explain what safe mode 
   is and why this is a problem for you.
3. If you have read howto 30 or already know what
   safe mode is: continue ;).
 
How to find out if safe mode is enabled:
Make a php file with:
<?php
phpinfo();
?>
Copy it to your server and call it:
Then search for "safe_mode".

If you want to use the "create folder" feature of
TFU and you have safe mode enabled on your server
you cannot simply extract the zip and use this feature.

The reason is that the owner of the script would be
different then the php user and you would get an error
because of safe mode restrictons. 

There are different ways to sove this:
- search for a hoster without safe mode ;).
- ask your hoster to turn it off
- use this installer.

This installer can actually be used to install any 
php script without having the safe mode problem anymore.

What it does: 
1. It copies the files used for installation
   The copied files have now the php user as owner!
2. It extracts the software in a subdirectory.

-> The extracted files are now owned by the php user and
   not the ftp user anymore: no safe mode restrictions anymore ;).

How you use it to install TFU:
1. Copy the 2 php files in this directoy to your 
   destination directory.
2. Put the files of tfu in a zip file called tfu.zip and 
   copy it to your destination directory. You can use the
   original tfu zip but this has a subfolder tfu already.
   If you want to avoid this - create your own zip file.
3. Set the destination dir to 777. This is needed that php
   can create a folder here.
4. Execute install_tfu.php - the zip will be installed in
   the subfolder tfu. If you want to have it installed into
   another dir you have to modify the installer file.
   
Feel free to change the installer to your needs. The installer 
has an url parameter ?remove=true - Then you can delete the
directory tfu with all it's content. 

I'm using this installer already on some web spaces without any
problems. 
Please note: You use this installer on your own risk.
I'm not responsible if it does not work for you because it's 
impossible to test all php server configurations. But it should be 
easy to modify the installer that it works for your server too ;).

Are there any drawbacks: Of course! If you use the installer the files
are owned by the php user and not the ftp user anymore. I have set all
files too 777 - therefore you can delete the files - but if you modify
any files by ftp they are owned by the ftp user again. Therefore you 
can get problems if you use the flash AND ftp to e.g. create folders.
Please choose one way of handling your files.

Have fun ;).

/Michael