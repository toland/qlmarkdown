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

/* prefixes for <automatic links>
 */
static char *autoprefix[] = { "http://", "https://", "ftp://", "news://" };
#define SZAUTOPREFIX	(sizeof autoprefix / sizeof autoprefix[0])

typedef int (*stfu)(const void*,const void*);


/* forward declarations */
static int iscodeblock(MMIOT*);
static void code(int, MMIOT*);
static void text(MMIOT *f);
static Paragraph *display(Paragraph*, MMIOT*);

/* externals from markdown.c */
int __mkd_footsort(Footnote *, Footnote *);

/*
 * push text into the generator input buffer
 */
static void
push(char *bfr, int size, MMIOT *f)
{
    while ( size-- > 0 )
	EXPAND(f->in) = *bfr++;
}


/* look <i> characters ahead of the cursor.
 */
static int
peek(MMIOT *f, int i)
{

    i += (f->isp-1);

    return (i >= 0) && (i < S(f->in)) ? T(f->in)[i] : EOF;
}


/* pull a byte from the input buffer
 */
static int
pull(MMIOT *f)
{
    return ( f->isp < S(f->in) ) ? T(f->in)[f->isp++] : EOF;
}


/* return a pointer to the current position in the input buffer.
 */
static char*
cursor(MMIOT *f)
{
    return T(f->in) + f->isp;
}


static int
isthisspace(MMIOT *f, int i)
{
    int c = peek(f, i);

    return isspace(c) || (c == EOF);
}


static int
isthisalnum(MMIOT *f, int i)
{
    int c = peek(f, i);

    return (c != EOF) && isalnum(c);
}


static int
isthisnonword(MMIOT *f, int i)
{
    return isthisspace(f, i) || ispunct(peek(f,i));
}


/* return/set the current cursor position
 */
#define mmiotseek(f,x)	(f->isp = x)
#define mmiottell(f)	(f->isp)


/* move n characters forward ( or -n characters backward) in the input buffer.
 */
static void
shift(MMIOT *f, int i)
{
    if (f->isp + i >= 0 )
	f->isp += i;
}


/* Qchar()
 */
static void
Qchar(char c, MMIOT *f)
{
    block *cur;
    
    if ( S(f->Q) == 0 ) {
	cur = &EXPAND(f->Q);
	memset(cur, 0, sizeof *cur);
	cur->b_type = bTEXT;
    }
    else
	cur = &T(f->Q)[S(f->Q)-1];

    EXPAND(cur->b_text) = c;
    
}


/* Qstring()
 */
static void
Qstring(char *s, MMIOT *f)
{
    while (*s)
	Qchar(*s++, f);
}


/* Qwrite()
 */
static void
Qwrite(char *s, int size, MMIOT *f)
{
    while (size-- > 0)
	Qchar(*s++, f);
}


/* Qprintf()
 */
static void
Qprintf(MMIOT *f, char *fmt, ...)
{
    char bfr[80];
    va_list ptr;

    va_start(ptr,fmt);
    vsnprintf(bfr, sizeof bfr, fmt, ptr);
    va_end(ptr);
    Qstring(bfr, f);
}


/* Qem()
 */
static void
Qem(MMIOT *f, char c, int count)
{
    block *p = &EXPAND(f->Q);

    memset(p, 0, sizeof *p);
    p->b_type = (c == '*') ? bSTAR : bUNDER;
    p->b_char = c;
    p->b_count = count;

    memset(&EXPAND(f->Q), 0, sizeof(block));
}


/* empair()
 */
static int
empair(MMIOT *f, int go, int level)
{
    
    int i;
    block *begin, *p;

    begin = &T(f->Q)[go];
    for (i=go+1; i < S(f->Q); i++) {
	p = &T(f->Q)[i];

	if ( (p->b_type != bTEXT) && (p->b_count <= 0) )
	    break;
	
	if ( p->b_type == begin->b_type ) {
	    if ( p->b_count == level )	/* exact match */
		return i-go;

	    if ( p->b_count > 2 )	/* fuzzy match */
		return i-go;
	}
    }
    return EOF;
}



static struct emtags {
    char open[10];
    char close[10];
    int size;
} emtags[] = {  { "<em>" , "</em>", 5 }, { "<strong>", "</strong>", 9 } };


