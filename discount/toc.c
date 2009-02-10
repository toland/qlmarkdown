/*
 * toc -- spit out a table of contents based on header blocks
 *
 * Copyright (C) 2008 Jjgod Jiang, David L Parsons.
 * The redistribution terms are provided in the COPYRIGHT file that must
 * be distributed with this source code.
 */
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "cstring.h"
#include "markdown.h"
#include "amalloc.h"

/* write an header index
 */
int
mkd_generatetoc(Document *p, FILE *output)
{
    Paragraph *pp;

    int last_hnumber = 0,
	first_hnumber = 0;

    if ( !(p && p->ctx) ) return -1;
    if ( ! (p->ctx->flags & TOC) ) return 0;

    for ( pp = p->code; pp ; pp = pp->next ) {
        if ( pp->typ == HDR && pp->text ) {
	    
	    if ( last_hnumber == pp->hnumber )
		fprintf(output, "%*s</li>\n", pp->hnumber, "");
	    else while ( last_hnumber > pp->hnumber ) {
		fprintf(output, "%*s</li>\n%*s</ul>\n",
				 last_hnumber, "",
				 last_hnumber-1,"");
		--last_hnumber;
	    }
	    
	    while ( pp->hnumber > last_hnumber ) {
		fprintf(output, "\n%*s<ul>\n", pp->hnumber, "");
		++last_hnumber;
	    }
	    fprintf(output, "%*s<li><a href=\"#", pp->hnumber, "");
	    mkd_string_to_anchor(T(pp->text->text), S(pp->text->text), putc, output);
	    fprintf(output, "\">");
	    mkd_text(T(pp->text->text), S(pp->text->text), output, 0);
	    fprintf(output, "</a>");
        }
    }

    while ( last_hnumber > 0 ) {
        fprintf(output, "%*s</li>\n%*s</ul>\n",
			last_hnumber, "", last_hnumber, "");
	--last_hnumber;
    }

    return 0;
}

