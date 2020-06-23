//
//  AVBmacros.h
//  AVB-analyze
//
//  Created by DJFio on 18/06/2020.
//  Copyright © 2020 DJFio. All rights reserved.
//

#ifndef AVBmacros_h
#define AVBmacros_h

//***********************************
//   Byteswap Macros LE<->BE
//***********************************

#define BSWAP_16(val) (((val) << 8) | (((val) >> 8) & 0xFF))
#define BSWAP_32(val) ((((val)>>24)&0xff) | (((val)<<8)&0xff0000) | (((val)>>8)&0xff00) | (((val)<<24)&0xff000000))
#define Byte_Cast16(val,use) (!(use) ?  (val) : BSWAP_16(val))
#define Byte_Cast32(val,use) (!(use) ?  (val) : BSWAP_32(val))

//***********************************
//   MAKEFOURCC Macro / constexpr
//***********************************

#ifndef MAKEFOURCC
#define MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
((uint32_t)(uint8_t)(ch0) | ((uint32_t)(uint8_t)(ch1) << 8) |   \
((uint32_t)(uint8_t)(ch2) << 16) | ((uint32_t)(uint8_t)(ch3) << 24 ))

#endif //defined(MAKEFOURCC)

constexpr uint32_t fourcc( char const p[5] )
{
    return (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
}


#endif /* AVBmacros_h */