static void
emclose(Cstring *s, int level)
{
    PREFIX(*s, emtags[level-1].close, emtags[level-1].size);
}


static void
emopen(Cstring *s, int level)
{
    SUFFIX(*s, emtags[level-1].open, emtags[level-1].size-1);
}


/* emmatch()
 */
static void
emmatch(MMIOT *f, int go)
{
    block *start = &T(f->Q)[go], *end;
    int e, e2, i, match;

    while ( start->b_count ) {
	switch (start->b_count) {
	case 2: e = empair(f,go,match=2);
		if ( e != EOF ) break;
	case 1: e = empair(f,go,match=1); break;
	default:
	    e = empair(f,go,1);
	    e2= empair(f,go,2);

	    if ( e == EOF || ((e2 != EOF) && (e2 >= e)) ) {
		e = e2;
		match = 2;
	    } 
	    else
		match = 1;
	}
	if ( e != EOF ) {
	    end = &T(f->Q)[go+e];
	    emclose(&end->b_post, match);
	    emopen(&start->b_text, match);
	    end->b_count -= match;
	}
	else {
	    for (i=0; i < match; i++)
		EXPAND(start->b_text) = start->b_char;
	}
	
	start->b_count -= match;
    }
}


/* emblock()
 */
static void
emblock(MMIOT *f)
{
    int i;
    block *p;

    for (i=0; i < S(f->Q); i++) {
	p = &T(f->Q)[i];
	
	if ( p->b_type != bTEXT ) emmatch(f, i);

	if ( S(p->b_post) ) { SUFFIX(f->out, T(p->b_post), S(p->b_post));
			      DELETE(p->b_post); }
	if ( S(p->b_text) ) { SUFFIX(f->out, T(p->b_text), S(p->b_text));
			      DELETE(p->b_text); }
    }
    S(f->Q) = 0;
}


/* generate html from a markup fragment
 */
static void
reparse(char *bfr, int size, int flags, MMIOT *f)
{
    MMIOT sub;

    ___mkd_initmmiot(&sub, f->footnotes);
    
    sub.flags = f->flags | flags;
    sub.base = f->base;

    push(bfr, size, &sub);
    EXPAND(sub.in) = 0;
    S(sub.in)--;
    
    text(&sub);
    emblock(&sub);
    
    Qwrite(T(sub.out), S(sub.out), f);

    ___mkd_freemmiot(&sub, f->footnotes);
}


/*
 * write out a url, escaping problematic characters
 */
static void
puturl(char *s, int size, MMIOT *f)
{
    unsigned char c;

    while ( size-- > 0 ) {
	c = *s++;

	if ( c == '&' )
	    Qstring("&amp;", f);
	else if ( c == '<' )
	    Qstring("&lt;", f);
	else if ( isalnum(c) || c == '.' || c == '-' || c == '_' || c == '/'
			     || c == '=' || c == '?' || c == ':' || c == '#' )
	    Qchar(c, f);
	else
	    Qprintf(f, "%%%02X", c);
    }
}


/* advance forward until the next character is not whitespace
 */
static int
eatspace(MMIOT *f)
{
    int c;

    for ( ; ((c=peek(f, 1)) != EOF) && isspace(c); pull(f) )
	;
    return c;
}


/* (match (a (nested (parenthetical (string.)))))
 */
static int
parenthetical(int in, int out, MMIOT *f)
{
    int size, indent, c;

    for ( indent=1,size=0; indent; size++ ) {
	if ( (c = pull(f)) == EOF )
	    return EOF;
	else if ( c == in )
	    ++indent;
	else if ( c == out )
	    --indent;
    }
    return size-1;
}


/* extract a []-delimited label from the input stream.
 */
static char *
linkylabel(MMIOT *f, int *sizep)
{
    char *ptr = cursor(f);

    if ( (*sizep = parenthetical('[',']',f)) != EOF )
	return ptr;
    return 0;
}


/* extract a (-prefixed url from the input stream.
 * the label is either of the format `<link>`, where I
 * extract until I find a >, or it is of the format
 * `text`, where I extract until I reach a ')' or
 * whitespace.
 */
