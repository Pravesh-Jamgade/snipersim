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
    bool allow_edge, allow_offset, allow_property;

    set<String> all_collected;

    int* already_permitted;

    map<UInt32, UInt32> super_user_tlb;

    ContextHint()
    {
        ea_start = ea_end = 0;
        oa_start = oa_end = 0;
        pa_start = pa_end = 0;
        
        edge_ready = offset_ready = property_ready = 2;
        allow_edge = allow_offset = allow_property = false;

        already_permitted = (int*)calloc(sizeof(int)*100, 0);
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
        
        f << "\naddress, ready, allowed\n";
        f << std::hex << "oa_start," << oa_start << ',' << offset_ready << ',' << allow_offset << '\n';
        f << std::hex << "oa_end," << oa_end << ',' << offset_ready << ',' << allow_offset <<'\n';

        f << std::hex << "ea_start," << ea_start << ',' << edge_ready << ',' << allow_edge <<'\n';
        f << std::hex << "ea_end," << ea_end << ',' << edge_ready << ',' << allow_edge << '\n';
        
        f << std::hex << "pa_start," << pa_start << ',' << property_ready << ',' << allow_property << '\n';
        f << std::hex << "pa_end," << pa_end << ',' << property_ready << ',' << allow_property << '\n';

        f.close();
    }

    void set_context(int type, IntPtr addr, int core_id)
    {
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
            case 765:
                {
                    if(already_permitted[core_id])
                        return;
                    
                    already_permitted[core_id] = true;

                    cout << "core_id: " << core_id << ", FINALIZE\n";
                    if(offset_ready==0) 
                        allow_offset = true;
                    
                    if(edge_ready==0)
                        allow_edge = true;
                    
                    if(property_ready==0)
                        allow_property = true;

                    fstream f;
                    f.open("userdebug.stat", std::fstream::in | std::fstream::out | std::fstream::app);
                    f << "*************************************************************************\n";
                    f << "core_id: " << core_id << '\n';
                    f << "start, end, ready, allowed\n";
                    f << std::hex << "oa_start," << oa_start << ',' << offset_ready << ',' << allow_offset << '\n';
                    f << std::hex << "oa_end," << oa_end << ',' << offset_ready <<  ',' << allow_offset <<'\n';

                    f << std::hex << "ea_start," << ea_start << ',' << edge_ready << ',' << allow_edge <<'\n';
                    f << std::hex << "ea_end," << ea_end << ',' << edge_ready << ',' << allow_edge << '\n';
                    
                    f << std::hex << "pa_start," << pa_start << ',' << property_ready << ',' << allow_property << '\n';
                    f << std::hex << "pa_end," << pa_end << ',' << property_ready << ',' << allow_property <<'\n';
                    f << "*************************************************************************\n";
                    f.close();
                }
                break;
            default:
                break;
                
        }
    }

    ARRAY_TYPE what_is_it(IntPtr req_addr, int level=0)
    {
        if(oa_start <= req_addr && allow_offset)
        {
            if(req_addr <= oa_end) 
            {
                all_collected.insert("OA");
                return ARRAY_TYPE::OA;
            }
        }

        else if(ea_start <= req_addr && allow_edge)
        {
            if(req_addr <= ea_end) 
            {
                all_collected.insert("EA");
                return ARRAY_TYPE::EA;
            }
        }

        else if(pa_start <= req_addr && allow_property)
        {
            if(req_addr <= pa_end)
            {
                all_collected.insert("PA");
                return ARRAY_TYPE::PA;
            }
        }
        return ARRAY_TYPE::NONE;
    }

    ARRAY_TYPE what_is_it_pp_enbable(IntPtr pa)
    {
        //reconstruct va from pa-va mapping and po
        UInt32 pp = pa >> 12;
        UInt32 po = pa & ((1<<12)-1);
        auto iter_pp = super_user_tlb.find(pp);
        if(iter_pp == super_user_tlb.end())
        {
            exit(-1);
        }
        
        UInt32 vp = iter_pp->second;
        IntPtr va = (vp<<12) | po;
        return what_is_it(va);
    }

    void add_tlb_entry(UInt32 va, UInt32 pa)
    {
        UInt32 vp = va >> 12;
        UInt32 pp = pa >> 12;
        super_user_tlb[pp]=vp;
    }
};


#endif