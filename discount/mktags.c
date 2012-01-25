/* block-level tags for passing html blocks through the blender
 */
#include <stdio.h>

#define __WITHOUT_AMALLOC 1
#include "cstring.h"
#include "tags.h"

STRING(struct kw) blocktags;


/* define a html block tag
 */
static void
define_one_tag(char *id, int selfclose)
{
    struct kw *p = &EXPAND(blocktags);

    p->id = id;
    p->size = strlen(id);
    p->selfclose = selfclose;
}


/* case insensitive string sort (for qsort() and bsearch() of block tags)
 */
static int
casort(struct kw *a, struct kw *b)
{
    if ( a->size != b->size )
	return a->size - b->size;
    return strncasecmp(a->id, b->id, b->size);
}


/* stupid cast to make gcc shut up about the function types being
 * passed into qsort() and bsearch()
 */
typedef int (*stfu)(const void*,const void*);


/* load in the standard collection of html tags that markdown supports
 */
main()
{
    int i;

#define KW(x)	define_one_tag(x, 0)
#define SC(x)	define_one_tag(x, 1)

    KW("STYLE");
    KW("SCRIPT");
    KW("ADDRESS");
    KW("BDO");
    KW("BLOCKQUOTE");
    KW("CENTER");
    KW("DFN");
    KW("DIV");
    KW("OBJECT");
    KW("H1");
    KW("H2");
    KW("H3");
    KW("H4");
    KW("H5");
    KW("H6");
    KW("LISTING");
    KW("NOBR");
    KW("UL");
    KW("P");
    KW("OL");
    KW("DL");
    KW("PLAINTEXT");
    KW("PRE");
    KW("TABLE");
    KW("WBR");
    KW("XMP");
    SC("HR");
    KW("IFRAME");
    KW("MAP");

    qsort(T(blocktags), S(blocktags), sizeof(struct kw), (stfu)casort);

    printf("static struct kw blocktags[] = {\n");
    for (i=0; i < S(blocktags); i++)
	printf("   { \"%s\", %d, %d },\n", T(blocktags)[i].id, T(blocktags)[i].size, T(blocktags)[i].selfclose );
    printf("};\n\n");
    printf("#define NR_blocktags %d\n", S(blocktags));
    exit(0);
}
