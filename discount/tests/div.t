. tests/functions.sh

title "%div% blocks"

rc=0
MARKDOWN_FLAGS=

try 'simple >%div% block' \
'>%this%
this this' \
'<div class="this"><p>this this</p></div>'

try 'two >%div% blocks in a row' \
'>%this%
this this

>%that%
that that' \
'<div class="this"><p>this this</p></div>

<div class="that"><p>that that</p></div>'

try '>%class:div%' \
'>%class:this%
this this' \
'<div class="this"><p>this this</p></div>'

try '>%id:div%' \
'>%id:this%
this this' \
'<div id="this"><p>this this</p></div>'

try 'nested >%div%' \
'>%this%
>>%that%
>>that

>%more%
more' \
'<div class="this"><div class="that"><p>that</p></div></div>

<div class="more"><p>more</p></div>'

try '%class% with _' '>%class:this_that%' '<div class="this_that"></div>'
try '%class% with -' '>%class:this-that%' '<div class="this-that"></div>'
try 'illegal %class%' '>%class:0zip%' '<blockquote><p>%class:0zip%</p></blockquote>'

summary $0
exit $rc
