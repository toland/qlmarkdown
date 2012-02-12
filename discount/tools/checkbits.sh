#! /bin/sh

trap "rm -f in.markdown.h in.mkdio.h" EXIT

grep '#define MKD_' markdown.h | awk '$3 ~ /0x0/ {print $2,$3;}' > in.markdown.h
grep '#define MKD_' mkdio.h | awk '$3 ~ /0x0/ {print $2,$3;}' > in.mkdio.h

diff -c -bw in.markdown.h in.mkdio.h
retcode=$?

exit $retcode
