//
//  AVBObjects.cpp
//  AVB-analyze
//
//  Created by DJFio on 18/06/2020.
//  Copyright © 2020 DJFio. All rights reserved.
//

// Handle Windows cases - Michael Haephrati
// ----------------------------------------
#ifdef _MSC_VER
#include "vcproj\stdafx.h"
#else
#include <sstream>
#endif

#include "AVBObjects.hpp"
#include "Printer.hpp"

// Handle Windows cases - Michael Haephrati
// ----------------------------------------
#ifndef _MSC_VER
#include "cout_redirect.hpp"

#ifndef redir_cout
#define redir_cout
#ifndef redir_flush
#define redir_flush
#endif	// redir_cout
#endif	// redir_flush
#endif	// _MSC_VER

//-----------------------------------------------------------------
//Base ATOM class
//-----------------------------------------------------------------



//TODO: --- ATOM::Atom base ---

avbATOM::avbATOM(): getvalue(&BOB){
    atom_fourcc = fourcc("ATOM");
    length = 0x03;
    BOB.resize(length);
    BOB[0]=0x2; //std object
    BOB[1]=0x1; //obj ver=1
    BOB[2]=0x3; //std end
    hasReferencesToOtherOBJs=0L;
}

void avbATOM::init(avbTOC *pParentTOC){
    pTOC = pParentTOC;
    getvalue.set_bswap(pParentTOC->getByteswap());
    referenceCount = 1;//referenced by parent
}
bool avbATOM::BOB_read(std::ifstream *f){
    hasReferencesToOtherOBJs=0L;
    f->read(reinterpret_cast<char*>(&atom_fourcc), 4);// position += 4;//ATOM name
    f->read(reinterpret_cast<char*>(&length),4);
    length = Byte_Cast32(length, getvalue.get_bswap());//position += 4;//ATOM size
    //read data[size]
    BOB.resize(length);
    f->read(BOB.data(),length);
    getvalue.set_pos(0L);
    assert ((BOB[0]==0x02)&&(BOB[length-1]==0x03));
    if ((BOB[0]==0x02)&&(BOB[length-1]==0x03)) BOBisValid = true;
    else BOBisValid = false;
    return BOBisValid;
}
bool avbATOM::create_object_from_BOB(){
    OBJisValid = BOBisValid;
    return OBJisValid;
}

std::string avbATOM::BOB_end_dump(void){
    std::stringstream log("");
    Printer p(&getvalue);
    uint32_t bytesleft = getvalue.bytes_left();
    /*checkerror*/if((bytesleft != 1)&&(bytesleft != 0)){
        if (bytesleft>0) bytesleft--;
        log  <<"#::unknown data :: |len: 0x"<< p._hexlify_u32(bytesleft) << std::endl;
        if (bytesleft<32) {log  <<"  |=>"<< p._dump(getvalue.get_pos(), bytesleft) << std::endl;} else
        {log  << p._pretty_dump(getvalue.get_pos(), bytesleft) << std::endl;}
    }
    assert (bytesleft != 0);
    getvalue.set_pos(length-1);
    log  <<"  |-@end: "<<p._hexlify_u8(getvalue._u8()) << std::endl;;
    return log.str();
}
std::string avbATOM::BOB_begin_dump(uint32_t pos){
    getvalue.set_pos(pos);
    std::stringstream log;
    Printer p(&getvalue);
    log  << "TOC#" << p._hexlify_u32(TOC_id)<< std::endl;;
    log <<"|" << p._fourcc_from_u32(atom_fourcc);
    log <<"|len:" << p._hexlify_u32(length) ;
    log <<"|referenced =" << p._hexlify_u32(referenceCount) << "|RefToOtherOBJ=" << p._hexlify_u32(hasReferencesToOtherOBJs) << std::endl;
    return log.str();
    
}

bool avbATOM::write(std::ofstream *f){
    if (BOB.size()<3) return false;
    assert ((BOB[0]==0x02)&&(BOB[length-1]==0x03));
    if ((BOB[0]!=0x02)||(BOB[length-1]!=0x03)) return false;
    f->write(reinterpret_cast<char*>(&atom_fourcc),4);
    uint32_t cvtlength= getvalue.cvt32(length);
    f->write(reinterpret_cast<char*>(&cvtlength),4);
    f->write(BOB.data(),length);
    return true;
}
std::string avbATOM::dump(void) {
    if (OBJisValid==false) return dumpInvalidObject();

    std::stringstream log;
    Printer p(&getvalue);
    log << BOB_begin_dump();
    log  <<"  |-@begin:" <<  p._hexlify_u8(getvalue._u8()) << std::endl;;
    log << BOB_end_dump() << std::endl;

    return log.str();
};
std::string avbATOM::dumpInvalidObject(void){
    std::stringstream log;
    Printer p(&getvalue);
    log << BOB_begin_dump();
    log  <<"  |-invalid_OBJECT:" << std::endl;;

    uint32_t bytesleft = getvalue.bytes_left();
    if(bytesleft != 0){
        log  <<"#::unknown data :: |len: 0x"<< p._hexlify_u32(bytesleft) << std::endl;
        if (bytesleft<32) {log  <<"  |=>"<< p._dump(getvalue.get_pos(), bytesleft) << std::endl;} else
        {log  << p._pretty_dump(getvalue.get_pos(), bytesleft) << std::endl;}
    }
    return log.str();
}

uint32_t avbATOM::read_objref(){
    uint32_t pObj = getvalue._objref();
    if (pObj != 0){
        hasReferencesToOtherOBJs++;
        pTOC->at(pObj)->referenceCount++;
    }
    return pObj;
}


//TODO: --- OBJD::Atom ---

class atom_OBJD: public avbATOM {
public:
    atom_OBJD(){
        atom_fourcc = fourcc("OBJD");
        length = 0x77;
        BOB.resize(length);
        TOCObjCount=0;
        TOCRootObject=0;
        memset(BINUID.UMIDx, 0x0, 0x20);
    }
    uint32_t TOCObjCount;
    uint32_t TOCRootObject;
    MDVxUUID BINUID;
    bool BOB_read(std::ifstream *f);
    std::string dump(void);

};

