/*
 * Licensed to the OpenAirInterface (OAI) Software Alliance under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The OpenAirInterface Software Alliance licenses this file to You under
 * the OAI Public License, Version 1.0  (the "License"); you may not use this file
 * except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.openairinterface.org/?page_id=698
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *-------------------------------------------------------------------------------
 * For more information about the OpenAirInterface (OAI) Software Alliance:
 *      contact@openairinterface.org
 */

/*! \file lte-enb.c
 * \brief Top-level threads for eNodeB
 * \author R. Knopp, F. Kaltenberger, Navid Nikaein
 * \date 2012
 * \version 0.1
 * \company Eurecom
 * \email: knopp@eurecom.fr,florian.kaltenberger@eurecom.fr, navid.nikaein@eurecom.fr
 * \note
 * \warning
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sched.h>
#include <linux/sched.h>
#include <signal.h>
#include <execinfo.h>
#include <getopt.h>
#include <sys/sysinfo.h>
#include "rt_wrapper.h"
#include <time.h>


//DLSH thread
#include <semaphore.h>
#include <assert.h>
#include <sched.h>
#include <pthread.h>
//=====

#undef MALLOC //there are two conflicting definitions, so we better make sure we don't use it at all

#include "assertions.h"
#include "msc.h"

#include "PHY/types.h"

#include "PHY/defs.h"

#undef MALLOC //there are two conflicting definitions, so we better make sure we don't use it at all
//#undef FRAME_LENGTH_COMPLEX_SAMPLES //there are two conflicting definitions, so we better make sure we don't use it at all

#include "../../ARCH/COMMON/common_lib.h"

//#undef FRAME_LENGTH_COMPLEX_SAMPLES //there are two conflicting definitions, so we better make sure we don't use it at all

#include "PHY/LTE_TRANSPORT/if4_tools.h"
#include "PHY/LTE_TRANSPORT/if5_tools.h"

#include "PHY/extern.h"
#include "SCHED/extern.h"
#include "LAYER2/MAC/extern.h"

#include "../../SIMU/USER/init_lte.h"

#include "LAYER2/MAC/defs.h"
#include "LAYER2/MAC/extern.h"
#include "LAYER2/MAC/proto.h"
#include "RRC/LITE/extern.h"
#include "PHY_INTERFACE/extern.h"

#ifdef SMBV
#include "PHY/TOOLS/smbv.h"
unsigned short config_frames[4] = {2,9,11,13};
#endif
#include "UTIL/LOG/log_extern.h"
#include "UTIL/OTG/otg_tx.h"
#include "UTIL/OTG/otg_externs.h"
#include "UTIL/MATH/oml.h"
#include "UTIL/LOG/vcd_signal_dumper.h"
#include "UTIL/OPT/opt.h"
#include "enb_config.h"
//#include "PHY/TOOLS/time_meas.h"

#ifndef OPENAIR2
#include "UTIL/OTG/otg_extern.h"
#endif

#if defined(ENABLE_ITTI)
# if defined(ENABLE_USE_MME)
#   include "s1ap_eNB.h"
#ifdef PDCP_USE_NETLINK
#   include "SIMULATION/ETH_TRANSPORT/proto.h"
#endif
# endif
#endif

#include "T.h"
//TEST//
#define NUM_THREAD 4

//#define DEBUG_THREADS 1

//#define USRP_DEBUG 1
struct timing_info_t {
  //unsigned int frame, hw_slot, last_slot, next_slot;
  RTIME time_min, time_max, time_avg, time_last, time_now;
  //unsigned int mbox0, mbox1, mbox2, mbox_target;
  unsigned int n_samples;
} timing_info;

// Fix per CC openair rf/if device update
// extern openair0_device openair0;

#if defined(ENABLE_ITTI)
extern volatile int             start_eNB;
extern volatile int             start_UE;
#endif
extern volatile int                    oai_exit;

extern openair0_config_t openair0_cfg[MAX_CARDS];

extern pthread_cond_t sync_cond;
extern pthread_mutex_t sync_mutex;
extern int sync_var;

//pthread_t                       main_eNB_thread;

time_stats_t softmodem_stats_mt; // main thread
time_stats_t softmodem_stats_hw; //  hw acquisition
time_stats_t softmodem_stats_rxtx_sf; // total tx time
time_stats_t softmodem_stats_rx_sf; // total rx time
int32_t **rxdata;
int32_t **txdata;

uint8_t seqno; //sequence number

static int                      time_offset[4] = {0,0,0,0};

/* mutex, cond and variable to serialize phy proc TX calls
 * (this mechanism may be relaxed in the future for better
 * performances)
 */
static struct {
  pthread_mutex_t  mutex_phy_proc_tx;
  pthread_cond_t   cond_phy_proc_tx;
  volatile uint8_t phy_proc_CC_id;
} sync_phy_proc;

extern double cpuf;

void exit_fun(const char* s);

void init_eNB(eNB_func_t node_function[], eNB_timing_t node_timing[],int nb_inst,eth_params_t *,int);
void stop_eNB(int nb_inst);


static inline void thread_top_init(char *thread_name,
				   int affinity,
				   uint64_t runtime,
				   uint64_t deadline,
				   uint64_t period) {

  MSC_START_USE();

#ifdef DEADLINE_SCHEDULER
printf("aaaaaaaaaaaaaaaaaaaa\n\n");
  struct sched_attr attr;

  unsigned int flags = 0;

  attr.size = sizeof(attr);
  attr.sched_flags = 0;
  attr.sched_nice = 0;
  attr.sched_priority = 0;

  attr.sched_policy   = SCHED_DEADLINE;
  attr.sched_runtime  = runtime;
  attr.sched_deadline = deadline;
  attr.sched_period   = period; 

  if (sched_setattr(0, &attr, flags) < 0 ) {
    perror("[SCHED] eNB tx thread: sched_setattr failed\n");
    exit(1);
  }

#else //LOW_LATENCY
  int policy, s, j;
  struct sched_param sparam;
  char cpu_affinity[1024];
  cpu_set_t cpuset;

  /* Set affinity mask to include CPUs 1 to MAX_CPUS */
  /* CPU 0 is reserved for UHD threads */
  /* CPU 1 is reserved for all RX_TX threads */
  /* Enable CPU Affinity only if number of CPUs >2 */
  CPU_ZERO(&cpuset);

#ifdef CPU_AFFINITY
printf("bbbbbbbbbbbbbbbbbbbbbbbb\n\n");
  if (get_nprocs() > 2)
  {
	  printf("ccccccccccccccccccccc\n\n");
    if (affinity == 0)
      CPU_SET(0,&cpuset);
    else
      for (j = 1; j < get_nprocs(); j++)
        CPU_SET(j, &cpuset);
    s = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
    if (s != 0)
    {
      perror( "pthread_setaffinity_np");
      exit_fun("Error setting processor affinity");
    }
  }
#endif //CPU_AFFINITY
if(strcmp("eNB_thread_single",thread_name)==0){
	CPU_SET(0,&cpuset);
	s = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
	if (s != 0)
	{
		perror( "pthread_setaffinity_np");
		exit_fun("Error setting processor affinity");
	}
}

printf("ddddddddddddddddddddddd\n\n");
  /* Check the actual affinity mask assigned to the thread */
  s = pthread_getaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
  if (s != 0) {
    perror( "pthread_getaffinity_np");
    exit_fun("Error getting processor affinity ");
  }
  memset(cpu_affinity,0,sizeof(cpu_affinity));
  for (j = 0; j < CPU_SETSIZE; j++)
    if (CPU_ISSET(j, &cpuset)) {  
      char temp[1024];
      sprintf (temp, " CPU_%d", j);
      strcat(cpu_affinity, temp);
    }

  memset(&sparam, 0, sizeof(sparam));
  sparam.sched_priority = sched_get_priority_max(SCHED_FIFO);
  policy = SCHED_FIFO ; 
  
  s = pthread_setschedparam(pthread_self(), policy, &sparam);
  if (s != 0) {
    perror("pthread_setschedparam : ");
    exit_fun("Error setting thread priority");
  }
  
  s = pthread_getschedparam(pthread_self(), &policy, &sparam);
  if (s != 0) {
    perror("pthread_getschedparam : ");
    exit_fun("Error getting thread priority");
  }

  LOG_I(HW, "[SCHED][eNB] %s started on CPU %d TID %ld, sched_policy = %s , priority = %d, CPU Affinity=%s \n",thread_name,sched_getcpu(),gettid(),
                   (policy == SCHED_FIFO)  ? "SCHED_FIFO" :
                   (policy == SCHED_RR)    ? "SCHED_RR" :
                   (policy == SCHED_OTHER) ? "SCHED_OTHER" :
                   "???",
                   sparam.sched_priority, cpu_affinity );

#endif //LOW_LATENCY

  mlockall(MCL_CURRENT | MCL_FUTURE);

}

static inline void wait_sync(char *thread_name) {

  printf( "waiting for sync (%s)\n",thread_name);
  pthread_mutex_lock( &sync_mutex );
  
  while (sync_var<0)
    pthread_cond_wait( &sync_cond, &sync_mutex );
  
  pthread_mutex_unlock(&sync_mutex);
  
  printf( "got sync (%s)\n", thread_name);

}

void do_OFDM_mod_rt(int subframe,PHY_VARS_eNB *phy_vars_eNB) {
     
  unsigned int aa,slot_offset, slot_offset_F;
  int dummy_tx_b[7680*4] __attribute__((aligned(32)));
  int i,j, tx_offset;
  int slot_sizeF = (phy_vars_eNB->frame_parms.ofdm_symbol_size)*
                   ((phy_vars_eNB->frame_parms.Ncp==1) ? 6 : 7);
  int len,len2;
  int16_t *txdata;
//  int CC_id = phy_vars_eNB->proc.CC_id;

  VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_ENB_SFGEN , 1 );

  slot_offset_F = (subframe<<1)*slot_sizeF;

  slot_offset = subframe*phy_vars_eNB->frame_parms.samples_per_tti;

  if ((subframe_select(&phy_vars_eNB->frame_parms,subframe)==SF_DL)||
      ((subframe_select(&phy_vars_eNB->frame_parms,subframe)==SF_S))) {
    //    LOG_D(HW,"Frame %d: Generating slot %d\n",frame,next_slot);

    for (aa=0; aa<phy_vars_eNB->frame_parms.nb_antennas_tx; aa++) {
      if (phy_vars_eNB->frame_parms.Ncp == EXTENDED) {
        PHY_ofdm_mod(&phy_vars_eNB->common_vars.txdataF[0][aa][slot_offset_F],
                     dummy_tx_b,
                     phy_vars_eNB->frame_parms.ofdm_symbol_size,
                     6,
                     phy_vars_eNB->frame_parms.nb_prefix_samples,
                     CYCLIC_PREFIX);
        PHY_ofdm_mod(&phy_vars_eNB->common_vars.txdataF[0][aa][slot_offset_F+slot_sizeF],
                     dummy_tx_b+(phy_vars_eNB->frame_parms.samples_per_tti>>1),
                     phy_vars_eNB->frame_parms.ofdm_symbol_size,
                     6,
                     phy_vars_eNB->frame_parms.nb_prefix_samples,
                     CYCLIC_PREFIX);
      } else {
        normal_prefix_mod(&phy_vars_eNB->common_vars.txdataF[0][aa][slot_offset_F],
                          dummy_tx_b,
                          7,
                          &(phy_vars_eNB->frame_parms));
	// if S-subframe generate first slot only
	if (subframe_select(&phy_vars_eNB->frame_parms,subframe) == SF_DL) 
	  normal_prefix_mod(&phy_vars_eNB->common_vars.txdataF[0][aa][slot_offset_F+slot_sizeF],
			    dummy_tx_b+(phy_vars_eNB->frame_parms.samples_per_tti>>1),
			    7,
			    &(phy_vars_eNB->frame_parms));
      }

      // if S-subframe generate first slot only
      if (subframe_select(&phy_vars_eNB->frame_parms,subframe) == SF_S)
	len = phy_vars_eNB->frame_parms.samples_per_tti>>1;
      else
	len = phy_vars_eNB->frame_parms.samples_per_tti;
      /*
      for (i=0;i<len;i+=4) {
	dummy_tx_b[i] = 0x100;
	dummy_tx_b[i+1] = 0x01000000;
	dummy_tx_b[i+2] = 0xff00;
	dummy_tx_b[i+3] = 0xff000000;
	}*/
      
      if (slot_offset+time_offset[aa]<0) {
	txdata = (int16_t*)&phy_vars_eNB->common_vars.txdata[0][aa][(LTE_NUMBER_OF_SUBFRAMES_PER_FRAME*phy_vars_eNB->frame_parms.samples_per_tti)+tx_offset];
        len2 = -(slot_offset+time_offset[aa]);
	len2 = (len2>len) ? len : len2;
	for (i=0; i<(len2<<1); i++) {
	  txdata[i] = ((int16_t*)dummy_tx_b)[i]<<openair0_cfg[0].iq_txshift;
	}
	if (len2<len) {
	  txdata = (int16_t*)&phy_vars_eNB->common_vars.txdata[0][aa][0];
	  for (j=0; i<(len<<1); i++,j++) {
	    txdata[j++] = ((int16_t*)dummy_tx_b)[i]<<openair0_cfg[0].iq_txshift;
	  }
	}
      }  
      else if ((slot_offset+time_offset[aa]+len)>(LTE_NUMBER_OF_SUBFRAMES_PER_FRAME*phy_vars_eNB->frame_parms.samples_per_tti)) {
	tx_offset = (int)slot_offset+time_offset[aa];
	txdata = (int16_t*)&phy_vars_eNB->common_vars.txdata[0][aa][tx_offset];
	len2 = -tx_offset+LTE_NUMBER_OF_SUBFRAMES_PER_FRAME*phy_vars_eNB->frame_parms.samples_per_tti;
	for (i=0; i<(len2<<1); i++) {
	  txdata[i] = ((int16_t*)dummy_tx_b)[i]<<openair0_cfg[0].iq_txshift;
	}
	txdata = (int16_t*)&phy_vars_eNB->common_vars.txdata[0][aa][0];
	for (j=0; i<(len<<1); i++,j++) {
	  txdata[j++] = ((int16_t*)dummy_tx_b)[i]<<openair0_cfg[0].iq_txshift;
	}
      }
      else {
	tx_offset = (int)slot_offset+time_offset[aa];
	txdata = (int16_t*)&phy_vars_eNB->common_vars.txdata[0][aa][tx_offset];

	for (i=0; i<(len<<1); i++) {
	  txdata[i] = ((int16_t*)dummy_tx_b)[i]<<openair0_cfg[0].iq_txshift;
	}
      }
      
     // if S-subframe switch to RX in second subframe
      /*
     if (subframe_select(&phy_vars_eNB->frame_parms,subframe) == SF_S) {
       for (i=0; i<len; i++) {
	 phy_vars_eNB->common_vars.txdata[0][aa][tx_offset++] = 0x00010001;
       }
     }
      */
     if ((((phy_vars_eNB->frame_parms.tdd_config==0) ||
	   (phy_vars_eNB->frame_parms.tdd_config==1) ||
	   (phy_vars_eNB->frame_parms.tdd_config==2) ||
	   (phy_vars_eNB->frame_parms.tdd_config==6)) && 
	   (subframe==0)) || (subframe==5)) {
       // turn on tx switch N_TA_offset before
       //LOG_D(HW,"subframe %d, time to switch to tx (N_TA_offset %d, slot_offset %d) \n",subframe,phy_vars_eNB->N_TA_offset,slot_offset);
       for (i=0; i<phy_vars_eNB->N_TA_offset; i++) {
         tx_offset = (int)slot_offset+time_offset[aa]+i-phy_vars_eNB->N_TA_offset/2;
         if (tx_offset<0)
           tx_offset += LTE_NUMBER_OF_SUBFRAMES_PER_FRAME*phy_vars_eNB->frame_parms.samples_per_tti;
	 
         if (tx_offset>=(LTE_NUMBER_OF_SUBFRAMES_PER_FRAME*phy_vars_eNB->frame_parms.samples_per_tti))
           tx_offset -= LTE_NUMBER_OF_SUBFRAMES_PER_FRAME*phy_vars_eNB->frame_parms.samples_per_tti;
	 
         phy_vars_eNB->common_vars.txdata[0][aa][tx_offset] = 0x00000000;
       }
     }
    }
  }
  VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_ENB_SFGEN , 0 );
}

void tx_fh_if5(PHY_VARS_eNB *eNB,eNB_rxtx_proc_t *proc) {
  VCD_SIGNAL_DUMPER_DUMP_VARIABLE_BY_NAME( VCD_SIGNAL_DUMPER_VARIABLES_TRX_TST, proc->timestamp_tx&0xffffffff );
  send_IF5(eNB, proc->timestamp_tx, proc->subframe_tx, &seqno, IF5_RRH_GW_DL);
}

void tx_fh_if5_mobipass(PHY_VARS_eNB *eNB,eNB_rxtx_proc_t *proc) {
  VCD_SIGNAL_DUMPER_DUMP_VARIABLE_BY_NAME( VCD_SIGNAL_DUMPER_VARIABLES_TRX_TST, proc->timestamp_tx&0xffffffff );
  send_IF5(eNB, proc->timestamp_tx, proc->subframe_tx, &seqno, IF5_MOBIPASS); 
}

void tx_fh_if4p5(PHY_VARS_eNB *eNB,eNB_rxtx_proc_t *proc) {    
  send_IF4p5(eNB,proc->frame_tx, proc->subframe_tx, IF4p5_PDLFFT, 0);
}

