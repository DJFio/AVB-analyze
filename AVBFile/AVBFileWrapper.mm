//
//  AVBFileWrapper.cpp
//  AVB-analyze
//
//  Created by DJFio on 15/06/2020.
//  Copyright © 2020 DJFio. All rights reserved.
//

#import "AVBFileWrapper.h"

#include <string>
#include "AVBFile.hpp"



//***********************************
//  AVBFileWrapper Implementation
//***********************************


@implementation AVBFileWrapper
{
    AVBFile *_thefile;
}

- (id)init
{
    self = [super init];
    if (self)
    {
        _thefile = new AVBFile;
    }
    return self;
}
- (void)dealloc
{
    delete _thefile;
}

-(BOOL) openURL:(NSURL*)url {
    return (BOOL)(_thefile->openFile(std::string([url.path UTF8String])) == true);
}
-(void) Print {
    _thefile->Dump(false);
}

@end
