#include "markdown.h"

NSData* renderMarkdown(NSURL* url)
{
	NSBundle* bundle = [NSBundle bundleWithIdentifier: @"com.fiatdev.QLMarkdown"];

    NSTask* task = [[NSTask alloc] init];
    [task setLaunchPath: [bundle pathForResource: @"Markdown" ofType: @"pl"]];
    [task setArguments: [NSArray arrayWithObjects: [url path], nil]];

    NSPipe* pipe = [NSPipe pipe];
    [task setStandardOutput: pipe];

    [task launch];

    return [[pipe fileHandleForReading] readDataToEndOfFile];
}