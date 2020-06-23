//
//  MDVxUUID.cpp
//  AVB-analyze
//
//  Created by DJFio on 22/06/2020.
//  Copyright © 2020 DJFio. All rights reserved.
//

#ifdef _MSC_VER
#include "vcProj/stdafx.h"
#else
#include <cstdint>
#include <memory>
#endif
#include "MDVxUUID.hpp"



MDVxUUID &MDVxUUID::makeFrom2(char * b){memcpy(UMIDx+16, b, 8);return *this;}
MDVxUUID &MDVxUUID::makeFrom8(char * b){memcpy(UMIDx, b, 32);return *this;}

void MDVxUUID::swapUUIDfromMXFLibtoMDVx(void){
    
    UMIDx[16] ^= UMIDx[19];
    UMIDx[19] ^= UMIDx[16];
    UMIDx[16] ^= UMIDx[19];
    
    UMIDx[17] ^= UMIDx[18];
    UMIDx[18] ^= UMIDx[17];
    UMIDx[17] ^= UMIDx[18];
    
    UMIDx[20] ^= UMIDx[21];
    UMIDx[21] ^= UMIDx[20];
    UMIDx[20] ^= UMIDx[21];
    
    UMIDx[22] ^= UMIDx[23];
    UMIDx[23] ^= UMIDx[22];
    UMIDx[22] ^= UMIDx[23];
    
}

uint32_t MDVxUUID::hash (void) const { return (octet[4].UMIDx32t)^(octet[5].UMIDx32t);} // simple hash
bool MDVxUUID::isEqualByUUID (const MDVxUUID *b) const
{
    if (b!=NULL) {
        if (this->hash() == b->hash())
        if ((this->octet[4].UMIDx32t == b->octet[4].UMIDx32t) &&
            (this->octet[5].UMIDx32t == b->octet[5].UMIDx32t))
        return true;
    }
    return false;
}
bool MDVxUUID::isLessByUUID (const MDVxUUID *b) const
{
    if (b!=NULL) {
        if (this->octet[4].UMIDx32t != b->octet[4].UMIDx32t){
            if (this->octet[4].UMIDx32t < b->octet[4].UMIDx32t) return true;
            else return false;
        }
        else if (this->octet[5].UMIDx32t != b->octet[5].UMIDx32t){
            if (this->octet[5].UMIDx32t < b->octet[5].UMIDx32t) return true;
            else return false;
        }
    }
    return false;
}

bool MDVxUUID::operator==(const MDVxUUID& b) const  { return (isEqualByUUID (&b));}
bool MDVxUUID::operator<(const MDVxUUID& b) const   { return (isLessByUUID  (&b));}


