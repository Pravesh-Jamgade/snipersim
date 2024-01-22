#ifndef DOA_H
#define DOA_H

#include <bits/stdc++.h>
#include <inttypes.h>
#include "fixed_types.h"
#include "log.h"

using namespace std;

class DOA
{
    class Meta
    {
        public:

        IntPtr doa;
        IntPtr evictions;
        
        Meta()
        {
            doa = 0;
            evictions = 0;
        }

        Meta(IntPtr doa, IntPtr evictions)
        {
            this->doa = doa;
            this->evictions = evictions;
        }
    };
    
    public:
    map<IntPtr, Meta> deadpage;

    String s;
    String name;
    fstream out;

    map<IntPtr, vector<Meta>> across_run;

    DOA(String name)
    {
        this->name = name;
    }

    ~DOA()
    {
        if(name.find("stlb") != string::npos)
        {
            this->name = name;
            s = name + String("_DEADCOUNT.log");
            out = LogStream::get_file_stream(s.c_str());
            
            for(auto entry: across_run){
                for(auto pa: entry.second){
                    out << std::hex << entry.first << "," << std::dec << "," << pa.doa << "," << pa.evictions << '\n';
                }
            }

            out.close();
        }
    }

    void func_add_evict(IntPtr page, int used)
    {
        auto foundPage = deadpage.find(page);
        if(foundPage==deadpage.end())   
        {
            deadpage.insert({page, Meta()});
        }
        
        if(used <= 0)
        {
            deadpage[page].doa++;
        }
        deadpage[page].evictions++;
    }

    // called by stlb doa upon eviction
    // if stlb page doa then doa corr between stlb and llc is complete. count corr++. 
    // count total_corr++.
    void func_track_corr(DOA* llc, IntPtr page, int used)
    {
        auto findPage = llc->deadpage.find(page);
        if(findPage==llc->deadpage.end())
        {
           return;
        }
        if(used <=0) 
            across_run[page].push_back(Meta(findPage->second.doa, findPage->second.evictions));
    }

};

#endif