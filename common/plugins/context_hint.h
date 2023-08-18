#ifndef CONTEXT_HINT_H
#define CONTEXT_HINT_H

#include <bits/stdc++.h>
#include "log.h"
#include "constant.h"
using namespace std;

//TODO: Currently considering even after address translation va remains same
// i.e running application without sudo to prevent it from using separate physical address

class ContextRange
{
    vector<UInt64> range;
    public:
    ContextRange()
    {

    }

    void push_addr(UInt64 addr)
    {
        range.push_back(addr);
    }

    bool find(UInt64 addr)
    {
        for(size_t i=0; i< range.size()-1; i++)
        {
            if(range[i] <= addr && addr <= range[i+1])
            {
                return true;
            }
        }
        return false;
        // if(addr < *(range.begin()) || addr > *(range.end()))
        //     return false;
        // return true;
    }
};

class ContextHint
{
    public:

    // (OA/EA/PA)
    ContextRange* ctxr;

    set<String> all_collected;

    int* already_permitted;

    map<UInt32, UInt32> super_user_tlb;

    ContextHint()
    {
        ctxr = new ContextRange[4];
    }

    ~ContextHint()
    {
        fstream f;
        f.open("userdebug.stat", std::fstream::in | std::fstream::out | std::fstream::app);
        
        f << "seen memory hierarhy, data types:\n";
        for(auto e: all_collected)
        {
            f << std::hex << e << '\n';
        }
        f.close();
    }

    void set_context(int type, IntPtr addr, int core_id)
    {
        if(addr == 0)
            return;
        
        
        switch(type)
        {
            case 1:
            case 2:
            case 3:
                ctxr[type].push_addr(addr);
                break;
            case 0:
                break;
            default:
                break;
                
        }
    }

    /*
        NA: 0
        OA: 1
        EA: 2
        PA: 3
    */
    ARRAY_TYPE what_is_it(IntPtr req_addr, int level=0)
    {
        for(int i=1; i< 4; i++)
        {
            if(ctxr[i].find(req_addr))
            {
                if(i==1) {
                    return ARRAY_TYPE::OA;
                }
                if(i==2) 
                {
                    return ARRAY_TYPE::EA;
                }
                if(i==3) 
                {
                    return ARRAY_TYPE::PA;
                }
            }
        }
        return ARRAY_TYPE::NONE;
    }

    // TODO: SEE why not working
    // ARRAY_TYPE what_is_it_pp_enbable(IntPtr pa)
    // {
    //     //reconstruct va from pa-va mapping and po
    //     UInt32 pp = pa >> 12;
    //     UInt32 po = pa & ((1<<12)-1);
    //     auto iter_pp = super_user_tlb.find(pp);
    //     if(iter_pp == super_user_tlb.end())
    //     {
    //         exit(-1);
    //     }
        
    //     UInt32 vp = iter_pp->second;
    //     IntPtr va = (vp<<12) | po;
    //     return what_is_it(va);
    // }

    // void add_tlb_entry(UInt32 va, UInt32 pa)
    // {
    //     UInt32 vp = va >> 12;
    //     UInt32 pp = pa >> 12;
    //     super_user_tlb[pp]=vp;
    // }
};


#endif