void proc_tx_high0(PHY_VARS_eNB *eNB,
		   eNB_rxtx_proc_t *proc,
		   relaying_type_t r_type,
		   PHY_VARS_RN *rn) {

  int offset = proc == &eNB->proc.proc_rxtx[0] ? 0 : 1;

  VCD_SIGNAL_DUMPER_DUMP_VARIABLE_BY_NAME( VCD_SIGNAL_DUMPER_VARIABLES_FRAME_NUMBER_TX0_ENB+offset, proc->frame_tx );
  VCD_SIGNAL_DUMPER_DUMP_VARIABLE_BY_NAME( VCD_SIGNAL_DUMPER_VARIABLES_SUBFRAME_NUMBER_TX0_ENB+offset, proc->subframe_tx );

  phy_procedures_eNB_TX(eNB,proc,r_type,rn,1);

  /* we're done, let the next one proceed */
  if (pthread_mutex_lock(&sync_phy_proc.mutex_phy_proc_tx) != 0) {
    LOG_E(PHY, "[SCHED][eNB] error locking PHY proc mutex for eNB TX proc\n");
    exit_fun("nothing to add");
  }	
  sync_phy_proc.phy_proc_CC_id++;
  sync_phy_proc.phy_proc_CC_id %= MAX_NUM_CCs;
  pthread_cond_broadcast(&sync_phy_proc.cond_phy_proc_tx);
  if (pthread_mutex_unlock(&sync_phy_proc.mutex_phy_proc_tx) != 0) {
    LOG_E(PHY, "[SCHED][eNB] error unlocking PHY proc mutex for eNB TX proc\n");
    exit_fun("nothing to add");
  }

}

void proc_tx_high(PHY_VARS_eNB *eNB,
		  eNB_rxtx_proc_t *proc,
		  relaying_type_t r_type,
		  PHY_VARS_RN *rn) {


  // do PHY high
  proc_tx_high0(eNB,proc,r_type,rn);

  // if TX fronthaul go ahead 
  if (eNB->tx_fh) eNB->tx_fh(eNB,proc);

}

void proc_tx_full(PHY_VARS_eNB *eNB,
		  eNB_rxtx_proc_t *proc,
		  relaying_type_t r_type,
		  PHY_VARS_RN *rn) {


  // do PHY high
  proc_tx_high0(eNB,proc,r_type,rn);
  
  pthread_cond_signal(&eNB->thread_ofdm.cond_tx);
  // do OFDM modulation
  //do_OFDM_mod_rt(proc->subframe_tx,eNB);
  // if TX fronthaul go ahead 
  //if (eNB->tx_fh) eNB->tx_fh(eNB,proc);



}

void proc_tx_rru_if4p5(PHY_VARS_eNB *eNB,
		       eNB_rxtx_proc_t *proc,
		       relaying_type_t r_type,
		       PHY_VARS_RN *rn) {

  uint32_t symbol_number=0;
  uint32_t symbol_mask, symbol_mask_full;
  uint16_t packet_type;

  int offset = proc == &eNB->proc.proc_rxtx[0] ? 0 : 1;

  VCD_SIGNAL_DUMPER_DUMP_VARIABLE_BY_NAME( VCD_SIGNAL_DUMPER_VARIABLES_FRAME_NUMBER_TX0_ENB+offset, proc->frame_tx );
  VCD_SIGNAL_DUMPER_DUMP_VARIABLE_BY_NAME( VCD_SIGNAL_DUMPER_VARIABLES_SUBFRAME_NUMBER_TX0_ENB+offset, proc->subframe_tx );

  /// **** recv_IF4 of txdataF from RCC **** ///             
  symbol_number = 0;
  symbol_mask = 0;
  symbol_mask_full = (1<<eNB->frame_parms.symbols_per_tti)-1;
  

  do { 
    recv_IF4p5(eNB, &proc->frame_tx, &proc->subframe_tx, &packet_type, &symbol_number);
    symbol_mask = symbol_mask | (1<<symbol_number);
  } while (symbol_mask != symbol_mask_full); 

  do_OFDM_mod_rt(proc->subframe_tx, eNB);
}

void proc_tx_rru_if5(PHY_VARS_eNB *eNB,eNB_rxtx_proc_t *proc) {
  int offset = proc == &eNB->proc.proc_rxtx[0] ? 0 : 1;

  VCD_SIGNAL_DUMPER_DUMP_VARIABLE_BY_NAME( VCD_SIGNAL_DUMPER_VARIABLES_FRAME_NUMBER_TX0_ENB+offset, proc->frame_tx );
  VCD_SIGNAL_DUMPER_DUMP_VARIABLE_BY_NAME( VCD_SIGNAL_DUMPER_VARIABLES_SUBFRAME_NUMBER_TX0_ENB+offset, proc->subframe_tx );
  /// **** recv_IF5 of txdata from BBU **** ///       
  recv_IF5(eNB, &proc->timestamp_tx, proc->subframe_tx, IF5_RRH_GW_DL);
}

int wait_CCs(eNB_rxtx_proc_t *proc) {

  struct timespec wait;

  wait.tv_sec=0;
  wait.tv_nsec=5000000L;

  if (pthread_mutex_timedlock(&sync_phy_proc.mutex_phy_proc_tx,&wait) != 0) {
    LOG_E(PHY, "[SCHED][eNB] error locking PHY proc mutex for eNB TX\n");
    exit_fun("nothing to add");
    return(-1);
  }
  
  // wait for our turn or oai_exit
  while (sync_phy_proc.phy_proc_CC_id != proc->CC_id && !oai_exit) {
    pthread_cond_wait(&sync_phy_proc.cond_phy_proc_tx,
		      &sync_phy_proc.mutex_phy_proc_tx);
  }
  
  if (pthread_mutex_unlock(&sync_phy_proc.mutex_phy_proc_tx) != 0) {
    LOG_E(PHY, "[SCHED][eNB] error unlocking PHY proc mutex for eNB TX\n");
    exit_fun("nothing to add");
    return(-1);
  }
  return(0);
}

static inline int rxtx(PHY_VARS_eNB *eNB,eNB_rxtx_proc_t *proc, char *thread_name) {

  start_meas(&softmodem_stats_rxtx_sf);
  // ****************************************
  // Common RX procedures subframe n
  phy_procedures_eNB_common_RX(eNB);
  
  // UE-specific RX processing for subframe n
  if (eNB->proc_uespec_rx) eNB->proc_uespec_rx(eNB, proc, no_relay );
  
  // *****************************************
  // TX processing for subframe n+4
  // run PHY TX procedures the one after the other for all CCs to avoid race conditions
  // (may be relaxed in the future for performance reasons)
  // *****************************************
  //if (wait_CCs(proc)<0) return(-1);
  
  if (oai_exit) return(-1);
  
  if (eNB->proc_tx)	eNB->proc_tx(eNB, proc, no_relay, NULL );
  
  if (release_thread(&proc->mutex_rxtx,&proc->instance_cnt_rxtx,thread_name)<0) return(-1);

  stop_meas( &softmodem_stats_rxtx_sf );
  
  return(0);
}

/*!
 * \brief The RX UE-specific and TX thread of eNB.
 * \param param is a \ref eNB_proc_t structure which contains the info what to process.
 * \returns a pointer to an int. The storage is not on the heap and must not be freed.
 */
static void* eNB_thread_rxtx( void* param ) {

  static int eNB_thread_rxtx_status;

  eNB_rxtx_proc_t *proc = (eNB_rxtx_proc_t*)param;
  PHY_VARS_eNB *eNB = PHY_vars_eNB_g[0][proc->CC_id];

  char thread_name[100];


  // set default return value
  eNB_thread_rxtx_status = 0;


  sprintf(thread_name,"RXn_TXnp4_%d\n",&eNB->proc.proc_rxtx[0] == proc ? 0 : 1);
  thread_top_init(thread_name,1,850000L,1000000L,2000000L);

  while (!oai_exit) {
    VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME( VCD_SIGNAL_DUMPER_FUNCTIONS_eNB_PROC_RXTX0+(proc->subframe_rx&1), 0 );

    if (wait_on_condition(&proc->mutex_rxtx,&proc->cond_rxtx,&proc->instance_cnt_rxtx,thread_name)<0) break;

    VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME( VCD_SIGNAL_DUMPER_FUNCTIONS_eNB_PROC_RXTX0+(proc->subframe_rx&1), 1 );

    
  
    if (oai_exit) break;

    if (rxtx(eNB,proc,thread_name) < 0) break;

  } // while !oai_exit

  VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME( VCD_SIGNAL_DUMPER_FUNCTIONS_eNB_PROC_RXTX0+(proc->subframe_rx&1), 0 );

  printf( "Exiting eNB thread RXn_TXnp4\n");

  eNB_thread_rxtx_status = 0;
  return &eNB_thread_rxtx_status;
}

#if defined(ENABLE_ITTI) && defined(ENABLE_USE_MME)
/* Wait for eNB application initialization to be complete (eNB registration to MME) */
static void wait_system_ready (char *message, volatile int *start_flag) {
  
  static char *indicator[] = {".    ", "..   ", "...  ", ".... ", ".....",
			      " ....", "  ...", "   ..", "    .", "     "};
  int i = 0;
  
  while ((!oai_exit) && (*start_flag == 0)) {
    LOG_N(EMU, message, indicator[i]);
    fflush(stdout);
    i = (i + 1) % (sizeof(indicator) / sizeof(indicator[0]));
    usleep(200000);
  }
  
  LOG_D(EMU,"\n");
}
#endif


// asynchronous UL with IF4p5 (RCC,RAU,eNodeB_BBU)
void fh_if5_asynch_UL(PHY_VARS_eNB *eNB,int *frame,int *subframe) {

  eNB_proc_t *proc       = &eNB->proc;
  LTE_DL_FRAME_PARMS *fp = &eNB->frame_parms;

  recv_IF5(eNB, &proc->timestamp_rx, *subframe, IF5_RRH_GW_UL); 

  proc->subframe_rx = (proc->timestamp_rx/fp->samples_per_tti)%10;
  proc->frame_rx    = (proc->timestamp_rx/(10*fp->samples_per_tti))&1023;

  if (proc->first_rx != 0) {
    proc->first_rx = 0;
    *subframe = proc->subframe_rx;
    *frame    = proc->frame_rx; 
  }
  else {
    if (proc->subframe_rx != *subframe) {
      LOG_E(PHY,"subframe_rx %d is not what we expect %d\n",proc->subframe_rx,*subframe);
      exit_fun("Exiting");
    }
    if (proc->frame_rx != *frame) {
      LOG_E(PHY,"subframe_rx %d is not what we expect %d\n",proc->frame_rx,*frame);  
      exit_fun("Exiting");
    }
  }
} // eNodeB_3GPP_BBU 

// asynchronous UL with IF4p5 (RCC,RAU,eNodeB_BBU)
void fh_if4p5_asynch_UL(PHY_VARS_eNB *eNB,int *frame,int *subframe) {

  LTE_DL_FRAME_PARMS *fp = &eNB->frame_parms;
  eNB_proc_t *proc       = &eNB->proc;

  uint16_t packet_type;
  uint32_t symbol_number,symbol_mask,symbol_mask_full,prach_rx;


  symbol_number = 0;
  symbol_mask = 0;
  symbol_mask_full = (1<<fp->symbols_per_tti)-1;
  prach_rx = 0;

  do {   // Blocking, we need a timeout on this !!!!!!!!!!!!!!!!!!!!!!!
    recv_IF4p5(eNB, &proc->frame_rx, &proc->subframe_rx, &packet_type, &symbol_number);
    if (proc->first_rx != 0) {
      *frame = proc->frame_rx;
      *subframe = proc->subframe_rx;
      proc->first_rx = 0;
    }
    else {
      if (proc->frame_rx != *frame) {
	LOG_E(PHY,"frame_rx %d is not what we expect %d\n",proc->frame_rx,*frame);
	exit_fun("Exiting");
      }
      if (proc->subframe_rx != *subframe) {
	LOG_E(PHY,"subframe_rx %d is not what we expect %d\n",proc->subframe_rx,*subframe);
	exit_fun("Exiting");
      }
    }
    if (packet_type == IF4p5_PULFFT) {
      symbol_mask = symbol_mask | (1<<symbol_number);
      prach_rx = (is_prach_subframe(fp, proc->frame_rx, proc->subframe_rx)>0) ? 1 : 0;                            
    } else if (packet_type == IF4p5_PRACH) {
      prach_rx = 0;
    }
  } while( (symbol_mask != symbol_mask_full) || (prach_rx == 1));    
  

} 


void fh_if5_asynch_DL(PHY_VARS_eNB *eNB,int *frame,int *subframe) {

  LTE_DL_FRAME_PARMS *fp = &eNB->frame_parms;
  eNB_proc_t *proc       = &eNB->proc;
  int subframe_tx,frame_tx;
  openair0_timestamp timestamp_tx;

  recv_IF5(eNB, &timestamp_tx, *subframe, IF5_RRH_GW_DL); 
      //      printf("Received subframe %d (TS %llu) from RCC\n",subframe_tx,timestamp_tx);

  subframe_tx = (timestamp_tx/fp->samples_per_tti)%10;
  frame_tx    = (timestamp_tx/(fp->samples_per_tti*10))&1023;

  if (proc->first_tx != 0) {
    *subframe = subframe_tx;
    *frame    = frame_tx;
    proc->first_tx = 0;
  }
  else {
    if (subframe_tx != *subframe) {
      LOG_E(PHY,"subframe_tx %d is not what we expect %d\n",subframe_tx,*subframe);
      exit_fun("Exiting");
    }
    if (frame_tx != *frame) { 
      LOG_E(PHY,"frame_tx %d is not what we expect %d\n",frame_tx,*frame);
      exit_fun("Exiting");
    }
  }
}

void fh_if4p5_asynch_DL(PHY_VARS_eNB *eNB,int *frame,int *subframe) {

  LTE_DL_FRAME_PARMS *fp = &eNB->frame_parms;
  eNB_proc_t *proc       = &eNB->proc;

  uint16_t packet_type;
  uint32_t symbol_number,symbol_mask,symbol_mask_full;
  int subframe_tx,frame_tx;

  symbol_number = 0;
  symbol_mask = 0;
  symbol_mask_full = (1<<fp->symbols_per_tti)-1;

  do {   // Blocking, we need a timeout on this !!!!!!!!!!!!!!!!!!!!!!!
    recv_IF4p5(eNB, &frame_tx, &subframe_tx, &packet_type, &symbol_number);
    if (proc->first_tx != 0) {
      *frame    = frame_tx;
      *subframe = subframe_tx;
      proc->first_tx = 0;
    }
    else {
      if (frame_tx != *frame) {
	LOG_E(PHY,"frame_tx %d is not what we expect %d\n",frame_tx,*frame);
	exit_fun("Exiting");
      }
      if (subframe_tx != *subframe) {
	LOG_E(PHY,"subframe_tx %d is not what we expect %d\n",subframe_tx,*subframe);
	exit_fun("Exiting");
      }
    }
    if (packet_type == IF4p5_PDLFFT) {
      symbol_mask = symbol_mask | (1<<symbol_number);
    }
    else {
      LOG_E(PHY,"Illegal IF4p5 packet type (should only be IF4p5_PDLFFT%d\n",packet_type);
      exit_fun("Exiting");
    }
  } while (symbol_mask != symbol_mask_full);    
  
  do_OFDM_mod_rt(subframe_tx, eNB);
} 

/*!
 * \brief The Asynchronous RX/TX FH thread of RAU/RCC/eNB/RRU.
 * This handles the RX FH for an asynchronous RRU/UE
 * \param param is a \ref eNB_proc_t structure which contains the info what to process.
 * \returns a pointer to an int. The storage is not on the heap and must not be freed.
 */
static void* eNB_thread_asynch_rxtx( void* param ) {

  static int eNB_thread_asynch_rxtx_status;

  eNB_proc_t *proc = (eNB_proc_t*)param;
  PHY_VARS_eNB *eNB = PHY_vars_eNB_g[0][proc->CC_id];



  int subframe=0, frame=0; 

  thread_top_init("thread_asynch",1,870000L,1000000L,1000000L);

  // wait for top-level synchronization and do one acquisition to get timestamp for setting frame/subframe

  wait_sync("thread_asynch");

  // wait for top-level synchronization and do one acquisition to get timestamp for setting frame/subframe
  printf( "waiting for devices (eNB_thread_asynch_rx)\n");

  wait_on_condition(&proc->mutex_asynch_rxtx,&proc->cond_asynch_rxtx,&proc->instance_cnt_asynch_rxtx,"thread_asynch");

  printf( "devices ok (eNB_thread_asynch_rx)\n");


  while (!oai_exit) { 
   
    if (oai_exit) break;   

    if (subframe==9) { 
      subframe=0;
      frame++;
      frame&=1023;
    } else {
      subframe++;
    }      

    if (eNB->fh_asynch) eNB->fh_asynch(eNB,&frame,&subframe);
    else AssertFatal(1==0, "Unknown eNB->node_function %d",eNB->node_function);
    
  }

  eNB_thread_asynch_rxtx_status=0;
  return(&eNB_thread_asynch_rxtx_status);
}





