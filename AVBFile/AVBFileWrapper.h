//
//  AVBFileWrapper.h
//  AVB-analyze
//
//  Created by DJFio on 04/06/2020.
//  Copyright © 2020 DJFio. All rights reserved.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface AVBFileWrapper : NSObject
-(BOOL) openURL:(NSURL*)url;
-(void) Print ;
@end

NS_ASSUME_NONNULL_END
