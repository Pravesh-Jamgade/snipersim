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
    public:

    SetTracker(int coreid, String cache_name, int sets, int associativity)
    {
        array_type_2_bucket.insert({1, Bucket()});
        array_type_2_bucket.insert({2, Bucket()});
        array_type_2_bucket.insert({3, Bucket()});

        this->coreid = coreid;
        this->cache_name = cache_name;
        this->sets = sets;
        this->associativity = associativity;
    }

    ~SetTracker()
    {
        fstream f;
        f.open("cache_mem.stat", fstream::app | fstream::out);
        f << coreid << ',' << cache_name << "," << sets << "," << associativity << '\n';
        
        _LOG_CUSTOM_LOGGER(Log::Warning, Log::C, "core, cache, array, reuse, frequency\n");
        _LOG_CUSTOM_LOGGER(Log::Warning, Log::D, "core, cache, array, access-before-eviction, frequency\n");
        for(auto e: array_type_2_bucket)
        {
            for(auto x: e.second.reuse_to_count)
            {
                _LOG_CUSTOM_LOGGER(Log::Warning, Log::C, "%d, %s, %d, %d, %d", coreid, cache_name, e.first, x.first, x.second);
            }
            for(auto x: e.second.access_before_eviction_to_count)
            {
                _LOG_CUSTOM_LOGGER(Log::Warning, Log::D, "%d, %s, %d, %d, %d", coreid, cache_name, e.first, x.first, x.second);
            }
        }
    }
    /*
    same as what is used for Sim()->get_array_type(IntPtr address);
        0 None (intialized)
        1 Index or offset
        2 Edge or neighbour
        3 property
    */
    SetTracker()
    {
        array_type_2_bucket.insert({1, Bucket()});
        array_type_2_bucket.insert({2, Bucket()});
        array_type_2_bucket.insert({3, Bucket()});
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
    int coreid, num_sets, associativity;
    String cache_name;
    public:
    
    /*which sets are used*/
    set<int> sampled_sets;

    /*set to its data*/
    SetTracker local;

    /*track per block writes until eviction*/
    UInt64** cache_writes;

    /*track per set lru bit information of calculating reuse distance*/
    UInt64** cache_lru_info;

    CacheSampleStat(){}
    CacheSampleStat(int coreid, String cache_name, int num_sets, int associativity)
    {
        this->num_sets=num_sets;
        this->coreid=coreid;
        this->cache_name = cache_name;

        this->associativity = associativity;

        this->local = SetTracker(coreid, cache_name, num_sets, associativity);

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

        cache_lru_info = (UInt64**)malloc(sizeof(UInt64)*num_sets);
        cache_writes = (UInt64**)malloc(sizeof(UInt64*)*num_sets);
        for(int i=0; i< num_sets; i++)
        {
            cache_writes[i] = (UInt64*)calloc(associativity, sizeof(UInt64));
            cache_lru_info[i] = (UInt64*)calloc(2*associativity, sizeof(UInt64));

            for(int j=0; j< associativity; j++)
            {
                cache_lru_info[i][j] = associativity-1;
            }
        }
    };

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
    void func_track_set(int set_index, int line_index, int array_type)
    {
        if(!func_is_it_sampled_set(set_index) || array_type<0)
            return;
        
        //track lru bit info as reuse distance
        local.compute_reuse(cache_lru_info[set_index][line_index], array_type);

        //udpate lru bit info
        for(int j=0; j< associativity; j++)
        {
            if(cache_lru_info[set_index][j] < cache_lru_info[set_index][line_index])
            {
                cache_lru_info[set_index][j]++;
            }
        }
        cache_lru_info[set_index][line_index] = 0;
    }

    /*
    1.writes seen by cache block before eviction
    2.here no difference in local/global objects, they are mainly for reuse distance
    */
    void func_track_writes_before_evict(int set_index, int replacement_index, int array_type)
    {
        if(!func_is_it_sampled_set(set_index) || array_type<0)
            return;
        local.compute_access_before_evict(cache_writes[set_index][replacement_index], array_type);//track accesses before eviction
        cache_writes[set_index][replacement_index] = 1;// reset for new cache block
    }

  


};
#endif