void rx_rf(PHY_VARS_eNB *eNB,int *frame,int *subframe) {

  eNB_proc_t *proc = &eNB->proc;
  LTE_DL_FRAME_PARMS *fp = &eNB->frame_parms;
  void *rxp[fp->nb_antennas_rx],*txp[fp->nb_antennas_tx]; 
  unsigned int rxs,txs;
  int i;
  int tx_sfoffset = 3;//(eNB->single_thread_flag == 1) ? 3 : 3;
  if (proc->first_rx==0) {
    
    // Transmit TX buffer based on timestamp from RX
    //    printf("trx_write -> USRP TS %llu (sf %d)\n", (proc->timestamp_rx+(3*fp->samples_per_tti)),(proc->subframe_rx+2)%10);
    VCD_SIGNAL_DUMPER_DUMP_VARIABLE_BY_NAME( VCD_SIGNAL_DUMPER_VARIABLES_TRX_TST, (proc->timestamp_rx+(tx_sfoffset*fp->samples_per_tti)-openair0_cfg[0].tx_sample_advance)&0xffffffff );
    VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME( VCD_SIGNAL_DUMPER_FUNCTIONS_TRX_WRITE, 1 );
    // prepare tx buffer pointers
	
    for (i=0; i<fp->nb_antennas_tx; i++)
      txp[i] = (void*)&eNB->common_vars.txdata[0][i][((proc->subframe_rx+tx_sfoffset)%10)*fp->samples_per_tti];
    
    txs = eNB->rfdevice.trx_write_func(&eNB->rfdevice,
				       proc->timestamp_rx+(tx_sfoffset*fp->samples_per_tti)-openair0_cfg[0].tx_sample_advance,
				       txp,
				       fp->samples_per_tti,
				       fp->nb_antennas_tx,
				       1);
    
    VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME( VCD_SIGNAL_DUMPER_FUNCTIONS_TRX_WRITE, 0 );
    
    
    
    if (txs !=  fp->samples_per_tti) {
      LOG_E(PHY,"TX : Timeout (sent %d/%d)\n",txs, fp->samples_per_tti);
      exit_fun( "problem transmitting samples" );
    }	
  }
  
  for (i=0; i<fp->nb_antennas_rx; i++)
    rxp[i] = (void*)&eNB->common_vars.rxdata[0][i][*subframe*fp->samples_per_tti];
  
  VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME( VCD_SIGNAL_DUMPER_FUNCTIONS_TRX_READ, 1 );

  rxs = eNB->rfdevice.trx_read_func(&eNB->rfdevice,
				    &(proc->timestamp_rx),
				    rxp,
				    fp->samples_per_tti,
				    fp->nb_antennas_rx);

  VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME( VCD_SIGNAL_DUMPER_FUNCTIONS_TRX_READ, 0 );
  
  proc->frame_rx    = (proc->timestamp_rx / (fp->samples_per_tti*10))&1023;
  proc->subframe_rx = (proc->timestamp_rx / fp->samples_per_tti)%10;
  proc->timestamp_tx = proc->timestamp_rx+(4*fp->samples_per_tti);
  //  printf("trx_read <- USRP TS %llu (sf %d, first_rx %d)\n", proc->timestamp_rx,proc->subframe_rx,proc->first_rx);  
  
  if (proc->first_rx == 0) {
    if (proc->subframe_rx != *subframe){
      LOG_E(PHY,"Received Timestamp doesn't correspond to the time we think it is (proc->subframe_rx %d, subframe %d)\n",proc->subframe_rx,*subframe);
      exit_fun("Exiting");
    }
    
    if (proc->frame_rx != *frame) {
      LOG_E(PHY,"Received Timestamp doesn't correspond to the time we think it is (proc->frame_rx %d frame %d)\n",proc->frame_rx,*frame);
      exit_fun("Exiting");
    }
  } else {
    proc->first_rx = 0;
    *frame = proc->frame_rx;
    *subframe = proc->subframe_rx;        
  }
  
  //printf("timestamp_rx %lu, frame %d(%d), subframe %d(%d)\n",proc->timestamp_rx,proc->frame_rx,frame,proc->subframe_rx,subframe);
  
  VCD_SIGNAL_DUMPER_DUMP_VARIABLE_BY_NAME( VCD_SIGNAL_DUMPER_VARIABLES_TRX_TS, proc->timestamp_rx&0xffffffff );
  
  if (rxs != fp->samples_per_tti)
    exit_fun( "problem receiving samples" );
  

  
}

void rx_fh_if5(PHY_VARS_eNB *eNB,int *frame, int *subframe) {

  LTE_DL_FRAME_PARMS *fp = &eNB->frame_parms;
  eNB_proc_t *proc = &eNB->proc;

  recv_IF5(eNB, &proc->timestamp_rx, *subframe, IF5_RRH_GW_UL); 

  proc->frame_rx    = (proc->timestamp_rx / (fp->samples_per_tti*10))&1023;
  proc->subframe_rx = (proc->timestamp_rx / fp->samples_per_tti)%10;
  
  if (proc->first_rx == 0) {
    if (proc->subframe_rx != *subframe){
      LOG_E(PHY,"Received Timestamp doesn't correspond to the time we think it is (proc->subframe_rx %d, subframe %d)\n",proc->subframe_rx,subframe);
      exit_fun("Exiting");
    }
    
    if (proc->frame_rx != *frame) {
      LOG_E(PHY,"Received Timestamp doesn't correspond to the time we think it is (proc->frame_rx %d frame %d)\n",proc->frame_rx,frame);
      exit_fun("Exiting");
    }
  } else {
    proc->first_rx = 0;
    *frame = proc->frame_rx;
    *subframe = proc->subframe_rx;        
  }      
  
  VCD_SIGNAL_DUMPER_DUMP_VARIABLE_BY_NAME( VCD_SIGNAL_DUMPER_VARIABLES_TRX_TS, proc->timestamp_rx&0xffffffff );

}


void rx_fh_if4p5(PHY_VARS_eNB *eNB,int *frame,int *subframe) {

  LTE_DL_FRAME_PARMS *fp = &eNB->frame_parms;
  eNB_proc_t *proc = &eNB->proc;

  int prach_rx;

  uint16_t packet_type;
  uint32_t symbol_number=0;
  uint32_t symbol_mask, symbol_mask_full;

  symbol_mask = 0;
  symbol_mask_full = (1<<fp->symbols_per_tti)-1;
  prach_rx = 0;

  do {   // Blocking, we need a timeout on this !!!!!!!!!!!!!!!!!!!!!!!
    recv_IF4p5(eNB, &proc->frame_rx, &proc->subframe_rx, &packet_type, &symbol_number);

    if (packet_type == IF4p5_PULFFT) {
      symbol_mask = symbol_mask | (1<<symbol_number);
      prach_rx = (is_prach_subframe(fp, proc->frame_rx, proc->subframe_rx)>0) ? 1 : 0;                            
    } else if (packet_type == IF4p5_PRACH) {
      prach_rx = 0;
    }

  } while( (symbol_mask != symbol_mask_full) || (prach_rx == 1));    

  //caculate timestamp_rx, timestamp_tx based on frame and subframe
   proc->timestamp_rx = ((proc->frame_rx * 10)  + proc->subframe_rx ) * fp->samples_per_tti ;
   proc->timestamp_tx = proc->timestamp_rx +  (4*fp->samples_per_tti);
 
 
  if (proc->first_rx == 0) {
    if (proc->subframe_rx != *subframe){
      LOG_E(PHY,"Received Timestamp doesn't correspond to the time we think it is (proc->subframe_rx %d, subframe %d)\n",proc->subframe_rx,*subframe);
      exit_fun("Exiting");
    }
    if (proc->frame_rx != *frame) {
      LOG_E(PHY,"Received Timestamp doesn't correspond to the time we think it is (proc->frame_rx %d frame %d)\n",proc->frame_rx,*frame);
      exit_fun("Exiting");
    }
  } else {
    proc->first_rx = 0;
    *frame = proc->frame_rx;
    *subframe = proc->subframe_rx;        
  }
  
  VCD_SIGNAL_DUMPER_DUMP_VARIABLE_BY_NAME( VCD_SIGNAL_DUMPER_VARIABLES_TRX_TS, proc->timestamp_rx&0xffffffff );
  
}

void rx_fh_slave(PHY_VARS_eNB *eNB,int *frame,int *subframe) {
  // This case is for synchronization to another thread
  // it just waits for an external event.  The actual rx_fh is handle by the asynchronous RX thread
  eNB_proc_t *proc=&eNB->proc;

  if (wait_on_condition(&proc->mutex_FH,&proc->cond_FH,&proc->instance_cnt_FH,"rx_fh_slave") < 0)
    return;

  release_thread(&proc->mutex_FH,&proc->instance_cnt_FH,"rx_fh_slave");

  
}


int wakeup_rxtx(eNB_proc_t *proc,eNB_rxtx_proc_t *proc_rxtx,LTE_DL_FRAME_PARMS *fp) {

  int i;
  struct timespec wait;
  
  wait.tv_sec=0;
  wait.tv_nsec=5000000L;

  /* accept some delay in processing - up to 5ms */
  for (i = 0; i < 10 && proc_rxtx->instance_cnt_rxtx == 0; i++) {
    LOG_W( PHY,"[eNB] Frame %d, eNB RXn-TXnp4 thread busy!! (cnt_rxtx %i)\n", proc_rxtx->frame_tx, proc_rxtx->instance_cnt_rxtx);
    usleep(500);
  }
  if (proc_rxtx->instance_cnt_rxtx == 0) {
    exit_fun( "TX thread busy" );
    return(-1);
  }

  // wake up TX for subframe n+4
  // lock the TX mutex and make sure the thread is ready
  if (pthread_mutex_timedlock(&proc_rxtx->mutex_rxtx,&wait) != 0) {
    LOG_E( PHY, "[eNB] ERROR pthread_mutex_lock for eNB RXTX thread %d (IC %d)\n", proc_rxtx->subframe_rx&1,proc_rxtx->instance_cnt_rxtx );
    exit_fun( "error locking mutex_rxtx" );
    return(-1);
  }
  
  ++proc_rxtx->instance_cnt_rxtx;
  
  // We have just received and processed the common part of a subframe, say n. 
  // TS_rx is the last received timestamp (start of 1st slot), TS_tx is the desired 
  // transmitted timestamp of the next TX slot (first).
  // The last (TS_rx mod samples_per_frame) was n*samples_per_tti, 
  // we want to generate subframe (n+4), so TS_tx = TX_rx+4*samples_per_tti,
  // and proc->subframe_tx = proc->subframe_rx+4
  proc_rxtx->timestamp_tx = proc->timestamp_rx + (4*fp->samples_per_tti);
  proc_rxtx->frame_rx     = proc->frame_rx;
  proc_rxtx->subframe_rx  = proc->subframe_rx;
  proc_rxtx->frame_tx     = (proc_rxtx->subframe_rx > 5) ? (proc_rxtx->frame_rx+1)&1023 : proc_rxtx->frame_rx;
  proc_rxtx->subframe_tx  = (proc_rxtx->subframe_rx + 4)%10;
  
  // the thread can now be woken up
  if (pthread_cond_signal(&proc_rxtx->cond_rxtx) != 0) {
    LOG_E( PHY, "[eNB] ERROR pthread_cond_signal for eNB RXn-TXnp4 thread\n");
    exit_fun( "ERROR pthread_cond_signal" );
    return(-1);
  }
  
  pthread_mutex_unlock( &proc_rxtx->mutex_rxtx );

  return(0);
}

void wakeup_slaves(eNB_proc_t *proc) {

  int i;
  struct timespec wait;
  
  wait.tv_sec=0;
  wait.tv_nsec=5000000L;
  
  for (i=0;i<proc->num_slaves;i++) {
    eNB_proc_t *slave_proc = proc->slave_proc[i];
    // wake up slave FH thread
    // lock the FH mutex and make sure the thread is ready
    if (pthread_mutex_timedlock(&slave_proc->mutex_FH,&wait) != 0) {
      LOG_E( PHY, "[eNB] ERROR pthread_mutex_lock for eNB CCid %d slave CCid %d (IC %d)\n",proc->CC_id,slave_proc->CC_id);
      exit_fun( "error locking mutex_rxtx" );
      break;
    }
    
    int cnt_slave            = ++slave_proc->instance_cnt_FH;
    slave_proc->frame_rx     = proc->frame_rx;
    slave_proc->subframe_rx  = proc->subframe_rx;
    slave_proc->timestamp_rx = proc->timestamp_rx;
    slave_proc->timestamp_tx = proc->timestamp_tx; 

    pthread_mutex_unlock( &slave_proc->mutex_FH );
    
    if (cnt_slave == 0) {
      // the thread was presumably waiting where it should and can now be woken up
      if (pthread_cond_signal(&slave_proc->cond_FH) != 0) {
	LOG_E( PHY, "[eNB] ERROR pthread_cond_signal for eNB CCid %d, slave CCid %d\n",proc->CC_id,slave_proc->CC_id);
          exit_fun( "ERROR pthread_cond_signal" );
	  break;
      }
    } else {
      LOG_W( PHY,"[eNB] Frame %d, slave CC_id %d thread busy!! (cnt_FH %i)\n",slave_proc->frame_rx,slave_proc->CC_id, cnt_slave);
      exit_fun( "FH thread busy" );
      break;
    }             
  }
}

/*!
 * \brief The Fronthaul thread of RRU/RAU/RCC/eNB
 * In the case of RRU/eNB, handles interface with external RF
 * In the case of RAU/RCC, handles fronthaul interface with RRU/RAU
 * \param param is a \ref eNB_proc_t structure which contains the info what to process.
 * \returns a pointer to an int. The storage is not on the heap and must not be freed.
 */

static void* eNB_thread_FH( void* param ) {
  
  static int eNB_thread_FH_status;

  eNB_proc_t *proc = (eNB_proc_t*)param;
  PHY_VARS_eNB *eNB = PHY_vars_eNB_g[0][proc->CC_id];
  LTE_DL_FRAME_PARMS *fp = &eNB->frame_parms;

  int subframe=0, frame=0; 

  // set default return value
  eNB_thread_FH_status = 0;

  thread_top_init("eNB_thread_FH",0,870000,1000000,1000000);

  wait_sync("eNB_thread_FH");

#if defined(ENABLE_ITTI) && defined(ENABLE_USE_MME)
  if (eNB->node_function < NGFI_RRU_IF5)
    wait_system_ready ("Waiting for eNB application to be ready %s\r", &start_eNB);
#endif 

  // Start IF device if any
  if (eNB->start_if) 
    if (eNB->start_if(eNB) != 0)
      LOG_E(HW,"Could not start the IF device\n");

  // Start RF device if any
  if (eNB->start_rf)
    if (eNB->start_rf(eNB) != 0)
      LOG_E(HW,"Could not start the RF device\n");

  // wakeup asnych_rxtx thread because the devices are ready at this point
  pthread_mutex_lock(&proc->mutex_asynch_rxtx);
  proc->instance_cnt_asynch_rxtx=0;
  pthread_mutex_unlock(&proc->mutex_asynch_rxtx);
  pthread_cond_signal(&proc->cond_asynch_rxtx);

  // This is a forever while loop, it loops over subframes which are scheduled by incoming samples from HW devices
  while (!oai_exit) {

    // these are local subframe/frame counters to check that we are in synch with the fronthaul timing.
    // They are set on the first rx/tx in the underly FH routines.
    if (subframe==9) { 
      subframe=0;
      frame++;
      frame&=1023;
    } else {
      subframe++;
    }      

 
    // synchronization on FH interface, acquire signals/data and block
    if (eNB->rx_fh) eNB->rx_fh(eNB,&frame,&subframe);
    else AssertFatal(1==0, "No fronthaul interface : eNB->node_function %d",eNB->node_function);

    T(T_ENB_MASTER_TICK, T_INT(0), T_INT(proc->frame_rx), T_INT(proc->subframe_rx));

    // At this point, all information for subframe has been received on FH interface
    // If this proc is to provide synchronization, do so
    wakeup_slaves(proc);
      
    // wake up RXn_TXnp4 thread for the subframe
    // choose even or odd thread for RXn-TXnp4 processing 
    if (wakeup_rxtx(proc,&proc->proc_rxtx[proc->subframe_rx&1],fp) < 0)
      break;

    // artifical sleep for very slow fronthaul
    if (eNB->frame_parms.N_RB_DL==6)
      rt_sleep_ns(800000LL);
  }
    
  printf( "Exiting FH thread \n");
 
  eNB_thread_FH_status = 0;
  return &eNB_thread_FH_status;
}


/*!
 * \brief The prach receive thread of eNB.
 * \param param is a \ref eNB_proc_t structure which contains the info what to process.
 * \returns a pointer to an int. The storage is not on the heap and must not be freed.
 */
static void* eNB_thread_prach( void* param ) {
  static int eNB_thread_prach_status;

  eNB_proc_t *proc = (eNB_proc_t*)param;
  PHY_VARS_eNB *eNB= PHY_vars_eNB_g[0][proc->CC_id];

  // set default return value
  eNB_thread_prach_status = 0;

  thread_top_init("eNB_thread_prach",1,500000L,1000000L,20000000L);

  while (!oai_exit) {
    
    if (oai_exit) break;

    if (wait_on_condition(&proc->mutex_prach,&proc->cond_prach,&proc->instance_cnt_prach,"eNB_prach_thread") < 0) break;
    
    prach_procedures(eNB);
    
    if (release_thread(&proc->mutex_prach,&proc->instance_cnt_prach,"eNB_prach_thread") < 0) break;
  }

  printf( "Exiting eNB thread PRACH\n");

  eNB_thread_prach_status = 0;
  return &eNB_thread_prach_status;
}

