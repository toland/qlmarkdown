/*
 * markdown: convert a single markdown document into html
 */
/*
 * Copyright (C) 2007 David L Parsons.
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
#include "pgm_options.h"
#include "tags.h"

#if HAVE_LIBGEN_H
#include <libgen.h>
#endif

#ifndef HAVE_BASENAME
#include <string.h>

char*
basename(char *p)
{
    char *ret = strrchr(p, '/');

    return ret ? (1+ret) : p;
}
#endif


char *pgm = "markdown";

char *
e_flags(const char *text, const int size, void *context)
{
    return (char*)context;
}


void
complain(char *fmt, ...)
{
    va_list ptr;

    fprintf(stderr, "%s: ", pgm);
    va_start(ptr, fmt);
    vfprintf(stderr, fmt, ptr);
    va_end(ptr);
    fputc('\n', stderr);
    fflush(stderr);
}


main(int argc, char **argv)
{
    int opt;
    int rc;
    mkd_flag_t flags = 0;
    int debug = 0;
    int toc = 0;
    int version = 0;
    int with_html5 = 0;
    int use_mkd_line = 0;
    char *extra_footnote_prefix = 0;
    char *urlflags = 0;
    char *text = 0;
    char *ofile = 0;
    char *urlbase = 0;
    char *q;
    MMIOT *doc;

    if ( q = getenv("MARKDOWN_FLAGS") )
	flags = strtol(q, 0, 0);

    pgm = basename(argv[0]);
    opterr = 1;

    while ( (opt=getopt(argc, argv, "5b:C:df:E:F:o:s:t:TV")) != EOF ) {
	switch (opt) {
	case '5':   with_html5 = 1;
		    break;
	case 'b':   urlbase = optarg;
		    break;
	case 'd':   debug = 1;
		    break;
	case 'V':   version++;
		    break;
	case 'E':   urlflags = optarg;
		    break;
	case 'F':   if ( strcmp(optarg, "?") == 0 ) {
			show_flags(0);
			exit(0);
		    }
		    else
			flags = strtol(optarg, 0, 0);
		    break;
	case 'f':   if ( strcmp(optarg, "?") == 0 ) {
			show_flags(1);
			exit(0);
		    }
		    else if ( !set_flag(&flags, optarg) )
			complain("unknown option <%s>", optarg);
		    break;
	case 't':   text = optarg;
		    use_mkd_line = 1;
		    break;
	case 'T':   toc = 1;
		    break;
	case 's':   text = optarg;
		    break;
	case 'C':   extra_footnote_prefix = optarg;
		    break;
	case 'o':   if ( ofile ) {
			complain("Too many -o options");
			exit(1);
		    }
		    if ( !freopen(ofile = optarg, "w", stdout) ) {
			perror(ofile);
			exit(1);
		    }
		    break;
	default:    fprintf(stderr, "usage: %s [-dTV] [-b url-base]"
				    " [-F bitmap] [-f {+-}flags]"
				    " [-o ofile] [-s text]"
				    " [-t text] [file]\n", pgm);
		    exit(1);
	}
    }

    if ( version ) {
	printf("%s: discount %s%s", pgm, markdown_version,
				  with_html5 ? " +html5":"");
	if ( version > 1 )
	    mkd_flags_are(stdout, flags, 0);
	putchar('\n');
	exit(0);
    }

    argc -= optind;
    argv += optind;

    if ( with_html5 )
	mkd_with_html5_tags();

    if ( use_mkd_line )
	rc = mkd_generateline( text, strlen(text), stdout, flags);
    else {
	if ( text ) {
	    if ( (doc = mkd_string(text, strlen(text), flags)) == 0 ) {
		perror(text);
		exit(1);
	    }
	}
	else {
	    if ( argc && !freopen(argv[0], "r", stdin) ) {
		perror(argv[0]);
		exit(1);
	    }
	    if ( (doc = mkd_in(stdin,flags)) == 0 ) {
		perror(argc ? argv[0] : "stdin");
		exit(1);
	    }
	}
	if ( urlbase )
	    mkd_basename(doc, urlbase);
	if ( urlflags ) {
	    mkd_e_data(doc, urlflags);
	    mkd_e_flags(doc, e_flags);
	}
	if ( extra_footnote_prefix )
	    mkd_ref_prefix(doc, extra_footnote_prefix);

	if ( debug )
	    rc = mkd_dump(doc, stdout, 0, argc ? basename(argv[0]) : "stdin");
	else {
	    rc = 1;
	    if ( mkd_compile(doc, flags) ) {
		rc = 0;
		if ( toc )
		    mkd_generatetoc(doc, stdout);
		mkd_generatehtml(doc, stdout);
		mkd_cleanup(doc);
	    }
	}
    }
    mkd_deallocate_tags();
    adump();
    exit( (rc == 0) ? 0 : errno );
}