static char*
linkyurl(MMIOT *f, int *sizep)
{
    int size = 0;
    char *ptr;
    int c;

    if ( (c = eatspace(f)) == EOF )
	return 0;

    ptr = cursor(f);

    if ( c == '<' ) {
	pull(f);
	ptr++;
	if ( (size = parenthetical('<', '>', f)) == EOF )
	    return 0;
    }
    else {
	for ( ; ((c=pull(f)) != ')') && !isspace(c); size++)
	    if ( c == EOF ) return 0;
	if ( c == ')' )
	    shift(f, -1);
    }
    *sizep = size;
    return ptr;
}


/* extract a =HHHxWWW size from the input stream
 */
static int
linkysize(MMIOT *f, int *heightp, int *widthp)
{
    int height=0, width=0;
    int c;

    *heightp = 0;
    *widthp = 0;

    if ( (c = eatspace(f)) != '=' ) 
	return (c != EOF);
    pull(f);	/* eat '=' */

    for ( c = pull(f); isdigit(c); c = pull(f))
	width = (width * 10) + (c - '0');

    if ( c == 'x' ) {
	for ( c = pull(f); isdigit(c); c = pull(f))
	    height = (height*10) + (c - '0');

	if ( c != EOF ) {
	    if ( !isspace(c) ) shift(f, -1);
	    *heightp = height;
	    *widthp = width;
	    return 1;
	}
    }
    return 0;
}


/* extract a )-terminated title from the input stream.
 */
static char*
linkytitle(MMIOT *f, int *sizep)
{
    int countq=0, qc, c, size;
    char *ret, *lastqc = 0;

    eatspace(f);
    if ( (qc=pull(f)) != '"' && qc != '\'' && qc != '(' )
	return 0;

    if ( qc == '(' ) qc = ')';

    for ( ret = cursor(f); (c = pull(f)) != EOF;  ) {
	if ( (c == ')') && countq ) {
	    size = (lastqc ? lastqc : cursor(f)) - ret;
	    *sizep = size-1;
	    return ret;
	}
	else if ( c == qc ) {
	    lastqc = cursor(f);
	    countq++;
	}
    }
    return 0;
}


/* look up (or construct) a footnote from the [xxx] link
 * at the head of the stream.
 */
static int
linkykey(int image, Footnote *val, MMIOT *f)
{
    Footnote *ret;
    Cstring mylabel;

    memset(val, 0, sizeof *val);

    if ( (T(val->tag) = linkylabel(f, &S(val->tag))) == 0 )
	return 0;

    eatspace(f);
    switch ( pull(f) ) {
    case '(':
	/* embedded link */
	if ( (T(val->link) = linkyurl(f,&S(val->link))) == 0 )
	    return 0;

	if ( image && !linkysize(f, &val->height, &val->width) )
	    return 0;

	T(val->title) = linkytitle(f, &S(val->title));

	return peek(f,0) == ')';

    case '[':
	/* footnote link */
	mylabel = val->tag;
	if ( (T(val->tag) = linkylabel(f, &S(val->tag))) == 0 )
	    return 0;

	if ( !S(val->tag) )
	    val->tag = mylabel;

	ret = bsearch(val, T(*f->footnotes), S(*f->footnotes),
	               sizeof *val, (stfu)__mkd_footsort);

	if ( ret ) {
	    val->tag = mylabel;
	    val->link = ret->link;
	    val->title = ret->title;
	    val->height = ret->height;
	    val->width = ret->width;
	    return 1;
	}
    }
    return 0;
}


/*
 * all the tag types that linkylinky can produce are
 * defined by this structure. 
 */
typedef struct linkytype {
    char      *pat;
    int      szpat;
    char *link_pfx;	/* tag prefix and link pointer  (eg: "<a href="\"" */
    char *link_sfx;	/* link suffix			(eg: "\""          */
    int        WxH;	/* this tag allows width x height arguments */
    char *text_pfx;	/* text prefix                  (eg: ">"           */
    char *text_sfx;	/* text suffix			(eg: "</a>"        */
    int      flags;	/* reparse flags */
} linkytype;

static linkytype imaget = { 0, 0, "<img src=\"", "\"",
			     1, " alt=\"", "\" />", DENY_IMG|INSIDE_TAG };
static linkytype linkt  = { 0, 0, "<a href=\"", "\"",
                             0, ">", "</a>", DENY_A };

