/* markdown: a C implementation of John Gruber's Markdown markup language.
 *
 * Copyright (C) 2007 David L Parsons.
 * The redistribution terms are provided in the COPYRIGHT file that must
 * be distributed with this source code.
 */
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>

#include "config.h"

#include "cstring.h"
#include "markdown.h"
#include "amalloc.h"

/* block-level tags for passing html blocks through the blender
 */
struct kw {
    char *id;
    int  siz;
} ;

#define KW(x)	{ x, sizeof(x)-1 }

static struct kw blocktags[] = { KW("!--"), KW("STYLE"), KW("SCRIPT"),
				 KW("ADDRESS"), KW("BDO"), KW("BLOCKQUOTE"),
				 KW("CENTER"), KW("DFN"), KW("DIV"), KW("H1"),
				 KW("H2"), KW("H3"), KW("H4"), KW("H5"),
				 KW("H6"), KW("LISTING"), KW("NOBR"),
				 KW("UL"), KW("P"), KW("OL"), KW("DL"),
				 KW("PLAINTEXT"), KW("PRE"), KW("TABLE"),
				 KW("WBR"), KW("XMP"), KW("HR"), KW("BR") };
#define SZTAGS	(sizeof blocktags / sizeof blocktags[0])
#define MAXTAG	11 /* sizeof "BLOCKQUOTE" */

typedef int (*stfu)(const void*,const void*);

typedef ANCHOR(Paragraph) ParagraphRoot;


/* case insensitive string sort (for qsort() and bsearch() of block tags)
 */
static int
casort(struct kw *a, struct kw *b)
{
    if ( a->siz != b->siz )
	return a->siz - b->siz;
    return strncasecmp(a->id, b->id, b->siz);
}


/* case insensitive string sort for Footnote tags.
 */
int
__mkd_footsort(Footnote *a, Footnote *b)
{
    int i;
    char ac, bc;

    if ( S(a->tag) != S(b->tag) )
	return S(a->tag) - S(b->tag);

    for ( i=0; i < S(a->tag); i++) {
	ac = tolower(T(a->tag)[i]);
	bc = tolower(T(b->tag)[i]);

	if ( isspace(ac) && isspace(bc) )
	    continue;
	if ( ac != bc )
	    return ac - bc;
    }
    return 0;
}


/* find the first blank character after position <i>
 */
static int
nextblank(Line *t, int i)
{
    while ( (i < S(t->text)) && !isspace(T(t->text)[i]) )
	++i;
    return i;
}


/* find the next nonblank character after position <i>
 */
static int
nextnonblank(Line *t, int i)
{
    while ( (i < S(t->text)) && isspace(T(t->text)[i]) )
	++i;
    return i;
}


/* find the first nonblank character on the Line.
 */
int
mkd_firstnonblank(Line *p)
{
    return nextnonblank(p,0);
}


static int
blankline(Line *p)
{
    return ! (p && (S(p->text) > p->dle) );
}


static Line *
skipempty(Line *p)
{
    while ( p && (p->dle == S(p->text)) )
	p = p->next;
    return p;
}


static char *
isopentag(Line *p)
{
    int i=0, len;
    struct kw key, *ret;

    if ( !p ) return 0;

    len = S(p->text);

    if ( len < 3 || T(p->text)[0] != '<' )
	return 0;

    /* find how long the tag is so we can check to see if
     * it's a block-level tag
     */
    for ( i=1; i < len && T(p->text)[i] != '>' 
		       && T(p->text)[i] != '/'
		       && !isspace(T(p->text)[i]); ++i )
	;

    key.id = T(p->text)+1;
    key.siz = i-1;
    
    if ( ret = bsearch(&key,blocktags,SZTAGS,sizeof key, (stfu)casort))
	return ret->id;

    return 0;
}


static int
selfclose(Line *t, char *tag)
{
    char *q = T(t->text);
    int siz = strlen(tag);
    int i;

    if ( strcasecmp(tag, "HR") == 0 || strcasecmp(tag, "BR") == 0 )
	/* <HR> and <BR> are self-closing block-level tags,
	 */
	return 1;

    i = S(t->text) - (siz + 3);

    /* we specialcase start and end tags on the same line.
     */
    return ( i > 0 ) && (q[i] == '<') && (q[i+1] == '/')
		     && (q[i+2+siz] == '>')
		     && (strncasecmp(&q[i+2], tag, siz) == 0);
}


