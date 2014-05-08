#!/bin/bash

status_msg () {
	if [ -t FD ] ; then
		echo -e "\033[1m$1\033[0m"
	else
		echo "$1"
	fi
}

error_msg () {
	if [ -t FD ] ; then
		echo -e "\033[31m$1\033[0m" >&2
	else
		echo "$1" >&2
	fi

	tput sgr0
}

# CD into script dir -1
cd "$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"/..

# Checkout the discount module if necessary
if [[ ! -a $PWD/discount/configure.sh ]] ; then
	status_msg "Updating the discount directory..."
	git submodule update --init
fi


status_msg "Running configure.sh..."
cd discount
./configure.sh --with-fenced-code

# make the blocktags
make blocktags

status_msg "Copying important files..."

if head -n 1 config.h | grep -q "^/\*$"; then
	# remove generated comments in config.h
	sed '1,/^ *\*\/ *$/ { d; }' <config.h >../discount-config/config.h && echo 'config.h'
else
	cp config.h ../discount-config/config.h && echo 'config.h'
	error_msg "Can't locate config.h comments!"
	error_msg "Check the diff before committing (and fix this script if you can)"
fi
cp mkdio.h ../discount-config/mkdio.h && echo 'mkdio.h'
cp blocktags ../discount-config/blocktags && echo 'blocktags'

status_msg "Clean files from working directory..."

# clean the working directory of generated files and folders
git clean -f -d

status_msg "Done!"
