//
//  MDVxUUID.hpp
//  AVB-analyze
//
//  Created by DJFio on 22/06/2020.
//  Copyright © 2020 DJFio. All rights reserved.
//

#ifndef MDVxUUID_hpp
#define MDVxUUID_hpp

typedef union MDVxOctet{
    uint8_t UMIDx[4];
    uint32_t UMIDx32t;
}MDVxOctet;

typedef union MDVxUUID {
    uint8_t UMIDx[32];
    MDVxOctet octet[8];
    MDVxUUID &makeFrom2(char * b);
    MDVxUUID &makeFrom8(char * b);
    void swapUUIDfromMXFLibtoMDVx(void);
    uint32_t hash (void) const ; // simple hash
    bool isEqualByUUID (const MDVxUUID *b) const;
    bool isLessByUUID (const MDVxUUID *b) const;
    bool operator==(const MDVxUUID& b) const;
    bool operator<(const MDVxUUID& b) const;
}MDVxUUID;



#endif /* MDVxUUID_hpp */