bool atom_OBJD::BOB_read(std::ifstream *f){
    BOBisValid = false;
    std::streamsize avbsize =f->tellg();
    f->seekg(0, std::ios::beg);
    assert (avbsize > 0x77);
    if (avbsize < 0x77) // the header entry in avb file is 0x77 bytes long
    {
//        std::cout << "problem with AVB File :: too small " << std::endl; redir_flush;
        return BOBisValid;

    }
    
    if (f->read(BOB.data(), 0x77))
    {
        hasReferencesToOtherOBJs=0L;
        char avbmagic[10] = AVB_MAGIC;
        if ((BOB[0]==0x6)&&(BOB[1]==0x0)) {getvalue.set_bswap(false);}
        else if ((BOB[0]==0x0)&&(BOB[1]==0x6)) {getvalue.set_bswap(true);}
        pTOC->setByteswap(getvalue.get_bswap());
        if (memcmp (&BOB.data()[2],avbmagic,10)!=0)
        {
            //                std::cout << "problem with AVB File :: Bad Magic bytes" << std::endl; redir_flush;
            return BOBisValid;
        }
        
        TOC_id = 0x0; //the OBJD  object
        
        
        //verify several things
        //and set basic values
        
        getvalue.set_pos(0x0L);
        getvalue.pos_advance(getvalue._u16());//getvalue._string();// Domain
        getvalue._fourcc_u32();//ATOM OBJD
        getvalue.pos_advance(getvalue._u16());//getvalue._string();//objdoc
        getvalue._u8();//version
        getvalue.pos_advance(getvalue._u16());//getvalue._string();//string:date

        TOCObjCount = getvalue._u32();//tocobj count
        pTOC->resizeTOC(TOCObjCount); //taking OBJD in account
        TOCRootObject = getvalue._objref();//tocrootobject
        hasReferencesToOtherOBJs++;
        pTOC->setRootObject(TOCRootObject);
        getvalue._fourcc_u32();    //IIII
        BINUID.makeFrom2(BOB.data()+getvalue.get_pos()); getvalue.pos_advance(8);
        getvalue._fourcc_u32();    //filetype
        getvalue._fourcc_u32();    //creator
        getvalue.pos_advance(getvalue._u16());//getvalue._string();        //creator[string]
        getvalue.pos_advance(16);  //[zeroes]
        
        assert( getvalue.get_pos()==0x77);
        if ( getvalue.get_pos()==0x77) BOBisValid=  true;
        
    }
    return avbATOM::create_object_from_BOB();

}
    std::string atom_OBJD::dump(void)
    {
        if (OBJisValid==false) return avbATOM::dumpInvalidObject();

        std::stringstream log;
        Printer p(&getvalue);
        
        log  << BOB_begin_dump ();
        getvalue._string(); // Domain
        log <<"-"<< getvalue._fourcc() << std::endl;//ATOM OBJD
        log <<"  |-01:aobjdoc         : "<< getvalue._string()<< std::endl; //objdoc
        log <<"  |-02:domain version=4: "<< p._hexlify_u8(getvalue._u8()) << std::endl; //version
        log <<"  |-03:lastsave        : "<< getvalue._string()<< std::endl; //string:date
        log <<"  |-04:numobj          : "<< p._hexlify_u32(getvalue._u32())<< std::endl;//tocobj count
        log <<"  |-05:rootobj:selfroot: "<< p._hexlify_u32(getvalue._objref())<< std::endl;//tocrootobject
        log <<"  |-06:order           : "<< getvalue._fourcc()<< std::endl;   //IIII
        log <<"  |-07:UID hi::lo      : "<< p._hexlify_short_UID(&BINUID)<< std::endl;    getvalue.pos_advance(8);
        log <<"  |-08:fourcc:filetype : "<< getvalue._fourcc()<< std::endl;  //filetype
        log <<"  |-09:fourcc:creator  : "<< getvalue._fourcc()<< std::endl;  //creator
        log <<"  |-10:CreatorVersion  : "<< getvalue._string()<< std::endl;   //creator[string]
        log <<"  |-11:reserved/spare  : "<< p._dump(getvalue.get_pos(),16)<< std::endl;  getvalue.pos_advance(16);//[zeroes]
        log << std::endl;
        
        return log.str();
    }



//TODO: ------------------* Bin ---
//TODO: --- ASET::Atom ---
//TODO: --- BVst::Atom ---

//TODO: --- ABIN::Atom ---
//TODO: --- BINF::Atom ---


class atom_ABIN: public avbATOM {
public:
    atom_ABIN(){
        atom_fourcc = fourcc("ABIN");
    }
    bool create_object_from_BOB();
    std::string dump(void);

    bool largebin = false;
    uint32_t BVst_id = 0;
    uint32_t ATTR_id = 0;
    MDVxUUID rootObjUID;
    uint32_t CMPOcount = 0;

};

bool atom_ABIN::create_object_from_BOB(){
    OBJisValid = false;
    if (BOBisValid!=true) return OBJisValid;

    getvalue.set_pos(0x0L);
    
    getvalue._u8_assert(0x2);//tag

    largebin  = (getvalue._u8()==0xf);
    BVst_id = read_objref();
    rootObjUID.makeFrom2(BOB.data()+getvalue.get_pos()); getvalue.pos_advance(8);
    if (!largebin){ CMPOcount = static_cast<uint32_t> (getvalue._u16());}
    else { CMPOcount = static_cast<uint32_t> (getvalue._u32());}
    for (uint32_t i = 0 ; i<CMPOcount;i++){
        read_objref();
        getvalue.pos_advance(9);
    }
    getvalue.pos_advance(7);
    for (uint32_t i = 0 ; i<6;i++){
        getvalue.pos_advance(2);
        getvalue.pos_advance(getvalue._u16());
        getvalue.pos_advance(getvalue._u16());
    }
    uint16_t sort_column_count =  getvalue._u16();
    for (uint16_t i = 0 ; i<sort_column_count;i++){
        getvalue.pos_advance(1);
        getvalue.pos_advance(getvalue._u16());
    }
    getvalue.pos_advance(34);
    ATTR_id = read_objref();
    getvalue.pos_advance(1);
    
    OBJisValid = BOBisValid;
    return OBJisValid;
}
std::string atom_ABIN::dump(void){
    if (OBJisValid==false) return avbATOM::dumpInvalidObject();

    std::stringstream log;
    Printer p(&getvalue);
    
    log  << BOB_begin_dump ();

    log  <<"  |-@begin/tag: " <<  p._hexlify_u8( getvalue._u8 ()) << std::endl;
    log  <<"  |-ver/bintype: " << (largebin ? ":large": ":notlarge" )<< std::endl; getvalue.pos_advance(1);
    log  <<"  |-TOC =>BVst: "<<p._hexlify_u32(BVst_id) << "|OBJ=>'"<< std::endl;getvalue.pos_advance(4);
    //TODO: unsert DUMP BVst here
    log  <<"  |-UID64: "<<p._hexlify_short_UID(&rootObjUID) << std::endl;getvalue.pos_advance(8);
    log  <<"  |-CMPOItems: "<<p._hexlify_u32(CMPOcount) << std::endl; if (!largebin){ getvalue.pos_advance(2); } else { getvalue.pos_advance(4);}
    for (uint32_t i = 0 ; i<CMPOcount;i++){
        uint32_t pObj = getvalue._objref();
        log  <<"  |-CMPOref# "<<i<<"|TOC=" <<p._hexlify_u32(pObj);
        log  <<"|X=" <<p._hexlify_u16(getvalue._u16());
        log  <<"|Y=" <<p._hexlify_u16(getvalue._u16());
        log  <<"|keyframe: " <<p._hexlify_u32(getvalue._u32());
        log  <<"|userplaced: " <<p._hexlify_u8(getvalue._u8());
        log  << "|OBJ=>'"<< std::endl;
        //TODO:add CMPO DUMP here;
        //                    log  <<")OBJ=>'" << getvalue._fourcc(refto)<<"'"<< std::endl;
    }
    log  <<"  |-display_mask: "<<p._hexlify_u32(getvalue._u32()) << std::endl;
    log  <<"  |-display_mode: "<<p._hexlify_u16(getvalue._u16()) << std::endl;
    log  <<"  |-sifted :"<<p._hexlify_u8(getvalue._u8()) << std::endl;
    for (uint32_t i = 0 ; i<6;i++){
        log  <<"  |-+-method: " <<p._hexlify_u16(getvalue._u16()) << std::endl;
        log  <<"  | |-string: " <<getvalue._string() << std::endl;
        log  <<"  | '-column: " <<getvalue._string() << std::endl;
    }
    uint16_t sort_column_count =  getvalue._u16();
    log  <<"  |-sort_column_count: "<<p._hexlify_u16(sort_column_count) << std::endl;
    for (uint16_t i = 0 ; i<sort_column_count;i++){
        log  <<"  |-+-direction: " <<p._hexlify_u8(getvalue._u8()) << std::endl;
        log  <<"  | '-col      : " <<getvalue._string() << std::endl;
    }
    log  <<"  |-mac_font: "<<p._hexlify_u16(getvalue._u16()) << std::endl;
    log  <<"  |-mac_font.size: "<<p._hexlify_u16(getvalue._u16()) << std::endl;
    log  <<"  |-mac_image scale: "<<p._hexlify_u16(getvalue._u16()) << std::endl;
    log  <<"  |-home rect |ver="<<p._hexlify_u16(getvalue._u16());
    log  <<"  |a="<<p._hexlify_u16(getvalue._u16());
    log  <<"  |b="<<p._hexlify_u16(getvalue._u16());
    log  <<"  |c="<<p._hexlify_u16(getvalue._u16());
    log  <<"  |d="<<p._hexlify_u16(getvalue._u16())<< std::endl;
    log  <<"  |-bg color  |ver="<<p._hexlify_u16(getvalue._u16());
    log  <<"  |r="<<p._hexlify_u16(getvalue._u16());
    log  <<"  |g="<<p._hexlify_u16(getvalue._u16());
    log  <<"  |b="<<p._hexlify_u16(getvalue._u16())<< std::endl;
    log  <<"  |-fg color  |ver="<<p._hexlify_u16(getvalue._u16());
    log  <<"  |r="<<p._hexlify_u16(getvalue._u16());
    log  <<"  |g="<<p._hexlify_u16(getvalue._u16());
    log  <<"  |b="<<p._hexlify_u16(getvalue._u16())<< std::endl;
    log  <<"  |-ql image scale: "<<p._hexlify_u16(getvalue._u16()) << std::endl;
    log  <<"  |-TOC =>ATTR: "<<p._hexlify_u32(ATTR_id) << "|OBJ=>'"  << std::endl; getvalue.pos_advance(4);
    //TODO add ATTR dump here << getvalue._fourcc(TOC[getvalue._u32(ATTR_id])<<"'"
    log  <<"  |-was iconic: "<<p._hexlify_u8(getvalue._u8())<< std::endl;

    //BINF
    if (atom_fourcc==fourcc("BINF")){
        log  <<"  |-(BINF unknown 02 01 + [obj]):"<<p._hexlify_u16(getvalue._u16())<<"|" <<p._hexlify_u32(getvalue._objref())<< "|=>OBJ'" << std::endl;
    }

    log << BOB_end_dump() << std::endl;

    return log.str();
}



