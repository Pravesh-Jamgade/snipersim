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

    ContextHint()
    {
        ea_start = ea_end = 0;
        oa_start = oa_end = 0;
        pa_start = pa_end = 0;
        
        edge_ready = offset_ready = property_ready = 2;
        allow_edge = allow_offset = allow_property = false;
    }

    ~ContextHint()
    {
        fstream f;
        f.open("userdebug.stat", std::fstream::in | std::fstream::out | std::fstream::app);
        for(auto e: all_collected)
        {
            f << std::hex << e << '\n';
        }
        
        f << std::hex << "oa_start," << oa_start << ',' << offset_ready << '\n';
        f << std::hex << "oa_end," << oa_end << ',' << offset_ready <<'\n';

        f << std::hex << "ea_start," << ea_start << ',' << edge_ready <<'\n';
        f << std::hex << "ea_end," << ea_end << ',' << edge_ready << '\n';
        
        f << std::hex << "pa_start," << pa_start << ',' << property_ready << '\n';
        f << std::hex << "pa_end," << pa_end << ',' << property_ready << '\n';

        f.close();
    }

    void set_context(int type, IntPtr addr)
    {
        switch(type)
        {
            case 1:
                cout << "offset_start, "<< addr << '\n';
                oa_start = addr;
                offset_ready--;
                break;
            case 2:
                cout << "offset_end. "<< addr << '\n';
                oa_end = addr;
                offset_ready--;
                break;
            case 3:
                cout << "edge_start, "<< addr << '\n';
                ea_start = addr;
                edge_ready--;
                break;
            case 4:
                cout << "edge_end, "<< addr << '\n';
                ea_end = addr;
                edge_ready--;
                break;
            case 5:
                cout << "property_start, "<< addr << '\n';
                pa_start = addr;
                property_ready--;
                break;
            case 6:
                cout << "property_end, "<< addr << '\n';
                pa_end = addr;
                property_ready--;
                break;
            case 765:
                {
                    cout << "FINALIZE\n";
                    if(offset_ready==0) 
                        allow_offset = true;
                    
                    if(edge_ready==0)
                        allow_edge = true;
                    
                    if(property_ready==0)
                        allow_property = true;

                    fstream f;
                    f.open("userdebug.stat", std::fstream::in | std::fstream::out | std::fstream::app);
                    f << "*************************************************************************\n";
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
};


#endif