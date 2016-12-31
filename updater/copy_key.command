#!/bin/bash
# Shell script to run multiple rsyncs to update gallery websites

# param1 = username
# param2 = URL of site, e.g. mysite.com
function _copy ()
{
	echo -------------------------------------
	echo Copy key to user $1
	echo -------------------------------------
	
	cp /home/girlswho/.ssh/id_rsa.pub /home/$1/.ssh/
	chown $1.$1 /home/$1/.ssh/id_rsa.pub
	cat /home/$1/.ssh/id_rsa.pub >>  /home/$1/.ssh/authorized_keys

	cp /home/girlswho/.ssh/mirror-rsync-key.pub /home/$1/.ssh/
	chown $1.$1 /home/$1/.ssh/mirror-rsync-key.pub
	cat /home/$1/.ssh/mirror-rsync-key.pub >>  /home/$1/.ssh/authorized_keys
}



_copy addmtc
_copy alissaqu
_copy alpsime
_copy artivismrocks
_copy artpress
_copy berlinsk
_copy caroline
_copy catstant
_copy cozymome
_copy danapopa
_copy davidgro
_copy deanchap
_copy dgphoto
_copy digross
_copy diymom
_copy evidence
_copy flashmob
_copy frontlin
_copy georgege
_copy girlswho
_copy inherownsweet
_copy insideoutsidepro
_copy justmiel
_copy karenrob
_copy kawsara
_copy khalideid
_copy lehmannhaupt
_copy lisakess
_copy madandmo
_copy massimos
_copy matthieu
_copy milosbic
_copy mimetic
_copy moslemregistry
_copy muslimcamp
_copy photoero
_copy storymad
_copy storystu
_copy thekelpf
_copy thenewre
_copy vanessaw
_copy wardiari




echo =========================================
echo END
echo =========================================