//TODO: ------------------* Components ---
//TODO: --- COMP::Atom --- Component
class    atom_COMP:public avbATOM
{
public:  atom_COMP(){ atom_fourcc = fourcc("COMP");}
    bool create_object_from_BOB();
    std::string dump(void);
};
bool atom_COMP::create_object_from_BOB(){
    OBJisValid = false;
    if (BOBisValid!=true) return OBJisValid;
    
    getvalue._u8_assert(0x02);
    getvalue._u8_assert(0x03);
    read_objref();;//left.bob
    read_objref();;//right.bob
    getvalue._u16();//self.mediakind
    getvalue._u32();//editrate.mantissa
    getvalue._u16();//editrate.exp10  editrate=float(mantissa) * pow(10, exp10)
    getvalue._string();//name
    getvalue._string();//effect.id
    read_objref();//self.attributes
    read_objref();//self.session attrs
    read_objref();//self.precomputed
    while (uint8_t it = getvalue.getnext_typetag()){
        switch (it){
            case 0x01:
                getvalue._u8_assert(0x48);
                read_objref();//self.paramlist
                break;
            default:
                assert(false); //unknown tag
                break;
        }
    }

    OBJisValid = BOBisValid;
    return OBJisValid;
}

std::string atom_COMP::dump(void) {
    if (OBJisValid==false) return avbATOM::dumpInvalidObject();

    std::stringstream log;
    Printer p(&getvalue);
    getvalue.set_pos(0L);
    
    log  << BOB_begin_dump ();
    
    
    getvalue._u8_assert(0x02);
    getvalue._u8_assert(0x03);
    
    log <<"  |-left.bob[obj]: " << p._hexlify_u32(getvalue._objref()) << std::endl;//left.bob
    log <<"  |-right.bob[obj]: " << p._hexlify_u32(getvalue._objref()) << std::endl;//right.bob
    log <<"  |-mediakind: " << p._hexlify_u16(getvalue._u16()) << std::endl;//self.mediakind
    log <<"  |-edit rate: " << getvalue._u32() << "/(10^"<< getvalue._s16() << ")" << std::endl;// editrate=float(mantissa) * pow(10, exp10)
    log <<"  |-name: " << getvalue._string() << std::endl;//name
    log <<"  |-effectid: " << getvalue._string()<< std::endl;//effect.id
    log <<"  |-Self.attr[obj]: " << p._hexlify_u32(getvalue._objref())<< std::endl;//self.attributes
    log <<"  |-Session.attr[obj]: " << p._hexlify_u32(getvalue._objref())<< std::endl;//self.session attrs
    log <<"  |-precomputed[obj]: " << p._hexlify_u32(getvalue._objref())<< std::endl;//self.precomputed
    while (uint8_t it = getvalue.getnext_typetag()){
        switch (it){
            case 0x01:
                getvalue._u8_assert(0x48);
                log <<"  |-param: " << p._hexlify_u32(getvalue._objref())<< std::endl;//self.paramlist
                break;
            default:
                assert(false); //unknown tag
                break;
        }
    }
    
    
    if (atom_fourcc == fourcc("COMP")) {log << BOB_end_dump() << std::endl;}

    return log.str();
};
//TODO: --- SEQU::COMP --- Sequence
class    atom_SEQU:public atom_COMP
{
public:  atom_SEQU(){ atom_fourcc = fourcc("SEQU");}
    bool create_object_from_BOB();
    std::string dump(void);
};
bool atom_SEQU::create_object_from_BOB(){
    OBJisValid = false;
    if (BOBisValid!=true) return OBJisValid;
    if (atom_COMP::create_object_from_BOB()==false) return false;
    
    OBJisValid = BOBisValid;
    return OBJisValid;
}

std::string atom_SEQU::dump(void) {
    
    if (OBJisValid==false) return avbATOM::dumpInvalidObject();
    
    std::stringstream log;
    log << atom_COMP::dump();
    
    Printer p(&getvalue);
    
    if (atom_fourcc == fourcc("SEQU")) {log << BOB_end_dump() << std::endl;}
    
    return log.str();
};
//TODO: --- CLIP::COMP --- Clip
class    atom_CLIP:public atom_COMP
{
public:  atom_CLIP(){ atom_fourcc = fourcc("CLIP");}
    bool create_object_from_BOB();
    std::string dump(void);
};
bool atom_CLIP::create_object_from_BOB(){
    OBJisValid = false;
    if (BOBisValid!=true) return OBJisValid;
    if (atom_COMP::create_object_from_BOB()==false) return false;
    
    OBJisValid = BOBisValid;
    return OBJisValid;
}

