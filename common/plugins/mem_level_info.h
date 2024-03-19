#ifndef MEM_LEVEL_H
#define MEM_LEVEL_H

#include <bits/stdc++.h>
#include "fixed_types.h"
#include "log.h"
#include "constant.h"

using namespace std;

class MemDataLogger
{
    private:
    // indexing: 0->none, 1->oa, 2->ea, 3->pa
    IntPtr* type_access_arr;
    IntPtr* type_miss_arr;
    IntPtr* type_hit_arr;
    
    // indexing: 0+1=1, 0+2=2, 0+3=3 [NONE replaced by oa,ea,pa]
    // indexing: 1+1+(2)=4, 1+2+(2)=5, 1+3+(2)=6 [oa reaplced by oa,ea,pa] 
    // indexing: 2+1+(4)=7, 2+2+(4)=8, 2+3+(4)=9 [ea reaplced by oa,ea,pa] 
    // indexing: 3+1+(6)=10, 3+2+(6)=11, 3+3+(6)=12 [pa replaced by oa,ea,pa] 
    IntPtr replaced_by[4][4] = {0};    
    String name;
    int coreid;
    static bool once_a;

    public:
    MemDataLogger()
    { 
        name= "", coreid=-1; 

        type_access_arr = (IntPtr*)calloc(ARRAY_TYPE_SIZE, sizeof(IntPtr));
        type_miss_arr = (IntPtr*)calloc(ARRAY_TYPE_SIZE, sizeof(IntPtr));
        type_hit_arr = (IntPtr*)calloc(ARRAY_TYPE_SIZE, sizeof(IntPtr));

    }

    MemDataLogger(int coreid, String name)
    {
        this->coreid = coreid;
        this->name = name;

        type_access_arr = (IntPtr*)calloc(ARRAY_TYPE_SIZE, sizeof(IntPtr));
        type_miss_arr = (IntPtr*)calloc(ARRAY_TYPE_SIZE, sizeof(IntPtr));
        type_hit_arr = (IntPtr*)calloc(ARRAY_TYPE_SIZE, sizeof(IntPtr));
    }

    void add_access(int type){type_access_arr[type]++;}
    void add_misses(int type){type_miss_arr[type]++;}
    void add_hits(int type){type_hit_arr[type]++;}

    IntPtr get_access(int type){return type_access_arr[type];}
    IntPtr get_misses(int type){return type_miss_arr[type];}
    IntPtr get_hits(int type){return type_hit_arr[type];}

    void PrintStat()
    {   
        if(!once_a)
        {
            _LOG_CUSTOM_LOGGER(Log::Warning, Log::LogCustom::A, "ARRAY, access, hit, core, cache\n");
            _LOG_CUSTOM_LOGGER(Log::Warning, Log::LogCustom::B, "NA->OA, NA->EA, NA->PA, OA->OA, OA->EA, OA->PA, EA->OA, EA->EA, EA->PA, PA->OA, PA->EA, PA->PA, core, cache\n");
            once_a = true;
        }

        
        for(int i=0; i< ARRAY_TYPE_SIZE; i++)
        {
            _LOG_CUSTOM_LOGGER(Log::Warning, Log::LogCustom::A, "%s,%ld,%ld,%ld,%s\n", get_arr_name(i).c_str(), type_access_arr[i], type_hit_arr[i], this->coreid, this->name.c_str());
        }
        
        _LOG_CUSTOM_LOGGER(Log::Warning, Log::LogCustom::B, "----------------------------------------------------------------\n");
        String s= "";
        for(int i=1; i< 4; i++)
        {
            String row = "";
            for(int j=0; j< 4; j++)
            {
                row = row + String(std::to_string(replaced_by[i][j]).c_str()) + ",";
            }
            s = row + '\n';
        }
        _LOG_CUSTOM_LOGGER(Log::Warning, Log::LogCustom::B, "%s %ld, %s\n", s.c_str(), this->coreid, this->name.c_str());
        _LOG_CUSTOM_LOGGER(Log::Warning, Log::LogCustom::B, "----------------------------------------------------------------\n");

    }

    /*
        compute index with (old + new + SOME_DELTA) to get index of array 
        which represent who-replaces-who, and then increase counter value
    */
    void replacing(int new_type, int old_type)
    {
        replaced_by[new_type][old_type]++;
    }   

};
#endif