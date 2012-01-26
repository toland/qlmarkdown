. tests/functions.sh

title "styles"

rc=0
MARKDOWN_FLAGS=

try 'single line' '<style> ul {display:none;} </style>' ''

ASK='<style>
ul {display:none;}
</style>'

try 'multiple lines' "$ASK" ''

try 'unclosed' '<style>foo' '<p><style>foo</p>'

UNCLOSED='<style>
text

text'

RESULT='<p><style>
text</p>

<p>text</p>'


try 'multiple lines unclosed' "$UNCLOSED" "$RESULT"

try -fnohtml 'unclosed with -fnohtml' '<style>foo' '<p>&lt;style>foo</p>'

summary $0
exit $rc
