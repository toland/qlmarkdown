. tests/functions.sh

title "reported defects"

rc=0
MARKDOWN_FLAGS=

try 'masses of non-block html' \
'<span>foo</span><br>
<br>
<span>bar</span><br>' \
'<p><span>foo</span><br>
<br>
<span>bar</span><br></p>'

summary $0
exit $rc
