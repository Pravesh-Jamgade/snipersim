#ifndef CONTEXT_HINT_H
#define CONTEXT_HINT_H

#include <bits/stdc++.h>
#include "log.h"
using namespace std;

//TODO: Currently considering even after address translation va remains same
// i.e running application without sudo to prevent it from using separate physical address

class ContextHint
{
    public:

    // (OA/EA/PA)
    uint64_t ea_start, ea_end;
    uint64_t oa_start, oa_end;
    uint64_t pa_start, pa_end;

    int edge_ready, offset_ready, property_ready;

    ContextHint(){
        ea_start = ea_end = 0;
        oa_start = oa_end = 0;
        pa_start = pa_end = 0;
        
        edge_ready = offset_ready = property_ready = 2;
    
        index_fs.open("verify_index.log", fstream::out);
        edge_fs.open("verify_edge.log", fstream::out);
        property_fs.open("verify_property.log", fstream::out);
        other_fs.open("verify_other.log", fstream::out);
    }

    ~ContextHint()
    {
        for(auto e: index_set) 
            index_fs << std::hex << e  << '\n';
        for(auto e: edge_set)
            edge_fs << std::hex << e << '\n';
        for(auto e: prop_set)
            property_fs << std::hex << e << '\n';
        for(auto e: other_set)
            other_fs << std::hex << e << '\n';
    }

    void set_context(int type, uint64_t addr)
    {
        if(type == 765)
        {
            cout << "++++++++++++++++++++++++++++++++++++\n";
            cout << "[PLUGIN] STARTING Kernel\n";
            cout << "++++++++++++++++++++++++++++++++++++\n";
            return;
        }
        if(type>=10) return;
        switch(type)
        {
            case 1:
                oa_start = addr;
                offset_ready--;
                break;
            case 2:
                oa_end = addr;
                offset_ready--;
                break;
            case 3:
                ea_start = addr;
                edge_ready--;
                break;
            case 4:
                ea_end = addr;
                edge_ready--;
                break;
            case 5:
                pa_start = addr;
                property_ready--;
                break;
            case 6:
                pa_end = addr;
                property_ready--;
                break;
        }
    }

    int what_is_it(uint64_t req_addr, int level=0)
    {
        if(oa_start <= req_addr && offset_ready==0)
        {
            if(req_addr <= oa_end) 
            {
                return 1;
            }
        }

        else if(ea_start <= req_addr && edge_ready==0)
        {
            if(req_addr <= ea_end) 
            {
                return 2;
            }
        }

        else if(pa_start <= req_addr && property_ready==0)
        {
            if(req_addr <= pa_end)
            {
                return 3;
            }
        }
        return 0;
    }
};


#endif