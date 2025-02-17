//
//  Header.h
//  html2md
//
//  Created by 秋星桥 on 2/17/25.
//

#ifndef html2md_objc_h
#define html2md_objc_h

#include <Foundation/Foundation.h>

@interface HTML2MD : NSObject

+ (NSString *)convertHTMLToMarkdown:(NSString *)html;

@end

#endif /* html2md_objc_h */
