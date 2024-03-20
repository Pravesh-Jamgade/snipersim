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
        IntPtr page_dead;
        
        Meta()
        {
            doa = 0;
            evictions = 0;
            page_dead = 0;
        }
    };
    
    public:
    map<IntPtr, Meta> deadpage;

    String s;
    String name;
    fstream out;

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
            out << "page, page_dead, all_doa, all_evictions\n";
            for(auto it: deadpage)
            {
                out << it.first << "," << it.second.page_dead << "," << it.second.doa << "," << it.second.evictions << '\n';
            }
            out.close();
        }
    }

    /// @brief Called by LLC, Tracking evicted block, if it is dead for its corresponding page
    /// @param page 
    /// @param used 
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

    // Called by stlb doa upon eviction
    // if stlb page doa then doa corr between stlb and llc is complete. count corr++. 
    // count total_corr++.

    /// @brief Called by STLB, Tracking dead page, if it is dead then track the number of dead blocks it has seen
    /// @param llc 
    /// @param page 
    /// @param used 
    void func_track_corr(DOA* llc, IntPtr page, int used)
    {
        // auto findPage = llc->deadpage.find(page);
        // if(findPage==llc->deadpage.end())
        // {
        //    return;
        // }
        if(used <=0)
        {
            if(deadpage.find(page) == deadpage.end())
                deadpage[page] = Meta();
                
            // STLB DOA Page keeping track of its corresponding DOA cache blocks and evictions
            deadpage[page].doa += llc->deadpage[page].doa;
            deadpage[page].evictions += llc->deadpage[page].evictions;

            // also count how many times current page is dead
            deadpage[page].page_dead += 1;

            // reset DOA structure of LLC 
            llc->deadpage[page] = Meta();
        }


    }

};

#endif