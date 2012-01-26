. tests/functions.sh

title "smarty pants"

rc=0
MARKDOWN_FLAGS=0x0; export MARKDOWN_FLAGS

try '(c) -> &copy;' '(c)' '<p>&copy;</p>'
try '(r) -> &reg;' '(r)' '<p>&reg;</p>'
try '(tm) -> &trade;' '(tm)' '<p>&trade;</p>'
try '... -> &hellip;' '...' '<p>&hellip;</p>'

try '"?--" -> &ndash;' '?--' '<p>?&ndash;</p>'
try '"?---" -> &mdash;' '?---' '<p>?&mdash;</p>'

try '"--" -> &ndash;' 'regular --' '<p>regular &ndash;</p>'
try 'A-B -> A-B' 'A-B' '<p>A-B</p>'
try '"fancy" -> &ldquo;fancy&rdquo;' '"fancy"' '<p>&ldquo;fancy&rdquo;</p>'
try "'fancy'" "'fancy'" '<p>&lsquo;fancy&rsquo;</p>'
try "don<b>'t -> don<b>&rsquo;t" "don<b>'t" '<p>don<b>&rsquo;t</p>'
try "don't -> don&rsquo;t" "don't" '<p>don&rsquo;t</p>'
try "it's -> it&rsquo;s" "it's" '<p>it&rsquo;s</p>'

summary $0
exit $rc
