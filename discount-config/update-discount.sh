#!/bin/bash
status_msg () {
echo -e "\033[1m$1\033[0m"
}

error_msg () {
echo -e "\033[31m$1\033[0m" >&2
tput sgr0
}

status_msg "Removing discount folder..."
rm -rf ../discount/

status_msg "Cloning discount ..."
git clone git://github.com/Orc/discount.git ../discount

./update.sh