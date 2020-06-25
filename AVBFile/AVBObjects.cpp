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
    referenceCount = 1;
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
    if ((BOB[0]==0x02)&&(BOB[length-1]==0x03)) return true;
    return false;
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

bool avbATOM::read(std::ifstream *f){
    return BOB_read(f);
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
    std::stringstream log;
    Printer p(&getvalue);
    log << BOB_begin_dump();
    log  <<"  |-@begin:" <<  p._hexlify_u8(getvalue._u8()) << std::endl;;
    log << BOB_end_dump() << std::endl;

    return log.str();
};


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
    bool read(std::ifstream *f);
    std::string dump(void);

};

bool atom_OBJD::read(std::ifstream *f){
    std::streamsize avbsize =f->tellg();
    f->seekg(0, std::ios::beg);
    assert (avbsize > 0x77);
    if (avbsize < 0x77) // the header entry in avb file is 0x77 bytes long
    {
//        std::cout << "problem with AVB File :: too small " << std::endl; redir_flush;
        return false;

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
            return false;
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
        TOCRootObject = getvalue._u32();//tocrootobject
        hasReferencesToOtherOBJs++;
        pTOC->setRootObject(TOCRootObject);
        getvalue._fourcc_u32();    //IIII
        BINUID.makeFrom2(BOB.data()+getvalue.get_pos()); getvalue.pos_advance(8);
        getvalue._fourcc_u32();    //filetype
        getvalue._fourcc_u32();    //creator
        getvalue.pos_advance(getvalue._u16());//getvalue._string();        //creator[string]
        getvalue.pos_advance(16);  //[zeroes]
        
        assert( getvalue.get_pos()==0x77);
        if ( getvalue.get_pos()==0x77) return true;
        
    }
    
    return false;

}
    std::string atom_OBJD::dump(void)
    {
        std::stringstream log;
        Printer p(&getvalue);

        log  << BOB_begin_dump ();
        getvalue._string(); // Domain
        log <<"-"<< getvalue._fourcc() << std::endl;//ATOM OBJD
        log <<"  |-01:aobjdoc         : "<< getvalue._string()<< std::endl; //objdoc
        log <<"  |-02:domain version=4: "<< p._hexlify_u8(getvalue._u8()) << std::endl; //version
        log <<"  |-03:lastsave        : "<< getvalue._string()<< std::endl; //string:date
        log <<"  |-04:numobj          : "<< p._hexlify_u32(getvalue._u32())<< std::endl;//tocobj count
        log <<"  |-05:rootobj:selfroot: "<< p._hexlify_u32(getvalue._u32())<< std::endl;//tocrootobject
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
    bool read(std::ifstream *f);
    std::string dump(void);

    bool largebin = false;
    uint32_t BVst_id = 0;
    uint32_t ATTR_id = 0;
    MDVxUUID rootObjUID;
    uint32_t CMPOcount = 0;

};

bool atom_ABIN::read(std::ifstream *f){
    if (avbATOM::BOB_read(f)==false) return false; // reading BOB
    getvalue.set_pos(0x0L);
    
    getvalue._u8_assert(0x2);//tag

    largebin  = (getvalue._u8()==0xf);
    BVst_id = getvalue._u32();
    hasReferencesToOtherOBJs++;
    pTOC->pATOM_atTOC(BVst_id)->referenceCount++;
    rootObjUID.makeFrom2(BOB.data()+getvalue.get_pos()); getvalue.pos_advance(8);
    if (!largebin){ CMPOcount = static_cast<uint32_t> (getvalue._u16());}
    else { CMPOcount = static_cast<uint32_t> (getvalue._u32());}
    hasReferencesToOtherOBJs+=CMPOcount;
    for (uint32_t i = 0 ; i<CMPOcount;i++){
        uint32_t pObj = getvalue._u32();
        getvalue.pos_advance(9);
        pTOC->pATOM_atTOC(pObj)->referenceCount++;
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
    ATTR_id = getvalue._u32();
    pTOC->pATOM_atTOC(ATTR_id)->referenceCount++;
    getvalue.pos_advance(1);
    return true;
}
std::string atom_ABIN::dump(void){
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
        uint32_t pObj = getvalue._u32();
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
        log  <<"  |-(BINF unknown 02 01 + TOC):"<<p._hexlify_u16(getvalue._u16())<<"|" <<p._hexlify_u32(getvalue._u32())<< "|=>OBJ'" << std::endl;
    }

    log << BOB_end_dump() << std::endl;

    return log.str();
}



