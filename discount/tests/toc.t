. tests/functions.sh


rc=0
MARKDOWN_FLAGS=

if ./markdown -V | grep ID-ANCHOR >/dev/null; then
    # old-style; uses id= tag (and collides
    # with #-style css)

    title "(old) table-of-contents support"
    
    try '-T -ftoc' 'table of contents' \
    '#H1
hi' \
'<ul>
 <li><a href="#H1">H1</a></li>
</ul>
<h1 id="H1">H1</h1>

<p>hi</p>'

    try '-T -ftoc' 'toc item with link' \
    '##[H2](H2) here' \
'<ul>
 <li>
 <ul>
  <li><a href="#H2.here">H2 here</a></li>
 </ul>
 </li>
</ul>
<h2 id="H2.here"><a href="H2">H2</a> here</h2>'  

    try '-T -ftoc' 'toc item with non-alpha start' \
    '#1 header' \
'<ul>
 <li><a href="#L1.header">1 header</a></li>
</ul>
<h1 id="L1.header">1 header</h1>'

else
    # new-style; uses a (depreciated) name=
    # inside a null <a> tag
    
    title "(new) table-of-contents support"
    
    try '-T -ftoc' 'table of contents' \
    '#H1
hi' \
'<ul>
 <li><a href="#H1">H1</a></li>
</ul>
<a name="H1"></a>
<h1>H1</h1>

<p>hi</p>'

    try '-T -ftoc' 'toc item with link' \
    '##[H2](H2) here' \
'<ul>
 <li>
 <ul>
  <li><a href="#H2.here">H2 here</a></li>
 </ul>
 </li>
</ul>
<a name="H2.here"></a>
<h2><a href="H2">H2</a> here</h2>'  

    try '-T -ftoc' 'toc item with non-alpha start' \
    '#1 header' \
'<ul>
 <li><a href="#L1.header">1 header</a></li>
</ul>
<a name="L1.header"></a>
<h1>1 header</h1>'
fi

summary $0
exit $rc
