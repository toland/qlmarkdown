#include <stdio.h>
#include <stdlib.h>
#include "discount/markdown.h"

char* convert_markdown_to_string(const char *str)
{
    char *out = NULL;
    
    Document *blob = mkd_string((char *)str, strlen(str), 0);
    mkd_compile(blob, 0);
    int sz = mkd_document(blob, &out);
    
    if(sz == 0)
        return NULL;
    else
        out[sz - 1] = 0;
    
    return out;
}