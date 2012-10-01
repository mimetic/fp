#!/bin/bash
# Shell script to run multiple rsyncs to update gallery websites

# param1 = username
# param2 = URL of site, e.g. mysite.com
function _rsync ()
{
	echo Update $2
	echo -------------------------------------
#	echo 'rsync -avzp --delete -e "ssh -i /Users/dgross/.ssh/mirror-rsync-key" --include-from=/Users/dgross/Sites/fp/updater/include.txt --exclude-from=/Users/dgross/Sites/fp/updater/exclude.txt ' $4 ' /Users/dgross/Sites/fp/*' $1@$2:$3
	rsync -avzp --delete -e "ssh -i /Users/dgross/.ssh/mirror-rsync-key" --include-from=/Users/dgross/Sites/fp/updater/include.txt --exclude-from=/Users/dgross/Sites/fp/updater/exclude.txt $4 /Users/dgross/Sites/fp/* $1@$2:$3
	echo -------------------------------------
	echo
}


echo =========================================
echo Update gallery websites with rsync
echo =========================================

_rsync girlswho girlswholike.us public_html/
_rsync mimetic mimetic.com public_html/
_rsync alpsime alpsime.com public_html/
_rsync atelius atelius.com public_html/
_rsync caroline carolineabitbol.com public_html/
_rsync dgphoto davidgrossphoto.com public_html/
_rsync fireseas fireseason2009.com public_html/
_rsync frontlin frontline-photos.com public_html/
_rsync milosbic milosbicanski.com public_html/
_rsync shahrzad shahrzadkamel.com public_html/
_rsync justmiel justmiel.com public_html/
_rsync karenrob karenrobinson.mimetic.com public_html/
_rsync georgege georgegeorgiou.net public_html/
_rsync vanessaw vanessawinship.com public_html/
_rsync lisakess lisakessler.net public_html/
_rsync massimos massimosciacca.mimetic.com public_html/
_rsync seekthef seekthefoufou.com public_html/
_rsync artpress artpressgallery.com public_html/
_rsync danapopa danapopa.com public_html/

echo =========================================
echo END
echo =========================================

