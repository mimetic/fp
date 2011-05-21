#!/bin/bash

# This is a SH script, not TCSH
# It works on my servers

# Load up your directories...
FPDirs[0]="girlswho"
FPDirs[1]="mimetic"
FPDirs[2]="alpsime"
FPDirs[3]="atelius"
FPDirs[4]="caroline"
FPDirs[5]="dgphoto"
FPDirs[6]="frontlin"
FPDirs[7]="milosbic"
FPDirs[8]="shahrzad"
FPDirs[9]="justmiel"
FPDirs[10]="georgege"
FPDirs[11]="vanessaw"
FPDirs[12]="lisakess"

# add more if needbe
# MediaTombDirectories[2]="/home/me/files"

# Setup find correctly.
export IFS=$'\n'

echo -------------
echo This script repairs permissions in the _themes directories

# Loop through our array.
for x in ${FPDirs[@]}
	do
		# Find all directories & subdirectories
		for i in $(find /home/$x/public_html/_themes -type d)
			do
				# Fix Permissions
				chmod -c 755 $i
				#chown -c me:user $i
			done

		# Find all Files
		for i in $(find /home/$x/public_html/_themes -type f)
			do
				# Fix Permissions
				chmod -c 644 $i
				#chown -c me:user $i
			done
	done

echo -------------
