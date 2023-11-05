#ifndef LOGSTREAM_H
#define LOGSTREAM_H

#include <iostream>
#include <map>
#include <vector>
#include <set>
#include <string>
#include <fstream>
#include <algorithm>

using namespace std;

class LogStream{
    public:
    static fstream get_file_stream(const char* name){
        fstream newfs;
        newfs.open(name, std::fstream::app | std::fstream::in);
        return newfs;
    }
    private:
    
};

#endif