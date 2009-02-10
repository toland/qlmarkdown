#include "markdown.h"
#include "discount/wrapper.h"

NSData* renderMarkdown(NSURL* url)
{
    NSString *source = [NSString stringWithContentsOfFile:[url path]];
    char *output = convert_markdown_to_string([source UTF8String]);
    NSData *data = [NSData dataWithBytes:output length:strlen(output)];
    
    return data;
}