static Line *
htmlblock(Paragraph *p, char *tag)
{
    Line *t = p->text, *ret;
    int closesize;
    char close[MAXTAG+4];

    if ( selfclose(t, tag) || (strlen(tag) >= MAXTAG) ) {
	ret = t->next;
	t->next = 0;
	return ret;
    }

    closesize = sprintf(close, "</%s>", tag);

    for ( ; t ; t = t->next) {
	if ( strncasecmp(T(t->text), close, closesize) == 0 ) {
	    ret = t->next;
	    t->next = 0;
	    return ret;
	}
    }
    return 0;
}


static Line *
comment(Paragraph *p, char *key)
{
    Line *t, *ret;

    for ( t = p->text; t ; t = t->next) {
	if ( strstr(T(t->text), "-->") ) {
	    ret = t->next;
	    t->next = 0;
	    return ret;
	}
    }
    return t;

}


/* footnotes look like ^<whitespace>{0,3}[stuff]: <content>$
 */
static int
isfootnote(Line *t)
{
    int i;

    if ( ( (i = t->dle) > 3) || (T(t->text)[i] != '[') )
	return 0;

    for ( ++i; i < S(t->text) ; ++i ) {
	if ( T(t->text)[i] == '[' )
	    return 0;
	else if ( T(t->text)[i] == ']' && T(t->text)[i+1] == ':' )
	    return 1;
    }
    return 0;
}


static int
isquote(Line *t)
{
    return ( T(t->text)[0] == '>' );
}


static int
dashchar(char c)
{
    return (c == '*') || (c == '-') || (c == '_');
}


static int
iscode(Line *t)
{
    return (t->dle >= 4);
}


static int
ishr(Line *t)
{
    int i, count=0;
    char dash = 0;
    char c;

    if ( iscode(t) ) return 0;

    for ( i = 0; i < S(t->text); i++) {
	c = T(t->text)[i];
	if ( (dash == 0) && dashchar(c) )
	    dash = c;

	if ( c == dash ) ++count;
	else if ( !isspace(c) )
	    return 0;
    }
    return (count >= 3);
}


static int
ishdr(Line *t, int *htyp)
{
    int i;


    /* first check for etx-style ###HEADER###
     */

    /* leading run of `#`'s ?
     */
    for ( i=0; T(t->text)[i] == '#'; ++i)
	;

    /* ANY leading `#`'s make this into an ETX header
     */
    if ( i ) {
	*htyp = ETX;
	return 1;
    }

    /* then check for setext-style HEADER
     *                             ======
     */

    if ( t->next ) {
	char *q = T(t->next->text);

	if ( (*q == '=') || (*q == '-') ) {
	    for (i=1; i < S(t->next->text); i++)
		if ( q[0] != q[i] )
		    return 0;
	    *htyp = SETEXT;
	    return 1;
	}
    }
    return 0;
}


static int
isdefinition(Line *t)
{
#if DL_TAG_EXTENSION
    return t && t->next
	     && (S(t->text) > 2)
	     && (t->dle == 0)
	     && (T(t->text)[0] == '=')
	     && (T(t->text)[S(t->text)-1] == '=')
	     && ( (t->next->dle >= 4) || isdefinition(t->next) );
#else
    return 0;
#endif
}


static int
islist(Line *t, int *trim)
{
    int i, j;
    char *q;
    
    if ( iscode(t) || blankline(t) || ishdr(t,&i) || ishr(t) )
	return 0;

    if ( isdefinition(t) ) {
	*trim = 4;
	return DL;
    }

    if ( strchr("*-+", T(t->text)[t->dle]) && isspace(T(t->text)[t->dle+1]) ) {
	i = nextnonblank(t, t->dle+1);
	*trim = (i > 4) ? 4 : i;
	return UL;
    }

    if ( (j = nextblank(t,t->dle)) > t->dle ) {
	if ( T(t->text)[j-1] == '.' ) {
#if ALPHA_LIST
	if ( (j == t->dle + 2) && isalpha(T(t->text)[t->dle]) ) {
	    j = nextnonblank(t,j);
	    *trim = j;
	    return AL;
	}
#endif
	    strtoul(T(t->text)+t->dle, &q, 10);
	    if ( (q > T(t->text)+t->dle) && (q == T(t->text) + (j-1)) ) {
		j = nextnonblank(t,j);
		*trim = j;
		return OL;
	    }
	}
    }
    return 0;
}


