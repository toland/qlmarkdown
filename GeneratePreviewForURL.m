#import <Cocoa/Cocoa.h>
#import <CoreFoundation/CoreFoundation.h>
#import <CoreServices/CoreServices.h>
#import <Foundation/Foundation.h>
#import <QuickLook/QuickLook.h>

#import "markdown.h"

OSStatus GeneratePreviewForURL(void *thisInterface, QLPreviewRequestRef preview, CFURLRef url,
                               CFStringRef contentTypeUTI, CFDictionaryRef options);
void CancelPreviewGeneration(void *thisInterface, QLPreviewRequestRef preview);

/* -----------------------------------------------------------------------------
   Generate a preview for file
   This function's job is to create preview for designated file
   -----------------------------------------------------------------------------
 */
OSStatus GeneratePreviewForURL(void *thisInterface, QLPreviewRequestRef preview, CFURLRef url,
                               CFStringRef contentTypeUTI, CFDictionaryRef options) {

  NSString *content = renderMarkdown((__bridge NSURL *)url);

  CFDictionaryRef previewProperties = (__bridge CFDictionaryRef) @{
    (__bridge NSString *)kQLPreviewPropertyTextEncodingNameKey : @"UTF-8",
    (__bridge NSString *)kQLPreviewPropertyMIMETypeKey : @"text/html",
  };

  QLPreviewRequestSetDataRepresentation(preview, (__bridge CFDataRef)[content dataUsingEncoding:NSUTF8StringEncoding],
                                        kUTTypeHTML, previewProperties);

  return noErr;
}

void CancelPreviewGeneration(void *thisInterface, QLPreviewRequestRef preview) {
  // Implement only if supported
}
