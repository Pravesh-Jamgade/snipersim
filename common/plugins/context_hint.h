#ifndef CONTEXT_HINT_H
#define CONTEXT_HINT_H

#include <bits/stdc++.h>
#include "log.h"
#include "constant.h"
using namespace std;

//TODO: Currently considering even after address translation va remains same
// i.e running application without sudo to prevent it from using separate physical address

class ContextHint
{
    public:

    // (OA/EA/PA)
    IntPtr ea_start, ea_end;
    IntPtr oa_start, oa_end;
    IntPtr pa_start, pa_end;

    int edge_ready, offset_ready, property_ready;

    set<String> all_collected;

    ContextHint()
    {
        ea_start = ea_end = 0;
        oa_start = oa_end = 0;
        pa_start = pa_end = 0;
        
        edge_ready = offset_ready = property_ready = 2;
    }

    ~ContextHint()
    {
        fstream f;
        f.open("all_access.stat", fstream::out);
        for(auto e: all_collected)
        {
            f << e << '\n';
        }
        f.close();
    }

    void set_context(int type, IntPtr addr)
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

    ARRAY_TYPE what_is_it(IntPtr req_addr, int level=0)
    {
        if(oa_start <= req_addr && offset_ready==0)
        {
            if(req_addr <= oa_end) 
            {
                all_collected.insert("OA");
                return ARRAY_TYPE::OA;
            }
        }

        else if(ea_start <= req_addr && edge_ready==0)
        {
            if(req_addr <= ea_end) 
            {
                all_collected.insert("EA");
                return ARRAY_TYPE::EA;
            }
        }

        else if(pa_start <= req_addr && property_ready==0)
        {
            if(req_addr <= pa_end)
            {
                all_collected.insert("PA");
                return ARRAY_TYPE::PA;
            }
        }
        return ARRAY_TYPE::NONE;
    }
};


#endif