. tests/functions.sh

rc=0
unset MARKDOWN_FLAGS
unset MKD_TABSTOP

eval `./markdown -V | tr ' ' '\n' | grep TAB`

if [ "${TAB:-4}" -eq 8 ]; then
    title "dealing with tabstop derangement"

    LIST='
 *  A
     *  B
	 *  C'

    try 'markdown with TAB=8' \
	"$LIST" \
	'<ul>
<li>A

<ul>
<li>B

<ul>
<li>C</li>
</ul>
</li>
</ul>
</li>
</ul>'

    try -F0x020000 'markdown with TAB=4' \
	"$LIST" \
	'<ul>
<li>A

<ul>
<li>B</li>
<li>C</li>
</ul>
</li>
</ul>'


summary $0
fi
exit $rc
