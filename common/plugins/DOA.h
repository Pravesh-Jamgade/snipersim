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
        IntPtr page_mostly_dead;
        IntPtr page_mostly_live;
        IntPtr page_evicted;
        IntPtr page_live;
        
        Meta()
        {
            page_doa = 0;
            page_mostly_dead = page_mostly_live = 0;
            page_evicted = 0;
            page_live = 0;
        }
    };
    
    public:
    map<IntPtr, Meta> deadpage;

    IntPtr total_doa;
    IntPtr total_mostly_dead;
    IntPtr total_mostly_live;
    IntPtr total_evicted;
    IntPtr total_live;

    String s;
    String name;
    fstream out;

    IntPtr corr = 0;
    IntPtr total_tracked = 0;

    DOA(String name)
    {
        this->name = name;
        s = name + String("_DEADCOUNT.log");
        out = LogStream::get_file_stream(s.c_str());

        total_doa = 0;
        total_mostly_dead = 0;
        total_mostly_live = 0;
        total_evicted = 0;
        total_live = 0;
    }

    ~DOA()
    {
        for(auto e: deadpage)
        {
            out << std::hex << e.first << std::dec << ',' << e.second.page_doa << ',' << e.second.page_mostly_dead << ',' << e.second.page_mostly_live << ',' << e.second.page_evicted << '\n';
        }
        out.close();

        std::cout << "name:"<< name << ", total_evicted:" << total_evicted << ", total_doa:" << total_doa <<", total_live:" << total_live << ", total_mostly_dead:" << total_mostly_dead <<", total_mostly_live:" << total_mostly_live << '\n';
        std::cout << "name:" << name << ", total_corr:"<<corr<<", total_tracked:"<<total_tracked<<'\n';
    }

    void func_add_evict(IntPtr page, int used)
    {
        auto foundPage = deadpage.find(page);
        if(foundPage==deadpage.end())
        {
            deadpage.insert({page, Meta()});
        }
        
        total_evicted++;
        deadpage[page].page_evicted++;

        if(used <= 0)
        {
            total_doa++;
            deadpage[page].page_doa++;
        }
        else
        {
            total_live++;
            deadpage[page].page_live++;
        }

        if(deadpage[page].page_doa > deadpage[page].page_live)
        {
            total_mostly_dead++;
            deadpage[page].page_mostly_dead++;
        }
        else
        {
            total_mostly_live++;
            deadpage[page].page_mostly_live++;
        }
    }

    // called by stlb doa upon eviction
    // if stlb page doa then doa corr between stlb and llc is complete. count corr++. 
    // count total_corr++.
    void func_track_corr(DOA* llc, IntPtr page, int used)
    {
        auto findPage = llc->deadpage.find(page);
        if(used<=0)
        {
            corr+=findPage->second.page_doa;
        }
        total_tracked+=findPage->second.page_doa;
        deadpage.erase(findPage);
    }

};

#endif