#include "markdown.h"
#include "discount-wrapper.h"

NSData* renderMarkdown(NSURL* url)
{
    NSString *styles = [NSString stringWithContentsOfFile:[[NSBundle bundleWithIdentifier: @"com.fiatdev.QLMarkdown"]
                                                           pathForResource:@"styles" ofType:@"css"]
                                                 encoding:NSUTF8StringEncoding
                                                    error:nil];

    NSStringEncoding usedEncoding = 0;
    NSError *e = nil;

    NSString *source = [NSString stringWithContentsOfURL:url usedEncoding:&usedEncoding error:&e];

    if (usedEncoding == 0) {
        NSLog(@"Wasn't able to determine encoding for file “%@”", [url path]);
    }

    char *output = convert_markdown_to_string([source UTF8String]);
    NSString *html = [NSString stringWithFormat:@"<!DOCTYPE html>"
                                                 "<meta charset=utf-8>"
                                                 "<style>%@</style>"
                                                 "<base href=\"%@\"/>"
                                                 "%@",
                                                 styles, url, [NSString stringWithUTF8String:output]];

    free(output);
    return [html dataUsingEncoding:NSUTF8StringEncoding];
}