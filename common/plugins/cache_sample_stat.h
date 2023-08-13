#ifndef CACHE_SAMPLE_STAT_H
#define CACHE_SAMPLE_STAT_H

#include <bits/stdc++.h>
#include "fixed_types.h"
#include "inttypes.h"

#define SAMPLE 256
using namespace std;

/*
1.reuse distance and its frequency
2.accesses before eviction and its frequency
*/
class Bucket
{
    public:
    map<int, IntPtr> reuse_to_count;
    map<int, IntPtr> access_before_eviction_to_count;

    Bucket(){}
    ~Bucket(){}
    void func_make_reuse_count(int reuse_name)
    {
        if(reuse_to_count.find(reuse_name) == reuse_to_count.end())
        {
            reuse_to_count.insert({reuse_name, 1});
            return;
        }
        reuse_to_count[reuse_name]++;
    }
    void func_make_access_count_before_evict(int access_name)
    {
        if(access_before_eviction_to_count.find(access_name) == access_before_eviction_to_count.end())
        {
            access_before_eviction_to_count.insert({access_name, 1});
            return;
        }
        access_before_eviction_to_count[access_name]++;
    }
};

/*tracks reuse distance and its count (frequency of reuse distance) for each of array type */
class SetTracker
{
    String cache_name;
    int coreid;
    map<int, Bucket> array_type_2_bucket;
    int sets, associativity;
    bool local;

    public:

    static bool once_a;

    SetTracker(){}

    SetTracker(int coreid, String cache_name, int sets, int associativity, bool local)
    {
        array_type_2_bucket.insert({1, Bucket()});
        array_type_2_bucket.insert({2, Bucket()});
        array_type_2_bucket.insert({3, Bucket()});

        this->coreid = coreid;
        this->cache_name = cache_name;
        this->sets = sets;
        this->associativity = associativity;
        this->local = local;
    }

    ~SetTracker()
    {
        PrintStat();
        fstream f;
        f.open("cache_mem.stat", fstream::app | fstream::out);
        f << coreid << ',' << cache_name << "," << sets << "," << associativity << '\n';
    }

    /*compute frequency of reuse distance*/
    void compute_reuse(int lru_dist, int array_type)
    {
        array_type_2_bucket[array_type].func_make_reuse_count(lru_dist);
    }

    void compute_access_before_evict(int access, int array_type)
    {
        array_type_2_bucket[array_type].func_make_access_count_before_evict(access);
    }

    void PrintStat()
    {
        if(!once_a)
        {
            _LOG_CUSTOM_LOGGER(Log::Warning, Log::C, "local, core, cache, array, reuse, frequency\n");
            _LOG_CUSTOM_LOGGER(Log::Warning, Log::D, "local, core, cache, array, access-before-eviction, frequency\n");
            once_a = true;
        }

        for(auto e: array_type_2_bucket)
        {
            for(auto x: e.second.reuse_to_count)
            {
                _LOG_CUSTOM_LOGGER(Log::Warning, Log::C, "%d, %d, %s, %d, %d, %d\n", local, coreid, cache_name.c_str(), e.first, x.first, x.second);
            }
            for(auto x: e.second.access_before_eviction_to_count)
            {
                _LOG_CUSTOM_LOGGER(Log::Warning, Log::D, "%d, %d, %s, %d, %d, %d\n", local, coreid, cache_name.c_str(), e.first, x.first, x.second);
            }
        }
    }
};

/*
same as what is used for Sim()->get_array_type(IntPtr address);
    0 None (intialized)
    1 Index or offset
    2 Edge or neighbour
    3 property
*/
class CacheSampleStat
{
    int coreid, num_sets;
    UInt32 associativity;
    String cache_name;
    public:
    
    /*which sets are used*/
    set<int> sampled_sets;

    /*set to its data*/
    SetTracker* local, *global;

    /*track per block writes until eviction*/
    // UInt64** cache_accesses;

    /*track per set lru bit information of calculating reuse distance*/
    // UInt64** cache_lru_info;

    /*last m_associative size history tracker*/
    class TAG
    {
        public:
        UInt64 tag;
        UInt64 load_hit, store_hit;
        UInt64 load_miss, store_miss;
        UInt64 fill_insert;
        UInt64 demand_accesses;
        UInt32 lru;
        bool valid;

