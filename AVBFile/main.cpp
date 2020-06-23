//
//  main.cpp
//  AVB-analyze
//
//  Created by DJFio on 22/06/2020.
//  Copyright © 2020 DJFio. All rights reserved.
//

#ifdef _MSC_VER
#include "vcProj/stdafx.h"
#else
#include <stdio.h>
#include <iostream>
#include <fstream>
#endif

#include "AVBFile.hpp"



//**************************
// main is here :)
//**************************


int main(int argc, char* argv[], char* envp[]) {
    //**************************
    // simple utility takes avb filename as input
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << "file.avb" << std::endl;
        return 1;
    }
    
    AVBFile *_thefile =  new AVBFile;
    _thefile->openFile(argv[1]);
    _thefile->Dump();
    delete _thefile;
    
    return 0;
}
