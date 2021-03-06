#!/bin/bash
# Shell script to run multiple rsyncs to update gallery websites

# param1 = username
# param2 = URL of site, e.g. mysite.com
function _rsync ()
{
	echo -------------------------------------
	echo Update $2
	#echo using port 50613
	echo -------------------------------------
	
	echo ===== REAL UPDATE, NOT DRY RUN !!!!! ======
	rsync -avzp --delete -e "ssh -i ~/.ssh/mirror-rsync-key" --include-from=/Users/dgross/Sites/fp/updater/include.txt --exclude-from=/Users/dgross/Sites/fp/updater/exclude.txt $4 /Users/dgross/Sites/fp/* $1@$2:$3
	
	# DRY RUN
	#echo ===== DRY RUN ON! ======
	#rsync -navzp --delete -e "ssh -i /Users/dgross/.ssh/mirror-rsync-key" --include-from=/Users/dgross/Sites/fp/updater/include.txt --exclude-from=/Users/dgross/Sites/fp/updater/exclude.txt $4 /Users/dgross/Sites/fp/* $1@$2:$3
	
	echo -------------------------------------
	echo
}


echo =========================================
echo Update gallery websites with rsync
echo =========================================


_rsync girlswho girlswholike.us public_html/
_rsync caroline carolineabitbol.com public_html/
_rsync milosbic milosbicanski.com public_html/
_rsync karenrob karenrobinson.co.uk public_html/
_rsync georgege georgegeorgiou.net public_html/
_rsync vanessaw vanessawinship.com public_html/
_rsync lisakess lisakessler.net public_html/
_rsync artpress artpressgallery.com public_html/
_rsync frontlin frontline-photos.com public_html/
_rsync matthieu matthieuchazal.com public_html/
_rsync danapopa danapopa.com public_html/

#_rsync dgphoto davidgrossphoto.com public_html/
#_rsync justmiel justmiel.com public_html/
#_rsync danapopa danapopa.com public_html/
#_rsync deanchap deanchapmanphotos.com public_html/
#_rsync massimos massimosciacca.com public_html/
#_rsync mimetic mimetic.com public_html/gallery/
#_rsync alpsime alpsime.com public_html/


echo =========================================
echo END
echo =========================================

