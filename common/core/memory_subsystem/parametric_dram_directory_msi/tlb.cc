#include "tlb.h"
#include "stats.h"


namespace ParametricDramDirectoryMSI
{
   
TLB::TLB(String name, String cfgname, core_id_t core_id, UInt32 num_entries, UInt32 associativity, TLB *next_level)
   : m_size(num_entries)
   , m_associativity(associativity)
   , m_cache(name + "_cache", cfgname, core_id, num_entries / associativity, associativity, SIM_PAGE_SIZE, "lru", CacheBase::PR_L1_CACHE)
   , m_next_level(next_level)
   , m_access(0)
   , m_miss(0)
   , tlb_core_id(core_id)
{
   LOG_ASSERT_ERROR((num_entries / associativity) * associativity == num_entries, "Invalid TLB configuration: num_entries(%d) must be a multiple of the associativity(%d)", num_entries, associativity);

   registerStatsMetric(name, core_id, "access", &m_access);
   registerStatsMetric(name, core_id, "miss", &m_miss);
   
}

bool
TLB::lookup(IntPtr address, IntPtr va_address, SubsecondTime now, bool allocate_on_miss)
{
   UInt64 va_page = va_address >> 12;
   UInt64 pa_page = address >> 12;
   int type_page = (int)Sim()->getContextHintObject()->what_is_it(va_address);

   Sim()->getPageCall()->page_map[tlb_core_id][va_page].page_granularity[type_page] = 1;

   if(Sim()->getPageCall()->page_map[tlb_core_id].find(address) == Sim()->getPageCall()->page_map[tlb_core_id].end())
   {
      Sim()->getPageCall()->page_map[tlb_core_id][va_page] = Page(pa_page);
   }

   if(m_cache.getName().find("dtlb") != string::npos)
   {
      Sim()->getPageCall()->page_map[tlb_core_id][va_page].first_level_access++;
   }
   else if(m_cache.getName().find("stlb") != string::npos)
   {
      Sim()->getPageCall()->page_map[tlb_core_id][va_page].second_level_access++;
   }

   bool hit = m_cache.accessSingleLine(address, Cache::LOAD, NULL, 0, now, true);

   m_access++;

   if (hit)
      return true;

   m_miss++;

   if(m_cache.getName().find("dtlb") != string::npos)
   {
      Sim()->getPageCall()->page_map[tlb_core_id][va_page].first_level_miss++;
   }
   else if(m_cache.getName().find("stlb") != string::npos)
   {
      Sim()->getPageCall()->page_map[tlb_core_id][va_page].second_level_miss++;
   }

   if (m_next_level)
   {
      hit = m_next_level->lookup(address, va_address, now, false /* no allocation */);    //saurabh
   }

   if (allocate_on_miss)
   {
      allocate(address, now);
   }

   return hit;
}

void
TLB::allocate(IntPtr address, SubsecondTime now)
{
   bool eviction;
   IntPtr evict_addr;
   CacheBlockInfo evict_block_info;
   m_cache.insertSingleLine(address, NULL, &eviction, &evict_addr, &evict_block_info, NULL, now);

   // Use next level as a victim cache
   if (eviction && m_next_level)
      m_next_level->allocate(evict_addr, now);
}

}