std::string atom_CLIP::dump(void) {
    
    if (OBJisValid==false) return avbATOM::dumpInvalidObject();
    
    std::stringstream log;
    log << atom_COMP::dump();
    
    Printer p(&getvalue);
    
    if (atom_fourcc == fourcc("CLIP")) {log << BOB_end_dump() << std::endl;}
    
    return log.str();
};
//TODO: --- SCLP::CLIP --- SourceClip
class    atom_SCLP:public atom_CLIP
{
public:  atom_SCLP(){ atom_fourcc = fourcc("SCLP");}
    bool create_object_from_BOB();
    std::string dump(void);
};
bool atom_SCLP::create_object_from_BOB(){
    OBJisValid = false;
    if (BOBisValid!=true) return OBJisValid;
    if (atom_CLIP::create_object_from_BOB()==false) return false;
    
    OBJisValid = BOBisValid;
    return OBJisValid;
}

std::string atom_SCLP::dump(void) {
    
    if (OBJisValid==false) return avbATOM::dumpInvalidObject();
    
    std::stringstream log;
    log << atom_CLIP::dump();
    
    Printer p(&getvalue);
    
    if (atom_fourcc == fourcc("SCLP")) {log << BOB_end_dump() << std::endl;}
    
    return log.str();
};
//TODO: --- TCCP::CLIP --- Timecode
class    atom_TCCP:public atom_CLIP
{
public:  atom_TCCP(){ atom_fourcc = fourcc("TCCP");}
    bool create_object_from_BOB();
    std::string dump(void);
};
bool atom_TCCP::create_object_from_BOB(){
    OBJisValid = false;
    if (BOBisValid!=true) return OBJisValid;
    if (atom_CLIP::create_object_from_BOB()==false) return false;
    
    OBJisValid = BOBisValid;
    return OBJisValid;
}

std::string atom_TCCP::dump(void) {
    
    if (OBJisValid==false) return avbATOM::dumpInvalidObject();
    
    std::stringstream log;
    log << atom_CLIP::dump();
    
    Printer p(&getvalue);
    
    if (atom_fourcc == fourcc("TCCP")) {log << BOB_end_dump() << std::endl;}
    
    return log.str();
};
//TODO: --- ECCP::CLIP --- Edgecode
class    atom_ECCP:public atom_CLIP
{
public:  atom_ECCP(){ atom_fourcc = fourcc("ECCP");}
    bool create_object_from_BOB();
    std::string dump(void);
};
bool atom_ECCP::create_object_from_BOB(){
    OBJisValid = false;
    if (BOBisValid!=true) return OBJisValid;
    if (atom_CLIP::create_object_from_BOB()==false) return false;
    
    OBJisValid = BOBisValid;
    return OBJisValid;
}

std::string atom_ECCP::dump(void) {
    
    if (OBJisValid==false) return avbATOM::dumpInvalidObject();
    
    std::stringstream log;
    log << atom_CLIP::dump();
    
    Printer p(&getvalue);
    
    if (atom_fourcc == fourcc("ECCP")) {log << BOB_end_dump() << std::endl;}
    
    return log.str();
};
//TODO: --- TRKR::CLIP --- TrackRef
class    atom_TRKR:public atom_CLIP
{
public:  atom_TRKR(){ atom_fourcc = fourcc("TRKR");}
    bool create_object_from_BOB();
    std::string dump(void);
};
bool atom_TRKR::create_object_from_BOB(){
    OBJisValid = false;
    if (BOBisValid!=true) return OBJisValid;
    if (atom_CLIP::create_object_from_BOB()==false) return false;
    
    OBJisValid = BOBisValid;
    return OBJisValid;
}

std::string atom_TRKR::dump(void) {
    
    if (OBJisValid==false) return avbATOM::dumpInvalidObject();
    
    std::stringstream log;
    log << atom_CLIP::dump();
    
    Printer p(&getvalue);
    
    if (atom_fourcc == fourcc("TRKR")) {log << BOB_end_dump() << std::endl;}
    
    return log.str();
};
//TODO: --- PRCL::CLIP --- ParamClip
class    atom_PRCL:public atom_CLIP
{
public:  atom_PRCL(){ atom_fourcc = fourcc("PRCL");}
    bool create_object_from_BOB();
    std::string dump(void);
};
bool atom_PRCL::create_object_from_BOB(){
    OBJisValid = false;
    if (BOBisValid!=true) return OBJisValid;
    if (atom_CLIP::create_object_from_BOB()==false) return false;
    
    OBJisValid = BOBisValid;
    return OBJisValid;
}

std::string atom_PRCL::dump(void) {
    
    if (OBJisValid==false) return avbATOM::dumpInvalidObject();
    
    std::stringstream log;
    log << atom_CLIP::dump();
    
    Printer p(&getvalue);
    
    if (atom_fourcc == fourcc("PRCL")) {log << BOB_end_dump() << std::endl;}
    
    return log.str();
};
//TODO: --- CTRL::CLIP --- ControlClip
class    atom_CTRL:public atom_CLIP
{
public:  atom_CTRL(){ atom_fourcc = fourcc("CTRL");}
    bool create_object_from_BOB();
    std::string dump(void);
};
bool atom_CTRL::create_object_from_BOB(){
    OBJisValid = false;
    if (BOBisValid!=true) return OBJisValid;
    if (atom_CLIP::create_object_from_BOB()==false) return false;
    
    OBJisValid = BOBisValid;
    return OBJisValid;
}

std::string atom_CTRL::dump(void) {
    
    if (OBJisValid==false) return avbATOM::dumpInvalidObject();
    
    std::stringstream log;
    log << atom_CLIP::dump();
    
    Printer p(&getvalue);
    
    if (atom_fourcc == fourcc("CTRL")) {log << BOB_end_dump() << std::endl;}
    
    return log.str();
};
//TODO: --- FILL::CLIP --- Filler
class    atom_FILL:public atom_CLIP
{
public:  atom_FILL(){ atom_fourcc = fourcc("FILL");}
    bool create_object_from_BOB();
    std::string dump(void);
};
bool atom_FILL::create_object_from_BOB(){
    OBJisValid = false;
    if (BOBisValid!=true) return OBJisValid;
    if (atom_CLIP::create_object_from_BOB()==false) return false;
    
    OBJisValid = BOBisValid;
    return OBJisValid;
}

std::string atom_FILL::dump(void) {
    
    if (OBJisValid==false) return avbATOM::dumpInvalidObject();
    
    std::stringstream log;
    log << atom_CLIP::dump();
    
    Printer p(&getvalue);
    
    if (atom_fourcc == fourcc("FILL")) {log << BOB_end_dump() << std::endl;}
    
    return log.str();
};