//TODO: ------------------* Components ---
//TODO: --- COMP::Atom --- Component
//TODO: --- SEQU::COMP --- Sequence
//TODO: --- CLIP::COMP --- Clip
//TODO: --- SCLP::CLIP --- SourceClip
//TODO: --- TCCP::CLIP --- Timecode
//TODO: --- ECCP::CLIP --- Edgecode
//TODO: --- TRKR::CLIP --- TrackRef
//TODO: --- PRCL::CLIP --- ParamClip
//TODO: --- CTRL::CLIP --- ControlClip
//TODO: --- FILL::CLIP --- Filler


//TODO: ------------------* Track groups ---
//TODO: --- TRKG::COMP --- TrackGroup
//TODO: --- TKFX::TRKG --- TrackEffect
//TODO: --- PVOL::TKFX --- PanVolumeEffect
//TODO: --- ASPI::TKFX --- AudioSuitePluginEffect
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



//TODO: ------------------*  Misc      ---
//TODO: --- FILE::Atom --- MacFileLocator
//TODO: --- WINF::Atom --- WinFileLocator


class    atom_FILE:public avbATOM
{
public:  atom_FILE(){ atom_fourcc = fourcc("FILE");}
    bool read(std::ifstream *f);
    std::string dump(void);
    std::string path;
    std::string path_posix;
    std::string path_utf8;
    std::string path2_utf8;
};
bool atom_FILE::read(std::ifstream *f){
    if (avbATOM::BOB_read(f)==false) return false; // reading BOB
    getvalue.set_pos(0x0L);
    getvalue._u8_assert(0x2);//tag
    getvalue._u8();//version
    path = getvalue._string();
    
    uint8_t it = getvalue.getnext_typetag();
    while (it){
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
        it = getvalue.getnext_typetag();
    }
    return true;
}

