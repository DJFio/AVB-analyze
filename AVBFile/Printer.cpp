//
//  printer.cpp
//  AVB-analyze
//
//  Created by DJFio on 22/06/2020.
//  Copyright © 2020 DJFio. All rights reserved.
//

// Handle Windows cases - Michael Haephrati
// ----------------------------------------
#ifdef _MSC_VER
#include "vcProj/stdafx.h"
#else
#include <string>
#include <vector>
#endif
using namespace std;

#include "Printer.hpp"

//***********************************
//   pretty print and dump
//***********************************

std::string Printer::_hexlify_u32(uint32_t x){
    numtostrbuf[0]='0';
    numtostrbuf[1]='x';
    if (!usebyteswap){
        for (int i = 0; i < 4; ++i){
            numtostrbuf[2+(i<<1)]= hex_chars[ ( reinterpret_cast<char*>(&x)[3-i] & 0xF0 ) >> 4 ];
            numtostrbuf[3+(i<<1)]= hex_chars[ ( reinterpret_cast<char*>(&x)[3-i] & 0x0F ) >> 0 ];
        }
    } else {
        for (int i = 0; i < 4; ++i){
            numtostrbuf[2+(i<<1)]= hex_chars[ ( reinterpret_cast<char*>(&x)[i] & 0xF0 ) >> 4 ];
            numtostrbuf[3+(i<<1)]= hex_chars[ ( reinterpret_cast<char*>(&x)[i] & 0x0F ) >> 0 ];
        }
    }
    return std::string(numtostrbuf,10);
}
std::string Printer::_hexlify_u16(uint16_t x){
    numtostrbuf[0]='0';
    numtostrbuf[1]='x';
    if (!usebyteswap){
        for (int i = 0; i < 2; ++i){
            numtostrbuf[2+(i<<1)]= hex_chars[ ( reinterpret_cast<char*>(&x)[1-i] & 0xF0 ) >> 4 ];
            numtostrbuf[3+(i<<1)]= hex_chars[ ( reinterpret_cast<char*>(&x)[1-i] & 0x0F ) >> 0 ];
        }
    } else {
        for (int i = 0; i < 2; ++i){
            numtostrbuf[2+(i<<1)]= hex_chars[ ( reinterpret_cast<char*>(&x)[i] & 0xF0 ) >> 4 ];
            numtostrbuf[3+(i<<1)]= hex_chars[ ( reinterpret_cast<char*>(&x)[i] & 0x0F ) >> 0 ];
        }
    }
    return std::string(numtostrbuf,6);
}
std::string Printer::_hexlify_u8(uint8_t x){
    numtostrbuf[0]='0';
    numtostrbuf[1]='x';
    numtostrbuf[2]= hex_chars[ ( *reinterpret_cast<char*>(&x) & 0xF0 ) >> 4 ];
    numtostrbuf[3]= hex_chars[ ( *reinterpret_cast<char*>(&x) & 0x0F ) >> 0 ];
    return std::string(numtostrbuf,4);
}
std::string  Printer::_hexlify_size_t(size_t x){
    numtostrbuf[0]='0';
    numtostrbuf[1]='x';
    if (!usebyteswap){
        for (int i = 0; i < sizeof(size_t); ++i){
            numtostrbuf[2+(i<<1)]= hex_chars[ ( reinterpret_cast<char*>(&x)[sizeof(size_t)-i] & 0xF0 ) >> 4 ];
            numtostrbuf[3+(i<<1)]= hex_chars[ ( reinterpret_cast<char*>(&x)[sizeof(size_t)-i] & 0x0F ) >> 0 ];
        }
    } else {
        for (int i = 0; i < 4; ++i){
            numtostrbuf[2+(i<<1)]= hex_chars[ ( reinterpret_cast<char*>(&x)[i] & 0xF0 ) >> 4 ];
            numtostrbuf[3+(i<<1)]= hex_chars[ ( reinterpret_cast<char*>(&x)[i] & 0x0F ) >> 0 ];
        }
    }
    return std::string(numtostrbuf,2+sizeof(size_t));
}
std::string  Printer::_dump (uint32_t pos,uint32_t len){
    if (getvalue->pBytes->size() < pos+len) return std::string("");
    char* ourdata = reinterpret_cast<char*>(getvalue->pBytes->data()+pos);
    const size_t charsinrow = len*4+(len/2)+3;// data(3+1)+ Dashes x2 +1 + endl or zero
    std::vector<char> pretty_data;
    pretty_data.resize(charsinrow+1);// +1 zero
    std::fill(pretty_data.begin(), pretty_data.end(), 0x20);
    char* printbuffer = pretty_data.data();
    
    //for each full row
    size_t z=0;
    size_t c=0;
    size_t rest = len;
    size_t d=0;
    //for the rest
    if (rest>0){
        
        for (int i = 0; i < rest; ++i){
            printbuffer[c++]= hex_chars[ ( ourdata[z+i] & 0xF0 ) >> 4 ];
            printbuffer[c++]= hex_chars[ ( ourdata[z+i] & 0x0F ) >> 0 ];
            printbuffer[c++]= 0x20;//space
            d+=3;
            if (((i+1) & 0x03)  == 0) {
                printbuffer[c++]= 0x7c;// line
                printbuffer[c++]= 0x20;// space
                d+=2;
            }
        }
        for (int i = 0; i < rest; ++i)
        {
            printbuffer[c++] = ((ourdata[z+i]<0x20)||(ourdata[z+i]==0x7f/*del*/ )) ? 0x2e/*dot*/ : ourdata[z+i];
            
        }
        printbuffer[c++]= 0x7c;// line
        printbuffer[c++]= 0x20;// space
        
        
        // print chars
        
    }
    
    return std::string(pretty_data.data(),pretty_data.size());
    
}

