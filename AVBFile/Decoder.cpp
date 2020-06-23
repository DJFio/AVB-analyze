//
//  decoder.cpp
//  Drag-Analyze-AVB
//
//  Created by DJFio on 18/06/2020.
//  Copyright © 2020 DJFio. All rights reserved.
//

#ifdef _MSC_VER
#include "vcProj/stdafx.h"
#endif

#include "Decoder.hpp"
#include "AVBmacros.h"


#define UTF8_ACCEPT 0
#define UTF8_REJECT 1

void Decoder::set_bswap(bool bswap){usebyteswap=bswap;}
bool Decoder::get_bswap(void){return usebyteswap;}

void Decoder::set_pos(uint32_t pos)
{
    if (pos<pBytes->size()) {cp=pos;}
    else {cp=0;}
    
}
uint32_t Decoder::get_pos(void){return cp;}
void Decoder::pos_advance(uint32_t pos){cp+=pos;}

uint32_t  Decoder::bytes_left(void)
{
    if (cp < pBytes->size()) return static_cast<uint32_t>(pBytes->size())-cp;
    return 0;
}

int32_t      Decoder::_s32(void)
{
    int32_t  val = Byte_Cast32 (*reinterpret_cast<int32_t*>(pBytes->data()+cp),usebyteswap);
    cp+=4;
    return val;
}
int16_t      Decoder::_s16(void)
{
    int16_t  val = Byte_Cast16 (*reinterpret_cast<int16_t*>(pBytes->data()+cp),usebyteswap);
    cp+=2;
    return val;
}
int8_t       Decoder::_s8 (void)
{
    int8_t  val = *reinterpret_cast<int8_t*>(pBytes->data()+cp);
    cp++;
    return val;
}
uint32_t     Decoder::_u32(void)
{
    uint32_t  val = Byte_Cast32 (*reinterpret_cast<uint32_t*>(pBytes->data()+cp),usebyteswap);
    cp+=4;
    return val;
}
uint16_t     Decoder::_u16(void)
{
    uint16_t  val = Byte_Cast16 (*reinterpret_cast<uint16_t*>(pBytes->data()+cp),usebyteswap);
    cp+=2;
    return val;
}
uint8_t      Decoder::_u8 (void)
{
    uint8_t  val = *reinterpret_cast<uint8_t*>(pBytes->data()+cp);
    cp++;
    return val;
}
uint32_t Decoder::cvt32(uint32_t val)
{
    return  Byte_Cast32 (val,usebyteswap);
}
uint32_t Decoder::cvt16(uint16_t val)
{
    return  Byte_Cast16 (val,usebyteswap);
}
uint8_t Decoder::_u8_assert(uint8_t tag)
{
    uint8_t  val = *reinterpret_cast<uint8_t*>(pBytes->data()+cp);
    cp++;
    assert (val==tag);
    return val;
}
uint8_t Decoder::getnext_typetag(void){
    if (this->_u8()!=0x01) {cp--; return 0x0;}
    return this->_u8();
}
std::string  Decoder::_string (void)
{
    uint16_t strsz = this->_u16();
    
    if (strsz > bytes_left()) {
        assert(false);
        cp-=2;
        return std::string("");
        
    }

    
    if ((*(pBytes->data()+cp) == 0x00)&&(strsz>0x02))
    {
        if (*(pBytes->data()+cp+1) == 0x00){
            // then it's a utf-8 string
            cp+=strsz;
            return std::string(pBytes->data()+cp+2-strsz,strsz-2);
        }};
    if (*(pBytes->data()+cp) == 0x00)
    {
        cp-=2;
        return std::string("");
    }
    uint32_t state = UTF8_ACCEPT;
    validate_utf8(&state, pBytes->data()+cp,strsz);
    cp+=strsz;
    if (state==UTF8_ACCEPT) {
        return std::string(pBytes->data()+cp-strsz,strsz);
    }
    return std::string("####### BAD STRING [len= ").append(std::to_string(strsz)).append("]");
    //TODO: Make suitable decoder for unknown strings.
//    return std::string(pBytes->data()+cp-strsz,strsz).append("[BAD STRING CHARACTERS len=").append(std::to_string(strsz)).append("]");
}

