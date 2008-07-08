#import <QuickLook/QuickLook.h>
#import <Cocoa/Cocoa.h>
#include "markdown.h"

/* -----------------------------------------------------------------------------
   Generate a preview for file

   This function's job is to create preview for designated file
   -------------------------------------------------------------------------- */

OSStatus GeneratePreviewForURL(void *thisInterface, QLPreviewRequestRef preview,
							   CFURLRef url, CFStringRef contentTypeUTI, 
							   CFDictionaryRef options)
{
    CFDataRef data = (CFDataRef) renderMarkdown((NSURL*) url);

    if (data) {
		CFDictionaryRef props = (CFDictionaryRef) [NSDictionary dictionary];
        QLPreviewRequestSetDataRepresentation(preview, data, kUTTypeHTML, props);
    }

    return noErr;
}

void CancelPreviewGeneration(void* thisInterface, QLPreviewRequestRef preview)
{
    // implement only if supported
}