/*
 * pseudo-protocols for [][];
 *
 * id: generates <a id="link">tag</a>
 * class: generates <span class="link">tag</span>
 * raw: just dump the link without any processing
 */
static linkytype specials[] = {
    { "id:", 3, "<a id=\"", "\"", 0, ">", "</a>", 0 },
    { "class:", 6, "<span class=\"", "\"", 0, ">", "</span>", 0 },
    { "raw:", 4, 0, 0, 0, 0, 0, 0 },
} ;

#define NR(x)	(sizeof x / sizeof x[0])

/* see if t contains one of our pseudo-protocols.
 */
static linkytype *
extratag(Cstring t)
{
    int i;
    linkytype *r;

    for ( i=0; i < NR(specials); i++ ) {
	r = &specials[i];
	if ( (S(t) > r->szpat) && (strncasecmp(T(t), r->pat, r->szpat) == 0) )
	    return r;
    }
    return 0;
}


/*
 * process embedded links and images
 */
static int
linkylinky(int image, MMIOT *f)
{
    int start = mmiottell(f);
    Footnote link;
    linkytype *tag;

    if ( !linkykey(image, &link, f) ) {
	mmiotseek(f, start);
	return 0;
    }

    if ( image )
	tag = &imaget;
    else if ( (f->flags & NO_PSEUDO_PROTO) || (tag = extratag(link.link)) == 0 )
	tag = &linkt;

    if ( f->flags & tag-> flags ) {
	mmiotseek(f, start);
	return 0;
    }

    if ( tag->link_pfx ) {
	Qstring(tag->link_pfx, f);
	if ( f->base && (T(link.link)[tag->szpat] == '/') )
	    puturl(f->base, strlen(f->base), f);
	puturl(T(link.link) + tag->szpat, S(link.link) - tag->szpat, f);
	Qstring(tag->link_sfx, f);

	if ( tag->WxH && link.height && link.width ) {
	    Qprintf(f," height=\"%d\"", link.height);
	    Qprintf(f, " width=\"%d\"", link.width);
	}

	if ( S(link.title) ) {
	    Qstring(" title=\"", f);
	    reparse(T(link.title), S(link.title), INSIDE_TAG, f);
	    Qchar('"', f);
	}

	Qstring(tag->text_pfx, f);
	reparse(T(link.tag), S(link.tag), tag->flags, f);
	Qstring(tag->text_sfx, f);
    }
    else
	Qwrite(T(link.link) + tag->szpat, S(link.link) - tag->szpat, f);

    return 1;
}


/* write a character to output, doing text escapes ( & -> &amp;,
 *                                          > -> &gt; < -> &lt; )
 */
static void
cputc(int c, MMIOT *f)
{
    switch (c) {
    case '&':   Qstring("&amp;", f); break;
    case '>':   Qstring("&gt;", f); break;
    case '<':   Qstring("&lt;", f); break;
    default :   Qchar(c, f); break;
    }
}

 
/*
 * convert an email address to a string of nonsense
 */
static void
mangle(char *s, int len, MMIOT *f)
{
    while ( len-- > 0 ) {
	Qstring("&#", f);
	Qprintf(f, COINTOSS() ? "x%02x;" : "%02d;", *((unsigned char*)(s++)) );
    }
}


/* before letting a tag through, validate against
 * DENY_A and DENY_IMG
 */
static int
forbidden_tag(MMIOT *f)
{
    int c = toupper(peek(f, 1));

    if ( f->flags & DENY_HTML )
	return 1;

    if ( c == 'A' && (f->flags & DENY_A) && !isthisalnum(f,2) )
	return 1;
    if ( c == 'I' && (f->flags & DENY_IMG)
		  && strncasecmp(cursor(f)+1, "MG", 2) == 0
		  && !isthisalnum(f,4) )
	return 1;
    return 0;
}



/* a < may be just a regular character, the start of an embedded html
 * tag, or the start of an <automatic link>.    If it's an automatic
 * link, we also need to know if it's an email address because if it
 * is we need to mangle it in our futile attempt to cut down on the
 * spaminess of the rendered page.
 */