std::string  Printer::_pretty_dump (uint32_t pos,uint32_t len){
    if (getvalue->pBytes->size() < pos+len) return std::string("");
    char* ourdata = reinterpret_cast<char*>(getvalue->pBytes->data()+pos);
    const size_t rowlen = 0x20;
    const size_t charsinrow = rowlen*4+16+1;// data(3+1)+8x Dashes x2 + endl or zero
    size_t rows = len/rowlen;
    std::vector<char> pretty_data;
    pretty_data.resize((rows+1)*charsinrow+1);// +1 zero
    std::fill(pretty_data.begin(), pretty_data.end(), 0x20);
    char* printbuffer = pretty_data.data();
    
    size_t rest = len-rows*rowlen;
    //for each full row
    size_t z=0;
    size_t c=0;
    while (z < rows){
        //print hex values
        
        for (int i = 0; i < rowlen; ++i){
            printbuffer[c++]  = hex_chars[ ( ourdata[z*rowlen+i] & 0xF0 ) >> 4 ];
            printbuffer[c++]= hex_chars[ ( ourdata[z*rowlen+i] & 0x0F ) >> 0 ];
            printbuffer[c++]= 0x20;//space
            if ((((i+1) & 0x03 ) == 0)&&(i+1 < rowlen)) {
                printbuffer[c++]= 0x7c;// line
                printbuffer[c++]= 0x20;// space
            }
        }
        // print chars
        printbuffer[c++]= 0x7c;// line
        printbuffer[c++]= 0x20;// space
        for (int i = 0; i < rowlen; ++i)
        {
            printbuffer[c++] = ((ourdata[z*rowlen+i]<0x20)||(ourdata[z*rowlen+i]==0x7f/*del*/ )) ? 0x2e/*dot*/ : ourdata[z*rowlen+i];
            
        }
        z++;
        if ((z>=rows)&&(rest==0)) continue;
        printbuffer[c++]  = 0xA;
        
    }
    z=rows*rowlen;
    size_t d=0;
    //for the rest
    if (rest>0){
        for (int i = 0; i < rest; ++i){
            printbuffer[c++]= hex_chars[ ( ourdata[z+i] & 0xF0 ) >> 4 ];
            printbuffer[c++]= hex_chars[ ( ourdata[z+i] & 0x0F ) >> 0 ];
            printbuffer[c++]= 0x20;//space
            d+=3;
            if (((i+1) & 0x03)  == 0) {
                printbuffer[c++]= 0x7c;// line
                printbuffer[c++]= 0x20;// space
                d+=2;
            }
        }
        c+=rowlen*3+rowlen/2-2-d;
        printbuffer[c++]= 0x7c;// line
        printbuffer[c++]= 0x20;// space
        // print chars
        for (int i = 0; i < rest; ++i)
        {
            printbuffer[c++] = ((ourdata[z+i]<0x20)||(ourdata[z+i]==0x7f/*del*/ )) ? 0x2e/*dot*/ : ourdata[z+i];
            
        }
    }
    
    return std::string(pretty_data.data(),pretty_data.size());
    
}