static Line *
headerblock(Paragraph *pp, int htyp)
{
    Line *ret = 0;
    Line *p = pp->text;
    int i, j;

    switch (htyp) {
    case SETEXT:
	    /* p->text is header, p->next->text is -'s or ='s
	     */
	    pp->hnumber = (T(p->next->text)[0] == '=') ? 1 : 2;
	    
	    ret = p->next->next;
	    ___mkd_freeLine(p->next);
	    p->next = 0;
	    break;

    case ETX:
	    /* p->text is ###header###, so we need to trim off
	     * the leading and trailing `#`'s
	     */

	    for (i=0; (T(p->text)[i] == T(p->text)[0]) && (i < S(p->text)-1); i++)
		;

	    pp->hnumber = i;

	    while ( (i < S(p->text)) && isspace(T(p->text)[i]) )
		++i;

	    CLIP(p->text, 0, i);

	    for (j=S(p->text); (j > 1) && (T(p->text)[j-1] == '#'); --j)
		;

	    while ( j && isspace(T(p->text)[j-1]) )
		--j;

	    S(p->text) = j;

	    ret = p->next;
	    p->next = 0;
	    break;
    }
    return ret;
}


static Line *
codeblock(Paragraph *p)
{
    Line *t = p->text, *r;

    /* HORRIBLE STANDARDS KLUDGE: the first line of every block
     * has trailing whitespace trimmed off.
     */
    while ( S(t->text) && isspace(T(t->text)[S(t->text)-1]) )
	--S(t->text);

    for ( ; t; t = r ) {
	CLIP(t->text,0,4);
	t->dle = mkd_firstnonblank(t);

	if ( !( (r = skipempty(t->next)) && iscode(r)) ) {
	    ___mkd_freeLineRange(t,r);
	    t->next = 0;
	    return r;
	}
    }
    return t;
}


static int
centered(Line *first, Line *last)
{

    if ( first&&last ) {
	int len = S(last->text);

	if ( (len > 2) && (strncmp(T(first->text), "->", 2) == 0)
		       && (strncmp(T(last->text)+len-2, "<-", 2) == 0) ) {
	    CLIP(first->text, 0, 2);
	    S(last->text) -= 2;
	    return CENTER;
	}
    }
    return 0;
}


static int
endoftextblock(Line *t, int toplevelblock)
{
    int z;

    if ( blankline(t)||isquote(t)||iscode(t)||ishdr(t,&z)||ishr(t) )
	return 1;

    /* HORRIBLE STANDARDS KLUDGE: Toplevel paragraphs eat absorb adjacent
     * list items, but sublevel blocks behave properly.
     */
    return toplevelblock ? 0 : islist(t,&z);
}


static Line *
textblock(Paragraph *p, int toplevel)
{
    Line *t, *next;

    for ( t = p->text; t ; t = next )
	if ( ((next = t->next) == 0) || endoftextblock(next, toplevel) ) {
	    p->align = centered(p->text, t);
	    t->next = 0;
	    return next;
	}
    return t;
}


/* length of the id: or class: kind in a special div-not-quote block
 */
static int
szmarkerclass(char *p)
{
    if ( strncasecmp(p, "id:", 3) == 0 )
	return 3;
    if ( strncasecmp(p, "class:", 6) == 0 )
	return 6;
    return 0;
}


/*
 * check if the first line of a quoted block is the special div-not-quote
 * marker %[kind:]name%
 */
static int
isdivmarker(Line *p)
{
#if DIV_QUOTE
    char *s = T(p->text);
    int len = S(p->text);
    int i;

    if ( !(len && s[0] == '%' && s[len-1] == '%') ) return 0;

    i = szmarkerclass(s+1);
    --len;

    while ( ++i < len )
	if ( !isalnum(s[i]) )
	    return 0;

    return 1;
#else
    return 0;
#endif
}


/*
 * accumulate a blockquote.
 *
 * one sick horrible thing about blockquotes is that even though
 * it just takes ^> to start a quote, following lines, if quoted,
 * assume that the prefix is ``>''.   This means that code needs
 * to be indented *5* spaces from the leading '>', but *4* spaces
 * from the start of the line.   This does not appear to be 
 * documented in the reference implementation, but it's the
 * way the markdown sample web form at Daring Fireball works.
 */
static Line *
quoteblock(Paragraph *p)
{
    Line *t, *q;
    int qp;

    for ( t = p->text; t ; t = q ) {
	if ( isquote(t) ) {
	    qp = (T(t->text)[1] == ' ') ? 2 : 1;
	    CLIP(t->text, 0, qp);
	    t->dle = mkd_firstnonblank(t);
	}

	if ( !(q = skipempty(t->next)) || ((q != t->next) && !isquote(q)) ) {
	    ___mkd_freeLineRange(t, q);
	    t = q;
	    break;
	}
    }
    if ( isdivmarker(p->text) ) {
	char *prefix = "class";
	int i;
	
	q = p->text;
	p->text = p->text->next;

	if ( (i = szmarkerclass(1+T(q->text))) == 3 )
	    /* and this would be an "%id:" prefix */
	    prefix="id";
	    
	if ( p->ident = malloc(4+i+S(q->text)) )
	    sprintf(p->ident, "%s=\"%.*s\"", prefix, S(q->text)-(i+2),
						     T(q->text)+(i+1) );

	___mkd_freeLine(q);
    }
    return t;
}