//TODO: ------------------* Track groups ---
//TODO: --- TRKG::COMP --- TrackGroup
class    atom_TRKG:public atom_COMP
{
public:  atom_TRKG(){ atom_fourcc = fourcc("TRKG");}
    bool create_object_from_BOB();
    std::string dump(void);
};
bool atom_TRKG::create_object_from_BOB(){
    OBJisValid = false;
    if (BOBisValid!=true) return OBJisValid;
    if (atom_COMP::create_object_from_BOB()==false) return false;
 
    getvalue._u8_assert(0x02);
    getvalue._u8_assert(0x08);
    getvalue._u8();//mc_mode
    getvalue._u32();//length
    getvalue._u32();//numscalars
    uint32_t trCount = getvalue._u32();//trackcount
    for (int i=0;i<trCount;i++){
        uint16_t flags = getvalue._u16();//flags for track
        
        if (flags & TRACK_LABEL_FLAG)
            getvalue._s16();
        
        else if (flags & TRACK_ATTRIBUTES_FLAG)
            read_objref();
        
        if (flags & TRACK_SESSION_ATTR_FLAG)
             read_objref();
        
        if (flags & TRACK_COMPONENT_FLAG)
             read_objref();
        
        if (flags & TRACK_FILLER_PROXY_FLAG)
             read_objref();
        
        if (flags & TRACK_BOB_DATA_FLAG)
             read_objref();
        
        if (flags & TRACK_CONTROL_CODE_FLAG)
            getvalue._s16();

        if (flags & TRACK_CONTROL_SUB_CODE_FLAG)
            getvalue._s16();

        if (flags & TRACK_START_POS_FLAG)
            getvalue._s32();

        if (flags & TRACK_READ_ONLY_FLAG)
            getvalue._u8();

        if (flags & TRACK_UNKNOWN_FLAGS)
            assert(false);//("Unknown Track Flag: %d" % flags);
        
    }
    while (uint8_t it = getvalue.getnext_typetag() ){
        switch (it){
            case 0x01:
                for (int i=0;i<trCount;i++)
                {
                    getvalue._u8_assert(0x45);
                    getvalue._s16();//locknumber
                }
                break;
            default:
                assert(false);
                break;
        }
    }

    return OBJisValid;
}

std::string atom_TRKG::dump(void) {
    if (OBJisValid==false) return avbATOM::dumpInvalidObject();

    std::stringstream log;
    log << atom_COMP::dump();

    Printer p(&getvalue);

    getvalue._u8_assert(0x02);
    getvalue._u8_assert(0x08);

    
    log <<"  |--------"<< std::endl;//left.bob
    log <<"  |-mcmode: " << p._hexlify_u8(getvalue._u8())<< std::endl;//mc_mode
    log <<"  |-length: " << getvalue._u32()<< std::endl;//length
    log <<"  |-numscalars: " << getvalue._u32()<< std::endl;//numscalars
    uint32_t trCount = getvalue._u32();//trackcount
    log <<"  |-track count: " << p._hexlify_u32(trCount) << std::endl;
    
    for (int i=0;i<trCount;i++){
        uint16_t flags = getvalue._u16();//flags for track
        log <<"  |-track["<< i <<"] flags: " << p._hexlify_u16(flags) ;

        if (flags & TRACK_LABEL_FLAG)
        {
            log <<"|label: " <<  p._hexlify_u16(getvalue._s16());
        }
        
        else if (flags & TRACK_ATTRIBUTES_FLAG)
        {
             log <<"|track_attr[obj]: " <<p._hexlify_u32(getvalue._objref());
        }
        
        if (flags & TRACK_SESSION_ATTR_FLAG)
        {
            log <<"|session_attr[obj]: " <<p._hexlify_u32(getvalue._objref());
        }
        
        if (flags & TRACK_COMPONENT_FLAG)
        {
             log <<"|comp[obj]: " <<p._hexlify_u32(getvalue._objref());
        }
        
        if (flags & TRACK_FILLER_PROXY_FLAG)
        {
             log <<"|filler_proxy[obj]: " <<p._hexlify_u32(getvalue._objref());
        }
        
        if (flags & TRACK_BOB_DATA_FLAG)
        {
            log <<"|bob_data[obj]: " <<p._hexlify_u32(getvalue._objref());
        }
        
        if (flags & TRACK_CONTROL_CODE_FLAG)
        {
            log <<"|control code: " << p._hexlify_u16(getvalue._s16());
        }
        
        if (flags & TRACK_CONTROL_SUB_CODE_FLAG)
        {
            log <<"|control sub code: " <<  p._hexlify_u16(getvalue._s16());
        }
        
        if (flags & TRACK_START_POS_FLAG)
        {
            log <<"|start pos: " << p._hexlify_u32(getvalue._s32());
        }
        
        if (flags & TRACK_READ_ONLY_FLAG)
        {
            log <<"|read only: " << p._hexlify_u8(getvalue._u8());
        }
        
        if (flags & TRACK_UNKNOWN_FLAGS)
            assert(false);//("Unknown Track Flag: %d" % flags);
        
        log << std::endl;
        
    }
    while (uint8_t it = getvalue.getnext_typetag() ){
        switch (it){
            case 0x01:
            {
                for (int i=0;i<trCount;i++)
                {
                    getvalue._u8_assert(0x45);
                    log <<"  |locknumber: " << p._hexlify_u16(getvalue._s16()) << std::endl;//locknumber
                }
            }
                break;
            default:
                assert(false);
                break;
        }
    }

    
    
    
    
    if (atom_fourcc == fourcc("TRKG")) {log << BOB_end_dump() << std::endl;}

    return log.str();
};
//TODO: --- TKFX::TRKG --- TrackEffect
class    atom_TKFX:public atom_TRKG
{
public:  atom_TKFX(){ atom_fourcc = fourcc("TKFX");}
    bool create_object_from_BOB();
    std::string dump(void);
};
bool atom_TKFX::create_object_from_BOB(){
    OBJisValid = false;
    if (BOBisValid!=true) return OBJisValid;
    if (atom_TRKG::create_object_from_BOB()==false) return false;
    
    OBJisValid = BOBisValid;
    return OBJisValid;
}

std::string atom_TKFX::dump(void) {
    
    if (OBJisValid==false) return avbATOM::dumpInvalidObject();
    
    std::stringstream log;
    log << atom_TRKG::dump();
    
    Printer p(&getvalue);
    
    if (atom_fourcc == fourcc("TKFX")) {log << BOB_end_dump() << std::endl;}
    
    return log.str();
};
//TODO: --- PVOL::TKFX --- PanVolumeEffect
class    atom_PVOL:public atom_TKFX
{
public:  atom_PVOL(){ atom_fourcc = fourcc("PVOL");}
    bool create_object_from_BOB();
    std::string dump(void);
};
bool atom_PVOL::create_object_from_BOB(){
    OBJisValid = false;
    if (BOBisValid!=true) return OBJisValid;
    if (atom_TKFX::create_object_from_BOB()==false) return false;
    
    OBJisValid = BOBisValid;
    return OBJisValid;
}

std::string atom_PVOL::dump(void) {
    
    if (OBJisValid==false) return avbATOM::dumpInvalidObject();
    
    std::stringstream log;
    log << atom_TKFX::dump();
    
    Printer p(&getvalue);
    
    if (atom_fourcc == fourcc("PVOL")) {log << BOB_end_dump() << std::endl;}
    
    return log.str();
};
//TODO: --- ASPI::TKFX --- AudioSuitePluginEffect
class    atom_ASPI:public atom_TKFX
{
public:  atom_ASPI(){ atom_fourcc = fourcc("ASPI");}
    bool create_object_from_BOB();
    std::string dump(void);
};
bool atom_ASPI::create_object_from_BOB(){
    OBJisValid = false;
    if (BOBisValid!=true) return OBJisValid;
    if (atom_TKFX::create_object_from_BOB()==false) return false;
    
    OBJisValid = BOBisValid;
    return OBJisValid;
}

