#import <Cocoa/Cocoa.h>
#import <CoreFoundation/CoreFoundation.h>
#import <CoreServices/CoreServices.h>
#import <Foundation/Foundation.h>
#import <QuickLook/QuickLook.h>
#import <WebKit/WebKit.h>

OSStatus GenerateThumbnailForURL(void *thisInterface, QLThumbnailRequestRef thumbnail, CFURLRef url,
                                 CFStringRef contentTypeUTI, CFDictionaryRef options, CGSize maxSize);
void CancelThumbnailGeneration(void *thisInterface, QLThumbnailRequestRef thumbnail);

/* -----------------------------------------------------------------------------
 Generate a thumbnail for file
 This function's job is to create thumbnail for designated file as fast as possible
 ----------------------------------------------------------------------------- */

OSStatus GenerateThumbnailForURL(void *thisInterface, QLThumbnailRequestRef thumbnail, CFURLRef url,
                                 CFStringRef contentTypeUTI, CFDictionaryRef options, CGSize maxSize) {

  NSString *content = [NSString stringWithContentsOfURL:(__bridge NSURL *)url encoding:NSUTF8StringEncoding error:nil];

  if (content) {
    // Encapsulate the content of the .strings file in HTML
    NSData *data = [content dataUsingEncoding:NSUTF8StringEncoding];

    NSRect _rect = NSMakeRect(0.0, 0.0, 600.0, 800.0);
    float _scale = maxSize.height / 800.0;
    NSSize _scaleSize = NSMakeSize(_scale, _scale);
    CGSize _thumbSize = NSSizeToCGSize(NSMakeSize((maxSize.width * (600.0 / 800.0)), maxSize.height));

    // Create the webview to display the thumbnail
    WebView *_webView = [[WebView alloc] initWithFrame:_rect];
    [_webView scaleUnitSquareToSize:_scaleSize];
    [_webView.mainFrame.frameView setAllowsScrolling:NO];
    [_webView.mainFrame loadData:data MIMEType:@"text/html" textEncodingName:@"utf-8" baseURL:nil];

    while ([_webView isLoading])
      CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0, true);
    [_webView display];

    // Draw the webview in the correct context
    CGContextRef _context = QLThumbnailRequestCreateContext(thumbnail, _thumbSize, false, NULL);
    if (_context) {
      NSGraphicsContext *_graphicsContext = [NSGraphicsContext graphicsContextWithGraphicsPort:(void *)_context
                                                                                       flipped:_webView.isFlipped];
      [_webView displayRectIgnoringOpacity:_webView.bounds inContext:_graphicsContext];

      QLThumbnailRequestFlushContext(thumbnail, _context);

      CFRelease(_context);
    }
  }

  return noErr;
}

void CancelThumbnailGeneration(void *thisInterface, QLThumbnailRequestRef thumbnail) {
  // implement only if supported
}