static Paragraph *Pp(ParagraphRoot *, Line *, int);
static Paragraph *compile(Line *, int, MMIOT *);


/*
 * pull in a list block.  A list block starts with a list marker and
 * runs until the next list marker, the next non-indented paragraph,
 * or EOF.   You do not have to indent nonblank lines after the list
 * marker, but multiple paragraphs need to start with a 4-space indent.
 */
static Line *
listitem(Paragraph *p, int indent)
{
    Line *t, *q;
    int clip = indent;
    int z;

    for ( t = p->text; t ; t = q) {
	CLIP(t->text, 0, clip);
	t->dle = mkd_firstnonblank(t);

	if ( (q = skipempty(t->next)) == 0 ) {
	    ___mkd_freeLineRange(t,q);
	    return 0;
	}

	/* after a blank line, the next block needs to start with a line
	 * that's indented 4 spaces, but after that the line doesn't
	 * need any indentation
	 */
	if ( q != t->next ) {
	    if (q->dle < 4) {
		q = t->next;
		t->next = 0;
		return q;
	    }
	    indent = 4;
	}

	if ( (q->dle < indent) && (ishr(q) || islist(q,&z)) && !ishdr(q,&z) ) {
	    q = t->next;
	    t->next = 0;
	    return q;
	}

	clip = (q->dle > indent) ? indent : q->dle;
    }
    return t;
}


static Line *
listblock(Paragraph *top, int trim, MMIOT *f)
{
    ParagraphRoot d = { 0, 0 };
    Paragraph *p;
    Line *q = top->text, *text;
    Line *label;
    int para = 0;

    while (( text = q )) {
	if ( top->typ == DL ) {
	    Line *lp;

	    for ( lp = label = text; lp ; lp = lp->next ) {
		text = lp->next;
		CLIP(lp->text, 0, 1);
		S(lp->text)--;
		if ( !isdefinition(lp->next) )
		    lp->next = 0;
	    }
	}
	else label = 0;

	p = Pp(&d, text, LISTITEM);
	text = listitem(p, trim);

	p->down = compile(p->text, 0, f);
	p->text = label;

	if ( para && (top->typ != DL) && p->down ) p->down->align = PARA;

	if ( !(q = skipempty(text)) || (islist(q, &trim) == 0) )
	    break;

	if ( para = (q != text) ) {
	    Line anchor;

	    anchor.next = text;
	    ___mkd_freeLineRange(&anchor, q);
	}

	if ( para && (top->typ != DL) && p->down ) p->down->align = PARA;
    }
    top->text = 0;
    top->down = T(d);
    return text;
}


static int
tgood(char c)
{
    switch (c) {
    case '\'':
    case '"': return c;
    case '(': return ')';
    }
    return 0;
}


/*
 * add a new (image or link) footnote to the footnote table
 */
static Line*
addfootnote(Line *p, MMIOT* f)
{
    int j, i;
    int c;
    Line *np = p->next;

    Footnote *foot = &EXPAND(*f->footnotes);
    
    CREATE(foot->tag);
    CREATE(foot->link);
    CREATE(foot->title);
    foot->height = foot->width = 0;

    for (j=i=p->dle+1; T(p->text)[j] != ']'; j++)
	EXPAND(foot->tag) = T(p->text)[j];

    EXPAND(foot->tag) = 0;
    S(foot->tag)--;
    j = nextnonblank(p, j+2);

    while ( (j < S(p->text)) && !isspace(T(p->text)[j]) )
	EXPAND(foot->link) = T(p->text)[j++];
    EXPAND(foot->link) = 0;
    S(foot->link)--;
    j = nextnonblank(p,j);

    if ( T(p->text)[j] == '=' ) {
	sscanf(T(p->text)+j, "=%dx%d", &foot->width, &foot->height);
	while ( (j < S(p->text)) && !isspace(T(p->text)[j]) )
	    ++j;
	j = nextnonblank(p,j);
    }


    if ( (j >= S(p->text)) && np && np->dle && tgood(T(np->text)[np->dle]) ) {
	___mkd_freeLine(p);
	p = np;
	np = p->next;
	j = p->dle;
    }

    if ( (c = tgood(T(p->text)[j])) ) {
	/* Try to take the rest of the line as a comment; read to
	 * EOL, then shrink the string back to before the final
	 * quote.
	 */
	++j;	/* skip leading quote */

	while ( j < S(p->text) )
	    EXPAND(foot->title) = T(p->text)[j++];

	while ( S(foot->title) && T(foot->title)[S(foot->title)-1] != c )
	    --S(foot->title);
	if ( S(foot->title) )	/* skip trailing quote */
	    --S(foot->title);
	EXPAND(foot->title) = 0;
	--S(foot->title);
    }

    ___mkd_freeLine(p);
    return np;
}