static void* eNB_thread_single( void* param ) {

  static int eNB_thread_single_status;

  eNB_proc_t *proc = (eNB_proc_t*)param;
  eNB_rxtx_proc_t *proc_rxtx = &proc->proc_rxtx[0];
  PHY_VARS_eNB *eNB = PHY_vars_eNB_g[0][proc->CC_id];

  int subframe=0, frame=0; 

  // set default return value
  eNB_thread_single_status = 0;

  thread_top_init("eNB_thread_single",0,870000,1000000,1000000);

  wait_sync("eNB_thread_single");

#if defined(ENABLE_ITTI) && defined(ENABLE_USE_MME)
  if (eNB->node_function < NGFI_RRU_IF5)
    wait_system_ready ("Waiting for eNB application to be ready %s\r", &start_eNB);
#endif 

  // Start IF device if any
  if (eNB->start_if) 
    if (eNB->start_if(eNB) != 0)
      LOG_E(HW,"Could not start the IF device\n");

  // Start RF device if any
  if (eNB->start_rf)
    if (eNB->start_rf(eNB) != 0)
      LOG_E(HW,"Could not start the RF device\n");

  // wakeup asnych_rxtx thread because the devices are ready at this point
  pthread_mutex_lock(&proc->mutex_asynch_rxtx);
  proc->instance_cnt_asynch_rxtx=0;
  pthread_mutex_unlock(&proc->mutex_asynch_rxtx);
  pthread_cond_signal(&proc->cond_asynch_rxtx);

  // This is a forever while loop, it loops over subframes which are scheduled by incoming samples from HW devices
  while (!oai_exit) {

    // these are local subframe/frame counters to check that we are in synch with the fronthaul timing.
    // They are set on the first rx/tx in the underly FH routines.
    if (subframe==9) { 
      subframe=0;
      frame++;
      frame&=1023;
    } else {
      subframe++;
    }      

    LOG_D(PHY,"eNB Fronthaul thread, frame %d, subframe %d\n",frame,subframe);
 
    // synchronization on FH interface, acquire signals/data and block
    if (eNB->rx_fh) eNB->rx_fh(eNB,&frame,&subframe);
    else AssertFatal(1==0, "No fronthaul interface : eNB->node_function %d",eNB->node_function);

    T(T_ENB_MASTER_TICK, T_INT(0), T_INT(proc->frame_rx), T_INT(proc->subframe_rx));

    proc_rxtx->subframe_rx = proc->subframe_rx;
    proc_rxtx->frame_rx    = proc->frame_rx;
    proc_rxtx->subframe_tx = (proc->subframe_rx+4)%10;
    proc_rxtx->frame_tx    = (proc->subframe_rx < 6) ? proc->frame_rx : (proc->frame_rx+1)&1023; 
    proc_rxtx->timestamp_tx = proc->timestamp_tx;

    // At this point, all information for subframe has been received on FH interface
    // If this proc is to provide synchronization, do so
    wakeup_slaves(proc);

    if (rxtx(eNB,proc_rxtx,"eNB_thread_single") < 0) break;
  }
  

  printf( "Exiting eNB_single thread \n");
 
  eNB_thread_single_status = 0;
  return &eNB_thread_single_status;

}

extern void init_fep_thread(PHY_VARS_eNB *, pthread_attr_t *);
extern void pdsch_procedures(PHY_VARS_eNB *,eNB_rxtx_proc_t *,LTE_eNB_DLSCH_t *, LTE_eNB_DLSCH_t *,LTE_eNB_UE_stats *,int ,int );
extern void init_td_thread(PHY_VARS_eNB *, pthread_attr_t *);
extern void init_te_thread(PHY_VARS_eNB *, pthread_attr_t *);
extern void pmch_procedures(PHY_VARS_eNB *,eNB_rxtx_proc_t *,PHY_VARS_RN *,relaying_type_t );
extern void common_signal_procedures (PHY_VARS_eNB *,eNB_rxtx_proc_t *);
extern void generate_eNB_dlsch_params(PHY_VARS_eNB *,eNB_rxtx_proc_t *,DCI_ALLOC_t *,const int );
extern void generate_eNB_ulsch_params(PHY_VARS_eNB *,eNB_rxtx_proc_t *,DCI_ALLOC_t *,const int );

//----------------Part of TX procedure----------------
static void *mac2phy_proc(void *ptr){
	PHY_VARS_eNB *eNB = PHY_vars_eNB_g[0][0];
	//clock_gettime(CLOCK_REALTIME, &eNB->tt16);
	//printf("mac2phy_proc consumes %ld nanoseconds!\n",eNB->tt16.tv_nsec-eNB->tt12.tv_nsec);
	static int mac2phy_proc_status;
	mac2phy_proc_status=0;
	//PHY_VARS_eNB *eNB = PHY_vars_eNB_g[0][0];
	eNB_rxtx_proc_t *proc;
    uint32_t i,aa;
    uint8_t harq_pid;
    DCI_PDU *DCI_pdu;
    DCI_PDU DCI_pdu_tmp;
    int8_t UE_id=0;
    uint8_t num_pdcch_symbols=0;
    uint8_t ul_subframe;
    uint32_t ul_frame;
    LTE_DL_FRAME_PARMS *fp;
    DCI_ALLOC_t *dci_alloc=(DCI_ALLOC_t *)NULL;
	/**********************************************************************/
	cpu_set_t cpuset; 
	//the CPU we whant to use
	int cpu = 1;
	int s;
	CPU_ZERO(&cpuset);       //clears the cpuset
	CPU_SET( cpu , &cpuset); //set CPU 0~7 on cpuset
	s = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
	if (s != 0)
	{
		perror( "pthread_setaffinity_np");
		exit_fun("Error setting processor affinity");
	}
	s = pthread_getaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
	if (s != 0) {
		perror( "pthread_getaffinity_np");
		exit_fun("Error getting processor affinity ");
	}
	/**********************************************************************/
	printf("[SCHED][eNB] MAC to PHY thread started on CPU %d TID %ld\n",sched_getcpu(),gettid());
	while(!oai_exit){
		///////////////////
		while(pthread_cond_wait(&eNB->thread_m2p.cond_tx, &eNB->thread_m2p.mutex_tx)!=0);
		proc = &eNB->proc.proc_rxtx[0];
		fp=&eNB->frame_parms;
		int frame = proc->frame_tx;
		int subframe = proc->subframe_tx;
		//////////////////
		//VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_ENB_MACTOPHY_THREAD_TX,1);
		//------//
		//VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_ENB_FAIL_TIME_TX,1);
		/*for (i=0; i<NUMBER_OF_UE_MAX; i++) {
			// If we've dropped the UE, go back to PRACH mode for this UE
			if ((frame==0)&&(subframe==0)) {
			  if (eNB->UE_stats[i].crnti > 0) {
				LOG_I(PHY,"UE %d : rnti %x\n",i,eNB->UE_stats[i].crnti);
			  }
			}
			if (eNB->UE_stats[i].ulsch_consecutive_errors == ULSCH_max_consecutive_errors) {
			  LOG_W(PHY,"[eNB %d, CC %d] frame %d, subframe %d, UE %d: ULSCH consecutive error count reached %u, triggering UL Failure\n",
					eNB->Mod_id,eNB->CC_id,frame,subframe, i, eNB->UE_stats[i].ulsch_consecutive_errors);
			  eNB->UE_stats[i].ulsch_consecutive_errors=0;
			  mac_xface->UL_failure_indication(eNB->Mod_id,
											   eNB->CC_id,
											   frame,
											   eNB->UE_stats[i].crnti,
											   subframe);			       
			}
		}
		//VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_ENB_FAIL_TIME_TX,0);

		//VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_ENB_MACTOPHY_TX,1);
		if (eNB->mac_enabled==1) {
			if (eNB->CC_id == 0) {
			  mac_xface->eNB_dlsch_ulsch_scheduler(eNB->Mod_id,0,frame,subframe);//,1);
			}
		}
		//VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_ENB_MACTOPHY_TX,0);

		//printf("proc subframe = %d,correct subframe = %d\n",eNB->proc.proc_rxtx[0].subframe_tx,test->subframe);
		//VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_ENB_RESET_TX,1);
		if (eNB->abstraction_flag==0) {
			for (aa=0; aa<fp->nb_antennas_tx_eNB; aa++) {      
			  memset(&eNB->common_vars.txdataF[0][aa][subframe*fp->ofdm_symbol_size*(fp->symbols_per_tti)],
					 0,fp->ofdm_symbol_size*(fp->symbols_per_tti)*sizeof(int32_t));
			}
		}*/
		//VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_ENB_RESET_TX,0);
		//////////Trigger control channel thread//////////
		eNB->complete_m2p = 1;
		//VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_ENB_PARSE_DCI_TX,1);
		//////////Keep generating DCI//////////
		if (eNB->mac_enabled==1) {
			// Parse DCI received from MAC
			//VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_ENB_PDCCH_TX,1);
			DCI_pdu = mac_xface->get_dci_sdu(eNB->Mod_id,
							 eNB->CC_id,
							 frame,
							 subframe);
	    }
		else {
			DCI_pdu = &DCI_pdu_tmp;
		#ifdef EMOS_CHANNEL
			fill_dci_emos(DCI_pdu,eNB);
		#else
			fill_dci(DCI_pdu,eNB,proc);
		#endif
		}
		// clear existing ulsch dci allocations before applying info from MAC  (this is table
		ul_subframe = pdcch_alloc2ul_subframe(fp,subframe);
		ul_frame = pdcch_alloc2ul_frame(fp,frame,subframe);
		if ((subframe_select(fp,ul_subframe)==SF_UL) ||
			(fp->frame_type == FDD)) {
				harq_pid = subframe2harq_pid(fp,ul_frame,ul_subframe);

				// clear DCI allocation maps for new subframe
				for (i=0; i<NUMBER_OF_UE_MAX; i++)
				  if (eNB->ulsch[i]) {
					eNB->ulsch[i]->harq_processes[harq_pid]->dci_alloc=0;
					eNB->ulsch[i]->harq_processes[harq_pid]->rar_alloc=0;
				  }
		}

		// clear previous allocation information for all UEs
		for (i=0; i<NUMBER_OF_UE_MAX; i++) {
			if (eNB->dlsch[i][0])
			  eNB->dlsch[i][0]->subframe_tx[subframe] = 0;
		}

		num_pdcch_symbols = DCI_pdu->num_pdcch_symbols;
		LOG_D(PHY,"num_pdcch_symbols %"PRIu8",(dci common %"PRIu8", dci uespec %"PRIu8"\n",num_pdcch_symbols,
			  DCI_pdu->Num_common_dci,DCI_pdu->Num_ue_spec_dci);
			  
		#if defined(SMBV) 
		  // Sets up PDCCH and DCI table
		  if (smbv_is_config_frame(frame) && (smbv_frame_cnt < 4) && ((DCI_pdu->Num_common_dci+DCI_pdu->Num_ue_spec_dci)>0)) {
			LOG_D(PHY,"[SMBV] Frame %3d, SF %d PDCCH, number of DCIs %d\n",frame,subframe,DCI_pdu->Num_common_dci+DCI_pdu->Num_ue_spec_dci);
			dump_dci(fp,&DCI_pdu->dci_alloc[0]);
			smbv_configure_pdcch(smbv_fname,(smbv_frame_cnt*10) + (subframe),num_pdcch_symbols,DCI_pdu->Num_common_dci+DCI_pdu->Num_ue_spec_dci);
		  }
		#endif

		VCD_SIGNAL_DUMPER_DUMP_VARIABLE_BY_NAME(VCD_SIGNAL_DUMPER_VARIABLES_DCI_INFO,DCI_pdu->num_pdcch_symbols);

		// loop over all DCIs for this subframe to generate DLSCH allocations
		for (i=0; i<DCI_pdu->Num_common_dci + DCI_pdu->Num_ue_spec_dci ; i++) {
			LOG_D(PHY,"[eNB] Subframe %d: DCI %d/%d : rnti %x, CCEind %d\n",subframe,i,DCI_pdu->Num_common_dci+DCI_pdu->Num_ue_spec_dci,DCI_pdu->dci_alloc[i].rnti,DCI_pdu->dci_alloc[i].firstCCE);
			VCD_SIGNAL_DUMPER_DUMP_VARIABLE_BY_NAME(VCD_SIGNAL_DUMPER_VARIABLES_DCI_INFO,DCI_pdu->dci_alloc[i].rnti);
			VCD_SIGNAL_DUMPER_DUMP_VARIABLE_BY_NAME(VCD_SIGNAL_DUMPER_VARIABLES_DCI_INFO,DCI_pdu->dci_alloc[i].format);
			VCD_SIGNAL_DUMPER_DUMP_VARIABLE_BY_NAME(VCD_SIGNAL_DUMPER_VARIABLES_DCI_INFO,DCI_pdu->dci_alloc[i].firstCCE);
			dci_alloc = &DCI_pdu->dci_alloc[i];

			if ((dci_alloc->rnti<= P_RNTI) && 
			(dci_alloc->ra_flag!=1)) {
			  if (eNB->mac_enabled==1)
			UE_id = find_ue((int16_t)dci_alloc->rnti,eNB);
			  else
			UE_id = i;
			}
			else UE_id=0;
			
			generate_eNB_dlsch_params(eNB,proc,dci_alloc,UE_id);

		  }

		  VCD_SIGNAL_DUMPER_DUMP_VARIABLE_BY_NAME(VCD_SIGNAL_DUMPER_VARIABLES_DCI_INFO,(frame*10)+subframe);

		  // Apply physicalConfigDedicated if needed
		  // This is for UEs that have received this IE, which changes these DL and UL configuration, we apply after a delay for the eNodeB UL parameters
		  phy_config_dedicated_eNB_step2(eNB);
		  
		  // Now loop again over the DCIs for UL configuration
		  for (i=0; i<DCI_pdu->Num_common_dci + DCI_pdu->Num_ue_spec_dci ; i++) {
			dci_alloc = &DCI_pdu->dci_alloc[i];

			if (dci_alloc->format == format0) {  // this is a ULSCH allocation
			  if (eNB->mac_enabled==1)
			UE_id = find_ue((int16_t)dci_alloc->rnti,eNB);
			  else
			UE_id = i;
			  
			  if (UE_id<0) { // should not happen, log an error and exit, this is a fatal error
			LOG_E(PHY,"[eNB %"PRIu8"] Frame %d: Unknown UE_id for rnti %"PRIx16"\n",eNB->Mod_id,frame,dci_alloc->rnti);
			mac_xface->macphy_exit("FATAL\n"); 
			  }
			  generate_eNB_ulsch_params(eNB,proc,dci_alloc,UE_id);
			}
		  }

		  // if we have DCI to generate do it now
		  if ((DCI_pdu->Num_common_dci + DCI_pdu->Num_ue_spec_dci)>0) {

		  } else { // for emulation!!
			eNB->num_ue_spec_dci[(subframe)&1]=0;
			eNB->num_common_dci[(subframe)&1]=0;
		  }
		  //////////Over generating DCI//////////
		  eNB->complete_dci = 1;
		  //VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_ENB_PARSE_DCI_TX,0);
		  //VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_ENB_NONEUESPEC_PDSCH_TX,1);
		  ///////////////////////////////////////
		  num_pdcch_symbols = get_num_pdcch_symbols(DCI_pdu->Num_ue_spec_dci+DCI_pdu->Num_common_dci,DCI_pdu->dci_alloc,fp,subframe);
      
		  // Check for SI activity
		  if ((eNB->dlsch_SI) && (eNB->dlsch_SI->active == 1)) {
			 pdsch_procedures(eNB,proc,eNB->dlsch_SI,(LTE_eNB_DLSCH_t*)NULL,(LTE_eNB_UE_stats*)NULL,0,num_pdcch_symbols);
			 
			 #if defined(SMBV) 
				 // Configures the data source of allocation (allocation is configured by DCI)
				 if (smbv_is_config_frame(frame) && (smbv_frame_cnt < 4)) {
				   LOG_D(PHY,"[SMBV] Frame %3d, Configuring SI payload in SF %d alloc %"PRIu8"\n",frame,(smbv_frame_cnt*10) + (subframe),smbv_alloc_cnt);
				   smbv_configure_datalist_for_alloc(smbv_fname, smbv_alloc_cnt++, (smbv_frame_cnt*10) + (subframe), DLSCH_pdu, input_buffer_length);
				 }
			 #endif
		   }
		   // Check for RA activity
		   if ((eNB->dlsch_ra) && (eNB->dlsch_ra->active == 1)) {
			 #if defined(SMBV) 
				 // Configures the data source of allocation (allocation is configured by DCI)
				 if (smbv_is_config_frame(frame) && (smbv_frame_cnt < 4)) {
				   LOG_D(PHY,"[SMBV] Frame %3d, Configuring RA payload in SF %d alloc %"PRIu8"\n",frame,(smbv_frame_cnt*10) + (subframe),smbv_alloc_cnt);
				   smbv_configure_datalist_for_alloc(smbv_fname, smbv_alloc_cnt++, (smbv_frame_cnt*10) + (subframe), dlsch_input_buffer, input_buffer_length);
				 }
			 #endif
			  LOG_D(PHY,"[eNB %"PRIu8"][RAPROC] Frame %d, subframe %d: Calling generate_dlsch (RA),Msg3 frame %"PRIu32", Msg3 subframe %"PRIu8"\n",
				    eNB->Mod_id,
				    frame, subframe,
				    eNB->ulsch[(uint32_t)UE_id]->Msg3_frame,
				    eNB->ulsch[(uint32_t)UE_id]->Msg3_subframe);
					
			 pdsch_procedures(eNB,proc,eNB->dlsch_ra,(LTE_eNB_DLSCH_t*)NULL,(LTE_eNB_UE_stats*)NULL,1,num_pdcch_symbols);
			 eNB->dlsch_ra->active = 0;
		   }  
		  eNB->complete_sch_SR = 1;
		  //VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_ENB_NONEUESPEC_PDSCH_TX,0);
		  //VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_ENB_MACTOPHY_THREAD_TX,0);
	}
	printf( "Exiting eNB thread mac2phy\n");
	return &mac2phy_proc_status;
	
}
static void *cch_proc(void *ptr){
	PHY_VARS_eNB *eNB = PHY_vars_eNB_g[0][0];
	//clock_gettime(CLOCK_REALTIME, &eNB->tt17);
	//printf("cch_proc consumes %ld nanoseconds!\n",eNB->tt17.tv_nsec-eNB->tt13.tv_nsec);
	static int control_channel_status;
	control_channel_status=0;
	control_channel *cch=(control_channel*)ptr;
	//PHY_VARS_eNB *eNB = PHY_vars_eNB_g[0][0];
	LTE_DL_FRAME_PARMS *fp;
	eNB_rxtx_proc_t *proc;
	relaying_type_t r_type;
	DCI_PDU *DCI_pdu; 
    DCI_PDU DCI_pdu_tmp;
	uint8_t num_pdcch_symbols=0;
	/**********************************************************************/
	cpu_set_t cpuset; 
	//the CPU we whant to use
	int cpu = 2;
	int s;
	CPU_ZERO(&cpuset);       //clears the cpuset
	CPU_SET( cpu , &cpuset); //set CPU 0~7 on cpuset
	s = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
	if (s != 0)
	{
		perror( "pthread_setaffinity_np");
		exit_fun("Error setting processor affinity");
	}
	s = pthread_getaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
	if (s != 0) {
		perror( "pthread_getaffinity_np");
		exit_fun("Error getting processor affinity ");
	}
	/**********************************************************************/
  printf("[SCHED][eNB] Control_channel thread started on CPU %d TID %ld\n",sched_getcpu(),gettid());
	while(!oai_exit)
	{
	   	while(pthread_cond_wait(&eNB->thread_cch.cond_tx, &eNB->thread_cch.mutex_tx)!=0);
		VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_ENB_CONTROL_CHANNEL_THREAD_TX,1);
		proc = &eNB->proc.proc_rxtx[0];
		fp=&eNB->frame_parms;
		int frame = proc->frame_tx;
		int subframe = proc->subframe_tx;
		r_type = cch->r_type;
		//VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_ENB_PMCH_PBCH_TX,1);
		if (is_pmch_subframe(frame,subframe,fp)) {
			pmch_procedures(eNB,proc,NULL,r_type);
		}
		else {
			// this is not a pmch subframe, so generate PSS/SSS/PBCH
			common_signal_procedures(eNB,proc);
		}
		//VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_ENB_PMCH_PBCH_TX,0);
		#if defined(SMBV) 
		// PBCH takes one allocation
		if (smbv_is_config_frame(frame) && (smbv_frame_cnt < 4)) {
			if (subframe==0)
			  smbv_alloc_cnt++;
		}
		#endif
		////////////////////Get DCI from MAC////////////////////
		//VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_PDCCH_TX,1);
		if (eNB->mac_enabled==1) {
		  	// Parse DCI received from MAC
			//VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_ENB_PDCCH_TX,1);
			DCI_pdu = mac_xface->get_dci_sdu(eNB->Mod_id,
											 eNB->CC_id,
											 frame,
											 subframe);
		}
		else {
			DCI_pdu = &DCI_pdu_tmp;
		#ifdef EMOS_CHANNEL
			fill_dci_emos(DCI_pdu,eNB);
		#else
			fill_dci(DCI_pdu,eNB,proc);
		#endif
		}
		if (eNB->abstraction_flag == 0) {
			if (DCI_pdu->Num_ue_spec_dci+DCI_pdu->Num_common_dci > 0) {
			  LOG_D(PHY,"[eNB %"PRIu8"] Frame %d, subframe %d: Calling generate_dci_top (pdcch) (common %"PRIu8",ue_spec %"PRIu8")\n",eNB->Mod_id,frame, subframe,
					DCI_pdu->Num_common_dci,DCI_pdu->Num_ue_spec_dci);
			}
			num_pdcch_symbols = generate_dci_top(DCI_pdu->Num_ue_spec_dci,
												 DCI_pdu->Num_common_dci,
												 DCI_pdu->dci_alloc,
												 0,
												 AMP,
												 fp,
												 eNB->common_vars.txdataF[0],
												 subframe);
	    }
		#ifdef PHY_ABSTRACTION // FIXME this ifdef seems suspicious
			else {
				LOG_D(PHY,"[eNB %"PRIu8"] Frame %d, subframe %d: Calling generate_dci_to_emul\n",eNB->Mod_id,frame, subframe);
				num_pdcch_symbols = generate_dci_top_emul(eNB,DCI_pdu->Num_ue_spec_dci,DCI_pdu->Num_common_dci,DCI_pdu->dci_alloc,subframe);
			}

		#endif
		//VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_PDCCH_TX,0);
		// if we have PHICH to generate
		//VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_ENB_PHICH_TX,1);
	    if (is_phich_subframe(fp,subframe))
		{
		    generate_phich_top(eNB,
							   proc,
							   AMP,
							   0);
		}
		//VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_ENB_PHICH_TX,0);
		//eNB->num_pdcch_symbols = num_pdcch_symbols;
		eNB->complete_cch = 1;
		VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_ENB_CONTROL_CHANNEL_THREAD_TX,0);
	}
	printf( "Exiting eNB thread control_channel\n");
	return &control_channel_status;
}
static void* ofdm_proc( void* arg){	
PHY_VARS_eNB *eNB = PHY_vars_eNB_g[0][0];
	//clock_gettime(CLOCK_REALTIME, &eNB->tt18);
	//printf("ofdm_proc consumes %ld nanoseconds!\n",eNB->tt18.tv_nsec-eNB->tt14.tv_nsec);
	static int ofdm_status;
	
	eNB_rxtx_proc_t *proc;
	int subframe;
	/**********************************************************************/
	cpu_set_t cpuset; 
	//the CPU we whant to use
	int cpu = 1;
	int s;
	CPU_ZERO(&cpuset);       //clears the cpuset
	CPU_SET( cpu , &cpuset); //set CPU 0~7 on cpuset
	s = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
	if (s != 0)
	{
		perror( "pthread_setaffinity_np");
		exit_fun("Error setting processor affinity");
	}
	s = pthread_getaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
	if (s != 0) {
		perror( "pthread_getaffinity_np");
		exit_fun("Error getting processor affinity ");
	}
	/**********************************************************************/
  printf("[SCHED][eNB] OFDM thread started on CPU %d TID %ld\n",sched_getcpu(),gettid());
	while(!oai_exit){
	  while(pthread_cond_wait(&eNB->thread_ofdm.cond_tx, &eNB->thread_ofdm.mutex_tx)!=0);
	  proc = &eNB->proc.proc_rxtx[0];
	  subframe = proc->subframe_tx;
	  // do OFDM modulation
	  do_OFDM_mod_rt(subframe,eNB);
	  // if TX fronthaul go ahead 
	  if (eNB->tx_fh) eNB->tx_fh(eNB,proc);
	}
	
	printf( "Exiting eNB thread do_ofdm\n");
	ofdm_status=0;
	return &ofdm_status;
}
static void* TX_timing_thread( void* arg){
	PHY_VARS_eNB *eNB = PHY_vars_eNB_g[0][0];
	//clock_gettime(CLOCK_REALTIME, &eNB->tt15);
	//printf("TX_timing_thread consumes %ld nanoseconds!\n",eNB->tt15.tv_nsec-eNB->tt11.tv_nsec);
	//PHY_VARS_eNB *eNB = PHY_vars_eNB_g[0][0];
	
	static int thread_master_status;
	
	int i;
	while(!oai_exit){
		if(eNB->flag_m2p==1){
			eNB->flag_m2p=0;
			pthread_cond_signal(&eNB->thread_m2p.cond_tx);
		}
		if(eNB->flag_cch==1){
			eNB->flag_cch=0;
			pthread_cond_signal(&eNB->thread_cch.cond_tx);
		}
		if(eNB->flag_sch==1){
			eNB->flag_sch=0;
			for(i=0;i<NUM_THREAD;i++){
				pthread_cond_signal(&eNB->thread_g[i].cond_tx);
			}
		}
		if(eNB->flag_ofdm==1){
			eNB->flag_ofdm=0;
			pthread_cond_signal(&eNB->thread_ofdm.cond_tx);
		}
		if(eNB->flag_pucch==1){
			eNB->flag_pucch=0;
			pthread_cond_signal(&eNB->thread_pucch.cond_rx);
		}
		if(eNB->flag_pusch==1){
			eNB->flag_pusch=0;
			pthread_cond_signal(&eNB->thread_pusch.cond_rx);
		}
		if(eNB->flag_viterbi==1){
			eNB->flag_viterbi=0;
			pthread_cond_signal(&eNB->thread_viterbi.cond_rx);
		}
		if(eNB->flag_turbo==1){
			eNB->flag_turbo=0;
			for(i=0;i<NUM_THREAD;++i){
				pthread_cond_signal(&eNB->thread_turbo[i].cond_rx);
			}
		}
	}
	
	printf( "Exiting eNB thread TX_timing_thread\n");
	thread_master_status=0;
	return &thread_master_status;
}
static void *multi_sch_proc(void *ptr){
	
	multi_share_channel *test =(multi_share_channel*) ptr;
	static int multi_sch_proc_status;
	int num_pdcch_symbols;
	multi_sch_proc_status=0;
	
	int UE_id = test->id;
	PHY_VARS_eNB *eNB;
	eNB = PHY_vars_eNB_g[0][0];
	eNB_rxtx_proc_t *proc;
	DCI_PDU *DCI_pdu; 
	DCI_PDU DCI_pdu_tmp;	
	LTE_DL_FRAME_PARMS *fp;
	int frame;
	int subframe;
	
	/*printf("[STOP] Thread %d is waiting...\n",test->id );
	while(pthread_cond_wait(&eNB->thread_g[test->id].cond_tx,&eNB->thread_g[test->id].mutex_tx)!=0);
	
	eNB->ue_status[UE_id]=1;
	
	printf("[WORK] Thread %d is working...\n",test->id );  */ 
    /**********************************************************************/
	cpu_set_t cpuset; 
	//the CPU we whant to use
	int cpu = 3+UE_id;
	int s;
	CPU_ZERO(&cpuset);       //clears the cpuset
	CPU_SET( cpu , &cpuset); //set CPU 0~7 on cpuset
	s = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
	if (s != 0)
	{
		perror( "pthread_setaffinity_np");
		exit_fun("Error setting processor affinity");
	}
	s = pthread_getaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
	if (s != 0) {
		perror( "pthread_getaffinity_np");
		exit_fun("Error getting processor affinity ");
	}
	/**********************************************************************/
  printf("[SCHED][eNB] PDSCH thread %d started on CPU %d TID %ld\n",UE_id,sched_getcpu(),gettid());
	while(!oai_exit){
		if(test->flag_wait==1){// 新的subframe初次進來等待
			while(pthread_cond_wait(&eNB->thread_g[test->id].cond_tx,&eNB->thread_g[test->id].mutex_tx)!=0);
			test->flag_wait = 0;
			eNB->ue_status[test->id]=1;
			UE_id=test->id;
			proc=&eNB->proc.proc_rxtx[0];
			frame = proc->frame_tx;
			subframe = proc->subframe_tx;
			fp=&eNB->frame_parms;
		}
		if (eNB->mac_enabled==1) {
			 // Parse DCI received from MAC    
			 DCI_pdu = mac_xface->get_dci_sdu(eNB->Mod_id,
											  eNB->CC_id,
											  frame,
											  subframe);
		}
		else {
			 DCI_pdu = &DCI_pdu_tmp;
		 #ifdef EMOS_CHANNEL
			 fill_dci_emos(DCI_pdu,eNB);
		 #else
			 fill_dci(DCI_pdu,eNB,proc);
		 #endif
		}
        num_pdcch_symbols = get_num_pdcch_symbols(DCI_pdu->Num_ue_spec_dci+DCI_pdu->Num_common_dci,DCI_pdu->dci_alloc,fp,subframe);
		VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_TX01+test->id,1);
		//printf("UE_status:%d rnti:%d active:%d \n",eNB->dlsch[(uint8_t)UE_id][0],eNB->dlsch[(uint8_t)UE_id][0]->rnti,eNB->dlsch[(uint8_t)UE_id][0]->active);

			if ((eNB->dlsch[(uint8_t)UE_id][0])&&
				(eNB->dlsch[(uint8_t)UE_id][0]->rnti>0)&&
				(eNB->dlsch[(uint8_t)UE_id][0]->active == 1)) {
                VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_WORK_TX01+test->id,1);
				//VCD_SIGNAL_DUMPER_DUMP_VARIABLE_BY_NAME(VCD_SIGNAL_DUMPER_VARIABLES_ENB_RB01+test->id,eNB->dlsch[(uint8_t)UE_id][0]->harq_processes[eNB->dlsch[(uint8_t)UE_id][0]->current_harq_pid]->nb_rb);
	pdsch_procedures(eNB,proc,eNB->dlsch[(uint8_t)UE_id][0],eNB->dlsch[(uint8_t)UE_id][1],&eNB->UE_stats[(uint32_t)UE_id],0,num_pdcch_symbols);
				VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_WORK_TX01+test->id,0);

      }

      else if ( (eNB->dlsch[(uint8_t)UE_id][0])&&
				(eNB->dlsch[(uint8_t)UE_id][0]->rnti>0)&&
				(eNB->dlsch[(uint8_t)UE_id][0]->active == 0)) {

	// clear subframe TX flag since UE is not scheduled for PDSCH in this subframe (so that we don't look for PUCCH later)
	eNB->dlsch[(uint8_t)UE_id][0]->subframe_tx[subframe]=0;
      }
	    VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_TX01+test->id,0);
		pthread_mutex_lock(&eNB->uestatus_mutex);
		
		eNB->complete_pdsch = eNB->complete_pdsch +1;
		
		
		for(int i=NUM_THREAD;i<NUMBER_OF_UE_MAX;i++)
		{	
			if(i==NUMBER_OF_UE_MAX-1 &&eNB->ue_status[i]==1){
				test->flag_wait = 1;
			}
			if(eNB->ue_status[i]==0){
				UE_id = i;
				eNB->ue_status[i]=1;	
				break;
			}		
		}
		pthread_mutex_unlock(&eNB->uestatus_mutex);
	}
	printf( "Exiting eNB thread multi_sch_proc %d\n",test->id);
	multi_sch_proc_status=0;
	return &multi_sch_proc_status;
} 
//----------------Part of RX procedure----------------

