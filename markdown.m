#include "markdown.h"
#include "discount/wrapper.h"

NSData* renderMarkdown(NSURL* url)
{
    NSString *styles = [NSString stringWithContentsOfFile:[[NSBundle bundleWithIdentifier: @"com.fiatdev.QLMarkdown"]
                                                           pathForResource:@"styles" ofType:@"css"]];
    
    NSString *source = [NSString stringWithContentsOfFile:[url path] encoding:NSUTF8StringEncoding error:nil];
    char *output = convert_markdown_to_string([source UTF8String]);
    NSString *html = [NSString stringWithFormat:@"<html>"
                                                 "<head>"
                                                 "<meta content='text/html; charset=UTF-8' http-equiv='Content-Type' />"
                                                 "<style type=\"text/css\">%@</style>"
                                                 "</head>"
                                                 "<body>%@</body>"
                                                 "</html>", 
                                                 styles, [NSString stringWithUTF8String:output]];
    
    free(output);
    return [html dataUsingEncoding:NSUTF8StringEncoding];
}