static int
maybe_tag_or_link(MMIOT *f)
{
    char *text;
    int c, size, i;
    int maybetag=1, maybeaddress=0;
    int mailto;

    if ( f->flags & INSIDE_TAG )
	return 0;

    for ( size=0; ((c = peek(f,size+1)) != '>') && !isspace(c); size++ ) {
	if ( ! (c == '/' || isalnum(c) || c == '~') )
	    maybetag=0;
	if ( c == '@' )
	    maybeaddress=1;
	else if ( c == EOF )
	    return 0;
    }

    if ( size == 0 )
	return 0;

    if ( maybetag  || (size >= 3 && strncmp(cursor(f), "!--", 3) == 0) ) {
	Qstring(forbidden_tag(f) ? "&lt;" : "<", f);
	while ( ((c = peek(f, 1)) != EOF) && (c != '>') )
	    cputc(pull(f), f);
	return 1;
    }

    if ( f->flags & DENY_A ) return 0;

    text = cursor(f);
    shift(f, size+1);

    for ( i=0; i < SZAUTOPREFIX; i++ )
	if ( strncasecmp(text, autoprefix[i], strlen(autoprefix[i])) == 0 ) {
	    Qstring("<a href=\"", f);
	    puturl(text,size,f);
	    Qstring("\">", f);
	    puturl(text,size,f);
	    Qstring("</a>", f);
	    return 1;
	}
    if ( maybeaddress ) {

	Qstring("<a href=\"", f);
	if ( (size > 7) && strncasecmp(text, "mailto:", 7) == 0 )
	    mailto = 7;
	else {
	    mailto = 0;
	    /* supply a mailto: protocol if one wasn't attached */
	    mangle("mailto:", 7, f);
	}

	mangle(text, size, f);
	Qstring("\">", f);
	mangle(text+mailto, size-mailto, f);
	Qstring("</a>", f);
	return 1;
    }

    shift(f, -(size+1));
    return 0;
} /* maybe_tag_or_link */


/* smartyquote code that's common for single and double quotes
 */
static int
smartyquote(int *flags, char typeofquote, MMIOT *f)
{
    int bit = (typeofquote == 's') ? 0x01 : 0x02;

    if ( bit & (*flags) ) {
	if ( isthisnonword(f,1) ) {
	    Qprintf(f, "&r%cquo;", typeofquote);
	    (*flags) &= ~bit;
	    return 1;
	}
    }
    else if ( isthisnonword(f,-1) && peek(f,1) != EOF ) {
	Qprintf(f, "&l%cquo;", typeofquote);
	(*flags) |= bit;
	return 1;
    }
    return 0;
}


static int
islike(MMIOT *f, char *s)
{
    int len;
    int i;

    if ( s[0] == '<' ) {
	if ( !isthisnonword(f, -1) )
	    return 0;
       ++s;
    }

    if ( !(len = strlen(s)) )
	return 0;

    if ( s[len-1] == '>' ) {
	if ( !isthisnonword(f,len-1) )
	    return 0;
	len--;
    }

    for (i=1; i < len; i++)
	if (tolower(peek(f,i)) != s[i])
	    return 0;
    return 1;
}


static struct smarties {
    char c0;
    char *pat;
    char *entity;
    int shift;
} smarties[] = {
    { '\'', "'s>",      "rsquo",  0 },
    { '\'', "'t>",      "rsquo",  0 },
    { '-',  "--",       "mdash",  1 },
    { '-',  "<->",      "ndash",  0 },
    { '.',  "...",      "hellip", 2 },
    { '.',  ". . .",    "hellip", 4 },
    { '(',  "(c)",      "copy",   2 },
    { '(',  "(r)",      "reg",    2 },
    { '(',  "(tm)",     "trade",  3 },
    { '3',  "<3/4>",    "frac34", 2 },
    { '3',  "<3/4ths>", "frac34", 2 },
    { '1',  "<1/2>",    "frac12", 2 },
    { '1',  "<1/4>",    "frac14", 2 },
    { '1',  "<1/4th>",  "frac14", 2 },
    { '&',  "&#0;",      0,       3 },
} ;
#define NRSMART ( sizeof smarties / sizeof smarties[0] )


/* Smarty-pants-style chrome for quotes, -, ellipses, and (r)(c)(tm)
 */