/*
 * allocate a paragraph header, link it to the
 * tail of the current document
 */
static Paragraph *
Pp(ParagraphRoot *d, Line *ptr, int typ)
{
    Paragraph *ret = calloc(sizeof *ret, 1);

    ret->text = ptr;
    ret->typ = typ;

    return ATTACH(*d, ret);
}



static Line*
consume(Line *ptr, int *eaten)
{
    Line *next;
    int blanks=0;

    for (; ptr && blankline(ptr); ptr = next, blanks++ ) {
	next = ptr->next;
	___mkd_freeLine(ptr);
    }
    if ( ptr ) *eaten = blanks;
    return ptr;
}


/*
 * break a collection of markdown input into
 * blocks of lists, code, html, and text to
 * be marked up.
 */
static Paragraph *
compile(Line *ptr, int toplevel, MMIOT *f)
{
    ParagraphRoot d = { 0, 0 };
    Paragraph *p = 0;
    char *key;
    Line *r;
    int para = toplevel;
    int hdr_type, list_type, indent;

    ptr = consume(ptr, &para);

    while ( ptr ) {
	if ( toplevel && !(f->flags & DENY_HTML) && (key = isopentag(ptr)) ) {
	    p = Pp(&d, ptr, strcmp(key, "STYLE") == 0 ? STYLE : HTML);
	    if ( strcmp(key, "!--") == 0 )
		ptr = comment(p, key);
	    else
		ptr = htmlblock(p, key);
	}
	else if ( iscode(ptr) ) {
	    p = Pp(&d, ptr, CODE);
	    ptr = codeblock(p);
	}
	else if ( ishr(ptr) ) {
	    p = Pp(&d, 0, HR);
	    r = ptr;
	    ptr = ptr->next;
	    ___mkd_freeLine(r);
	}
	else if (( list_type = islist(ptr, &indent) )) {
	    p = Pp(&d, ptr, list_type);
	    ptr = listblock(p, indent, f);
	}
	else if ( isquote(ptr) ) {
	    p = Pp(&d, ptr, QUOTE);
	    ptr = quoteblock(p);
	    p->down = compile(p->text, 1, f);
	    p->text = 0;
	}
	else if ( ishdr(ptr, &hdr_type) ) {
	    p = Pp(&d, ptr, HDR);
	    ptr = headerblock(p, hdr_type);
	}
	else if ( toplevel && (isfootnote(ptr)) ) {
	    ptr = consume(addfootnote(ptr, f), &para);
	    continue;
	}
	else {
	    p = Pp(&d, ptr, MARKUP);
	    ptr = textblock(p, toplevel);
	}

	if ( (para||toplevel) && !p->align )
	    p->align = PARA;

	para = toplevel;
	ptr = consume(ptr, &para);

	if ( para && !p->align )
	    p->align = PARA;

    }
    return T(d);
}


static void
initialize()
{
    static int first = 1;

    if ( first-- > 0 ) {
	first = 0;
	INITRNG(time(0));
	qsort(blocktags, SZTAGS, sizeof blocktags[0], (stfu)casort);
    }
}


/*
 * the guts of the markdown() function, ripped out so I can do
 * debugging.
 */

/*
 * prepare and compile `text`, returning a Paragraph tree.
 */
int
mkd_compile(Document *doc, int flags)
{
    if ( !doc )
	return 0;

    if ( doc->compiled )
	return 1;

    doc->compiled = 1;
    memset(doc->ctx, 0, sizeof(MMIOT) );
    doc->ctx->flags = flags & USER_FLAGS;
    doc->ctx->base = doc->base;
    CREATE(doc->ctx->in);
    doc->ctx->footnotes = malloc(sizeof doc->ctx->footnotes[0]);
    CREATE(*doc->ctx->footnotes);

    initialize();

    doc->code = compile(T(doc->content), 1, doc->ctx);
    qsort(T(*doc->ctx->footnotes), S(*doc->ctx->footnotes),
		        sizeof T(*doc->ctx->footnotes)[0],
			           (stfu)__mkd_footsort);
    memset(&doc->content, 0, sizeof doc->content);
    return 1;
}

