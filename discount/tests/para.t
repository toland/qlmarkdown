. tests/functions.sh

title "paragraph blocking"

rc=0
MARKDOWN_FLAGS=

try 'paragraph followed by code' \
    'a
    b' \
    '<p>a
    b</p>'

try 'single-line paragraph' 'a' '<p>a</p>'

summary $0
exit $rc
