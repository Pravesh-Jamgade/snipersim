#ifndef CACHE_LEVEL_H
#define CACHE_LEVEL_H

#include <bits/stdc++.h>
#include "fixed_types.h"
#include "log.h"

using namespace std;

#define ARRAY_TYPE 3

class MemDataLogger
{
    private:
    // indexing: 0->oa, 1->ea, 2->pa
    uint64_t type_access_arr[ARRAY_TYPE]={0};
    uint64_t type_miss_arr[ARRAY_TYPE]={0};
    uint64_t type_hit_arr[ARRAY_TYPE]={0};
    
    // indexing: 0+0=0, 0+1=1, 0+2=2 [oa reaplced by oa,ea,pa] starting at index 0 to 2
    // indexing: 1+0+(2)=3, 1+1+(2)=4, 1+2+(2)=5 [ea reaplced by oa,ea,pa] starting at index 3 to 5
    // indexing: 2+0+(4)=6, 2+1+(4)=7, 2+2+(4)=8 [pa replaced by oa,ea,pa] starting at index 6 to 8
    uint64_t replaced_by[10]={0};    
    String name;
    int coreid;

    public:
    MemDataLogger(){ name= "", coreid=-1;}
    MemDataLogger(int coreid, String name)
    {
        this->coreid = coreid;
        this->name = name;
    }

    void add_access(int type){type_access_arr[type]++;}
    void add_misses(int type){type_miss_arr[type]++;}
    void add_hits(int type){type_hit_arr[type]++;}

    uint64_t get_access(int type){return type_access_arr[type];}
    uint64_t get_misses(int type){return type_miss_arr[type];}
    uint64_t get_hits(int type){return type_hit_arr[type];}

    void PrintStat()
    {
        for(int i=0; i< 2; i++)
        {
            cout << std::dec << i << "," << type_access_arr[i] << "," << type_miss_arr[i] << "," << this->coreid <<","<< this->name << '\n';
            _LOG_CUSTOM_LOGGER(Log::Warning, Log::LogCustom::A, "%d,%d,%d,%d,%s\n", i, type_access_arr[i], type_miss_arr[i], this->coreid, this->name.c_str());
        }

        String s= "";
        for(int i=0; i< 9; i++)
        {
            s = s + String(to_string(replaced_by[i]).c_str()) + ",";
        }
        cout << s << '\n';
        _LOG_CUSTOM_LOGGER(Log::Warningm, Log::LogCustom::B, "%s\n", s.c_str());
    }

    void replacing(int new_type, int old_type)
    {
        int index = new_type + old_type;
        if(old_type == 0) index += 0;
        if(old_type == 1) index += 2;
        if(old_type == 2) index += 5;
        replaced_by[index]++;
    }   

};
#endif