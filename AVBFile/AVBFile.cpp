//
//  AVBFile.cpp
//
//
//  Created by DJFio on 04/06/2020.
//  Copyright © 2020 DJFio. All rights reserved.
//
//*******************************************************************************************************************************
//  AVB file
//  Created by DJFio on 04/06/2020.
//  Copyright © 2020 DJFio. All rights reserved.
//*******************************************************************************************************************************

// Handle Windows cases - Michael Haephrati
// ----------------------------------------
#ifdef _MSC_VER
#include "vcproj\stdafx.h"
#else
#include <iostream>
#include <fstream>
#include <sstream>
#endif
// Handle Windows cases - Michael Haephrati
// ----------------------------------------
#ifndef _MSC_VER
#include "cout_redirect.hpp"
#endif



#include "utility.h"
#include "AVBFile.hpp"
#include "AVBmacros.h"

#ifndef redir_cout
#define redir_cout
#ifndef redir_flush
#define redir_flush
#endif
#endif




AVBFile::AVBFile(): getvalue(&bytes)
{
}


AVBFile::~AVBFile()
{
}


 bool AVBFile::openFile (std::string filename) {
     
     redir_cout;
     std::cout << "[file:" << filename << "]" << std::endl;
     redir_flush;

     avbfile.open( filename, std::ios::binary | std::ios::ate);
     if (!avbfile.good()){
//         std::cout << "problem with AVB File :: bad file name? " << std::endl; redir_flush;
         return false;
     }
     bool result = theTOC.read(&avbfile);
     assert (result);
     avbfile.close();
     return result;

}

void AVBFile::Dump(bool EnableHexDump){
    redir_cout;
    std::cout << theTOC.dump();
    redir_flush;
}
