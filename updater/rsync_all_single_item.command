#!/bin/bash
# Shell script to run multiple rsyncs to update gallery websites

# param1 = username
# param2 = URL of site, e.g. mysite.com
function _rsync ()
{
	echo Update $2
	echo -------------------------------------
	rsync -avzp --delete -e "ssh -i /Users/dgross/.ssh/mirror-rsync-key" --include-from=/Users/dgross/Sites/fp/updater/include.txt --exclude-from=/Users/dgross/Sites/fp/updater/exclude.txt $4 /Users/dgross/Sites/fp/* $1@$2:$3

	echo -------------------------------------
	echo
}


echo =========================================
echo Update gallery websites with rsync
echo =========================================

_rsync girlswho girlswholike.us public_html/

echo =========================================
echo END
echo =========================================