static int
smartypants(int c, int *flags, MMIOT *f)
{
    int i;

    if ( f->flags & DENY_SMARTY )
	return 0;

    for ( i=0; i < NRSMART; i++)
	if ( (c == smarties[i].c0) && islike(f, smarties[i].pat) ) {
	    if ( smarties[i].entity )
		Qprintf(f, "&%s;", smarties[i].entity);
	    shift(f, smarties[i].shift);
	    return 1;
	}

    switch (c) {
    case '<' :  return 0;
    case '\'':  if ( smartyquote(flags, 's', f) ) return 1;
		break;

    case '"':	if ( smartyquote(flags, 'd', f) ) return 1;
		break;

    case '`':   if ( peek(f, 1) == '`' ) {
		    int j = 2;

		    while ( (c=peek(f,j)) != EOF ) {
			if ( c == '\\' )
			    j += 2;
			else if ( c == '`' )
			    break;
			else if ( c == '\'' && peek(f, j+1) == '\'' ) {
			    Qstring("&ldquo;", f);
			    reparse(cursor(f)+1, j-2, 0, f);
			    Qstring("&rdquo;", f);
			    shift(f,j+1);
			    return 1;
			}
			else ++j;
		    }

		}
		break;
    }
    return 0;
} /* smartypants */


#define tag_text(f)	(f->flags & INSIDE_TAG)


static void
text(MMIOT *f)
{
    int c, j;
    int rep;
    int smartyflags = 0;

    while ( (c = pull(f)) != EOF ) {
	if ( smartypants(c, &smartyflags, f) )
	    continue;
	switch (c) {
	case 0:     break;

	case '>':   if ( tag_text(f) )
			Qstring("&gt;", f);
		    else
			Qchar(c, f);
		    break;

	case '"':   if ( tag_text(f) )
			Qstring("&quot;", f);
		    else
			Qchar(c, f);
		    break;
			
	case '!':   if ( peek(f,1) == '[' ) {
			pull(f);
			if ( tag_text(f) || !linkylinky(1, f) )
			    Qstring("![", f);
		    }
		    else
			Qchar(c, f);
		    break;
	case '[':   if ( tag_text(f) || !linkylinky(0, f) )
			Qchar(c, f);
		    break;
#if SUPERSCRIPT
	/* A^B -> A<sup>B</sup> */
	case '^':   if ( (f->flags & (STRICT|INSIDE_TAG)) || isthisspace(f,-1) || isthisspace(f,1) )
			Qchar(c,f);
		    else {
			char *sup = cursor(f);
			int len = 0;
			Qstring("<sup>",f);
			while ( !isthisspace(f,1+len) ) {
			    ++len;
			}
			shift(f,len);
			reparse(sup, len, 0, f);
			Qstring("</sup>", f);
		    }
		    break;
#endif
	case '_':
#if RELAXED_EMPHASIS
	/* Underscores don't count if they're in the middle of a word */
		    if ( (!(f->flags & STRICT))
			     && ((isthisspace(f,-1) && isthisspace(f,1))
			      || (isthisalnum(f,-1) && isthisalnum(f,1))) ){
			Qchar(c, f);
			break;
		    }
		    /* else fall into the regular old emphasis case */
#endif
	case '*':   if ( tag_text(f) )
			Qchar(c, f);
		    else {
			for (rep = 1; peek(f,1) == c; pull(f) )
			    ++rep;
			Qem(f,c,rep);
		    }
		    break;
	
	case '`':   if ( tag_text(f) || !iscodeblock(f) )
			Qchar(c, f);
		    else {
			Qstring("<code>", f);
			if ( peek(f, 1) == '`' ) {
			    pull(f);
			    code(2, f);
			}
			else
			    code(1, f);
			Qstring("</code>", f);
		    }
		    break;

	case '\\':  switch ( c = pull(f) ) {
		    case '&':   Qstring("&amp;", f);
				break;
		    case '<':   Qstring("&lt;", f);
				break;
		    case '\\':
		    case '>': case '#': case '.': case '-':
		    case '+': case '{': case '}': case ']':
		    case '(': case ')': case '"': case '\'':
		    case '!': case '[': case '*': case '_':
		    case '`':	Qchar(c, f);
				break;
		    default:
				Qchar('\\', f);
				if ( c != EOF )
				    shift(f,-1);
				break;
		    }
		    break;

	case '<':   if ( !maybe_tag_or_link(f) )
			Qstring("&lt;", f);
		    break;

	case '&':   j = (peek(f,1) == '#' ) ? 2 : 1;
		    while ( isthisalnum(f,j) )
			++j;

		    if ( peek(f,j) != ';' )
			Qstring("&amp;", f);
		    else
			Qchar(c, f);
		    break;

	default:    Qchar(c, f);
		    break;
	}
    }
    /* truncate the input string after we've finished processing it */
    S(f->in) = f->isp = 0;
} /* text */


