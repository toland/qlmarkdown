/*
 * makepage: Use mkd_xhtmlpage() to convert markdown input to a
 *           fully-formed xhtml page.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <mkdio.h>
#include "config.h"
#include "pgm_options.h"

#ifndef HAVE_BASENAME
char*
basename(char *p)
{
    char *ret = strrchr(p, '/');

    return ret ? (1+ret) : p;
}
#endif

char *pgm = "makepage";

main(argc, argv)
int argc;
char **argv;
{
    MMIOT *doc;
    char *q;
    int version = 0;
    int opt;
    mkd_flag_t flags = 0;

    if ( (q = getenv("MARKDOWN_FLAGS")) )
	flags = strtol(q, 0, 0);

    opterr = 1;

    while ( (opt=getopt(argc, argv, "F:f:V")) != EOF ) {
	switch (opt) {
	case 'V':   version++;
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
			fprintf(stderr, "unknown option <%s>\n", optarg);
		    break;
	default:    fprintf(stderr, "usage: %s [-V] [-F bitmap] [-f {+-}flags]"
				    " [file]\n", pgm);
		    exit(1);
	}
    }
    
    argc -= optind;
    argv += optind;
    
    if ( version ) {
	printf("%s: discount %s", pgm, markdown_version);
	if ( version > 1 )
	    mkd_flags_are(stdout, flags, 0);
	putchar('\n');
	exit(0);
    }    
    
    if ( (argc > 1) && !freopen(argv[1], "r", stdin) ) {
	perror(argv[1]);
	exit(1);
    }

    if ( (doc = mkd_in(stdin, flags)) == 0 ) {
	perror( (argc > 1) ? argv[1] : "stdin" );
	exit(1);
    }

    exit(mkd_xhtmlpage(doc, flags, stdout));
}
