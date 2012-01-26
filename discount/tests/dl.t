. tests/functions.sh

title "definition lists"

eval `./markdown -V | tr ' ' '\n' | grep '^DL='`

DL=${DL:-BOTH}

rc=0
MARKDOWN_FLAGS=

SRC='
=this=
    is an ugly
=test=
    eh?'

RSLT='<dl>
<dt>this</dt>
<dd>is an ugly</dd>
<dt>test</dt>
<dd>eh?</dd>
</dl>'

if [ "$DL" = "DISCOUNT" -o "$DL" = "BOTH" ]; then
    try -fdefinitionlist '=tag= generates definition lists' "$SRC" "$RSLT"

	try 'one item with two =tags=' \
	    '=this=
=is=
    A test, eh?' \
	    '<dl>
<dt>this</dt>
<dt>is</dt>
<dd>A test, eh?</dd>
</dl>'


	try -fnodefinitionlist '=tag= does nothing' "$SRC" \
	    '<p>=this=
    is an ugly
=test=
    eh?</p>'
fi

if [ "$DL" = "EXTRA" -o "$DL" = "BOTH" ]; then
    try 'markdown extra-style definition lists' \
'foo
: bar' \
'<dl>
<dt>foo</dt>
<dd>bar</dd>
</dl>'

    try '... with two <dt>s in a row' \
'foo
bar
: baz' \
'<dl>
<dt>foo</dt>
<dt>bar</dt>
<dd>baz</dd>
</dl>'

    try '... with two <dd>s in a row' \
'foo
: bar
: baz' \
'<dl>
<dt>foo</dt>
<dd>bar</dd>
<dd>baz</dd>
</dl>'

    try '... with blanks between list items' \
'foo
: bar

zip
: zap' \
'<dl>
<dt>foo</dt>
<dd>bar</dd>
<dt>zip</dt>
<dd>zap</dd>
</dl>'

fi

summary $0
exit $rc
