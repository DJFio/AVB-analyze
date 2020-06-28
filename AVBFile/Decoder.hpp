//
//  decoder.hpp
//  AVB-analyze
//
//  Created by DJFio on 18/06/2020.
//  Copyright © 2020 DJFio. All rights reserved.
//

#ifndef decoder_hpp
#define decoder_hpp

#include <stdint.h>
#include <string>
#include <vector>
#include "MDVxUUID.hpp"


//***********************************
// decoder declaration
//***********************************

class Decoder {
public:
    Decoder (std::vector<char> * pVector,bool MM=false) : usebyteswap(MM),pBytes(pVector) ,cp(0)
    {};
    
    std::vector<char> * pBytes;

    void set_bswap(bool bswap=false);
    bool get_bswap(void);
    
    void set_pos(uint32_t pos);
    void pos_advance(uint32_t pos);
    
    uint8_t _u8_assert(uint8_t tag);
    uint8_t getnext_typetag(void);

    uint32_t get_pos(void);
    uint32_t  bytes_left(void);
 
    int32_t      _s32(void);
    int16_t      _s16(void);
    int8_t       _s8 (void);
    
    uint32_t     _u32(void);
    uint16_t     _u16(void);
    uint8_t      _u8 (void);
    uint32_t     _objref(void);

    std::string  _string (void);
    std::string  _fourcc (void);
    uint32_t     _fourcc_u32 (void);
    
    uint32_t cvt32(uint32_t val);
    uint32_t cvt16(uint16_t val);
    
    void readSMPTELabel (MDVxUUID * uid);
    void readUID        (MDVxUUID * uid);
    void readMobID      (MDVxUUID * uid);



private:
    uint32_t inline decodeutf(uint32_t* state, uint32_t* codep, uint32_t byte);
    uint32_t validate_utf8(uint32_t *state, char *str, size_t len);

    uint32_t     cp;
    bool         usebyteswap = false;


};






#endif /* decoder_hpp */