std::string atom_ASPI::dump(void) {
    
    if (OBJisValid==false) return avbATOM::dumpInvalidObject();
    
    std::stringstream log;
    log << atom_TKFX::dump();
    
    Printer p(&getvalue);
    
    if (atom_fourcc == fourcc("ASPI")) {log << BOB_end_dump() << std::endl;}
    
    return log.str();
};
//TODO: --- EQMB::TKFX --- EqualizerMultiBand
//TODO: --- WARP::TRKG --- TimeWarp
//TODO: --- MASK::WARP --- CaptureMask
//TODO: --- STRB::WARP --- StrobeEffect
//TODO: --- SPED::WARP --- MotionEffect
//TODO: --- REPT::WARP --- Repeat
//TODO: --- RSET::TRKG --- EssenceGroup
//TODO: --- TNFX::TRKG --- TransitionEffect
//TODO: --- SLCT::TRKG --- Selector
//TODO: --- CMPO::TRKG --- Composition
class    atom_CMPO:public atom_TRKG
{
public:  atom_CMPO(){ atom_fourcc = fourcc("CMPO");}
    MDVxUUID mobid;
    uint8_t mobtype;
    uint32_t usagecode;
    
    bool create_object_from_BOB();
    std::string dump(void);
};
bool atom_CMPO::create_object_from_BOB(){
    OBJisValid = false;
    if (BOBisValid!=true) return OBJisValid;
    if (atom_TRKG::create_object_from_BOB()==false) return false;

    getvalue._u8_assert(0x02);
    getvalue._u8_assert(0x02);
    
    getvalue._u32(); //mobidhi
    getvalue._u32(); //mobidlo
    getvalue._u32(); //datetime
    mobtype = getvalue._u8(); //mobtype
    usagecode = getvalue._u32(); //usagecode
    read_objref();//descriptor
    
    while (uint8_t it = getvalue.getnext_typetag() ){
        switch (it){
            case 0x01:
                getvalue._u8_assert(0x47);
                getvalue._u32(); //last modified
                break;
            case 0x02:
                getvalue._u8_assert(0x41);
                getvalue.readMobID(&mobid);
                break;
            default:
                assert(false);
                break;
        }
    }
    
    OBJisValid = BOBisValid;
    return OBJisValid;
}

std::string atom_CMPO::dump(void) {
    
    if (OBJisValid==false) return avbATOM::dumpInvalidObject();

    std::stringstream log;
    log << atom_TRKG::dump();

    Printer p(&getvalue);
    
    getvalue._u8_assert(0x02);
    getvalue._u8_assert(0x02);
    
    
    log <<"  |--------"<< std::endl;//left.bob
    log <<"  |-mobidhi: " << p._hexlify_u32(getvalue._u32())<< std::endl;//mobidhi
    log <<"  |-mobidlo: " << p._hexlify_u32(getvalue._u32())<< std::endl;//mobidlo
    log <<"  |-lastmod: " << p._hexlify_u32(getvalue._u32())<< std::endl;//last modified
    log <<"  |-mobtype: " << p._hexlify_u8(getvalue._u8())<< std::endl;//mobtype
    log <<"  |-usagecode: " << p._hexlify_u32(getvalue._u32())<< std::endl; //usagecode
    log <<"  |-descriptor[obj]: " << p._hexlify_u32(getvalue._objref())<< std::endl; //usagecode

    while (uint8_t it = getvalue.getnext_typetag() ){
        switch (it){
            case 0x01:
                getvalue._u8_assert(0x47);
                log <<"  |-creation time: " << p._hexlify_u32(getvalue._u32())<< std::endl;//creation time
                break;
            case 0x02:
                getvalue._u8_assert(0x41);
                 log <<"  |-mobid: " << p._hexlify_UID(&mobid) << std::endl;
                 getvalue.readMobID(&mobid);
                break;
            default:
                assert(false);
                break;
        }
    }
    
    if (atom_fourcc == fourcc("CMPO")) {log << BOB_end_dump() << std::endl;}

    return log.str();
};


//TODO: ------------------*  Misc      ---
//TODO: --- FILE::Atom --- MacFileLocator
//TODO: --- WINF::Atom --- WinFileLocator


class    atom_FILE:public avbATOM
{
public:  atom_FILE(){ atom_fourcc = fourcc("FILE");}
    bool create_object_from_BOB();
    std::string dump(void);
    
    std::string path;
    std::string path_posix;
    std::string path_utf8;
    std::string path2_utf8;
};
bool atom_FILE::create_object_from_BOB(){
    OBJisValid = false;
    if (BOBisValid!=true) return OBJisValid;

    getvalue.set_pos(0x0L);
    getvalue._u8_assert(0x2);//tag
    getvalue._u8();//version
    path = getvalue._string();
    
    while (uint8_t it = getvalue.getnext_typetag()){
        switch (it){
            case 0x01:
                getvalue._u8_assert(0x4C);
                path_posix = getvalue._string();
                break;
            case 0x02:
                getvalue._u8_assert(0x4C);
                path_utf8 = getvalue._string();
                break;
            case 0x03:
                getvalue._u8_assert(0x4C);
                path2_utf8 = getvalue._string();
                break;
            default:
                assert(false);
                break;
        }
    }
    OBJisValid = BOBisValid;
    return OBJisValid;
}

std::string atom_FILE::dump(void) {
    if (OBJisValid==false) return avbATOM::dumpInvalidObject();

    std::stringstream log;
    getvalue.set_pos(0L);
    log  << BOB_begin_dump ();
    getvalue._u8_assert(0x2);//tag
    getvalue._u8_assert(0x2);//version
    log <<"  |-path: "  <<  getvalue._string() << std::endl;
    while (uint8_t it = getvalue.getnext_typetag()){
        switch (it){
            case 0x01:
                getvalue._u8_assert(0x4C);
                log <<"  |-path_posix: " <<getvalue._string()  << std::endl;
                break;
            case 0x02:
                getvalue._u8_assert(0x4C);
                log <<"  |-path_utf8: " << getvalue._string() << std::endl;
                break;
            case 0x03:
                getvalue._u8_assert(0x4C);
                log <<"  |-path2_utf8: " << getvalue._string()  << std::endl;
                break;
            default:
                assert(false);
                break;
        }
    }

    log << BOB_end_dump() << std::endl;

    return log.str();
};



//TODO: --- URLL::Atom --- URLLocator
//TODO: --- GRFX::Atom --- GraphicEffect
//TODO: --- SHLP::Atom --- ShapeList
//TODO: --- CCFX::Atom --- ColorCorrectionEffect
//TODO: --- FXPS::Atom --- EffectParamList
//TODO: --- AVUP::Atom --- CFUserParam
//TODO: --- PRIT::Atom --- ParameterItems
//TODO: --- MSML::Atom --- MSMLocator