extern uint8_t extract_cqi_crc(uint8_t *,uint8_t );
extern void pucch_procedures(PHY_VARS_eNB *,eNB_rxtx_proc_t *,int ,int );
static void* viterbi_proc( void* arg){
	PHY_VARS_eNB *eNB = PHY_vars_eNB_g[0][0];
	//clock_gettime(CLOCK_REALTIME, &eNB->tt38);
	//printf("viterbi_proc consumes %ld nanoseconds!\n",eNB->tt38.tv_nsec-eNB->tt34.tv_nsec);
	static int viterbi_status;
	//PHY_VARS_eNB *eNB = PHY_vars_eNB_g[0][0];
	LTE_DL_FRAME_PARMS *frame_parms;
	eNB_rxtx_proc_t *proc;
	int subframe;
	int i,UE_id;
	uint8_t harq_pid;
	LTE_eNB_ULSCH_t *ulsch;
	LTE_UL_eNB_HARQ_t *ulsch_harq;
	unsigned int O_RCC;
	/**********************************************************************/
	cpu_set_t cpuset; 
	//the CPU we whant to use
	int cpu = 1;
	int s;
	CPU_ZERO(&cpuset);       //clears the cpuset
	CPU_SET( cpu , &cpuset); //set CPU 0~7 on cpuset
	s = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
	if (s != 0)
	{
		perror( "pthread_setaffinity_np");
		exit_fun("Error setting processor affinity");
	}
	s = pthread_getaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
	if (s != 0) {
		perror( "pthread_getaffinity_np");
		exit_fun("Error getting processor affinity ");
	}
	/**********************************************************************/
  printf("[SCHED][eNB] Viterbi thread started on CPU %d TID %ld\n",sched_getcpu(),gettid());
	while(!oai_exit){
		while(pthread_cond_wait(&eNB->thread_viterbi.cond_rx,&eNB->thread_viterbi.mutex_rx)!=0);
		//Update parameters
		frame_parms = &eNB->frame_parms;
		proc = &eNB->proc.proc_rxtx[0];
		subframe = proc->subframe_rx;
		harq_pid = subframe2harq_pid(frame_parms,proc->frame_rx,subframe);
		uint8_t dummy_w_cc[3*(MAX_CQI_BITS+8+32)];
		uint8_t o_flip[8];
		//VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_viterbi,1);
		uint8_t complete_parameter=0,wait_parameter[NUMBER_OF_UE_MAX];
		for(UE_id=0;UE_id<NUMBER_OF_UE_MAX;++UE_id) wait_parameter[UE_id] = 1;
	  while(complete_parameter!=NUMBER_OF_UE_MAX){
		for(UE_id=0;UE_id<NUMBER_OF_UE_MAX;++UE_id){
		  ulsch = eNB->ulsch[UE_id];
		  ulsch_harq = ulsch->harq_processes[harq_pid];
		  if(eNB->active_UE[UE_id]==0&&wait_parameter[UE_id]==1) {
			wait_parameter[UE_id] = 0;
			complete_parameter = complete_parameter+1;
		  }
		  else if(eNB->active_UE[UE_id]==1&&eNB->complete_viterbi_parameter[UE_id]==1&&wait_parameter[UE_id]==1){//等decoding個別UE前製作業/參數處理完成
			wait_parameter[UE_id] = 0;
			complete_parameter = complete_parameter+1;
			if(eNB->Q_CQI[UE_id]>0){
				//VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_viterbi,1);
				
				memset((void *)&dummy_w_cc[0],0,3*(ulsch_harq->Or1+8+32));

				O_RCC = generate_dummy_w_cc(ulsch_harq->Or1+8,
											&dummy_w_cc[0]);


				lte_rate_matching_cc_rx(O_RCC,
										eNB->Q_CQI[UE_id],
										ulsch_harq->o_w,
										dummy_w_cc,
										ulsch_harq->q);

				sub_block_deinterleaving_cc((unsigned int)(ulsch_harq->Or1+8),
											&ulsch_harq->o_d[96],
											&ulsch_harq->o_w[0]);

				memset(o_flip,0,1+((8+ulsch_harq->Or1)/8));
				//phy_viterbi_lte_sse2(ulsch_harq->o_d+96,o_flip,8+ulsch_harq->Or1);

				if (extract_cqi_crc(o_flip,ulsch_harq->Or1) == (crc8(o_flip,ulsch_harq->Or1)>>24))
				  ulsch_harq->cqi_crc_status = 1;
				else
				  ulsch_harq->cqi_crc_status = 0;

				if (ulsch->harq_processes[harq_pid]->Or1<=32) {
				  ulsch_harq->o[3] = o_flip[0] ;
				  ulsch_harq->o[2] = o_flip[1] ;
				  ulsch_harq->o[1] = o_flip[2] ;
				  ulsch_harq->o[0] = o_flip[3] ;
				} else {
				  ulsch_harq->o[7] = o_flip[0] ;
				  ulsch_harq->o[6] = o_flip[1] ;
				  ulsch_harq->o[5] = o_flip[2] ;
				  ulsch_harq->o[4] = o_flip[3] ;
				  ulsch_harq->o[3] = o_flip[4] ;
				  ulsch_harq->o[2] = o_flip[5] ;
				  ulsch_harq->o[1] = o_flip[6] ;
				  ulsch_harq->o[0] = o_flip[7] ;
				}
			#ifdef DEBUG_ULSCH_DECODING
				printf("ulsch_decoding: Or1=%d\n",ulsch_harq->Or1);

				for (i=0; i<1+((8+ulsch_harq->Or1)/8); i++)
				  printf("ulsch_decoding: O[%d] %d\n",i,ulsch_harq->o[i]);

				if (ulsch_harq->cqi_crc_status == 1)
				  printf("RX CQI CRC OK (%x)\n",extract_cqi_crc(o_flip,ulsch_harq->Or1));
				else
				  printf("RX CQI CRC NOT OK (%x)\n",extract_cqi_crc(o_flip,ulsch_harq->Or1));

			#endif
			//VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_viterbi,0);
			}
			//Trigger complete_viterbi[UE_id]
			eNB->complete_viterbi[UE_id] = 1;
		  }
	    }
	  }
	  //VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_viterbi,0);//降下viterbi VCD結束
	  eNB->complete_viterbi_thread = 1;
	}
	printf( "Exiting eNB thread viterbi_proc_thread\n");
	viterbi_status=0;
	return &viterbi_status;
}
/*int check_turbo(PHY_VARS_eNB *eNB){
	int ret=1;
	int i;
	for(i=0;i<NUMBER_OF_UE_MAX;i++) {
		ret &= eNB->complete_turbo[i];
		if(ret!=1) break;
	}
	return ret;
}*/
static void* turbo_proc( void* arg){
	static int turbo_proc_status;
	turbo *test =(turbo*) arg;
	PHY_VARS_eNB *eNB = PHY_vars_eNB_g[0][0];
	LTE_DL_FRAME_PARMS *frame_parms;
	eNB_rxtx_proc_t *proc;
	uint8_t UE_id=-1;
	uint8_t harq_pid=0;
	int subframe;
	int i;
	uint8_t go=0;
	/**********************************************************************/
	cpu_set_t cpuset;  
	//the CPU we whant to use
	int cpu = 3+test->id;
	int s;
	CPU_ZERO(&cpuset);       //clears the cpuset
	CPU_SET( cpu , &cpuset); //set CPU 0~7 on cpuset
	s = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
	if (s != 0)
	{
		perror( "pthread_setaffinity_np");
		exit_fun("Error setting processor affinity");
	}
	s = pthread_getaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
	if (s != 0) {
		perror( "pthread_getaffinity_np");
		exit_fun("Error getting processor affinity ");
	}
	/**********************************************************************/
  printf("[SCHED][eNB] Turbo thread %d started on CPU %d TID %ld\n",test->id,sched_getcpu(),gettid());
	while(!oai_exit){
		if(test->flag_wait==1){// 新的subframe初次進來等待
			while(pthread_cond_wait(&eNB->thread_turbo[test->id].cond_rx,&eNB->thread_turbo[test->id].mutex_rx)!=0);
			test->flag_wait = 0;
			go = 0;
			proc=&eNB->proc.proc_rxtx[0];//Update parameters
			subframe = proc->subframe_rx;
			frame_parms = &eNB->frame_parms;
			harq_pid = subframe2harq_pid(frame_parms,proc->frame_rx,subframe);
			//先不給UE_id與job
		}
		if(go==1){//有job時
			VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_turbo01+test->id,1);
		#ifdef RX_pressuretest
			if(UE_id<NUMBER_OF_UE_MAX){
				ulsch_decoding(eNB,proc,
							   UE_id,//nothing
							   0,// control_only_flag
							   0,//nothing
							   0);//nothing
			}
			VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_WORK_turbo01+test->id,1);
			if(UE_id<NUMBER_OF_UE_MAX){
				eNB->ret[UE_id] = eNB->td(eNB,UE_id,harq_pid,eNB->ulsch[UE_id]->harq_processes[harq_pid]->nb_rb>20 ? 1 : 0);
			}
			else{
				eNB->td(eNB,UE_id,harq_pid,eNB->ulsch[0]->harq_processes[harq_pid]->nb_rb>20 ? 1 : 0);
			}
			VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_WORK_turbo01+test->id,0);
		#else
			//Do decoding parameter(Combined with turbo decoding)
			for(UE_id=0;UE_id<NUMBER_OF_UE_MAX;UE_id++){
				if(eNB->active_UE[UE_id]){
					ulsch_decoding(eNB,proc,
									  UE_id,//nothing
									  0,// control_only_flag
									  0,//nothing
									  0);//nothing
					VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_WORK_turbo01+test->id,1);
					eNB->ret[UE_id] = eNB->td(eNB,UE_id,harq_pid,eNB->ulsch[UE_id]->harq_processes[harq_pid]->nb_rb>20 ? 1 : 0);
					VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_WORK_turbo01+test->id,0);
				}
				
			}
			eNB->complete_turbo = 0xFFFF;
		#endif
			VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_turbo01+test->id,0);
	    }
		//--Mutex--
		pthread_mutex_lock(&eNB->turbo_status_mutex);
		//if(go==1) eNB->complete_turbo |= (1<<UE_id);
		go = 0;
	
	#ifdef RX_pressuretest
		if(eNB->turbo_status==0xFFFFFFFF)
	#else
		if(eNB->turbo_status==0xFFFF)
	#endif
		{
			test->flag_wait = 1;
			VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_turbo,0);//降下turbo VCD結束
		}
		else {
				int j;
		    #ifdef RX_pressuretest
				j = 2*NUMBER_OF_UE_MAX;
			#else
				j = NUMBER_OF_UE_MAX;
			#endif
				int temp=1;
				for(i=0;i<j;++i)//此可改成group當判斷
				{
					if(eNB->active_UE[i]==1) temp &= eNB->complete_pusch[i];
					/*uint32_t temp;
					uint32_t temp_com;
					temp = eNB->turbo_status&(1<<i);
					temp_com = eNB->complete_turbo&(1<<0);
					if(i<NUMBER_OF_UE_MAX){
						if(eNB->active_UE[i]==0&&temp==0){
							eNB->turbo_status |= (1<<i);
							eNB->complete_turbo |= (1<<i);
						}
						else if(eNB->active_UE[i]==1&&eNB->complete_pusch[i]==1&&temp==0){
							eNB->turbo_status |= (1<<i);
							UE_id=i;
							go = 1;
							break;//以group為break單位
						}		
					}*/
				}
				if(temp==1) {
					eNB->turbo_status = 0xFFFF;
					go = 1;
				}
		}
		pthread_mutex_unlock(&eNB->turbo_status_mutex);
	}
	printf( "Exiting eNB thread turbo_proc_thread %d\n",test->id);
	turbo_proc_status=0;
	return &turbo_proc_status;
}
static void* pucch_proc( void* arg){
	PHY_VARS_eNB *eNB = PHY_vars_eNB_g[0][0];
	//clock_gettime(CLOCK_REALTIME, &eNB->tt36);
	//printf("pucch_proc consumes %ld nanoseconds!\n",eNB->tt36.tv_nsec-eNB->tt32.tv_nsec);
	static int pucch_status;
	//PHY_VARS_eNB *eNB = PHY_vars_eNB_g[0][0];
	LTE_DL_FRAME_PARMS *frame_parms;
	eNB_rxtx_proc_t *proc;
	int subframe;
	int i;
	uint8_t harq_pid;
	/**********************************************************************/
	cpu_set_t cpuset; 
	//the CPU we whant to use
	int cpu = 1;
	int s;
	CPU_ZERO(&cpuset);       //clears the cpuset
	CPU_SET( cpu , &cpuset); //set CPU 0~7 on cpuset
	s = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
	if (s != 0)
	{
		perror( "pthread_setaffinity_np");
		exit_fun("Error setting processor affinity");
	}
	s = pthread_getaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
	if (s != 0) {
		perror( "pthread_getaffinity_np");
		exit_fun("Error getting processor affinity ");
	}
	/**********************************************************************/
  printf("[SCHED][eNB] PUCCH thread started on CPU %d TID %ld\n",sched_getcpu(),gettid());
	while(!oai_exit){
		while(pthread_cond_wait(&eNB->thread_pucch.cond_rx,&eNB->thread_pucch.mutex_rx)!=0);
		//Update parameters
		frame_parms = &eNB->frame_parms;
		proc = &eNB->proc.proc_rxtx[0];
		subframe = proc->subframe_rx;
		harq_pid = subframe2harq_pid(frame_parms,proc->frame_rx,subframe);
		VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_PUCCH,1);
		for (i=0; i<NUMBER_OF_UE_MAX; i++) {
			pucch_procedures(eNB,proc,i,harq_pid);
		}
		VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_PUCCH,0);
		//Trigger pucch finishing
	    eNB->complete_pucch = 1;
	}
	printf( "Exiting eNB thread pucch\n");
	pucch_status=0;
	return &pucch_status;
}
static void* pusch_proc( void* arg){
	PHY_VARS_eNB *eNB = PHY_vars_eNB_g[0][0];
	//clock_gettime(CLOCK_REALTIME, &eNB->tt37);
	//printf("pusch_proc consumes %ld nanoseconds!\n",eNB->tt37.tv_nsec-eNB->tt33.tv_nsec);
	static int pusch_status;
	//PHY_VARS_eNB *eNB = PHY_vars_eNB_g[0][0];
	LTE_DL_FRAME_PARMS *frame_parms;
	eNB_rxtx_proc_t *proc;
	int frame,subframe;
	int i;
	uint8_t harq_pid,nPRS;
	uint32_t round;
	LTE_DL_FRAME_PARMS *fp;
	/**********************************************************************/
	cpu_set_t cpuset; 
	//the CPU we whant to use
	int cpu = 2;
	int s;
	CPU_ZERO(&cpuset);       //clears the cpuset
	CPU_SET( cpu , &cpuset); //set CPU 0~7 on cpuset
	s = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
	if (s != 0)
	{
		perror( "pthread_setaffinity_np");
		exit_fun("Error setting processor affinity");
	}
	s = pthread_getaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
	if (s != 0) {
		perror( "pthread_getaffinity_np");
		exit_fun("Error getting processor affinity ");
	}
	/**********************************************************************/
  printf("[SCHED][eNB] PUSCH thread started on CPU %d TID %ld\n",sched_getcpu(),gettid());
	while(!oai_exit){
		while(pthread_cond_wait(&eNB->thread_pusch.cond_rx,&eNB->thread_pusch.mutex_rx)!=0);
		//Update parameters
		frame_parms = &eNB->frame_parms;
		proc = &eNB->proc.proc_rxtx[0];
		frame    = proc->frame_rx;
		subframe = proc->subframe_rx;
		harq_pid = subframe2harq_pid(frame_parms,proc->frame_rx,subframe);
		fp=&eNB->frame_parms;
		VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_PUSCH,1);
		for (i=0; i<NUMBER_OF_UE_MAX; i++) {
			if(eNB->active_UE[i]==1){
			// UE is has ULSCH scheduling
			  round = eNB->ulsch[i]->harq_processes[harq_pid]->round;
			  for (int rb=0;
				   rb<=eNB->ulsch[i]->harq_processes[harq_pid]->nb_rb;
			   rb++) {
			int rb2 = rb+eNB->ulsch[i]->harq_processes[harq_pid]->first_rb;
			eNB->rb_mask_ul[rb2>>5] |= (1<<(rb2&31));
			  }
				
			  if (eNB->ulsch[i]->Msg3_flag == 1) {
				LOG_D(PHY,"[eNB %d] frame %d, subframe %d: Scheduling ULSCH Reception for Msg3 in Sector %d\n",
					  eNB->Mod_id,
					  frame,
					  subframe,
					  eNB->UE_stats[i].sector);
			VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_ENB_ULSCH_MSG3,1);
			  } else {
				LOG_D(PHY,"[eNB %d] frame %d, subframe %d: Scheduling ULSCH Reception for UE %d Mode %s\n",
					  eNB->Mod_id,
					  frame,
					  subframe,
					  i,
					  mode_string[eNB->UE_stats[i].mode]);
			  }
			  nPRS = fp->pusch_config_common.ul_ReferenceSignalsPUSCH.nPRS[subframe<<1];

			  eNB->ulsch[i]->cyclicShift = (eNB->ulsch[i]->harq_processes[harq_pid]->n_DMRS2 + fp->pusch_config_common.ul_ReferenceSignalsPUSCH.cyclicShift +
							nPRS)%12;

			  if (fp->frame_type == FDD ) {
				int sf = (subframe<4) ? (subframe+6) : (subframe-4);

				if (eNB->dlsch[i][0]->subframe_tx[sf]>0) { // we have downlink transmission
				  eNB->ulsch[i]->harq_processes[harq_pid]->O_ACK = 1;
				} else {
				  eNB->ulsch[i]->harq_processes[harq_pid]->O_ACK = 0;
				}
			  }
			  LOG_D(PHY,
					"[eNB %d][PUSCH %d] Frame %d Subframe %d Demodulating PUSCH: dci_alloc %d, rar_alloc %d, round %d, first_rb %d, nb_rb %d, mcs %d, TBS %d, rv %d, cyclic_shift %d (n_DMRS2 %d, cyclicShift_common %d, nprs %d), O_ACK %d \n",
					eNB->Mod_id,harq_pid,frame,subframe,
					eNB->ulsch[i]->harq_processes[harq_pid]->dci_alloc,
					eNB->ulsch[i]->harq_processes[harq_pid]->rar_alloc,
					eNB->ulsch[i]->harq_processes[harq_pid]->round,
					eNB->ulsch[i]->harq_processes[harq_pid]->first_rb,
					eNB->ulsch[i]->harq_processes[harq_pid]->nb_rb,
					eNB->ulsch[i]->harq_processes[harq_pid]->mcs,
					eNB->ulsch[i]->harq_processes[harq_pid]->TBS,
					eNB->ulsch[i]->harq_processes[harq_pid]->rvidx,
					eNB->ulsch[i]->cyclicShift,
					eNB->ulsch[i]->harq_processes[harq_pid]->n_DMRS2,
					fp->pusch_config_common.ul_ReferenceSignalsPUSCH.cyclicShift,
					nPRS,
					eNB->ulsch[i]->harq_processes[harq_pid]->O_ACK);
					
			  eNB->pusch_stats_rb[i][(frame*10)+subframe] = eNB->ulsch[i]->harq_processes[harq_pid]->nb_rb;
			  eNB->pusch_stats_round[i][(frame*10)+subframe] = eNB->ulsch[i]->harq_processes[harq_pid]->round;
			  eNB->pusch_stats_mcs[i][(frame*10)+subframe] = eNB->ulsch[i]->harq_processes[harq_pid]->mcs;
			  start_meas(&eNB->ulsch_demodulation_stats);
			  //printf("eNB %d\n",eNB->UE_stats[i].sector);	
			  if (eNB->abstraction_flag==0) {
				//printf("OAI rx_ulsch \n");
				rx_ulsch(eNB,proc,
						 eNB->UE_stats[i].sector,  // this is the effective sector id
						 i,
						 eNB->ulsch,
						 0);
			  }
		#ifdef PHY_ABSTRACTION
			  else {
				rx_ulsch_emul(eNB,proc,
							  eNB->UE_stats[i].sector,  // this is the effective sector id
							  i);
			  }
		#endif
			  stop_meas(&eNB->ulsch_demodulation_stats);
			  //Trigger rx_ulsch[UE_id] for completing
			  eNB->complete_pusch[i] = 1;
			}
		}
		VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_PUSCH,0);
	}
	printf( "Exiting eNB thread pusch\n");
	pusch_status=0;
	return &pusch_status;
}
static void* RX_timing_thread( void* arg){
	PHY_VARS_eNB *eNB = PHY_vars_eNB_g[0][0];
	//clock_gettime(CLOCK_REALTIME, &eNB->tt35);
	//printf("RX_timing_thread consumes %ld nanoseconds!\n",eNB->tt35.tv_nsec-eNB->tt31.tv_nsec);
	static int thread_master_status;
	
	int i;
	while(!oai_exit){
		if(eNB->flag_pucch==1){
			eNB->flag_pucch=0;
			pthread_cond_signal(&eNB->thread_pucch.cond_rx);
		}
		if(eNB->flag_pusch==1){
			eNB->flag_pusch=0;
			pthread_cond_signal(&eNB->thread_pusch.cond_rx);
		}
		if(eNB->flag_viterbi==1){
			eNB->flag_viterbi=0;
			pthread_cond_signal(&eNB->thread_viterbi.cond_rx);
		}
		if(eNB->flag_turbo==1){
			eNB->flag_turbo=0;
			for(i=0;i<NUM_THREAD;++i){
				pthread_cond_signal(&eNB->thread_turbo[i].cond_rx);
			}
		}
	}
	printf( "Exiting eNB thread RX_timing_thread\n");
	thread_master_status=0;
	return &thread_master_status;
}

