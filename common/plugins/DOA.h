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

        IntPtr page_doa;
        IntPtr page_live;
        
        Meta()
        {
            page_doa = 0;
            page_live = 0;
        }

        Meta(IntPtr doa, IntPtr live)
        {
            page_doa=doa;
            page_live=live;
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
                    out << std::hex << entry.first << "," << std::dec << pa.page_doa << "," << pa.page_live << '\n';
                }
            }

            out.close();
        }

        if(name.find("L3") != string::npos)
        {
            this->name = name;
            s = name + String("_DEADCOUNT.log");
            out = LogStream::get_file_stream(s.c_str());
            
            for(auto entry: deadpage)
            {
                out << std::hex << entry.first << "," << std::dec << entry.second.page_doa << "," << entry.second.page_live << '\n';
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
        
        cout << name << ", " << used << '\n';
        if(used <= 0)
        {
            deadpage[page].page_doa++;
        }
        else
        {
            deadpage[page].page_live++;
        }
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
        across_run[page].push_back(Meta(findPage->second.page_doa, findPage->second.page_live));
        llc->deadpage.erase(findPage);
    }

};

#endif