static int
iscodeblock(MMIOT *f)
{
    int i=1, single = 1, c;
    
    if ( peek(f,i) == '`' ) {
	single=0;
	i++;
    }
    while ( (c=peek(f,i)) != EOF ) {
	if ( (c == '`') && (single || peek(f,i+1) == '`') )
	    return 1;
	else if ( c == '\\' )
	    i++;
	i++;
    }
    return 0;
    
}

static int
endofcode(int escape, int offset, MMIOT *f)
{
    switch (escape) {
    case 2: if ( peek(f, offset+1) == '`' ) {
		shift(f,1);
    case 1:     shift(f,offset);
		return 1;
	    }
    default:return 0;
    }
}


/* the only characters that have special meaning in a code block are
 * `<' and `&' , which are /always/ expanded to &lt; and &amp;
 */
static void
code(int escape, MMIOT *f)
{
    int c;

    if ( escape && (peek(f,1) == ' ') )
	shift(f,1);

    while ( (c = pull(f)) != EOF ) {
	switch (c) {
	case ' ':   if ( peek(f,1) == '`' && endofcode(escape, 1, f) )
			return;
		    Qchar(c, f);
		    break;

	case '`':   if ( endofcode(escape, 0, f) )
			return;
		    Qchar(c, f);
		    break;

	case '\\':  cputc(c, f);
		    if ( peek(f,1) == '>' || (c = pull(f)) == EOF )
			break;
	
	default:    cputc(c, f);
		    break;
	}
    }
} /* code */


/* print a header block
 */
static void
printheader(Paragraph *pp, MMIOT *f)
{
    Qprintf(f, "<h%d", pp->hnumber);
    if ( f->flags & TOC ) {
	Qprintf(f, " id=\"", pp->hnumber);
	mkd_string_to_anchor(T(pp->text->text), S(pp->text->text), Qchar, f);
	Qchar('"', f);
    }
    Qchar('>', f);
    push(T(pp->text->text), S(pp->text->text), f);
    text(f);
    Qprintf(f, "</h%d>", pp->hnumber);
}


static int
printblock(Paragraph *pp, MMIOT *f)
{
    Line *t = pp->text;
    static char *Begin[] = { "", "<p>", "<center>"  };
    static char *End[]   = { "", "</p>","</center>" };

    while (t) {
	if ( S(t->text) ) {
	    if ( S(t->text) > 2 && T(t->text)[S(t->text)-2] == ' '
				&& T(t->text)[S(t->text)-1] == ' ') {
		push(T(t->text), S(t->text)-2, f);
		push("<br/>\n", 6, f);
	    }
	    else {
		push(T(t->text), S(t->text), f);
		if ( t->next )
		    push("\n", 1, f);
	    }
	}
	t = t->next;
    }
    Qstring(Begin[pp->align], f);
    text(f);
    Qstring(End[pp->align], f);
    return 1;
}


static void
printcode(Line *t, MMIOT *f)
{
    int blanks;

    for ( blanks = 0; t ; t = t->next )
	if ( S(t->text) > t->dle ) {
	    while ( blanks ) {
		push("\n", 1, f);
		--blanks;
	    }
	    push(T(t->text), S(t->text), f);
	    push("\n", 1, f);
	}
	else blanks++;

    Qstring("<pre><code>", f);
    code(0, f);
    Qstring("</code></pre>", f);
}


static void
printhtml(Line *t, MMIOT *f)
{
    int blanks;
    
    for ( blanks=0; t ; t = t->next )
	if ( S(t->text) ) {
	    for ( ; blanks; --blanks ) 
		Qchar('\n', f);

	    Qwrite(T(t->text), S(t->text), f);
	    Qchar('\n', f);
	}
	else
	    blanks++;
}