void init_eNB_proc(int inst) {
  
  int i,j;
  int CC_id;
  PHY_VARS_eNB *eNB;
  eNB_proc_t *proc;
  eNB_rxtx_proc_t *proc_rxtx;
  pthread_attr_t *attr0=NULL,*attr1=NULL,*attr_FH=NULL,*attr_prach=NULL,*attr_asynch=NULL,*attr_single=NULL,*attr_fep=NULL,*attr_td=NULL,*attr_te;
  struct timespec tt1;
  for (CC_id=0; CC_id<MAX_NUM_CCs; CC_id++) {
    eNB = PHY_vars_eNB_g[inst][CC_id];
#ifndef OCP_FRAMEWORK
    LOG_I(PHY,"Initializing eNB %d CC_id %d (%s,%s),\n",inst,CC_id,eNB_functions[eNB->node_function],eNB_timing[eNB->node_timing]);
#endif
    proc = &eNB->proc;
	pthread_attr_init( &eNB->thread_m2p.attr_m2p);
    pthread_attr_init( &eNB->thread_cch.attr_cch);
    pthread_attr_init( &eNB->thread_ofdm.attr_ofdm);
	for( i=0;i<NUM_THREAD;++i){
		pthread_attr_init( &eNB->thread_g[i].attr_sch);
	}
    pthread_attr_init( &eNB->thread_pucch.attr_ucch);
    pthread_attr_init( &eNB->thread_pusch.attr_usch);
    //pthread_attr_init( &eNB->thread_viterbi.attr_viterbi);
	for( i=0;i<NUM_THREAD;++i){
		pthread_attr_init( &eNB->thread_turbo[i].attr_turbo);
	}
	//----------------Part of TX procedure----------------
	//clock_gettime(CLOCK_REALTIME, &eNB->tt11);
	/*pthread_create( &eNB->pthread_TX_timing_thread, NULL, TX_timing_thread, NULL);
	printf("[CREATE] TX master thread \n");*/
	pthread_mutex_init( &eNB->thread_m2p.mutex_tx, NULL);
	pthread_cond_init( &eNB->thread_m2p.cond_tx, NULL);
	eNB->flag_m2p=0;
	//clock_gettime(CLOCK_REALTIME, &eNB->tt12);
	pthread_create(&(eNB->thread_m2p.pthread_m2p),&eNB->thread_m2p.attr_m2p,mac2phy_proc,NULL);
    printf("[CREATE] MAC to PHY thread \n");
	pthread_mutex_init( &(eNB->thread_cch.mutex_tx), NULL);
	pthread_cond_init( &(eNB->thread_cch.cond_tx), NULL);
	eNB->flag_cch=0;
	//clock_gettime(CLOCK_REALTIME, &eNB->tt13);
	pthread_create(&(eNB->thread_cch.pthread_cch),&eNB->thread_cch.attr_cch,cch_proc,&eNB->thread_cch);
	printf("[CREATE] Control_channel thread \n");
	pthread_mutex_init( &(eNB->thread_ofdm.mutex_tx), NULL);
	pthread_cond_init( &(eNB->thread_ofdm.cond_tx), NULL);
	eNB->flag_ofdm=0;
	//clock_gettime(CLOCK_REALTIME, &eNB->tt14);
	pthread_create(&(eNB->thread_ofdm.pthread_ofdm),&eNB->thread_ofdm.attr_ofdm,ofdm_proc,NULL);
	printf("[CREATE] OFDM thread \n");
	
	eNB->flag_sch=0;
	for( j=0;j<NUMBER_OF_UE_MAX;j++)
		eNB->ue_status[j]=0;
	for( i=0;i<NUM_THREAD;++i){
		pthread_mutex_init( &eNB->thread_g[i].mutex_tx, NULL);
		pthread_cond_init( &eNB->thread_g[i].cond_tx, NULL);

		eNB->thread_g[i].id = i;
		eNB->thread_g[i].flag_wait = 1;
		//clock_gettime(CLOCK_REALTIME, &eNB->tt21);
		pthread_create(&(eNB->thread_g[i].pthread_tx),&eNB->thread_g[i].attr_sch,multi_sch_proc,&eNB->thread_g[i]);
		//clock_gettime(CLOCK_REALTIME, &eNB->tt22);
		//printf("PDSCH thread %d consumes %ld nanoseconds!\n",eNB->thread_g[i].id,eNB->tt22.tv_nsec-eNB->tt21.tv_nsec);
		printf("[CREATE] PDSCH thread %d \n",eNB->thread_g[i].id);
	}
	
	//----------------Part of RX procedure----------------
	//clock_gettime(CLOCK_REALTIME, &eNB->tt31);
	/*pthread_create( &eNB->pthread_RX_timing_thread, NULL, RX_timing_thread, NULL);
	printf("[CREATE] RX master thread \n");*/
	eNB->flag_pucch=0;
	pthread_mutex_init( &eNB->thread_pucch.mutex_rx, NULL);
	pthread_cond_init( &eNB->thread_pucch.cond_rx, NULL);
	//clock_gettime(CLOCK_REALTIME, &eNB->tt32);
	pthread_create(&(eNB->thread_pucch.pthread_rx),&eNB->thread_pucch.attr_ucch,pucch_proc,NULL);
	printf("[CREATE] PUCCH thread\n");
	eNB->flag_pusch=0;
	pthread_mutex_init( &eNB->thread_pusch.mutex_rx, NULL);
	pthread_cond_init( &eNB->thread_pusch.cond_rx, NULL);
	//clock_gettime(CLOCK_REALTIME, &eNB->tt33);
	pthread_create(&(eNB->thread_pusch.pthread_rx),&eNB->thread_pusch.attr_usch,pusch_proc,NULL);
	printf("[CREATE] PUSCH thread\n");
	
	/*eNB->flag_viterbi=0;
	pthread_mutex_init( &eNB->thread_viterbi.mutex_rx, NULL);
	pthread_cond_init( &eNB->thread_viterbi.cond_rx, NULL);
	pthread_create(&(eNB->thread_viterbi.pthread_rx),NULL,viterbi_proc,NULL);
	printf("[CREATE] Viterbi thread\n");*/
	
	eNB->flag_turbo=0;
	/*for( j=0;j<NUMBER_OF_UE_MAX;j++)
		eNB->turbo_status[j] = 0;*/
	eNB->turbo_status = 0;
	for(i=0;i<NUM_THREAD;++i)
	{
		pthread_mutex_init( &eNB->thread_turbo[i].mutex_rx, NULL);
		pthread_cond_init( &eNB->thread_turbo[i].cond_rx, NULL);

		eNB->thread_turbo[i].id = i;
		eNB->thread_turbo[i].flag_wait = 1;
		//clock_gettime(CLOCK_REALTIME, &eNB->tt41);
		pthread_create(&(eNB->thread_turbo[i].pthread_rx),&eNB->thread_turbo[i].attr_turbo,turbo_proc,&eNB->thread_turbo[i]);
		//clock_gettime(CLOCK_REALTIME, &eNB->tt42);
		//printf("turbo_proc thread %d consumes %ld nanoseconds!\n",eNB->thread_turbo[i].id,eNB->tt42.tv_nsec-eNB->tt41.tv_nsec);
		printf("[CREATE] Turbo thread %d \n",eNB->thread_turbo[i].id);
	}
	/**************Rename pthread function name******************/
	char name[16];
	i=0;//CC_id=0
	//TX
	/*snprintf( name, sizeof(name), "TXtiming thread", NULL );
    pthread_setname_np(eNB->pthread_TX_timing_thread, name );*/
	snprintf( name, sizeof(name), " MAC to PHY thread", NULL );
    pthread_setname_np(eNB->thread_m2p.pthread_m2p, name );
	snprintf( name, sizeof(name), " Control_channel thread", NULL );
    pthread_setname_np(eNB->thread_cch.pthread_cch, name );
	snprintf( name, sizeof(name), " OFDM thread", NULL );
    pthread_setname_np(eNB->thread_ofdm.pthread_ofdm, name );
	for(i=0;i<NUM_THREAD;++i)
	{
		snprintf( name, sizeof(name), " PDSCH thread %d", i );
		pthread_setname_np(eNB->thread_g[i].pthread_tx, name );
	}
	i=0;
	//RX
	/*snprintf( name, sizeof(name), "RXtiming thread", NULL );
    pthread_setname_np(eNB->pthread_RX_timing_thread, name );*/
	snprintf( name, sizeof(name), " PUCCH thread", NULL );
    pthread_setname_np(eNB->thread_pucch.pthread_rx, name );
	snprintf( name, sizeof(name), " PUSCH thread", NULL );
    pthread_setname_np(eNB->thread_pusch.pthread_rx, name );
	/*snprintf( name, sizeof(name), " Viterbi thread", NULL );
    pthread_setname_np(eNB->thread_viterbi.pthread_rx, name );*/
	for(i=0;i<NUM_THREAD;++i)
	{
		snprintf( name, sizeof(name), " Turbo thread %d", i );
		pthread_setname_np(eNB->thread_turbo[i].pthread_rx, name );
	}
	i=0;
	
    proc_rxtx = proc->proc_rxtx;
    proc_rxtx[0].instance_cnt_rxtx = -1;
    proc_rxtx[1].instance_cnt_rxtx = -1;
    proc->instance_cnt_prach       = -1;
    proc->instance_cnt_FH          = -1;
    proc->instance_cnt_asynch_rxtx = -1;
    proc->CC_id = CC_id;    
    
    proc->first_rx=1;
    proc->first_tx=1;

    pthread_mutex_init( &proc_rxtx[0].mutex_rxtx, NULL);
    pthread_mutex_init( &proc_rxtx[1].mutex_rxtx, NULL);
    pthread_cond_init( &proc_rxtx[0].cond_rxtx, NULL);
    pthread_cond_init( &proc_rxtx[1].cond_rxtx, NULL);

    pthread_mutex_init( &proc->mutex_prach, NULL);
    pthread_mutex_init( &proc->mutex_asynch_rxtx, NULL);

    pthread_cond_init( &proc->cond_prach, NULL);
    pthread_cond_init( &proc->cond_FH, NULL);
    pthread_cond_init( &proc->cond_asynch_rxtx, NULL);

    pthread_attr_init( &proc->attr_FH);
    pthread_attr_init( &proc->attr_prach);
    pthread_attr_init( &proc->attr_asynch_rxtx);
    pthread_attr_init( &proc->attr_single);
    pthread_attr_init( &proc->attr_fep);
    pthread_attr_init( &proc->attr_td);
    pthread_attr_init( &proc->attr_te);
    pthread_attr_init( &proc_rxtx[0].attr_rxtx);
    pthread_attr_init( &proc_rxtx[1].attr_rxtx);
#ifndef DEADLINE_SCHEDULER
    attr0       = &proc_rxtx[0].attr_rxtx;
    attr1       = &proc_rxtx[1].attr_rxtx;
    attr_FH     = &proc->attr_FH;
    attr_prach  = &proc->attr_prach;
    attr_asynch = &proc->attr_asynch_rxtx;
    attr_single = &proc->attr_single;
    attr_fep    = &proc->attr_fep;
    attr_td     = &proc->attr_td;
    attr_te     = &proc->attr_te; 
#endif

    if (eNB->single_thread_flag==0) {
      pthread_create( &proc_rxtx[0].pthread_rxtx, attr0, eNB_thread_rxtx, &proc_rxtx[0] );
      pthread_create( &proc_rxtx[1].pthread_rxtx, attr1, eNB_thread_rxtx, &proc_rxtx[1] );
      pthread_create( &proc->pthread_FH, attr_FH, eNB_thread_FH, &eNB->proc );
    }
    else {
      pthread_create(&proc->pthread_single, attr_single, eNB_thread_single, &eNB->proc);
      init_fep_thread(eNB,attr_fep);
      init_td_thread(eNB,attr_td);
      init_te_thread(eNB,attr_te);
    }
    pthread_create( &proc->pthread_prach, attr_prach, eNB_thread_prach, &eNB->proc );
    if ((eNB->node_timing == synch_to_other) ||
	(eNB->node_function == NGFI_RRU_IF5) ||
	(eNB->node_function == NGFI_RRU_IF4p5))


      pthread_create( &proc->pthread_asynch_rxtx, attr_asynch, eNB_thread_asynch_rxtx, &eNB->proc );

    //char name[16];
    if (eNB->single_thread_flag == 0) {
      snprintf( name, sizeof(name), "RXTX0 %d", i );
      pthread_setname_np( proc_rxtx[0].pthread_rxtx, name );
      snprintf( name, sizeof(name), "RXTX1 %d", i );
      pthread_setname_np( proc_rxtx[1].pthread_rxtx, name );
      snprintf( name, sizeof(name), "FH %d", i );
      pthread_setname_np( proc->pthread_FH, name );
    }
    else {
      snprintf( name, sizeof(name), " %d", i );
      pthread_setname_np( proc->pthread_single, name );
    }
  }

  //for multiple CCs: setup master and slaves
  for (CC_id=0; CC_id<MAX_NUM_CCs; CC_id++) {
    eNB = PHY_vars_eNB_g[inst][CC_id];

    if (eNB->node_timing == synch_to_ext_device) { //master
      eNB->proc.num_slaves = MAX_NUM_CCs-1;
      eNB->proc.slave_proc = (eNB_proc_t**)malloc(eNB->proc.num_slaves*sizeof(eNB_proc_t*));

      for (i=0; i< eNB->proc.num_slaves; i++) {
        if (i < CC_id)  eNB->proc.slave_proc[i] = &(PHY_vars_eNB_g[inst][i]->proc);
        if (i >= CC_id)  eNB->proc.slave_proc[i] = &(PHY_vars_eNB_g[inst][i+1]->proc);
      }
    }
  }


  /* setup PHY proc TX sync mechanism */
  pthread_mutex_init(&sync_phy_proc.mutex_phy_proc_tx, NULL);
  pthread_cond_init(&sync_phy_proc.cond_phy_proc_tx, NULL);
  sync_phy_proc.phy_proc_CC_id = 0;
}