        TAG(){
            valid = false;
            tag=lru=0;
            load_hit = load_miss = store_hit = store_miss = 0;
            fill_insert = 0;
            demand_accesses = 0;
        }

        TAG(int associativity){
            valid = false;
            tag=lru=0;
            load_hit = load_miss = store_hit = store_miss = 0;
            fill_insert = 0;
            demand_accesses = 0;
        }
        
        void reset(UInt32 associativity)
        {
            demand_accesses = load_hit = load_miss = store_hit = store_miss = fill_insert = 0;
            tag = 0;
            valid = false;
            lru = associativity;
        }
    };

    /*m = associattivity: CAN BE VARIED depends upon how long history we want to keep*/
    TAG*** last_m_tag_tracker, ***m_lru_info;

    CacheSampleStat(){}
    CacheSampleStat(int coreid, String cache_name, int num_sets, UInt32 associativity)
    {
        this->num_sets=num_sets;
        this->coreid=coreid;
        this->cache_name = cache_name;

        this->associativity = associativity;

        this->local = new SetTracker(coreid, cache_name, num_sets, associativity, true);
        this->global = new SetTracker(coreid, cache_name, num_sets, associativity, false);

        int sample_size = SAMPLE;
        if(this->num_sets<SAMPLE)
            sample_size = this->num_sets;
        
        // keep track of which sets to observe
        for(int i=0; i< sample_size; i++)
        {
            int temp = rand()%num_sets;
            while(sampled_sets.find(temp)!=sampled_sets.end())
            {
                temp = rand()%num_sets;
            }

            sampled_sets.insert(temp);
        }

        last_m_tag_tracker = (TAG***)malloc(sizeof(TAG**)*num_sets);
        m_lru_info = (TAG***)malloc(sizeof(TAG**)*num_sets);

        for(int i=0; i< num_sets; i++)
        {
            m_lru_info[i] = (TAG**)calloc(associativity, sizeof(TAG));
            last_m_tag_tracker[i] = (TAG**)calloc(associativity, sizeof(TAG)); //history asssociativity KNOB

            for(UInt32 j=0; j< associativity; j++)
            {
                last_m_tag_tracker[i][j] = new TAG(associativity);
                m_lru_info[i][j] = new TAG(associativity);
            }
        }
    };

    ~CacheSampleStat()
    {
        delete local;
        delete global;
    }

    /*check if set is amongst sampled set: true if yes*/
    bool func_is_it_sampled_set(int set_no)
    {
        return sampled_sets.find(set_no)!=sampled_sets.end();
    }

    /*
    1. local looks at cache set blocks reuse distance during its presence in cache set
    2. global looks at cache set blocks reuse distance after eviction
    */
    /*upon hit [ miss(reuse dist=infinite or -1) we are not tracking that's why called local] track reuse distance*/
    void func_track_hit_event(UInt32 set_index, UInt32 line_index, int array_type, bool is_load)
    {
        if(!func_is_it_sampled_set(set_index) || array_type<=-1)
            return;
        
        //track lru bit info as reuse distance
        local->compute_reuse(m_lru_info[set_index][line_index]->lru, array_type);

        //udpate lru bit info
        for(UInt32 j=0; j< associativity; j++)
        {
            if(m_lru_info[set_index][j]->lru < m_lru_info[set_index][line_index]->lru)
            {
                m_lru_info[set_index][j]->lru++;
            }
        }
        
        m_lru_info[set_index][line_index]->lru = 0;
        if(is_load)
            m_lru_info[set_index][line_index]->load_hit++;
        else 
            m_lru_info[set_index][line_index]->store_hit++;

        m_lru_info[set_index][line_index]->demand_accesses++;
    }