std::string  Printer::_hexlify_UID (MDVxUUID* uid)  {
    if (uid==NULL) return std::string ("{null uid object}\0");
    int c=2;
    for (int i = 0; i < 32; ++i){
        printbuffer[c]  = hex_chars[ ( uid->UMIDx[i] & 0xF0 ) >> 4 ];
        printbuffer[c+1]= hex_chars[ ( uid->UMIDx[i] & 0x0F ) >> 0 ];
        c+=2;
        if (((i+1)<32)&&((i+1) % 4  == 0)) c+=2;
    }
    return std::string (printbuffer);
}
std::string  Printer::_hexlify_short_UID (MDVxUUID* uid)  {
    if (uid==NULL) return std::string ("{null uid object}\0");
    int c=2;
    for (int i = 16; i < 24; ++i){
        printbuffer[c]  = hex_chars[ ( uid->UMIDx[i] & 0xF0 ) >> 4 ];
        printbuffer[c+1]= hex_chars[ ( uid->UMIDx[i] & 0x0F ) >> 0 ];
        c+=2;
        if (((i+1)<32)&&((i+1) % 4  == 0)) c+=2;
    }
    return std::string (printbuffer+2,18);
}

std::string  Printer::_fourcc_from_u32 (uint32_t fourcc){
    return std::string(reinterpret_cast<char*>(&fourcc),4);
}
#ifndef ignore_this
vector<MatchResult> Printer::GetStringsToVector(void)
{
	vector<MatchResult> result;
	MatchResult singleItem;

	singleItem.FileName = filename;

	singleItem.ProjectName = projectname;

	getStringForUID(&filePackageUID);
	singleItem.filepackageUID = printbuffer;

	getStringForUID(&materialSourcePackageUID);
	singleItem.SourcePackageUID = printbuffer;

	result.push_back(singleItem);

	return result;

}

void Printer::getStringForUID(MDVxUUID* uid)
{

	if (uid == NULL) return;
	int c = 2;
	for (int i = 0; i < 32; ++i)
	{
		printbuffer[c] = hex_chars[(uid->UMIDx[i] & 0xF0) >> 4];
		printbuffer[c + 1] = hex_chars[(uid->UMIDx[i] & 0x0F) >> 0];
		c += 2;
		if (((i + 1) < 32) && ((i + 1) % 4 == 0)) c += 2;
	}
	return;
}

#endif

//std::string

//convert(std::int64_t seconds_since_1904)
//{
//    using namespace date;
//    using namespace std::chrono;
//    constexpr auto offset = sys_days{January/1/1970} - sys_days{January/1/1904};
//    return format("%T %m.%d, %Y", sys_seconds{seconds{seconds_since_1904}} - offset);
//}