/*!
 * \brief Terminate eNB TX and RX threads.
 */
void kill_eNB_proc(int inst) {

  int *status;
  PHY_VARS_eNB *eNB;
  eNB_proc_t *proc;
  eNB_rxtx_proc_t *proc_rxtx;
  for (int CC_id=0; CC_id<MAX_NUM_CCs; CC_id++) {
    eNB=PHY_vars_eNB_g[inst][CC_id];
    
    proc = &eNB->proc;
    proc_rxtx = &proc->proc_rxtx[0];
    
#ifdef DEBUG_THREADS
    printf( "Killing TX CC_id %d thread %d\n", CC_id, i );
#endif
    
    proc_rxtx[0].instance_cnt_rxtx = 0; // FIXME data race!
    proc_rxtx[1].instance_cnt_rxtx = 0; // FIXME data race!
    proc->instance_cnt_prach = 0;
    proc->instance_cnt_FH = 0;
    pthread_cond_signal( &proc_rxtx[0].cond_rxtx );    
    pthread_cond_signal( &proc_rxtx[1].cond_rxtx );
    pthread_cond_signal( &proc->cond_prach );
    pthread_cond_signal( &proc->cond_FH );
    pthread_cond_broadcast(&sync_phy_proc.cond_phy_proc_tx);
	
	//----------------Part of TX procedure----------------
	/*pthread_join(eNB->pthread_TX_timing_thread,(void**)&status);
	printf("[KILL] TX main thread\n");
	*/
	pthread_cond_signal(&eNB->thread_m2p.cond_tx);
	pthread_join(eNB->thread_m2p.pthread_m2p,(void**)&status);
	pthread_mutex_destroy(&eNB->thread_m2p.mutex_tx);
	pthread_cond_destroy(&eNB->thread_m2p.cond_tx);
	printf("[KILL] mac2phy thread\n");
	
	pthread_cond_signal(&eNB->thread_cch.cond_tx);
	pthread_join(eNB->thread_cch.pthread_cch,(void**)&status);
	pthread_mutex_destroy(&eNB->thread_cch.mutex_tx);
	pthread_cond_destroy(&eNB->thread_cch.cond_tx);
	printf("[KILL] control_channel thread\n");
	
	pthread_cond_signal(&eNB->thread_ofdm.cond_tx);
	pthread_join(eNB->thread_ofdm.pthread_ofdm,(void**)&status);
	pthread_mutex_destroy(&eNB->thread_ofdm.mutex_tx);
	pthread_cond_destroy(&eNB->thread_ofdm.cond_tx);
	printf("[KILL] ofdm thread\n");

	int k;
	for( k=0;k<NUM_THREAD;k++){
		pthread_cond_signal(&eNB->thread_g[k].cond_tx);
		pthread_join(eNB->thread_g[k].pthread_tx,(void**)&status);
		pthread_mutex_destroy( &(eNB->thread_g[k].mutex_tx) );
		pthread_cond_destroy( &(eNB->thread_g[k].cond_tx) );
	}
	printf("[KILL] %d pdsch thread\n",k);
	
	//----------------Part of RX procedure----------------
	/*pthread_join(eNB->pthread_RX_timing_thread,(void**)&status);
	printf("[KILL] RX main thread\n");*/
	
	pthread_cond_signal(&eNB->thread_pucch.cond_rx);
	pthread_join(eNB->thread_pucch.pthread_rx,(void**)&status);
	pthread_mutex_destroy( &(eNB->thread_pucch.mutex_rx) );
	pthread_cond_destroy( &(eNB->thread_pucch.cond_rx) );
	printf("[KILL] pucch thread\n");
	pthread_cond_signal(&eNB->thread_pusch.cond_rx);
	pthread_join(eNB->thread_pusch.pthread_rx,(void**)&status);
	pthread_mutex_destroy( &(eNB->thread_pusch.mutex_rx) );
	pthread_cond_destroy( &(eNB->thread_pusch.cond_rx) );
	printf("[KILL] pusch thread\n");
	/*pthread_cond_signal(&eNB->thread_viterbi.cond_rx);
	pthread_join(eNB->thread_viterbi.pthread_rx,(void**)&status);
	pthread_mutex_destroy( &(eNB->thread_viterbi.mutex_rx) );
	pthread_cond_destroy( &(eNB->thread_viterbi.cond_rx) );
	printf("[KILL] viterbi thread\n");*/
	
	for( k=0;k<NUM_THREAD;k++){
		pthread_cond_signal(&eNB->thread_turbo[k].cond_rx);
		pthread_join(eNB->thread_turbo[k].pthread_rx,(void**)&status);
		pthread_mutex_destroy( &(eNB->thread_turbo[k].mutex_rx) );
		pthread_cond_destroy( &(eNB->thread_turbo[k].cond_rx) );
	}
	printf("[KILL] %d turbo thread\n",k);
	//----------------------------------------------------
	
    pthread_join( proc->pthread_FH, (void**)&status ); 
    pthread_mutex_destroy( &proc->mutex_FH );
    pthread_cond_destroy( &proc->cond_FH );
            
    pthread_join( proc->pthread_prach, (void**)&status );    
    pthread_mutex_destroy( &proc->mutex_prach );
    pthread_cond_destroy( &proc->cond_prach );         

    int i;
    for (i=0;i<2;i++) {
      pthread_join( proc_rxtx[i].pthread_rxtx, (void**)&status );
      pthread_mutex_destroy( &proc_rxtx[i].mutex_rxtx );
      pthread_cond_destroy( &proc_rxtx[i].cond_rxtx );
    }
  }
}