    /*evicting from cache, insert into history
        @param: set_index, line_index, array_type, tag 
    */
    void func_track_evict_event(UInt32 set_index, UInt32 line_index, int array_type, UInt64 tag)
    {
        if(!func_is_it_sampled_set(set_index) || array_type<=-1)
            return;

        UInt32 index = 0xffff;

        // insert @ first "invalid"
        for(UInt32 j=0; j< associativity; j++)
        {
            if(!last_m_tag_tracker[set_index][j]->valid)
            {
                index = j;
                break;
            }
        }

        // if invalid not found; do "replacement"
        if(index == 0xffff)
        {
            for(UInt32 j=0; j< associativity; j++)
            {
                if(last_m_tag_tracker[set_index][j]->lru == associativity-1)
                {
                    index = j;
                    break;
                }
            }
            
            if(index == 0xffff) {
                std::cout << "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXxx\n";
                std::cout << "should nat happen\n";
                std::cout << "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXxx\n";
                exit(-1);
            }

            for(UInt32 j=0; j< associativity; j++)
            {
                if(last_m_tag_tracker[set_index][j]->lru < last_m_tag_tracker[set_index][index]->lru)
                {
                    last_m_tag_tracker[set_index][j]->lru++;
                }
            }
        }

        // insert @ invalid pos
        else
        {   
            // move others to lru 
            for(UInt32 j=0; j< associativity; j++)
            {
                if(j!=index && last_m_tag_tracker[set_index][j]->valid)
                {
                    last_m_tag_tracker[set_index][j]->lru++;
                }
            }
        }

        // insert tag at index location and make it valid
        if(index != 0xffff)
        {
            last_m_tag_tracker[set_index][index]->tag = tag;
            last_m_tag_tracker[set_index][index]->valid = true;
            last_m_tag_tracker[set_index][index]->lru = 0;
        }

    }

    /*
        miss for incomming req. A.
        We are dealing here, for requested A only. 
        Looking it into history queue to check whether it was seen earlier.
        If so, remove it and adjust lrubits accordingly for others.
        Insert this entry into main LRU chain.
       
        maintain history of length "associativity"
        miss in cache   -> "found" in history then remove from history
                        -> "not found" in history then okk

        miss causes insertion with/without eviction, track such writes, lru_info
    */
    void func_track_miss_event(UInt32 set_index, UInt32 index, int req_array_type, UInt64 req_tag)
    {
        if(!func_is_it_sampled_set(set_index) || req_array_type<=-1)
            return;
        
        // check if missed address found in hitstory, if found then remove it from history
        bool found = false;
        UInt32 pos = -1;

        // history associativity KNOB
        for(UInt32 j=0; j< associativity; j++)
        {
            if(last_m_tag_tracker[set_index][j]->valid && last_m_tag_tracker[set_index][j]->tag == req_tag)
            {
                pos = j;
                found = true;
                // these data structure helps us to keep track of mem. ref. which are moved out of
                // cache already. Its associativity (depends upon how long history interval we want to keep)
                // is kept same (can be changed). But its distance would be
                // always past orig_cache_associativity + j
                global->compute_reuse(associativity+j, req_array_type);// its the cache associativity NOT the KNOB
                break;
            }
        }
        
        // chaning history queue lru values for 'pos'
        if(found)
        {
            // if found: adjust lru bits of all entries having lru values above pos's lru value
            for(UInt32 i=0; i< associativity; i++)
            {
                if(last_m_tag_tracker[set_index][i]->valid && 
                    last_m_tag_tracker[set_index][i]->lru > last_m_tag_tracker[set_index][pos]->lru)
                {
                    last_m_tag_tracker[set_index][i]->lru--;
                    if(last_m_tag_tracker[set_index][i]->lru>associativity)
                    {
                        printf("THIS is wrong\n");
                        exit(-1);
                    }                
                }
            }
            
            //remove entry as it will now move to main lru chain 
            last_m_tag_tracker[set_index][pos]->lru = associativity;
            last_m_tag_tracker[set_index][pos]->tag = 0;
            last_m_tag_tracker[set_index][pos]->valid = false;
        }

        // it is now moved to main lru chain
        m_lru_info[set_index][index]->lru = 0;
        m_lru_info[set_index][index]->tag = req_tag;
        m_lru_info[set_index][index]->valid = true;

    }

    /*
    1.access seen by cache block before eviction
    2.here no difference in local/global objects, they are mainly for reuse distance
    3.reset cache_accesses = 0, as these block is evicted by other 
    */
    void func_track_access_before_evict(UInt32 set_index, UInt32 replacement_index, int array_type)
    {
        if(!func_is_it_sampled_set(set_index) || array_type<=-1)
            return;
        local->compute_access_before_evict(m_lru_info[set_index][replacement_index]->demand_accesses, array_type);//track accesses before eviction
        m_lru_info[set_index][replacement_index]->reset(associativity);// reset for new cache block
    }

    void print()
    {
        // local->PrintStat();
        // global->PrintStat();
    }
};
#endif