//
//  html2md.m
//  html2md
//
//  Created by 秋星桥 on 2/17/25.
//

#import <Foundation/Foundation.h>

#include "html2md.h"
#include "include/html2md_objc.h"

#include <string>

@implementation HTML2MD

+ (NSString *)convertHTMLToMarkdown:(NSString *)html {
    const char *htmlStr = [html UTF8String];
    std::string outputMarkdown = html2md::Convert(htmlStr);
    NSString *markdownStr = [NSString stringWithUTF8String:outputMarkdown.c_str()];
    return markdownStr;
}

@end
