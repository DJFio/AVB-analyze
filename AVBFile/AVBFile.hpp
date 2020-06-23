//
//  AVBFile.hpp
//
//
//  Created by DJFio on 04/06/2020.
//  Copyright © 2020 DJFio. All rights reserved.
//

#ifndef AVBFile_hpp
#define AVBFile_hpp


#include <fstream>
#include <vector>
#include <map>
#include "Decoder.hpp"
#include "AVBObjects.hpp"
#include "MDVxUUID.hpp"

class AVBFile {
public:
    AVBFile();
    ~AVBFile();
    
    std::streamsize avbsize = 0;
    std::ifstream avbfile;
    std::vector<char> bytes;
    Decoder getvalue;
    size_t position = 0;
    size_t ItemsCount = 0;
    
    avbTOC theTOC;

    
    bool openFile (std::string filename) ;
    void Dump (bool EnableHexDump = false);
    
    std::map<MDVxUUID, MDVxUUID> getAll_MSML_UIDS (void);
    
    
};

#endif /* AVBFile_hpp */
