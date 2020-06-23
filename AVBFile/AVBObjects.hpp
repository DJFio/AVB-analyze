//
//  AVBObjects.hpp
//  Drag-Analyze-AVB
//
//  Created by DJFio on 18/06/2020.
//  Copyright © 2020 DJFio. All rights reserved.
//

#ifndef AVBObjects_hpp
#define AVBObjects_hpp


#include <ios>
#include <fstream>
#include <memory>
#include <vector>
#include <map>

#include "AVBTypes.hpp"
#include "AVBmacros.h"
#include "Decoder.hpp"




class avbTOC;

class avbATOM {
public:
    avbATOM();
    virtual ~avbATOM(){};
    
    avbTOC * pTOC;  //pointer to parent - all atoms directly attached to TOC
    uint32_t TOC_id;//unique number;
    uint32_t referenceCount = 0L;    //number of references to this atom (+1=toc,and +1 from each object) ; atom with=0 references must be deleted from TOC
    uint32_t hasReferencesToOtherOBJs = 0L;
    uint32_t    atom_fourcc;   // = fourcc('ATOM');
    uint32_t    length;        // BOB Length
    std::vector<char>BOB;      // bunch of bytes - the object
    Decoder getvalue;

    uint32_t hash (void) const { return TOC_id;} // simple hash
    bool operator==(const avbATOM& b) const  { return TOC_id==b.TOC_id;}
    bool operator< (const avbATOM& b) const  { return TOC_id< b.TOC_id;}
 



    virtual void init(avbTOC * pParentTOC);
    virtual bool read(std::ifstream *f);
    virtual bool write(std::ofstream *f);
    virtual std::string dump(void);
    
    bool BOB_read(std::ifstream *f);
    std::string BOB_end_dump(void);
    std::string BOB_begin_dump(uint32_t pos=0L);
};



class ATOMBase {
public:
    ATOMBase() {}
    virtual ~ATOMBase() {}
    virtual std::unique_ptr<avbATOM> Create() = 0;
};
template< class T >
class ATOMvariant : public ATOMBase {
public:
    ATOMvariant() {}
    virtual ~ATOMvariant() {}
    virtual std::unique_ptr<avbATOM> Create() { return std::make_unique<T>(); }
};


class ATOMFactory
{
public:
    ATOMFactory();

    std::unique_ptr<avbATOM> Create( uint32_t type )
    {
        TSwitchToVariant::iterator it = m_switchToVariant.find( type );
        if( it == m_switchToVariant.end() ) {
            it = m_switchToVariant.find( fourcc("ATOM")); // try to return default object
            if( it == m_switchToVariant.end() ) return nullptr; 
        }
        return it->second->Create();
    }
    
private:
    
    template< uint32_t type, typename T >
    void Register()
    {
        Register( type, std::make_unique<ATOMvariant<T>>() );
    }
    
    void Register( uint32_t type, std::unique_ptr<ATOMBase>&& creator )
    {
        m_switchToVariant[type] = std::move(creator);
    }
    
    typedef std::map<uint32_t, std::unique_ptr<ATOMBase> > TSwitchToVariant;
    TSwitchToVariant m_switchToVariant;
};




class avbTOC {
public:
    
    void allocate (uint32_t numslots)  {TOC.resize(numslots);}
    void setByteswap(bool use = false) {usebyteswap = use;}
    bool getByteswap(void) {return usebyteswap;}
    void setRootObject(uint32_t tocid) {TOCRootObject=tocid;}
    void resizeTOC(uint32_t numobjects) {TOC.resize(numobjects+1);}//taking in account OBJD root object which has index[0]
    avbATOM * pATOM_atTOC(uint32_t TOC_id);
    bool read(std::ifstream * f);
//    bool write(std::ofstream f);
    std::string dump (void);

private:
//    avbATOM * add_atom(avbATOM * atom);
//    bool delete_atom(uint32_t TOC_id); //decrease refcount and find all references to this atom.
//    bool delete_atom(avbATOM * atom);  //decrease refcount and find all references to this atom.
//    void swap (uint32_t TOC_id_1,uint32_t TOC_id_2);

    bool   usebyteswap = false;
    uint32_t next_TOCid=0;
    uint32_t TOCRootObject;
    ATOMFactory factory;
    std::vector <std::unique_ptr<avbATOM>> TOC;

};



#endif /* AVBObjects_hpp */
