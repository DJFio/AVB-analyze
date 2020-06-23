//
//  printer.hpp
//  AVB-analyze
//
//  Created by DJFio on 22/06/2020.
//  Copyright © 2020 DJFio. All rights reserved.
//



#ifndef printer_hpp
#define printer_hpp
#include "MDVxUUID.hpp"
#include "Decoder.hpp"



class Printer {
public:
    Printer(Decoder * dec):getvalue(dec){};

    std::string  _fourcc_from_u32 (uint32_t fourcc);

    std::string  _hexlify_size_t(size_t x);
    std::string  _hexlify_u32(uint32_t x);
    std::string  _hexlify_u16(uint16_t x);
    std::string  _hexlify_u8 (uint8_t x);

    std::string  _pretty_dump (uint32_t pos,uint32_t len);
    std::string  _dump (uint32_t pos,uint32_t len);
    std::string  _hexlify_UID (MDVxUUID* uid)  ;
    std::string  _hexlify_short_UID (MDVxUUID* uid);
private:
    Decoder * getvalue;
    bool         usebyteswap = false;
    char printbuffer[84] = {"{ xxxxxxxx::xxxxxxxx::xxxxxxxx::xxxxxxxx::xxxxxxxx::xxxxxxxx::xxxxxxxx::xxxxxxxx }\0"};
    char const   hex_chars[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
    char         numtostrbuf[128];


	// Added by Michael Haephrati

	char filename[256];      // just name without path              (ASCII)
	char projectname[256];   // project relationship from .pmr file (ASCII)
	MDVxUUID filePackageUID; // file package UID - is a unique file identifier for each MXF or OMF file
	MDVxUUID materialSourcePackageUID; // file package UID - is a unique "clip" identifier (bunch of files)

	void getStringForUID(MDVxUUID* uid);
	std::vector<MatchResult> GetStringsToVector(void);

    
};
#endif /* printer_hpp */
