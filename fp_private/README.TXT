For security, these Paypal values are hidden outside the document root.

Set permissions of fp_private to 0740
Set permissions of contents of fp_private to 0640
Owner should be account owner

cd fp_private
chmod 640 *

-rw-r-----   1 caroline caroline  777 Oct 19 13:41 paypal_masspay_config.inc
-rw-r-----   1 caroline caroline   13 Oct 19 13:41 pp_secret.txt
-rw-r--r--   1 caroline caroline  395 Oct 19 13:41 README.TXT