// Copyright (c) 2008-2009 Bjoern Hoehrmann <bjoern@hoehrmann.de>
// See http://bjoern.hoehrmann.de/utf-8/decoder/dfa/ for details.

static const uint8_t utf8d[] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 00..1f
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 20..3f
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 40..5f
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 60..7f
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9, // 80..9f
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7, // a0..bf
    8,8,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, // c0..df
    0xa,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x4,0x3,0x3, // e0..ef
    0xb,0x6,0x6,0x6,0x5,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8, // f0..ff
    0x0,0x1,0x2,0x3,0x5,0x8,0x7,0x1,0x1,0x1,0x4,0x6,0x1,0x1,0x1,0x1, // s0..s0
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,1, // s1..s2
    1,2,1,1,1,1,1,2,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1, // s3..s4
    1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,3,1,3,1,1,1,1,1,1, // s5..s6
    1,3,1,1,1,1,1,3,1,3,1,1,1,1,1,1,1,3,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // s7..s8
};

uint32_t inline
Decoder::decodeutf(uint32_t* state, uint32_t* codep, uint32_t byte) {
    uint32_t type = utf8d[byte];
    
    *codep = (*state != UTF8_ACCEPT) ?
    (byte & 0x3fu) | (*codep << 6) :
    (0xff >> type) & (byte);
    
    *state = utf8d[256 + *state*16 + type];
    return *state;
}

uint32_t inline Decoder::validate_utf8(uint32_t *state, char *str, size_t len) {
    size_t i;
    uint32_t type;
    
    for (i = 0; i < len; i++) {
        // We don't care about the codepoint, so this is
        // a simplified version of the decode function.
        type = utf8d[(uint8_t)str[i]];
        *state = utf8d[256 + (*state) * 16 + type];
        
        if (*state == UTF8_REJECT)
            break;
    }
    
    return *state;
}





std::string  Decoder::_fourcc (void){ cp+=4; return std::string(pBytes->data()+cp-4,4);}
uint32_t     Decoder::_fourcc_u32 (void)
{
    uint32_t fcc= MAKEFOURCC(pBytes->data()[cp], pBytes->data()[cp+1],pBytes->data()[cp+2], pBytes->data()[cp+3]);
    cp+=4;
    return fcc;
}


void  Decoder::readSMPTELabel (MDVxUUID * uid){
    assert (this->_s32()==0x0CL);
    assert (this->bytes_left()>=0xC);
    memcpy(uid->UMIDx, pBytes->data()+cp, 0xC);
    cp+=0xC;
}
void  Decoder::readUID        (MDVxUUID * uid){
    this->_u8_assert(65);
    assert( this->_s32()==8);
    uid->UMIDx[24]=this->_u8();
    uid->UMIDx[25]=this->_u8();
    uid->UMIDx[26]=this->_u8();
    uid->UMIDx[27]=this->_u8();
    uid->UMIDx[28]=this->_u8();
    uid->UMIDx[29]=this->_u8();
    uid->UMIDx[30]=this->_u8();
    uid->UMIDx[31]=this->_u8();
}
void  Decoder::readMobID      (MDVxUUID * uid){
    readSMPTELabel (uid);
    this->_u8_assert(68);
    uid->UMIDx[12]=this->_u8();
    this->_u8_assert(68);
    uid->UMIDx[13]=this->_u8();
    this->_u8_assert(68);
    uid->UMIDx[14]=this->_u8();
    this->_u8_assert(68);
    uid->UMIDx[15]=this->_u8();
    this->_u8_assert(72);
    uid->UMIDx[16]=this->_u8();
    uid->UMIDx[17]=this->_u8();
    uid->UMIDx[18]=this->_u8();
    uid->UMIDx[19]=this->_u8();
    this->_u8_assert(70);
    uid->UMIDx[20]=this->_u8();
    uid->UMIDx[21]=this->_u8();
    this->_u8_assert(70);
    uid->UMIDx[22]=this->_u8();
    uid->UMIDx[23]=this->_u8();
    this->readUID(uid);
}
