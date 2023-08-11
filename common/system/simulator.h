#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "config.h"
#include "log.h"
#include "inst_mode.h"

#include <decoder.h>

//*
#include "context_hint.h"

class _Thread;
class SyscallServer;
class SyncServer;
class MagicServer;
class ClockSkewMinimizationServer;
class StatsManager;
class Transport;
class CoreManager;
class Thread;
class ThreadManager;
class ThreadStatsManager;
class SimThreadManager;
class HooksManager;
class ClockSkewMinimizationManager;
class FastForwardPerformanceManager;
class TraceManager;
class DvfsManager;
class SamplingManager;
class FaultinjectionManager;
class TagsManager;
class RoutineTracer;
class MemoryTracker;
namespace config { class Config; }

class Simulator
{
public:
   Simulator();
   ~Simulator();

   void start();

   static Simulator* getSingleton() { return m_singleton; }
   static void setConfig(config::Config * cfg, Config::SimulationMode mode);
   static void allocate();
   static void release();

   SyscallServer* getSyscallServer() { return m_syscall_server; }
   SyncServer* getSyncServer() { return m_sync_server; }
   MagicServer* getMagicServer() { return m_magic_server; }
   ClockSkewMinimizationServer* getClockSkewMinimizationServer() { return m_clock_skew_minimization_server; }
   CoreManager *getCoreManager() { return m_core_manager; }
   SimThreadManager *getSimThreadManager() { return m_sim_thread_manager; }
   ThreadManager *getThreadManager() { return m_thread_manager; }
   ClockSkewMinimizationManager *getClockSkewMinimizationManager() { return m_clock_skew_minimization_manager; }
   FastForwardPerformanceManager *getFastForwardPerformanceManager() { return m_fastforward_performance_manager; }
   Config *getConfig() { return &m_config; }
   config::Config *getCfg() {
      //if (! m_config_file_allowed)
      //   LOG_PRINT_ERROR("getCfg() called after init, this is not nice\n");
      return m_config_file;
   }
   void hideCfg() { m_config_file_allowed = false; }
   StatsManager *getStatsManager() { return m_stats_manager; }
   ThreadStatsManager *getThreadStatsManager() { return m_thread_stats_manager; }
   DvfsManager *getDvfsManager() { return m_dvfs_manager; }
   HooksManager *getHooksManager() { return m_hooks_manager; }
   SamplingManager *getSamplingManager() { return m_sampling_manager; }
   FaultinjectionManager *getFaultinjectionManager() { return m_faultinjection_manager; }
   TraceManager *getTraceManager() { return m_trace_manager; }
   TagsManager *getTagsManager() { return m_tags_manager; }
   RoutineTracer *getRoutineTracer() { return m_rtn_tracer; }
   MemoryTracker *getMemoryTracker() { return m_memory_tracker; }
   void setMemoryTracker(MemoryTracker *memory_tracker) { m_memory_tracker = memory_tracker; }

   //saurabh
   IntPtr Neigh_count_On_Total_Access = 0;
   IntPtr Neigh_count_On_Hit_L1_I = 0;
   IntPtr Neigh_count_On_Hit_L1_D = 0;
   IntPtr Neigh_count_On_Hit_L2 = 0;
   IntPtr Neigh_count_On_Hit_L3 = 0;
   IntPtr Neigh_count_On_Hit_L4 = 0;
   IntPtr Neigh_count_On_Hit_else = 0;
   IntPtr Neigh_count_On_Miss = 0;
   IntPtr Index_count_On_Total_Access = 0;
   IntPtr Index_count_On_Hit_L1_I = 0;
   IntPtr Index_count_On_Hit_L1_D = 0;
   IntPtr Index_count_On_Hit_L2 = 0;
   IntPtr Index_count_On_Hit_L3 = 0;
   IntPtr Index_count_On_Hit_L4 = 0;
   IntPtr Index_count_On_Hit_else = 0;
   IntPtr Index_count_On_Miss = 0;
   int flag_N =0;
   int flag_I =0;
   IntPtr Virtual_Neigh_Start = 0, Virtual_Neigh_End =0;
   IntPtr Virtual_Index_Start = 0, Virtual_Index_End =0;
   bool neigh_bounds_ready, index_bounds_ready, property_bounds_ready;

   /*
      0 None
      1 Index or Offset
      2 Edge or Neighbour
      3 property
   */
   int get_array_type(IntPtr address)
   {
      // if(neigh_bounds_ready){
         if(Virtual_Neigh_Start <= address && address <= Virtual_Neigh_End){
            return 2;
         }
      // }
      // if(index_bounds_ready){
         if(Virtual_Index_Start <= address && address <= Virtual_Index_End){
            return 1;
         }
      // }
      
      return 0;
   }
   //saurabh
  
   bool isRunning() { return m_running; }
   static void enablePerformanceModels();
   static void disablePerformanceModels();

   void setInstrumentationMode(InstMode::inst_mode_t new_mode, bool update_barrier);
   InstMode::inst_mode_t getInstrumentationMode() { return InstMode::inst_mode; }

   // Access to the Decoder library for the simulator run
   void createDecoder();
   dl::Decoder *getDecoder();

   //*
   void set_context(int type, uint64_t addr);
   ContextHint* getContextHintObject(){return context_hint;}
   
private:
   Config m_config;
   Log m_log;
   TagsManager *m_tags_manager;
   SyscallServer *m_syscall_server;
   SyncServer *m_sync_server;
   MagicServer *m_magic_server;
   ClockSkewMinimizationServer *m_clock_skew_minimization_server;
   StatsManager *m_stats_manager;
   Transport *m_transport;
   CoreManager *m_core_manager;
   ThreadManager *m_thread_manager;
   ThreadStatsManager *m_thread_stats_manager;
   SimThreadManager *m_sim_thread_manager;
   ClockSkewMinimizationManager *m_clock_skew_minimization_manager;
   FastForwardPerformanceManager *m_fastforward_performance_manager;
   TraceManager *m_trace_manager;
   DvfsManager *m_dvfs_manager;
   HooksManager *m_hooks_manager;
   SamplingManager *m_sampling_manager;
   FaultinjectionManager *m_faultinjection_manager;
   RoutineTracer *m_rtn_tracer;
   MemoryTracker *m_memory_tracker;

   //*
   ContextHint* context_hint;

   bool m_running;
   bool m_inst_mode_output;

   static Simulator *m_singleton;

   static config::Config *m_config_file;
   static bool m_config_file_allowed;
   static Config::SimulationMode m_mode;
   
   // Object to access the decoder library with the correct configuration
   static dl::Decoder *m_decoder;
   // Surrogate to create a Decoder object for a specific architecture
   dl::DecoderFactory *m_factory;

   void printInstModeSummary();
};

__attribute__((unused)) static Simulator *Sim()
{
   return Simulator::getSingleton();
}

#endif // SIMULATOR_H