class    atom_MSML:public avbATOM
{
public:  atom_MSML(){ atom_fourcc = fourcc("MSML");}
    bool create_object_from_BOB();
    std::string dump(void);
    MDVxUUID mobid;
    std::string lastknownvolume;
};
bool atom_MSML::create_object_from_BOB(){
    OBJisValid = false;
    if (BOBisValid!=true) return OBJisValid;
    getvalue.set_pos(0x0L);
    getvalue._u8_assert(0x2);//tag
    getvalue._u8_assert(0x2);//version
    mobid.makeFrom2(BOB.data()+getvalue.get_pos());
    getvalue.pos_advance(8);
    lastknownvolume = getvalue._string();
    OBJisValid = BOBisValid;
    return OBJisValid;
}

std::string atom_MSML::dump(void) {
    if (OBJisValid==false) return avbATOM::dumpInvalidObject();

    std::stringstream log;
    Printer p(&getvalue);
    getvalue.set_pos(0L);
    log  << BOB_begin_dump ();
    getvalue._u8_assert(0x2);//tag
    getvalue._u8_assert(0x2);//version
    log <<"  |-mobid: " << p._hexlify_short_UID(&mobid)<< std::endl;
    getvalue.pos_advance(8);
    log <<"  |-lastknownvolume: " << lastknownvolume<< std::endl;
    getvalue.pos_advance(getvalue._u16());
    
    while (uint8_t it = getvalue.getnext_typetag()){
        switch (it){
            case 0x01:
                getvalue._u8_assert(0x47);
                log <<"  |-domain_type: " <<getvalue._u32()  << std::endl;
                break;
            case 0x02:
            {
                getvalue._u8_assert(0x41);
                MDVxUUID smptemobid;
                getvalue.readMobID(&smptemobid);
                log <<"  |-mobid: " <<  p._hexlify_UID(&smptemobid)<< std::endl;
                break;
            }
            case 0x03:
                getvalue._u8_assert(0x4C);
                log <<"  |-last_known_volume_utf8: " << getvalue._string()  << std::endl;
                break;
            default:
                assert(false);
                break;
        }
    }
    
    log << BOB_end_dump() << std::endl;
    
    return log.str();
};


//TODO: --- APOS::Atom ---
//TODO: --- ABOB::Atom ---
//TODO: --- DIDP::Atom ---
//TODO: --- MPGP::Atom ---
//TODO: --- MCBR::Atom ---
//TODO: --- MCMR::Atom ---
//TODO: --- TMBC::Atom --- (Marker)
//TODO: --- TKMN::Atom ---
//TODO: --- TKDS::Atom ---
//TODO: --- TKPS::Atom ---
//TODO: --- TKDA::Atom ---
//TODO: --- TKPA::Atom ---


//TODO: ------------------*  Essence   ---
//TODO: --- MDES::Atom ---
//TODO: --- mdes::Atom ---
//TODO: --- MDTP::Atom ---
//TODO: --- MDFM::Atom ---
//TODO: --- MDNG::Atom ---
//TODO: --- MDFL::Atom ---
//TODO: --- MULD::Atom ---
//TODO: --- WAVE::Atom ---
//TODO: --- AIFC::Atom ---
//TODO: --- PCMA::Atom ---
//TODO: --- DIDD::Atom ---
//TODO: --- CDCI::Atom ---
//TODO: --- MPGI::Atom ---
//TODO: --- JPED::Atom ---
//TODO: --- RGBA::Atom ---



//TODO: ------------------* Attributes ---
//TODO: --- ATTR::Atom --- (Attributes)

class    atom_ATTR:public avbATOM
{
public:  atom_ATTR(){ atom_fourcc = fourcc("ATTR");attrCount=0;}
    bool create_object_from_BOB();
    std::string dump(void);
    uint32_t attrCount ;
};
bool atom_ATTR::create_object_from_BOB(){
    OBJisValid = false;
    if (BOBisValid!=true) return OBJisValid;
    getvalue.set_pos(0x0L);
    getvalue._u8_assert(0x2);//tag
    getvalue._u8_assert(0x1);//version
    attrCount = getvalue._u32();
    uint32_t i=0;
    while (i < attrCount){
        uint32_t attrtype = getvalue._u32();
        getvalue.pos_advance(getvalue._u16());//getvalue._string();
        switch (attrtype){
            case ATTR_IntegerAttribute:
            {
                getvalue._u32();
                break;
            }
            case ATTR_StringAttribute:
            {
                getvalue.pos_advance(getvalue._u16());//getvalue._string();
                break;
            }
            case ATTR_ObjectAttribute:
            {
               read_objref();
               break;
            }
            case ATTR_DataValueAttribute:
            {
                uint32_t datasize = getvalue._u32();
                getvalue.pos_advance(datasize);
                break;
            }
            default:
                assert(false);
                break;
        }
        i++;
    }
    OBJisValid = BOBisValid;
    return OBJisValid;
}

std::string atom_ATTR::dump(void) {
    if (OBJisValid==false) return avbATOM::dumpInvalidObject();

    std::stringstream log;
    Printer p(&getvalue);
    getvalue.set_pos(0L);
    log  << BOB_begin_dump ();
    getvalue._u8_assert(0x2);//tag
    getvalue._u8_assert(0x1);//version
    getvalue.pos_advance(4);
    uint32_t i=0;
    while (i < attrCount){
        uint32_t attrtype = getvalue._u32();
        log  <<"  |->attr:"<<getvalue._string();
        switch (attrtype){
            case ATTR_IntegerAttribute:
            {
                log <<" [int] " <<p._hexlify_u32(getvalue._u32()) << std::endl;
                break;
            }
            case ATTR_StringAttribute:
            {
                log <<" [string]: " <<getvalue._string()  << std::endl;
                break;
            }
            case ATTR_ObjectAttribute:
            {
                log <<" [obj] " << p._hexlify_u32(getvalue._objref())  << std::endl;
                break;
            }
            case ATTR_DataValueAttribute:
            {
                uint32_t datasize = getvalue._u32();
                if ( datasize > 32)
                {
                log <<" [data size= " <<p._hexlify_u32(datasize)<<"]"<< std::endl;
                log << p._pretty_dump(getvalue.get_pos(),datasize) << std::endl;
                }
                else
                {
                    log <<" [data size= " <<p._hexlify_u32(datasize)<<"] |";
                    log << p._dump(getvalue.get_pos(),datasize) << std::endl;
                }
                getvalue.pos_advance(datasize);
                break;
            }
            default:
                assert(false);
                break;
        }
        i++;
    
    }
    log << BOB_end_dump() << std::endl;
    return log.str();
};


//TODO: --- PRLS::Atom --- (Parameter list)
//TODO: --- TMCS::Atom --- (Marker List)

//TODO: ------------------* Uknown ---
//TODO: --- ANCD::Atom ---
//TODO: --- Iprj::Atom ---
//TODO: --- RLED::Atom ---
//TODO: --- SETG::Atom ---
//TODO: --- SLPI::Atom ---


//-----------------------------------------------------------------
//TODO:class prototype
//class    atom_FILE:public avbATOM
//{
//public:  atom_FILE(){ atom_fourcc = fourcc("X3X3");}
//    bool create_object_from_BOB();
//    std::string dump(void);
//};
//bool atom_FILE::::create_object_from_BOB(){
//    OBJisValid = false;
//    if (BOBisValid!=true) return OBJisValid;
//    //methods here
//    OBJisValid = BOBisValid;
//    return OBJisValid;
//}
//
//std::string atom_FILE::dump(void) {
//    if (OBJisValid==false) return avbATOM::dumpInvalidObject();
//    std::stringstream log;
//
//    log  << BOB_begin_dump ();
//    log << BOB_end_dump( position) << std::endl;
//
//    return log.str();
//};