std::string atom_FILE::dump(void) {
    std::stringstream log;
    getvalue.set_pos(0L);
    log  << BOB_begin_dump ();
    getvalue._u8_assert(0x2);//tag
    getvalue._u8_assert(0x2);//version
    log <<"  |-path: "  <<  getvalue._string() << std::endl;
    uint8_t it = getvalue.getnext_typetag();
    while (it){
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
        it = getvalue.getnext_typetag();
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
public:  atom_MSML(){ atom_fourcc = fourcc("FILE");}
    bool read(std::ifstream *f);
    std::string dump(void);
    MDVxUUID mobid;
    std::string lastknownvolume;
};
bool atom_MSML::read(std::ifstream *f){
    if (avbATOM::BOB_read(f)==false) return false; // reading BOB
    getvalue.set_pos(0x0L);
    getvalue._u8_assert(0x2);//tag
    getvalue._u8_assert(0x2);//version
    mobid.makeFrom2(BOB.data()+getvalue.get_pos());
    getvalue.pos_advance(8);
    lastknownvolume = getvalue._string();
    return true;
}

std::string atom_MSML::dump(void) {
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
    
    uint8_t it = getvalue.getnext_typetag();
    while (it){
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
        it = getvalue.getnext_typetag();
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

#endif
class    atom_ATTR:public avbATOM
{
public:  atom_ATTR(){ atom_fourcc = fourcc("ATTR");attrCount=0;}
    bool read(std::ifstream *f);
    std::string dump(void);
    uint32_t attrCount ;
};
bool atom_ATTR::read(std::ifstream *f){
    if (avbATOM::BOB_read(f)==false) return false; // reading BOB
    getvalue.set_pos(0x0L);
    getvalue._u8_assert(0x2);//tag
    getvalue._u8_assert(0x1);//version
    attrCount = getvalue._u32();
    return true;
}

std::string atom_ATTR::dump(void) {
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
                log <<" [obj] " << p._hexlify_u32(getvalue._u32())  << std::endl;
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
//public:  atom_FILE(){ atom_fourcc = fourcc("FILE");}
//    bool read(std::ifstream *f);
//    std::string dump(void);
//};
//bool atom_FILE::read(std::ifstream *f){
//    if (avbATOM::BOB_read(f)==false) return false; // reading BOB
//    uint32_t cp=0;cp++;
//    return true;
//}
//
//std::string atom_FILE::dump(void) {
//    std::stringstream log;
//    uint32_t position = 0x0;
//    uint32_t cp=0;cp++;
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
    bool result = TOC[0]->read(f);
    if (result!=true)return false;
    char fcc[4];
    while (++next_TOCid<TOC.size() )
    {
        f->read(fcc,4);
        f->seekg(-4, std::ios::cur);
        TOC[next_TOCid] = factory.Create(MAKEFOURCC(fcc[0], fcc[1], fcc[2], fcc[3]));
        TOC[next_TOCid]->init(this);
        TOC[next_TOCid]->TOC_id=next_TOCid;
        if (TOC[next_TOCid]->read(f)==false) next_TOCid--;
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
avbATOM *  avbTOC::pATOM_atTOC(uint32_t TOC_id){
    return TOC[TOC_id].get();
};


//-----------------------------------------------------------------
//TODO: add more types to ATOMFactory

ATOMFactory::ATOMFactory()
{

    this -> Register<fourcc("ATOM"), avbATOM>();
    this -> Register<fourcc("OBJD"), atom_OBJD>();
    this -> Register<fourcc("ABIN"),atom_ABIN>(); //MCBin root object
    this -> Register<fourcc("BINF"),atom_ABIN>(); //MCFirst root object
    this -> Register<fourcc("FILE"),atom_FILE>(); //AMacFileLocator
    this -> Register<fourcc("WINF"),atom_FILE>(); //AWinFileLocator
    this -> Register<fourcc("ATTR"),atom_ATTR>(); //Attributes
    this -> Register<fourcc("MSML"),atom_MSML>(); //MSMLocator

    
    
    
    //        this -> Register<fourcc("BINF"),atom_BINF>();
    
//            this -> Register<fourcc("AIFC"),atom_AIFC>(); //AIFFDescriptor
    //        this -> Register<fourcc("ANCD"),atom_ANCD>();
    //        this -> Register<fourcc("ASPI"),atom_ASPI>();
    //        this -> Register<fourcc("AVUP"),atom_AVUP>();
    //        this -> Register<fourcc("BVst"),atom_BVst>(); //ABinViewSetting
    //        this -> Register<fourcc("CCFX"),atom_CCFX>(); //Color correction FX
    //        this -> Register<fourcc("CDCI"),atom_CDCI>(); //CDCIDescriptor
    //        this -> Register<fourcc("CMPO"),atom_CMPO>(); //AComposition
    //        this -> Register<fourcc("CTRL"),atom_CTRL>();
    //        this -> Register<fourcc("DIDP"),atom_DIDP>(); //DIDPosition
    //        this -> Register<fourcc("ECCP"),atom_ECCP>(); //AEdgecodeClip
    //        this -> Register<fourcc("EQMB"),atom_EQMB>();

    //        this -> Register<fourcc("FILL"),atom_FILL>(); //AFillerClip
    //        this -> Register<fourcc("FXPS"),atom_FXPS>(); //AEffectKFList
    //        this -> Register<fourcc("GRFX"),atom_GRFX>(); //AGraphicEffectAttr
    //        this -> Register<fourcc("Iprj"),atom_Iprj>();
    //        this -> Register<fourcc("JPED"),atom_JPED>();
    //        this -> Register<fourcc("MASK"),atom_MASK>(); //ACaptureMask
//            this -> Register<fourcc("MCBR"),atom_MCBR>(); //Bin Reference
    //        this -> Register<fourcc("MCMR"),atom_MCMR>();
    //        this -> Register<fourcc("MDES"),atom_MDES>(); //AMediaDesc
    //        this -> Register<fourcc("mdes"),atom_mdes>(); //AMediaDesc - omf2MDES
    //        this -> Register<fourcc("MDFL"),atom_MDFL>(); //AMediaFile
    //        this -> Register<fourcc("MDFM"),atom_MDFM>();
    //        this -> Register<fourcc("MDNG"),atom_MDNG>();
    //        this -> Register<fourcc("MDTP"),atom_MDTP>(); //ATapeMediaDesc
    //        this -> Register<fourcc("MPGI"),atom_MPGI>();
    //        this -> Register<fourcc("MPGP"),atom_MPGP>();
    //        this -> Register<fourcc("MULD"),atom_MULD>();
    //        this -> Register<fourcc("PCMA"),atom_PCMA>(); //PCM audio
    //        this -> Register<fourcc("PRCL"),atom_PRCL>(); //AParamClip
    //        this -> Register<fourcc("PRIT"),atom_PRIT>(); //aPAram???
    //        this -> Register<fourcc("PRLS"),atom_PRLS>(); //AParamList
    //        this -> Register<fourcc("PVOL"),atom_PVOL>(); //APanVolEffect
    //        this -> Register<fourcc("REPT"),atom_REPT>();
    //        this -> Register<fourcc("RGBA"),atom_RGBA>(); //RGBADescriptor
    //        this -> Register<fourcc("RLED"),atom_RLED>();
    //        this -> Register<fourcc("RSET"),atom_RSET>(); //ARepSet
    //        this -> Register<fourcc("SCLP"),atom_SCLP>(); //ASourceClip
    //        this -> Register<fourcc("SEQU"),atom_SEQU>(); //ASequence
    //        this -> Register<fourcc("SETG"),atom_SETG>(); //ASettingGroup - Settings only -
    //        this -> Register<fourcc("SHLP"),atom_SHLP>();
    //        this -> Register<fourcc("SLCT"),atom_SLCT>();
    //        this -> Register<fourcc("SLPI"),atom_SLPI>();
    //        this -> Register<fourcc("SPED"),atom_SPED>();
    //        this -> Register<fourcc("STRB"),atom_STRB>();
    //        this -> Register<fourcc("TCCP"),atom_TCCP>(); //ATimecodeClip
    //        this -> Register<fourcc("TKDA"),atom_TKDA>();
    //        this -> Register<fourcc("TKDS"),atom_TKDS>();
    //        this -> Register<fourcc("TKFX"),atom_TKFX>(); //ATrackEffect
    //        this -> Register<fourcc("TKPA"),atom_TKPA>();
    //        this -> Register<fourcc("TKPS"),atom_TKPS>();
    //        this -> Register<fourcc("TMBC"),atom_TMBC>(); //ATmpCrumb
    //        this -> Register<fourcc("TMCS"),atom_TMCS>(); //TmpBreadCrumbs
    //        this -> Register<fourcc("TNFX"),atom_TNFX>(); //ATransEffect
    //        this -> Register<fourcc("TRKR"),atom_TRKR>(); //ATrackRef
    //        this -> Register<fourcc("URLL"),atom_URLL>();
    //        this -> Register<fourcc("WAVE"),atom_WAVE>(); //WAVEDescriptor
    }





