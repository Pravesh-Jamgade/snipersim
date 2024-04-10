#ifndef TLB_HELPER_H
#define TLB_HELPER_H
//***
#include <bits/stdc++.h>
#include "fixed_types.h"
#include "log.h"
#include "constant.h"

class Page{
public:
UInt64 first_level_hit;
UInt64 first_level_miss;
UInt64 first_level_access;
UInt64 second_level_hit;
UInt64 second_level_miss;
UInt64 second_level_access;
UInt64 ppa;
bool *page_granularity;

Page()
{
    ppa = 0;
    first_level_hit = first_level_miss = first_level_access = 0;
    second_level_hit = second_level_miss = second_level_access = 0;
    page_granularity = (bool*)calloc(4, sizeof(bool));
}

Page(UInt64 page)
{
    ppa = page;
    first_level_hit = first_level_miss = first_level_access = 0;
    second_level_hit = second_level_miss = second_level_access = 0;
    page_granularity = (bool*)calloc(4, sizeof(bool));
}
};

class PageCall
{
public:
std::map<UInt64, Page> page_map[NUM_CPUS]; 
PageCall(){}
~PageCall()
{
    ofstream of;
    of.open("tlb_evict.log", ios::out);
    of << "cpu, vp, pp, flh, flm, fla, slh, slm, sla, type, mix\n";
    for(int i=0; i< NUM_CPUS; i++)
    {
        for(auto e : page_map[i])
        {
            int count = 0;
            int type = 0;
            for(int i=0; i< 4; i++)
            {
                if(e.second.page_granularity[i] == 1)
                    type = i;
                count += e.second.page_granularity[i];
            }
            
            of << i << "," << e.first << "," << e.second.ppa << "," 
                << e.second.first_level_hit << ","<< e.second.first_level_miss << "," << e.second.first_level_access << "," 
                << e.second.second_level_hit << "," << e.second.second_level_miss << "," << e.second.second_level_access << "," 
                << type << "," << count << '\n';
        }
    }
}
};
#endif