/* markdown: a C implementation of John Gruber's Markdown markup language.
 *
 * Copyright (C) 2007-2011 David L Parsons.
 * The redistribution terms are provided in the COPYRIGHT file that must
 * be distributed with this source code.
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <mkdio.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>

#include "config.h"
#include "amalloc.h"

#if HAVE_LIBGEN_H
#include <libgen.h>
#endif

static struct _opt {
    char *name;
    char *desc;
    int off;
    int skip;
    int sayenable;
    mkd_flag_t flag;
} opts[] = {
    { "tabstop",       "default (4-space) tabstops", 0, 0, 1, MKD_TABSTOP  },
    { "image",         "images",                     1, 0, 1, MKD_NOIMAGE  },
    { "links",         "links",                      1, 0, 1, MKD_NOLINKS  },
    { "relax",         "emphasis inside words",      1, 1, 1, MKD_STRICT   },
    { "strict",        "emphasis inside words",      0, 0, 1, MKD_STRICT   },
    { "tables",        "tables",                     1, 0, 1, MKD_NOTABLES },
    { "header",        "pandoc-style headers",       1, 0, 1, MKD_NOHEADER },
    { "html",          "raw html",                   1, 0, 1, MKD_NOHTML   },
    { "ext",           "extended protocols",         1, 0, 1, MKD_NO_EXT   },
    { "cdata",         "generate cdata",             0, 0, 0, MKD_CDATA    },
    { "smarty",        "smartypants",                1, 0, 1, MKD_NOPANTS  },
    { "pants",         "smartypants",                1, 1, 1, MKD_NOPANTS  },
    { "toc",           "tables of contents",         0, 0, 1, MKD_TOC      },
    { "autolink",      "autolinking",                0, 0, 1, MKD_AUTOLINK },
    { "safelink",      "safe links",                 0, 0, 1, MKD_SAFELINK },
    { "strikethrough", "strikethrough",              1, 0, 1, MKD_NOSTRIKETHROUGH },
    { "del",           "strikethrough",              1, 1, 1, MKD_NOSTRIKETHROUGH },
    { "superscript",   "superscript",                1, 0, 1, MKD_NOSUPERSCRIPT },
    { "emphasis",      "emphasis inside words",      0, 0, 1, MKD_NORELAXED },
    { "divquote",      ">%class% blockquotes",       1, 0, 1, MKD_NODIVQUOTE },
    { "alphalist",     "alpha lists",                1, 0, 1, MKD_NOALPHALIST },
    { "definitionlist","definition lists",           1, 0, 1, MKD_NODLIST },
    { "1.0",           "markdown 1.0 compatibility", 0, 0, 1, MKD_1_COMPAT },
    { "footnotes",     "markdown extra footnotes",   0, 0, 1, MKD_EXTRA_FOOTNOTE },
    { "footnote",      "markdown extra footnotes",   0, 1, 1, MKD_EXTRA_FOOTNOTE },
} ;

#define NR(x)	(sizeof x / sizeof x[0])


typedef int (*stfu)(const void *, const void *);

int
sort_by_name(struct _opt *a, struct _opt *b)
{
    return strcmp(a->name,b->name);
}

int
sort_by_flag(struct _opt *a, struct _opt *b)
{
    return a->flag - b->flag;
}


void
show_flags(int byname)
{
    int i;

    if ( byname ) {
	qsort(opts, NR(opts), sizeof(opts[0]), (stfu)sort_by_name);
    
	for (i=0; i < NR(opts); i++)
	    if ( ! opts[i].skip )
		fprintf(stderr, "%16s : %s\n", opts[i].name, opts[i].desc);
    }
    else {
	qsort(opts, NR(opts), sizeof(opts[0]), (stfu)sort_by_flag);
	
	for (i=0; i < NR(opts); i++)
	    if ( ! opts[i].skip ) {
		fprintf(stderr, "%08lx : ", (long)opts[i].flag);
		if ( opts[i].sayenable )
		    fprintf(stderr, opts[i].off ? "disable " : "enable ");
		fprintf(stderr, "%s\n", opts[i].desc);
	    }
    }
}
    

int
set_flag(mkd_flag_t *flags, char *optionstring)
{
    int i;
    int enable;
    char *arg;

    for ( arg = strtok(optionstring, ","); arg; arg = strtok(NULL, ",") ) {
	if ( *arg == '+' || *arg == '-' )
	    enable = (*arg++ == '+') ? 1 : 0;
	else if ( strncasecmp(arg, "no", 2) == 0 ) {
	    arg += 2;
	    enable = 0;
	}
	else
	    enable = 1;

	for ( i=0; i < NR(opts); i++ )
	    if ( strcasecmp(arg, opts[i].name) == 0 )
		break;

	if ( i < NR(opts) ) {
	    if ( opts[i].off )
		enable = !enable;
		
	    if ( enable )
		*flags |= opts[i].flag;
	    else
		*flags &= ~opts[i].flag;
	}
	else
	    return 0;
    }
    return 1;
}
