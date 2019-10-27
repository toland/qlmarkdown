#include "discount-wrapper.h"

#include "markdown.h"

NSString *renderMarkdown(NSURL *url) {
  @autoreleasepool {
    NSString *domainName = @"com.fiatdev.QLMarkdown";

    NSString *styles = [[NSString alloc]
        initWithContentsOfFile:[[NSBundle bundleWithIdentifier:domainName] pathForResource:@"styles" ofType:@"css"]
                      encoding:NSUTF8StringEncoding
                         error:nil];

    NSStringEncoding usedEncoding = 0;
    NSError *e = nil;

    NSString *source = [[NSString alloc] initWithContentsOfURL:url usedEncoding:&usedEncoding error:&e];

    if (usedEncoding == 0) {
      NSLog(@"Wasn't able to determine encoding for file “%@”", [url path]);
    }

    char *output = convert_markdown_to_string([source UTF8String]);
    NSString *html = [NSString stringWithFormat:@"<!DOCTYPE html>\n"
                                                 "<html>\n"
                                                 "<head>\n"
                                                 "<meta charset=\"utf-8\">\n"
                                                 "<style type=\"text/css\">\n%@</style>\n"
                                                 "<base href=\"%@\"/>\n"
                                                 "</head>\n"
                                                 "<body>\n"
                                                 "%@"
                                                 "</body>\n"
                                                 "</html>",
                                                styles, url, [NSString stringWithUTF8String:output]];

//    free(output);

    return html;
  }
}
