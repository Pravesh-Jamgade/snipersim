#ifndef CACHE_H
#define CACHE_H

#include "cache_base.h"
#include "cache_set.h"
#include "cache_block_info.h"
#include "utils.h"
#include "hash_map_set.h"
#include "cache_perf_model.h"
#include "shmem_perf_model.h"
#include "log.h"
#include "core.h"
#include "fault_injection.h"

// Define to enable the set usage histogram
//#define ENABLE_SET_USAGE_HIST

#include "cache_sample_stat.h"

class Cache : public CacheBase
{
   private:
      bool m_enabled;

      // Cache counters
      UInt64 m_num_accesses;
      UInt64 m_num_hits;

      // Generic Cache Info
      cache_t m_cache_type;
      CacheSet** m_sets;
      CacheSetInfo* m_set_info;

      FaultInjector *m_fault_injector;

      #ifdef ENABLE_SET_USAGE_HIST
      UInt64* m_set_usage_hist;
      #endif

      CacheSampleStat* cache_sample_stat;

      bool once_0, once_1, once_2, once_3, once_12, once_13, once_21, once_23, once_31, once_32;
      bool flag;
      
   public:

      // constructors/destructors
      Cache(String name,
            String cfgname,
            core_id_t core_id,
            UInt32 num_sets,
            UInt32 associativity, UInt32 cache_block_size,
            String replacement_policy,
            cache_t cache_type,
            hash_t hash = CacheBase::HASH_MASK,
            FaultInjector *fault_injector = NULL,
            AddressHomeLookup *ahl = NULL);
      ~Cache();

      Lock& getSetLock(IntPtr addr);

      bool invalidateSingleLine(IntPtr addr);
      CacheBlockInfo* accessSingleLine(IntPtr addr,
            access_t access_type, Byte* buff, UInt32 bytes, SubsecondTime now, bool update_replacement);
      void insertSingleLine(IntPtr addr, Byte* fill_buff,
            bool* eviction, IntPtr* evict_addr,
            CacheBlockInfo* evict_block_info, Byte* evict_buff, SubsecondTime now, CacheCntlr *cntlr = NULL);
      CacheBlockInfo* peekSingleLine(IntPtr addr);

      CacheBlockInfo* peekBlock(UInt32 set_index, UInt32 way) const { return m_sets[set_index]->peekBlock(way); }

      // Update Cache Counters
      void updateCounters(bool cache_hit);
      void updateHits(Core::mem_op_t mem_op_type, UInt64 hits);

      void enable() { m_enabled = true; }
      void disable() { m_enabled = false; }

      void func_splitAddress(IntPtr addr, IntPtr& tag, UInt32& set_index,
                  UInt32& block_offset){
                        splitAddress(addr, tag, set_index, block_offset);
                  }
      
      void func_cache_name(IntPtr addr){
            _LOG_CUSTOM_LOGGER(Log::Warning, Log::DBG, "%s, %d\n", m_name.c_str(), addr);
      }

      void func_track_hit_event(UInt32 set_index, UInt32 line_index, int array_type, bool is_load)
      {
            cache_sample_stat->func_track_hit_event(set_index, line_index, array_type, is_load);
      }

      void func_find_line_index(IntPtr tag, UInt32 set_index, UInt32& line_index, bool& is_found)
      {
            CacheSet* set = m_sets[set_index];
            CacheBlockInfo* cache_block_info = set->find(tag, &line_index);   
            is_found = cache_block_info!=nullptr;
      }
};

template <class T>
UInt32 moduloHashFn(T key, UInt32 hash_fn_param, UInt32 num_buckets)
{
   return (key >> hash_fn_param) % num_buckets;
}

#endif /* CACHE_H */