//TODO: --- avbTOC::Methods ---
//-----------------------------------------------------------------

bool avbTOC::read(std::ifstream * f){
    next_TOCid = 0;
    std::unique_ptr<avbATOM> OBJD = factory.Create(fourcc("OBJD"));
    TOC.push_back(std::move(OBJD));
    TOC[0]->init(this);
    bool result = TOC[0]->BOB_read(f);
    if (result!=true)return false;
    char fcc[4];
    while (++next_TOCid<TOC.size() )
    {
        f->read(fcc,4);
        f->seekg(-4, std::ios::cur);
        TOC[next_TOCid] = factory.Create(MAKEFOURCC(fcc[0], fcc[1], fcc[2], fcc[3]));
        TOC[next_TOCid]->init(this);
        TOC[next_TOCid]->TOC_id=next_TOCid;
        if (TOC[next_TOCid]->BOB_read(f)==false) break;
    }
    
    for (uint32_t i = 1;i<next_TOCid;i++){
        TOC[i]->create_object_from_BOB();
    }

    return result;
}
std::string avbTOC::dump (void){
    std::string log;
    for (uint32_t i = 0;i<TOC.size();i++){
        log.append(TOC[i]->dump());
    }
    return log;
}

avbATOM *  avbTOC:: at (uint32_t TOC_id){
    return TOC.at(TOC_id).get();
};


//-----------------------------------------------------------------
//TODO: add more types to ATOMFactory

ATOMFactory::ATOMFactory()
{

    this -> Register<fourcc("ATOM"),avbATOM>();
    this -> Register<fourcc("OBJD"),atom_OBJD>();
    this -> Register<fourcc("ABIN"),atom_ABIN>(); //ABIN root object
    this -> Register<fourcc("BINF"),atom_ABIN>(); //BINF root object
    
    this -> Register<fourcc("FILE"),atom_FILE>(); //FILE MacFileLocator
    this -> Register<fourcc("WINF"),atom_FILE>(); //WINF WinFileLocator
    this -> Register<fourcc("ATTR"),atom_ATTR>(); //ATTR Attributes
    this -> Register<fourcc("MSML"),atom_MSML>(); //MSMLocator

    this -> Register<fourcc("CMPO"),atom_CMPO>(); //CMPO Composition COMP|TRKG|CMPO

 
  
//    this -> Register<fourcc("AIFC"),atom_AIFC>(); //AIFFDescriptor
//    this -> Register<fourcc("ANCD"),atom_ANCD>();
    this -> Register<fourcc("ASPI"),atom_ASPI>();
//    this -> Register<fourcc("AVUP"),atom_AVUP>();
//    this -> Register<fourcc("BVst"),atom_BVst>(); //ABinViewSetting
//    this -> Register<fourcc("CCFX"),atom_CCFX>(); //Color correction FX
//    this -> Register<fourcc("CDCI"),atom_CDCI>(); //CDCIDescriptor
    this -> Register<fourcc("CTRL"),atom_CTRL>();
//    this -> Register<fourcc("DIDP"),atom_DIDP>(); //DIDPosition
    this -> Register<fourcc("ECCP"),atom_ECCP>(); //AEdgecodeClip
//    this -> Register<fourcc("EQMB"),atom_EQMB>();
    this -> Register<fourcc("FILL"),atom_FILL>(); //AFillerClip
//    this -> Register<fourcc("FXPS"),atom_FXPS>(); //AEffectKFList
//    this -> Register<fourcc("GRFX"),atom_GRFX>(); //AGraphicEffectAttr
//    this -> Register<fourcc("Iprj"),atom_Iprj>();
//    this -> Register<fourcc("JPED"),atom_JPED>();
//    this -> Register<fourcc("MASK"),atom_MASK>(); //ACaptureMask
//    this -> Register<fourcc("MCBR"),atom_MCBR>(); //Bin Reference
//    this -> Register<fourcc("MCMR"),atom_MCMR>();
//    this -> Register<fourcc("MDES"),atom_MDES>(); //AMediaDesc
//    this -> Register<fourcc("mdes"),atom_mdes>(); //AMediaDesc - omf2MDES
//    this -> Register<fourcc("MDFL"),atom_MDFL>(); //AMediaFile
//    this -> Register<fourcc("MDFM"),atom_MDFM>();
//    this -> Register<fourcc("MDNG"),atom_MDNG>();
//    this -> Register<fourcc("MDTP"),atom_MDTP>(); //ATapeMediaDesc
//    this -> Register<fourcc("MPGI"),atom_MPGI>();
//    this -> Register<fourcc("MPGP"),atom_MPGP>();
//    this -> Register<fourcc("MULD"),atom_MULD>();
//    this -> Register<fourcc("PCMA"),atom_PCMA>(); //PCM audio
    this -> Register<fourcc("PRCL"),atom_PRCL>(); //AParamClip
//    this -> Register<fourcc("PRIT"),atom_PRIT>(); //aPAram???
//    this -> Register<fourcc("PRLS"),atom_PRLS>(); //AParamList
    this -> Register<fourcc("PVOL"),atom_PVOL>(); //APanVolEffect
//    this -> Register<fourcc("REPT"),atom_REPT>();
//    this -> Register<fourcc("RGBA"),atom_RGBA>(); //RGBADescriptor
//    this -> Register<fourcc("RLED"),atom_RLED>();
//    this -> Register<fourcc("RSET"),atom_RSET>(); //ARepSet
    this -> Register<fourcc("SCLP"),atom_SCLP>(); //ASourceClip
    this -> Register<fourcc("SEQU"),atom_SEQU>(); //ASequence
//    this -> Register<fourcc("SETG"),atom_SETG>(); //ASettingGroup - Settings only -
//    this -> Register<fourcc("SHLP"),atom_SHLP>();
//    this -> Register<fourcc("SLCT"),atom_SLCT>();
//    this -> Register<fourcc("SLPI"),atom_SLPI>();
//    this -> Register<fourcc("SPED"),atom_SPED>();
//    this -> Register<fourcc("STRB"),atom_STRB>();
    this -> Register<fourcc("TCCP"),atom_TCCP>(); //ATimecodeClip
//    this -> Register<fourcc("TKDA"),atom_TKDA>();
//    this -> Register<fourcc("TKDS"),atom_TKDS>();
    this -> Register<fourcc("TKFX"),atom_TKFX>(); //ATrackEffect
//    this -> Register<fourcc("TKPA"),atom_TKPA>();
//    this -> Register<fourcc("TKPS"),atom_TKPS>();
//    this -> Register<fourcc("TMBC"),atom_TMBC>(); //ATmpCrumb
//    this -> Register<fourcc("TMCS"),atom_TMCS>(); //TmpBreadCrumbs
//    this -> Register<fourcc("TNFX"),atom_TNFX>(); //ATransEffect
    this -> Register<fourcc("TRKR"),atom_TRKR>(); //ATrackRef
//    this -> Register<fourcc("URLL"),atom_URLL>();
//    this -> Register<fourcc("WAVE"),atom_WAVE>(); //WAVEDescriptor
    }





