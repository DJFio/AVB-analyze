//
//  utility.h
//  Drag-Analyze-AVB
//
//  Created by DJFio on 15/06/2020.
//  Copyright © 2020 DJFio. All rights reserved.
//

#ifndef utility_h
#define utility_h
#include <vector>

//***********************************
//   vector Extract<> template
//***********************************
//// extracting a float at position 6 of bytes vector
//extract<float>(bytes, 6);
//// extracting a char at position 10 of bytes vector
//extract<char>(bytes, 10);

template <typename T>
T extract(const std::vector<char> &v, int pos)
{
    T value;
    memcpy(&value, &v[pos], sizeof(T));
    return value;
}


template<typename ... Args>
std::string format(const std::string &fmt, Args ... args)
{
    // C++11 specify that string store elements continously
    std::string ret;
    
    auto sz = std::snprintf(nullptr, 0, fmt.c_str(), args...);
    ret.reserve(sz + 1); ret.resize(sz);    // to be sure there have room for \0
    std::snprintf(&ret.front(), ret.capacity() + 1, fmt.c_str(), args...);
    return ret;
}



#endif /* utility_h */
