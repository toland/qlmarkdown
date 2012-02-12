/*
 * docheader -- get values from the document header
 *
 * Copyright (C) 2007 David L Parsons.
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

static char *
onlyifset(Line *l)
{
    char *ret = T(l->text) + l->dle;

    return ret[0] ? ret : 0;
}

char *
mkd_doc_title(Document *doc)
{
    if ( doc && doc->title )
	return onlyifset(doc->title);
    return 0;
}


char *
mkd_doc_author(Document *doc)
{
    if ( doc && doc->author )
	return onlyifset(doc->author);
    return 0;
}


char *
mkd_doc_date(Document *doc)
{
    if ( doc && doc->date )
	return onlyifset(doc->date);
    return 0;
}