/* this function maps the phy_vars_eNB tx and rx buffers to the available rf chains.
   Each rf chain is is addressed by the card number and the chain on the card. The
   rf_map specifies for each CC, on which rf chain the mapping should start. Multiple
   antennas are mapped to successive RF chains on the same card. */
int setup_eNB_buffers(PHY_VARS_eNB **phy_vars_eNB, openair0_config_t *openair0_cfg) {

  int i, CC_id;
  int j;

  uint16_t N_TA_offset = 0;

  LTE_DL_FRAME_PARMS *frame_parms;

  for (CC_id=0; CC_id<MAX_NUM_CCs; CC_id++) {
    if (phy_vars_eNB[CC_id]) {
      frame_parms = &(phy_vars_eNB[CC_id]->frame_parms);
      printf("setup_eNB_buffers: frame_parms = %p\n",frame_parms);
    } else {
      printf("phy_vars_eNB[%d] not initialized\n", CC_id);
      return(-1);
    }

    if (frame_parms->frame_type == TDD) {
      if (frame_parms->N_RB_DL == 100)
        N_TA_offset = 624;
      else if (frame_parms->N_RB_DL == 50)
        N_TA_offset = 624/2;
      else if (frame_parms->N_RB_DL == 25)
        N_TA_offset = 624/4;
    }

 

    if (openair0_cfg[CC_id].mmapped_dma == 1) {
    // replace RX signal buffers with mmaped HW versions
      
      for (i=0; i<frame_parms->nb_antennas_rx; i++) {
	printf("Mapping eNB CC_id %d, rx_ant %d\n",CC_id,i);
	free(phy_vars_eNB[CC_id]->common_vars.rxdata[0][i]);
	phy_vars_eNB[CC_id]->common_vars.rxdata[0][i] = openair0_cfg[CC_id].rxbase[i];
	
	
	
	printf("rxdata[%d] @ %p\n",i,phy_vars_eNB[CC_id]->common_vars.rxdata[0][i]);
	
	for (j=0; j<16; j++) {
	  printf("rxbuffer %d: %x\n",j,phy_vars_eNB[CC_id]->common_vars.rxdata[0][i][j]);
	  phy_vars_eNB[CC_id]->common_vars.rxdata[0][i][j] = 16-j;
	}
      }
      
      for (i=0; i<frame_parms->nb_antennas_tx; i++) {
	printf("Mapping eNB CC_id %d, tx_ant %d\n",CC_id,i);
	free(phy_vars_eNB[CC_id]->common_vars.txdata[0][i]);
	phy_vars_eNB[CC_id]->common_vars.txdata[0][i] = openair0_cfg[CC_id].txbase[i];//(int32_t*) openair0_exmimo_pci[rf_map[CC_id].card].dac_head[rf_map[CC_id].chain+i];
	
	printf("txdata[%d] @ %p\n",i,phy_vars_eNB[CC_id]->common_vars.txdata[0][i]);
	
	for (j=0; j<16; j++) {
	  printf("txbuffer %d: %x\n",j,phy_vars_eNB[CC_id]->common_vars.txdata[0][i][j]);
	  phy_vars_eNB[CC_id]->common_vars.txdata[0][i][j] = 16-j;
	}
      }
    }
    else {  // not memory-mapped DMA 
    

      rxdata = (int32_t**)malloc16(frame_parms->nb_antennas_rx*sizeof(int32_t*));
      txdata = (int32_t**)malloc16(frame_parms->nb_antennas_tx*sizeof(int32_t*));
      
      for (i=0; i<frame_parms->nb_antennas_rx; i++) {
	free(phy_vars_eNB[CC_id]->common_vars.rxdata[0][i]);
	rxdata[i] = (int32_t*)(32 + malloc16(32+frame_parms->samples_per_tti*10*sizeof(int32_t))); // FIXME broken memory allocation
	phy_vars_eNB[CC_id]->common_vars.rxdata[0][i] = rxdata[i]-N_TA_offset; // N_TA offset for TDD         FIXME! N_TA_offset > 16 => access of unallocated memory
	memset(rxdata[i], 0, frame_parms->samples_per_tti*10*sizeof(int32_t));
	printf("rxdata[%d] @ %p (%p) (N_TA_OFFSET %d)\n", i, phy_vars_eNB[CC_id]->common_vars.rxdata[0][i],rxdata[i],N_TA_offset);      
      }
      
      for (i=0; i<frame_parms->nb_antennas_tx; i++) {
	free(phy_vars_eNB[CC_id]->common_vars.txdata[0][i]);
	txdata[i] = (int32_t*)(32 + malloc16(32 + frame_parms->samples_per_tti*10*sizeof(int32_t))); // FIXME broken memory allocation
	phy_vars_eNB[CC_id]->common_vars.txdata[0][i] = txdata[i];
	memset(txdata[i],0, frame_parms->samples_per_tti*10*sizeof(int32_t));
	printf("txdata[%d] @ %p\n", i, phy_vars_eNB[CC_id]->common_vars.txdata[0][i]);
      }
    }
  }

  return(0);
}


void reset_opp_meas(void) {

  int sfn;
  reset_meas(&softmodem_stats_mt);
  reset_meas(&softmodem_stats_hw);
  
  for (sfn=0; sfn < 10; sfn++) {
    reset_meas(&softmodem_stats_rxtx_sf);
    reset_meas(&softmodem_stats_rx_sf);
  }
}


void print_opp_meas(void) {

  int sfn=0;
  print_meas(&softmodem_stats_mt, "Main ENB Thread", NULL, NULL);
  print_meas(&softmodem_stats_hw, "HW Acquisation", NULL, NULL);
  
  for (sfn=0; sfn < 10; sfn++) {
    print_meas(&softmodem_stats_rxtx_sf,"[eNB][total_phy_proc_rxtx]",NULL, NULL);
    print_meas(&softmodem_stats_rx_sf,"[eNB][total_phy_proc_rx]",NULL,NULL);
  }
}
 
int start_if(PHY_VARS_eNB *eNB) {
  return(eNB->ifdevice.trx_start_func(&eNB->ifdevice));
}

int start_rf(PHY_VARS_eNB *eNB) {
  return(eNB->rfdevice.trx_start_func(&eNB->rfdevice));
}

extern void eNB_fep_rru_if5(PHY_VARS_eNB *eNB);
extern void eNB_fep_full(PHY_VARS_eNB *eNB);
extern void eNB_fep_full_2thread(PHY_VARS_eNB *eNB);
extern void do_prach(PHY_VARS_eNB *eNB);

void init_eNB(eNB_func_t node_function[], eNB_timing_t node_timing[],int nb_inst,eth_params_t *eth_params,int single_thread_flag) {
  
  int CC_id;
  int inst;
  PHY_VARS_eNB *eNB;
  int ret;

  for (inst=0;inst<nb_inst;inst++) {
    for (CC_id=0;CC_id<MAX_NUM_CCs;CC_id++) {
      eNB = PHY_vars_eNB_g[inst][CC_id]; 
      eNB->node_function      = node_function[CC_id];
      eNB->node_timing        = node_timing[CC_id];
      eNB->abstraction_flag   = 0;
      eNB->single_thread_flag = single_thread_flag;
#ifndef OCP_FRAMEWORK
      LOG_I(PHY,"Initializing eNB %d CC_id %d : (%s,%s)\n",inst,CC_id,eNB_functions[node_function[CC_id]],eNB_timing[node_timing[CC_id]]);
#endif

      switch (node_function[CC_id]) {
      case NGFI_RRU_IF5:
	eNB->do_prach             = NULL;
	eNB->fep                  = eNB_fep_rru_if5;
	eNB->td                   = NULL;
	eNB->te                   = NULL;
	eNB->proc_uespec_rx       = NULL;
	eNB->proc_tx              = NULL;
	eNB->tx_fh                = NULL;
	eNB->rx_fh                = rx_rf;
	eNB->start_rf             = start_rf;
	eNB->start_if             = start_if;
	eNB->fh_asynch            = fh_if5_asynch_DL;
	ret = openair0_device_load(&eNB->rfdevice, &openair0_cfg[CC_id]);
        if (ret<0) {
          printf("Exiting, cannot initialize rf device\n");
          exit(-1);
        }
	eNB->rfdevice.host_type   = RRH_HOST;
	eNB->ifdevice.host_type   = RRH_HOST;
        ret = openair0_transport_load(&eNB->ifdevice, &openair0_cfg[CC_id], (eth_params+CC_id));
	printf("openair0_transport_init returns %d for CC_id %d\n",ret,CC_id);
        if (ret<0) {
          printf("Exiting, cannot initialize transport protocol\n");
          exit(-1);
        }
	break;
      case NGFI_RRU_IF4p5:
	eNB->do_prach             = do_prach;
	eNB->fep                  = eNB_fep_full;//(single_thread_flag==1) ? eNB_fep_full_2thread : eNB_fep_full;
	eNB->td                   = NULL;
	eNB->te                   = NULL;
	eNB->proc_uespec_rx       = NULL;
	eNB->proc_tx              = NULL;//proc_tx_rru_if4p5;
	eNB->tx_fh                = NULL;
	eNB->rx_fh                = rx_rf;
	eNB->fh_asynch            = fh_if4p5_asynch_DL;
	eNB->start_rf             = start_rf;
	eNB->start_if             = start_if;
	ret = openair0_device_load(&eNB->rfdevice, &openair0_cfg[CC_id]);
        if (ret<0) {
          printf("Exiting, cannot initialize rf device\n");
          exit(-1);
        }
	eNB->rfdevice.host_type   = RRH_HOST;
	eNB->ifdevice.host_type   = RRH_HOST;
        ret = openair0_transport_load(&eNB->ifdevice, &openair0_cfg[CC_id], (eth_params+CC_id));
	printf("openair0_transport_init returns %d for CC_id %d\n",ret,CC_id);
        if (ret<0) {
          printf("Exiting, cannot initialize transport protocol\n");
          exit(-1);
        }

	malloc_IF4p5_buffer(eNB);

	break;
      case eNodeB_3GPP:
	eNB->do_prach             = do_prach;
	eNB->fep                  = eNB_fep_full;//(single_thread_flag==1) ? eNB_fep_full_2thread : eNB_fep_full;
	eNB->td                   = ulsch_decoding_data;//(single_thread_flag==1) ? ulsch_decoding_data_2thread : ulsch_decoding_data;
	eNB->te                   = dlsch_encoding;//(single_thread_flag==1) ? dlsch_encoding_2threads : dlsch_encoding;
	eNB->proc_uespec_rx       = phy_procedures_eNB_uespec_RX;
	eNB->proc_tx              = proc_tx_full;
	eNB->tx_fh                = NULL;
	eNB->rx_fh                = rx_rf;
	eNB->start_rf             = start_rf;
	eNB->start_if             = NULL;
        eNB->fh_asynch            = NULL;
	ret = openair0_device_load(&eNB->rfdevice, &openair0_cfg[CC_id]);
        if (ret<0) {
          printf("Exiting, cannot initialize rf device\n");
          exit(-1);
        }
	eNB->rfdevice.host_type   = BBU_HOST;
	eNB->ifdevice.host_type   = BBU_HOST;
	break;
      case eNodeB_3GPP_BBU:
	eNB->do_prach       = do_prach;
	eNB->fep            = eNB_fep_full;//(single_thread_flag==1) ? eNB_fep_full_2thread : eNB_fep_full;
	eNB->td             = ulsch_decoding_data;//(single_thread_flag==1) ? ulsch_decoding_data_2thread : ulsch_decoding_data;
	eNB->te             = dlsch_encoding;//(single_thread_flag==1) ? dlsch_encoding_2threads : dlsch_encoding;
	eNB->proc_uespec_rx = phy_procedures_eNB_uespec_RX;
	eNB->proc_tx        = proc_tx_full;
        if (eNB->node_timing == synch_to_other) {
           eNB->tx_fh          = tx_fh_if5_mobipass;
           eNB->rx_fh          = rx_fh_slave;
           eNB->fh_asynch      = fh_if5_asynch_UL;

        }
        else {
           eNB->tx_fh          = tx_fh_if5;
           eNB->rx_fh          = rx_fh_if5;
           eNB->fh_asynch      = NULL;
        }

	eNB->start_rf       = NULL;
	eNB->start_if       = start_if;
	eNB->rfdevice.host_type   = BBU_HOST;

	eNB->ifdevice.host_type   = BBU_HOST;

        ret = openair0_transport_load(&eNB->ifdevice, &openair0_cfg[CC_id], (eth_params+CC_id));
        printf("openair0_transport_init returns %d for CC_id %d\n",ret,CC_id);
        if (ret<0) {
          printf("Exiting, cannot initialize transport protocol\n");
          exit(-1);
        }
	break;
      case NGFI_RCC_IF4p5:
	eNB->do_prach             = do_prach;
	eNB->fep                  = NULL;
	eNB->td                   = ulsch_decoding_data;//(single_thread_flag==1) ? ulsch_decoding_data_2thread : ulsch_decoding_data;
	eNB->te                   = dlsch_encoding;//(single_thread_flag==1) ? dlsch_encoding_2threads : dlsch_encoding;
	eNB->proc_uespec_rx       = phy_procedures_eNB_uespec_RX;
	eNB->proc_tx              = proc_tx_high;
	eNB->tx_fh                = tx_fh_if4p5;
	eNB->rx_fh                = rx_fh_if4p5;
	eNB->start_rf             = NULL;
	eNB->start_if             = start_if;
        eNB->fh_asynch            = (eNB->node_timing == synch_to_other) ? fh_if4p5_asynch_UL : NULL;
	eNB->rfdevice.host_type   = BBU_HOST;
	eNB->ifdevice.host_type   = BBU_HOST;
        ret = openair0_transport_load(&eNB->ifdevice, &openair0_cfg[CC_id], (eth_params+CC_id));
        printf("openair0_transport_init returns %d for CC_id %d\n",ret,CC_id);
        if (ret<0) {
          printf("Exiting, cannot initialize transport protocol\n");
          exit(-1);
        }
	malloc_IF4p5_buffer(eNB);

	break;
      case NGFI_RAU_IF4p5:
	eNB->do_prach       = do_prach;
	eNB->fep            = NULL;

	eNB->td             = ulsch_decoding_data;//(single_thread_flag==1) ? ulsch_decoding_data_2thread : ulsch_decoding_data;
	eNB->te             = dlsch_encoding;//(single_thread_flag==1) ? dlsch_encoding_2threads : dlsch_encoding;
	eNB->proc_uespec_rx = phy_procedures_eNB_uespec_RX;
	eNB->proc_tx        = proc_tx_high;
	eNB->tx_fh          = tx_fh_if4p5; 
	eNB->rx_fh          = rx_fh_if4p5; 
        eNB->fh_asynch      = (eNB->node_timing == synch_to_other) ? fh_if4p5_asynch_UL : NULL;
	eNB->start_rf       = NULL;
	eNB->start_if       = start_if;

	eNB->rfdevice.host_type   = BBU_HOST;
	eNB->ifdevice.host_type   = BBU_HOST;
        ret = openair0_transport_load(&eNB->ifdevice, &openair0_cfg[CC_id], (eth_params+CC_id));
        printf("openair0_transport_init returns %d for CC_id %d\n",ret,CC_id);
        if (ret<0) {
          printf("Exiting, cannot initialize transport protocol\n");
          exit(-1);
        } 
	break;	
	malloc_IF4p5_buffer(eNB);

      }
    }

    if (setup_eNB_buffers(PHY_vars_eNB_g[inst],&openair0_cfg[CC_id])!=0) {
      printf("Exiting, cannot initialize eNodeB Buffers\n");
      exit(-1);
    }

    init_eNB_proc(inst);
  }

  sleep(1);
  LOG_D(HW,"[lte-softmodem.c] eNB threads created\n");
  

}


void stop_eNB(int nb_inst) {

  for (int inst=0;inst<nb_inst;inst++) {
    printf("Killing eNB %d processing threads\n",inst);
    kill_eNB_proc(inst);
  }
}