static void
htmlify(Paragraph *p, char *block, char *arguments, MMIOT *f)
{
    emblock(f);
    if ( block )
	Qprintf(f, arguments ? "<%s %s>" : "<%s>", block, arguments);
    emblock(f);

    while (( p = display(p, f) )) {
	emblock(f);
	Qstring("\n\n", f);
    }

    if ( block )
	 Qprintf(f, "</%s>", block);
    emblock(f);
}


#if DL_TAG_EXTENSION
static void
definitionlist(Paragraph *p, MMIOT *f)
{
    Line *tag;

    if ( p ) {
	Qstring("<dl>\n", f);

	for ( ; p ; p = p->next) {
	    for ( tag = p->text; tag; tag = tag->next ) {
		Qstring("<dt>", f);
		reparse(T(tag->text), S(tag->text), 0, f);
		Qstring("</dt>\n", f);
	    }

	    htmlify(p->down, "dd", p->ident, f);
	}

	Qstring("</dl>", f);
    }
}
#endif


static void
listdisplay(int typ, Paragraph *p, MMIOT* f)
{
    if ( p ) {
	Qprintf(f, "<%cl", (typ==UL)?'u':'o');
	if ( typ == AL )
	    Qprintf(f, " type=a");
	Qprintf(f, ">\n");

	for ( ; p ; p = p->next ) {
	    htmlify(p->down, "li", p->ident, f);
	    Qchar('\n', f);
	}

	Qprintf(f, "</%cl>\n", (typ==UL)?'u':'o');
    }
}


/* dump out a Paragraph in the desired manner
 */
static Paragraph*
display(Paragraph *p, MMIOT *f)
{
    if ( !p ) return 0;
    
    switch ( p->typ ) {
    case STYLE:
    case WHITESPACE:
	break;

    case HTML:
	printhtml(p->text, f);
	break;
	
    case CODE:
	printcode(p->text, f);
	break;
	
    case QUOTE:
	htmlify(p->down, p->ident ? "div" : "blockquote", p->ident, f);
	break;
	
    case UL:
    case OL:
    case AL:
	listdisplay(p->typ, p->down, f);
	break;

#if DL_TAG_EXTENSION
    case DL:
	definitionlist(p->down, f);
	break;
#endif

    case HR:
	Qstring("<hr />", f);
	break;

    case HDR:
	printheader(p, f);
	break;

    default:
	printblock(p, f);
	break;
    }
    return p->next;
}


/*
 * dump out stylesheet sections.
 */
static int
stylesheets(Paragraph *p, FILE *f)
{
    Line* q;

    for ( ; p ; p = p->next ) {
	if ( p->typ == STYLE ) {
	    for ( q = p->text; q ; q = q->next )
		if ( fwrite(T(q->text), S(q->text), 1, f) == 1 )
		    putc('\n', f);
		else
		    return EOF;
	}
	if ( p->down && (stylesheets(p->down, f) == EOF) )
	    return EOF;
    }
    return 0;
}


/* return a pointer to the compiled markdown
 * document.
 */
int
mkd_document(Document *p, char **res)
{
    if ( p && p->compiled ) {
	if ( ! p->html ) {
	    htmlify(p->code, 0, 0, p->ctx);
	    p->html = 1;
	}

	*res = T(p->ctx->out);
	return S(p->ctx->out);
    }
    return EOF;
}


/*  public interface for reparse()
 */
int
mkd_text(char *bfr, int size, FILE *output, int flags)
{
    MMIOT f;

    ___mkd_initmmiot(&f, 0);
    f.flags = flags & USER_FLAGS;
    
    reparse(bfr, size, 0, &f);
    emblock(&f);
    if ( flags & CDATA_OUTPUT )
	___mkd_xml(T(f.out), S(f.out), output);
    else
	fwrite(T(f.out), S(f.out), 1, output);

    ___mkd_freemmiot(&f, 0);
    return 0;
}


/* dump any embedded styles
 */
int
mkd_style(Document *d, FILE *f)
{
    if ( d && d->compiled )
	return stylesheets(d->code, f);
    return EOF;
}

