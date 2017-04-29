#include "markdown.h"
#include "discount-wrapper.h"

NSData* renderMarkdown(NSURL* url)
{
    @autoreleasepool {
        NSString *domainName = @"com.fiatdev.QLMarkdown";
        
        // command line switch theme
        NSDictionary *defaults = [[NSUserDefaults standardUserDefaults] persistentDomainForName:domainName];
        NSString *styleName = [defaults valueForKey:@"theme"];
        
        if (styleName == nil || [[styleName lowercaseString] isEqual:@"default"]) {
            styleName = @"github";
        }
        NSString *styles = [[NSString alloc] initWithContentsOfFile:[[NSBundle bundleWithIdentifier:domainName]
                                                                     pathForResource:styleName ofType:@"css"]
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
                          "<style>\n%@</style>\n"
                          "<base href=\"%@\"/>\n"
                          "</head>\n"
                          "<body>\n"
                          "%@"
                          "</body>\n"
                          "</html>",
                          styles, url, [NSString stringWithUTF8String:output]];
        
        free(output);
        return [html dataUsingEncoding:NSUTF8StringEncoding];
    }
}
