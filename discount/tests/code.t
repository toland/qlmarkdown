. tests/functions.sh

title "code blocks"

rc=0
MARKDOWN_FLAGS=

try 'format for code block html' \
'    this is
    code' \
    '<pre><code>this is
code
</code></pre>'

try 'mismatched backticks' '```tick``' '<p><code>`tick</code></p>'
try 'mismatched backticks(2)' '``tick```' '<p>``tick```</p>'
try 'unclosed single backtick' '`hi there' '<p>`hi there</p>'
try 'unclosed double backtick' '``hi there' '<p>``hi there</p>'
try 'triple backticks' '```hi there```' '<p><code>hi there</code></p>'
try 'quadruple backticks' '````hi there````' '<p><code>hi there</code></p>'
try 'remove space around code' '`` hi there ``' '<p><code>hi there</code></p>'
try 'code containing backticks' '`` a```b ``' '<p><code>a```b</code></p>'
try 'backslash before backtick' '`a\`' '<p><code>a\</code></p>'
try '`>`' '`>`' '<p><code>&gt;</code></p>'
try '`` ` ``' '`` ` ``' '<p><code>`</code></p>'
try '````` ``` `' '````` ``` `' '<p><code>``</code> `</p>'
try '````` ` ```' '````` ` ```' '<p><code>`` `</code></p>'
try 'backslashes in code(1)' '    printf "%s: \n", $1;' \
'<pre><code>printf "%s: \n", $1;
</code></pre>'
try 'backslashes in code(2)' '`printf "%s: \n", $1;`' \
'<p><code>printf "%s: \n", $1;</code></p>'

if ./markdown -V | grep FENCED-CODE >/dev/null; then

try 'fenced code block' \
'~~~
code!
~~~' \
    '<pre><code>code!
</code></pre>'

try 'fenced code block in list' \
'1. ~~~
code block
~~~' \
'<ol>
<li><pre><code>code block
</code></pre></li>
</ol>'

try 'fenced code block in blockquote' \
'>~~~
code
~~~' \
'<blockquote><pre><code>code
</code></pre></blockquote>'

try 'unterminated fenced code block' \
'~~~
code' \
'<p>~~~
code</p>'

try 'fenced code block with tildes' \
'~~~~~
~~~
code with tildes
~~~
~~~~~' \
'<pre><code>~~~
code with tildes
~~~
</code></pre>'

try 'paragraph with trailing fenced block' \
'text text text
text text text
~~~
code code code?
~~~' \
'<p>text text text
text text text
~~~
code code code?
~~~</p>'

fi

summary $0
exit $rc
