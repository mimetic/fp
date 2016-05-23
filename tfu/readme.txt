---------------------------------------------------------
TWG Flash Uploader 3.2 Freeware - README

For the latest version please go to 
http://www.tinywebgallery.com/en/tfu/web_overview.php
---------------------------------------------------------

Requirements
------------
Browser:    Flash >= 8 plugin
            JDK >= 1.4 if you want to use the TFU File Split Applet
Web Server: php >= 4.3.5 with gdlib > 2.0

Installation
------------
1. Extract the content of the zip to a directory on your webspace.
2. change the permissions of the folder upload to 777
   (this is only the  folder for testing - I think you will change this later ;))
3. Call tfu.htm.

Try to upload something! If you get an http 406 error extract the .htaccess file 
from the htaccess.zip and store it in the directory of the flash! For other errors
please check in the forum on the website!
Errors are written to the tfu.log. You should check tfu.log for errors if it does
exist and something is not working as expected.

If you want offer your users the TFU File Split Applet please see the 
fsa/fsa_signed.htm

Do NOT update the integrated version of TFU with this standalone 
version. TWG uses modified php files because they handle 
user management as well and have different paths! 

Setup
-----
By default the user is logged in automatically and you can use
the TWG Flash Uploader right away.

The tfu_config.php is the main configuration file. 
There are 2 interesting settings you should look at:
 - $login  - you can impement your own autentification by setting this flag! 
             If you use "auth" a login screen appears.
 - $folder - The folder where your uploads will be saved!
 
You can create a file called my_tfu_config.php where only you only store your changes.
Then updating is very easy because you can overwrite all files.

The php files that are included can be used as a base how to use this flash
in your own applications! Especially the handling of the login and the folder
is very static here. If you want to see a running example with authorization 
and different working folders you can check the code of the TWG Admin of the
TinyWebGallery (http://www.tinywebgallery.com/en/tfu/web_overview.php)

Features
--------
- Upload your files by simply adding them to the upload queue and press 'Upload'
- Display of filename, size and date in the upload queue
- Estimated upload/download time
- Enhanced Upload Email notification
- Remove file sizes shown in a nice grid
- Support for splitted files
- TFU File Split Applet
- Preloader that shows the loading status
- Remote directory browsing
- Delete and rename of files
- Delete and rename of folders
- Creation of folders
- Remote folder view (can be disabled)
- Preview images on the remote folder
- Big preview of an image by clicking on the preview
- Available in 13 languages - with selector in flash + languages are stored in xml - therefore easily exendable by everyone!
- Basic user management already included
- Remove one or multiple files from the upload queue
- Included login mechanism
- Prodected against unauthorised use!
- Image resize option (server side resize script is included) for jpg, gif, png
- Autoremoval from duplicate files in the upload queue
- Autodetection of existing files on the server
- Detection of the upload file limit of the server
- Define allowed and not allowed file extension 
- sha-1 encoded passwords
- Support for image magick for resizing files
- Fully configuable - every features can be turned on/off

The freeware edition has a 3 MB limit of the upload queue. 
Please go to http://www.tinywebgallery.com/en/tfu/web_overview.php to register
TWG Flash Uploader for your domain.

The standard edition has the following bonus features:
- Unlimited Upload queue (no 3MB limit anymore)
- View and edit of text files (new 2.7)
- Download of files (extra button possible since 2.6)
- Javascript events after the upload is finished and when selecting a file
- Set a limit for files for a upload directory and the upload queue
- Title and some text labels can be changed by configuration
- Professional license: Colors can be changed (new 2.5.1)
- Professional license: Files/folders can be copied/moved (2.6) 
- Professional license: Description mode (2.7) 

See the website for additional features.

If you want to translate the Uploader to your language: Translate the default 
xml file to your language, store it in UTF-8 and send it back to me.

License
-------
Please read the license.txt for details.

Have fun using TWG Flash Uploader
Michael