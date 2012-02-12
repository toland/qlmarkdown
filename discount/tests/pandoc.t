. tests/functions.sh

title "pandoc headers"

rc=0
MARKDOWN_FLAGS=

HEADER='% title
% author(s)
% date'


try 'valid header' "$HEADER" ''
try -F0x00010000 'valid header with -F0x00010000' "$HEADER" '<p>% title
% author(s)
% date</p>'

try 'invalid header' \
	'% title
% author(s)
a pony!' \
	'<p>% title
% author(s)
a pony!</p>'

try 'offset header' \
	'
% title
% author(s)
% date' \
	'<p>% title
% author(s)
% date</p>'

try 'indented header' \
	'  % title
% author(s)
% date' \
	'<p>  % title
% author(s)
% date</p>'

summary $0
exit $rc
