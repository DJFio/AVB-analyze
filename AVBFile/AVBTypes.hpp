//
//  AVBTypes.hpp
//  AVB-analyze
//
//  Created by DJFio on 22/06/2020.
//  Copyright © 2020 DJFio. All rights reserved.
//

#ifndef AVBTypes_hpp
#define AVBTypes_hpp


//***********************************
//   Types
//***********************************

#define AVB_MAGIC {0x44,0x6F,0x6D,0x61,0x69,0x6E,0x44,0x4A,0x42,0x4F}

#define enum_FORCE_LONG                0x7FFFFFFFL

enum AVB_error{
    AVB_err_noerr = 0L,
    AVB_err_error = 1L,
    AVB_err_file_too_small,
    AVB_err_bad_magic_bytes,
    
    
    
    ERR_MAX  = enum_FORCE_LONG
    
} ;


enum ATTRType
{
    ATTR_NullAttribute = 0L,
    ATTR_IntegerAttribute,
    ATTR_StringAttribute,
    ATTR_ObjectAttribute,
    ATTR_DataValueAttribute,
    
    ATTR_MAX  = enum_FORCE_LONG
};



#endif /* AVBTypes_hpp */
