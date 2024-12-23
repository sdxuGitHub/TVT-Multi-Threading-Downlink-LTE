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

/*! \file vcd_signal_dumper.h
 * \brief Output functions call to VCD file which is readable by gtkwave.
 * \author ROUX Sebastien
 * \author S. Roux
 * \maintainer: navid nikaein
 * \date 2012 - 2104
 * \version 0.1
 * \company Eurecom
 * \email: navid.nikaein@eurecom.fr
 * \note
 * \warning
 */

#ifndef VCD_SIGNAL_DUMPER_H_
#define VCD_SIGNAL_DUMPER_H_

//#define ENABLE_USE_CPU_EXECUTION_TIME

/* WARNING: if you edit the enums below, update also string definitions in vcd_signal_dumper.c */
typedef enum {
  VCD_SIGNAL_DUMPER_VARIABLES_ENB_UL_C01,
  VCD_SIGNAL_DUMPER_VARIABLES_ENB_UL_C02,
  VCD_SIGNAL_DUMPER_VARIABLES_ENB_UL_C03,
  VCD_SIGNAL_DUMPER_VARIABLES_ENB_UL_C04,
  VCD_SIGNAL_DUMPER_VARIABLES_ENB_UL_C05,
  VCD_SIGNAL_DUMPER_VARIABLES_ENB_UL_C06,
  VCD_SIGNAL_DUMPER_VARIABLES_ENB_UL_C07,
  VCD_SIGNAL_DUMPER_VARIABLES_ENB_UL_C08,
  VCD_SIGNAL_DUMPER_VARIABLES_ENB_UL_C09,
  VCD_SIGNAL_DUMPER_VARIABLES_ENB_UL_C10,
  VCD_SIGNAL_DUMPER_VARIABLES_ENB_UL_C11,
  VCD_SIGNAL_DUMPER_VARIABLES_ENB_UL_C12,
  VCD_SIGNAL_DUMPER_VARIABLES_ENB_UL_C13,
  VCD_SIGNAL_DUMPER_VARIABLES_ENB_UL_C14,
  VCD_SIGNAL_DUMPER_VARIABLES_ENB_UL_C15,
  VCD_SIGNAL_DUMPER_VARIABLES_ENB_UL_C16,
  VCD_SIGNAL_DUMPER_VARIABLES_ENB_UL_mcs01,
  VCD_SIGNAL_DUMPER_VARIABLES_ENB_UL_mcs02,
  VCD_SIGNAL_DUMPER_VARIABLES_ENB_UL_mcs03,
  VCD_SIGNAL_DUMPER_VARIABLES_ENB_UL_mcs04,
  VCD_SIGNAL_DUMPER_VARIABLES_ENB_UL_mcs05,
  VCD_SIGNAL_DUMPER_VARIABLES_ENB_UL_mcs06,
  VCD_SIGNAL_DUMPER_VARIABLES_ENB_UL_mcs07,
  VCD_SIGNAL_DUMPER_VARIABLES_ENB_UL_mcs08,
  VCD_SIGNAL_DUMPER_VARIABLES_ENB_UL_mcs09,
  VCD_SIGNAL_DUMPER_VARIABLES_ENB_UL_mcs10,
  VCD_SIGNAL_DUMPER_VARIABLES_ENB_UL_mcs11,
  VCD_SIGNAL_DUMPER_VARIABLES_ENB_UL_mcs12,
  VCD_SIGNAL_DUMPER_VARIABLES_ENB_UL_mcs13,
  VCD_SIGNAL_DUMPER_VARIABLES_ENB_UL_mcs14,
  VCD_SIGNAL_DUMPER_VARIABLES_ENB_UL_mcs15,
  VCD_SIGNAL_DUMPER_VARIABLES_ENB_UL_mcs16,
  VCD_SIGNAL_DUMPER_VARIABLES_ENB_UL_RB01,
  VCD_SIGNAL_DUMPER_VARIABLES_ENB_UL_RB02,
  VCD_SIGNAL_DUMPER_VARIABLES_ENB_UL_RB03,
  VCD_SIGNAL_DUMPER_VARIABLES_ENB_UL_RB04,
  VCD_SIGNAL_DUMPER_VARIABLES_ENB_UL_RB05,
  VCD_SIGNAL_DUMPER_VARIABLES_ENB_UL_RB06,
  VCD_SIGNAL_DUMPER_VARIABLES_ENB_UL_RB07,
  VCD_SIGNAL_DUMPER_VARIABLES_ENB_UL_RB08,
  VCD_SIGNAL_DUMPER_VARIABLES_ENB_UL_RB09,
  VCD_SIGNAL_DUMPER_VARIABLES_ENB_UL_RB10,
  VCD_SIGNAL_DUMPER_VARIABLES_ENB_UL_RB11,
  VCD_SIGNAL_DUMPER_VARIABLES_ENB_UL_RB12,
  VCD_SIGNAL_DUMPER_VARIABLES_ENB_UL_RB13,
  VCD_SIGNAL_DUMPER_VARIABLES_ENB_UL_RB14,
  VCD_SIGNAL_DUMPER_VARIABLES_ENB_UL_RB15,
  VCD_SIGNAL_DUMPER_VARIABLES_ENB_UL_RB16,
  VCD_SIGNAL_DUMPER_VARIABLES_ENB_MAC_ACTIVE_UE,
  VCD_SIGNAL_DUMPER_VARIABLES_ENB_RB01,
  VCD_SIGNAL_DUMPER_VARIABLES_ENB_RB02,
  VCD_SIGNAL_DUMPER_VARIABLES_ENB_RB03,
  VCD_SIGNAL_DUMPER_VARIABLES_ENB_RB04,
  VCD_SIGNAL_DUMPER_VARIABLES_ENB_RB05,
  VCD_SIGNAL_DUMPER_VARIABLES_ENB_RB06,
  VCD_SIGNAL_DUMPER_VARIABLES_ENB_RB07,
  VCD_SIGNAL_DUMPER_VARIABLES_ENB_RB08,
  VCD_SIGNAL_DUMPER_VARIABLES_ENB_RB09,
  VCD_SIGNAL_DUMPER_VARIABLES_ENB_RB10,
  VCD_SIGNAL_DUMPER_VARIABLES_ENB_RB11,
  VCD_SIGNAL_DUMPER_VARIABLES_ENB_RB12,
  VCD_SIGNAL_DUMPER_VARIABLES_ENB_RB13,
  VCD_SIGNAL_DUMPER_VARIABLES_ENB_RB14,
  VCD_SIGNAL_DUMPER_VARIABLES_ENB_RB15,
  VCD_SIGNAL_DUMPER_VARIABLES_ENB_RB16,
  VCD_SIGNAL_DUMPER_VARIABLES_FRAME_NUMBER_TX0_ENB = 0,
  VCD_SIGNAL_DUMPER_VARIABLES_FRAME_NUMBER_TX1_ENB,
  VCD_SIGNAL_DUMPER_VARIABLES_FRAME_NUMBER_RX0_ENB,
  VCD_SIGNAL_DUMPER_VARIABLES_FRAME_NUMBER_RX1_ENB,
  VCD_SIGNAL_DUMPER_VARIABLES_SUBFRAME_NUMBER_TX0_ENB,
  VCD_SIGNAL_DUMPER_VARIABLES_SUBFRAME_NUMBER_TX1_ENB,
  VCD_SIGNAL_DUMPER_VARIABLES_SUBFRAME_NUMBER_RX0_ENB,
  VCD_SIGNAL_DUMPER_VARIABLES_SUBFRAME_NUMBER_RX1_ENB,
  VCD_SIGNAL_DUMPER_VARIABLES_RUNTIME_TX_ENB,
  VCD_SIGNAL_DUMPER_VARIABLES_RUNTIME_RX_ENB,
  VCD_SIGNAL_DUMPER_VARIABLES_FRAME_NUMBER_TX0_UE,
  VCD_SIGNAL_DUMPER_VARIABLES_FRAME_NUMBER_TX1_UE,
  VCD_SIGNAL_DUMPER_VARIABLES_FRAME_NUMBER_RX0_UE,
  VCD_SIGNAL_DUMPER_VARIABLES_FRAME_NUMBER_RX1_UE,
  VCD_SIGNAL_DUMPER_VARIABLES_SUBFRAME_NUMBER_TX0_UE,
  VCD_SIGNAL_DUMPER_VARIABLES_SUBFRAME_NUMBER_TX1_UE,
  VCD_SIGNAL_DUMPER_VARIABLES_SUBFRAME_NUMBER_RX0_UE,
  VCD_SIGNAL_DUMPER_VARIABLES_SUBFRAME_NUMBER_RX1_UE,
  VCD_SIGNAL_DUMPER_VARIABLES_MISSED_SLOTS_ENB,
  VCD_SIGNAL_DUMPER_VARIABLES_DAQ_MBOX,
  VCD_SIGNAL_DUMPER_VARIABLES_UE_OFFSET_MBOX,
  VCD_SIGNAL_DUMPER_VARIABLES_UE_RX_OFFSET,
  VCD_SIGNAL_DUMPER_VARIABLES_DIFF,
  VCD_SIGNAL_DUMPER_VARIABLES_HW_SUBFRAME,
  VCD_SIGNAL_DUMPER_VARIABLES_HW_FRAME,
  VCD_SIGNAL_DUMPER_VARIABLES_HW_SUBFRAME_RX,
  VCD_SIGNAL_DUMPER_VARIABLES_HW_FRAME_RX,
  VCD_SIGNAL_DUMPER_VARIABLES_TXCNT,
  VCD_SIGNAL_DUMPER_VARIABLES_RXCNT,
  VCD_SIGNAL_DUMPER_VARIABLES_TRX_TS,
  VCD_SIGNAL_DUMPER_VARIABLES_TRX_TST,
  VCD_SIGNAL_DUMPER_VARIABLES_TX_TS,
  VCD_SIGNAL_DUMPER_VARIABLES_RX_TS,
  VCD_SIGNAL_DUMPER_VARIABLES_RX_HWCNT,
  VCD_SIGNAL_DUMPER_VARIABLES_RX_LHWCNT,
  VCD_SIGNAL_DUMPER_VARIABLES_TX_HWCNT,
  VCD_SIGNAL_DUMPER_VARIABLES_TX_LHWCNT,
  VCD_SIGNAL_DUMPER_VARIABLES_RX_PCK,
  VCD_SIGNAL_DUMPER_VARIABLES_TX_PCK,
  VCD_SIGNAL_DUMPER_VARIABLES_RX_SEQ_NUM,
  VCD_SIGNAL_DUMPER_VARIABLES_RX_SEQ_NUM_PRV,
  VCD_SIGNAL_DUMPER_VARIABLES_TX_SEQ_NUM,
  VCD_SIGNAL_DUMPER_VARIABLES_CNT,
  VCD_SIGNAL_DUMPER_VARIABLES_DUMMY_DUMP,
  VCD_SIGNAL_DUMPER_VARIABLE_ITTI_SEND_MSG,
  VCD_SIGNAL_DUMPER_VARIABLE_ITTI_POLL_MSG,
  VCD_SIGNAL_DUMPER_VARIABLE_ITTI_RECV_MSG,
  VCD_SIGNAL_DUMPER_VARIABLE_ITTI_ALLOC_MSG,
  VCD_SIGNAL_DUMPER_VARIABLE_MP_ALLOC,
  VCD_SIGNAL_DUMPER_VARIABLE_MP_FREE,
  VCD_SIGNAL_DUMPER_VARIABLES_UE_INST_CNT_RX,
  VCD_SIGNAL_DUMPER_VARIABLES_UE_INST_CNT_TX,
  VCD_SIGNAL_DUMPER_VARIABLES_DCI_INFO,
  VCD_SIGNAL_DUMPER_VARIABLES_UE0_BSR,
  VCD_SIGNAL_DUMPER_VARIABLES_UE0_BO,
  VCD_SIGNAL_DUMPER_VARIABLES_UE0_SCHEDULED,
  VCD_SIGNAL_DUMPER_VARIABLES_UE0_TIMING_ADVANCE,
  VCD_SIGNAL_DUMPER_VARIABLES_UE0_SR_ENERGY,
  VCD_SIGNAL_DUMPER_VARIABLES_UE0_SR_THRES,
  VCD_SIGNAL_DUMPER_VARIABLES_UE0_RSSI0,
  VCD_SIGNAL_DUMPER_VARIABLES_UE0_RSSI1,
  VCD_SIGNAL_DUMPER_VARIABLES_UE0_RSSI2,
  VCD_SIGNAL_DUMPER_VARIABLES_UE0_RSSI3,
  VCD_SIGNAL_DUMPER_VARIABLES_UE0_RSSI4,
  VCD_SIGNAL_DUMPER_VARIABLES_UE0_RSSI5,
  VCD_SIGNAL_DUMPER_VARIABLES_UE0_RSSI6,
  VCD_SIGNAL_DUMPER_VARIABLES_UE0_RSSI7,
  VCD_SIGNAL_DUMPER_VARIABLES_UE0_RES0,
  VCD_SIGNAL_DUMPER_VARIABLES_UE0_RES1,
  VCD_SIGNAL_DUMPER_VARIABLES_UE0_RES2,
  VCD_SIGNAL_DUMPER_VARIABLES_UE0_RES3,
  VCD_SIGNAL_DUMPER_VARIABLES_UE0_RES4,
  VCD_SIGNAL_DUMPER_VARIABLES_UE0_RES5,
  VCD_SIGNAL_DUMPER_VARIABLES_UE0_RES6,
  VCD_SIGNAL_DUMPER_VARIABLES_UE0_RES7,
  VCD_SIGNAL_DUMPER_VARIABLES_UE0_MCS0,
  VCD_SIGNAL_DUMPER_VARIABLES_UE0_MCS1,
  VCD_SIGNAL_DUMPER_VARIABLES_UE0_MCS2,
  VCD_SIGNAL_DUMPER_VARIABLES_UE0_MCS3,
  VCD_SIGNAL_DUMPER_VARIABLES_UE0_MCS4,
  VCD_SIGNAL_DUMPER_VARIABLES_UE0_MCS5,
  VCD_SIGNAL_DUMPER_VARIABLES_UE0_MCS6,
  VCD_SIGNAL_DUMPER_VARIABLES_UE0_MCS7,
  VCD_SIGNAL_DUMPER_VARIABLES_UE0_RB0,
  VCD_SIGNAL_DUMPER_VARIABLES_UE0_RB1,
  VCD_SIGNAL_DUMPER_VARIABLES_UE0_RB2,
  VCD_SIGNAL_DUMPER_VARIABLES_UE0_RB3,
  VCD_SIGNAL_DUMPER_VARIABLES_UE0_RB4,
  VCD_SIGNAL_DUMPER_VARIABLES_UE0_RB5,
  VCD_SIGNAL_DUMPER_VARIABLES_UE0_RB6,
  VCD_SIGNAL_DUMPER_VARIABLES_UE0_RB7,
  VCD_SIGNAL_DUMPER_VARIABLES_UE0_ROUND0,
  VCD_SIGNAL_DUMPER_VARIABLES_UE0_ROUND1,
  VCD_SIGNAL_DUMPER_VARIABLES_UE0_ROUND2,
  VCD_SIGNAL_DUMPER_VARIABLES_UE0_ROUND3,
  VCD_SIGNAL_DUMPER_VARIABLES_UE0_ROUND4,
  VCD_SIGNAL_DUMPER_VARIABLES_UE0_ROUND5,
  VCD_SIGNAL_DUMPER_VARIABLES_UE0_ROUND6,
  VCD_SIGNAL_DUMPER_VARIABLES_UE0_ROUND7,
  VCD_SIGNAL_DUMPER_VARIABLES_UE0_SFN0,
  VCD_SIGNAL_DUMPER_VARIABLES_UE0_SFN1,
  VCD_SIGNAL_DUMPER_VARIABLES_UE0_SFN2,
  VCD_SIGNAL_DUMPER_VARIABLES_UE0_SFN3,
  VCD_SIGNAL_DUMPER_VARIABLES_UE0_SFN4,
  VCD_SIGNAL_DUMPER_VARIABLES_UE0_SFN5,
  VCD_SIGNAL_DUMPER_VARIABLES_UE0_SFN6,
  VCD_SIGNAL_DUMPER_VARIABLES_UE0_SFN7,
  VCD_SIGNAL_DUMPER_VARIABLES_LAST,
  VCD_SIGNAL_DUMPER_VARIABLES_END = VCD_SIGNAL_DUMPER_VARIABLES_LAST,
} vcd_signal_dump_variables;

typedef enum {
  /* softmodem signals  */
  VCD_SIGNAL_DUMPER_FUNCTIONS_RT_SLEEP=0,
  VCD_SIGNAL_DUMPER_FUNCTIONS_TRX_READ,
  VCD_SIGNAL_DUMPER_FUNCTIONS_TRX_WRITE,
  VCD_SIGNAL_DUMPER_FUNCTIONS_TRX_READ_IF,
  VCD_SIGNAL_DUMPER_FUNCTIONS_TRX_WRITE_IF,
  VCD_SIGNAL_DUMPER_FUNCTIONS_eNB_PROC_RXTX0,
  VCD_SIGNAL_DUMPER_FUNCTIONS_eNB_PROC_RXTX1,
  VCD_SIGNAL_DUMPER_FUNCTIONS_UE_THREAD_SYNCH,
  VCD_SIGNAL_DUMPER_FUNCTIONS_UE_THREAD_RXTX0,
  VCD_SIGNAL_DUMPER_FUNCTIONS_UE_THREAD_RXTX1,

  /* RRH signals  */ 
  VCD_SIGNAL_DUMPER_FUNCTIONS_eNB_TX,
  VCD_SIGNAL_DUMPER_FUNCTIONS_eNB_RX,
  VCD_SIGNAL_DUMPER_FUNCTIONS_eNB_TRX,
  VCD_SIGNAL_DUMPER_FUNCTIONS_eNB_TM,
  VCD_SIGNAL_DUMPER_FUNCTIONS_eNB_RX_SLEEP,
  VCD_SIGNAL_DUMPER_FUNCTIONS_eNB_TX_SLEEP,
  VCD_SIGNAL_DUMPER_FUNCTIONS_eNB_PROC_SLEEP,
  VCD_SIGNAL_DUMPER_FUNCTIONS_TRX_READ_RF,
  VCD_SIGNAL_DUMPER_FUNCTIONS_TRX_WRITE_RF,

  /* PHY signals  */
  VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_ENB_CONTROL_CHANNEL_THREAD_TX,
  VCD_SIGNAL_DUMPER_FUNCTIONS_UE_SYNCH,
  VCD_SIGNAL_DUMPER_FUNCTIONS_UE_SLOT_FEP,
  VCD_SIGNAL_DUMPER_FUNCTIONS_UE_RRC_MEASUREMENTS,
  VCD_SIGNAL_DUMPER_FUNCTIONS_UE_GAIN_CONTROL,
  VCD_SIGNAL_DUMPER_FUNCTIONS_UE_ADJUST_SYNCH,
  VCD_SIGNAL_DUMPER_FUNCTIONS_UE_MEASUREMENT_PROCEDURES,
  VCD_SIGNAL_DUMPER_FUNCTIONS_UE_PDCCH_PROCEDURES,
  VCD_SIGNAL_DUMPER_FUNCTIONS_UE_PBCH_PROCEDURES,
  VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_PROCEDURES_ENB_TX,
  VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_PROCEDURES_ENB_TX1,
  VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_PROCEDURES_ENB_RX_COMMON,
  VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_PROCEDURES_ENB_RX_COMMON1,
  VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_PROCEDURES_ENB_RX_UESPEC,
  VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_PROCEDURES_ENB_RX_UESPEC1,
  VCD_SIGNAL_DUMPER_FUNCTIONS_ENB_SLOT_FEP,
  VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_PROCEDURES_UE_TX,
  VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_PROCEDURES_UE_RX,
  VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_PROCEDURES_ENB_LTE,
  VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_PROCEDURES_UE_LTE,
  VCD_SIGNAL_DUMPER_FUNCTIONS_PDSCH_THREAD,
  VCD_SIGNAL_DUMPER_FUNCTIONS_DLSCH_THREAD0,
  VCD_SIGNAL_DUMPER_FUNCTIONS_DLSCH_THREAD1,
  VCD_SIGNAL_DUMPER_FUNCTIONS_DLSCH_THREAD2,
  VCD_SIGNAL_DUMPER_FUNCTIONS_DLSCH_THREAD3,
  VCD_SIGNAL_DUMPER_FUNCTIONS_DLSCH_THREAD4,
  VCD_SIGNAL_DUMPER_FUNCTIONS_DLSCH_THREAD5,
  VCD_SIGNAL_DUMPER_FUNCTIONS_DLSCH_THREAD6,
  VCD_SIGNAL_DUMPER_FUNCTIONS_DLSCH_THREAD7,
  VCD_SIGNAL_DUMPER_FUNCTIONS_DLSCH_DECODING0,
  VCD_SIGNAL_DUMPER_FUNCTIONS_DLSCH_DECODING1,
  VCD_SIGNAL_DUMPER_FUNCTIONS_DLSCH_DECODING2,
  VCD_SIGNAL_DUMPER_FUNCTIONS_DLSCH_DECODING3,
  VCD_SIGNAL_DUMPER_FUNCTIONS_DLSCH_DECODING4,
  VCD_SIGNAL_DUMPER_FUNCTIONS_DLSCH_DECODING5,
  VCD_SIGNAL_DUMPER_FUNCTIONS_DLSCH_DECODING6,
  VCD_SIGNAL_DUMPER_FUNCTIONS_DLSCH_DECODING7,
  VCD_SIGNAL_DUMPER_FUNCTIONS_RX_PDCCH,
  VCD_SIGNAL_DUMPER_FUNCTIONS_DCI_DECODING,
  VCD_SIGNAL_DUMPER_FUNCTIONS_RX_PHICH,
  VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_UE_CONFIG_SIB2,
  VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_CONFIG_SIB1_ENB,
  VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_CONFIG_SIB2_ENB,
  VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_CONFIG_DEDICATED_ENB,
  VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_UE_COMPUTE_PRACH,
  VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_ENB_ULSCH_MSG3,
  VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_ENB_ULSCH_DECODING0,
  VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_ENB_ULSCH_DECODING1,
  VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_ENB_ULSCH_DECODING2,
  VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_ENB_ULSCH_DECODING3,
  VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_ENB_ULSCH_DECODING4,
  VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_ENB_ULSCH_DECODING5,
  VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_ENB_ULSCH_DECODING6,
  VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_ENB_ULSCH_DECODING7,
  VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_ENB_SFGEN,
  VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_ENB_PRACH_RX,
  VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_ENB_PDCCH_TX,
  VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_ENB_RS_TX,
  VCD_SIGNAL_DUMPER_FUNCTIONS_UE_GENERATE_PRACH,
  VCD_SIGNAL_DUMPER_FUNCTIONS_UE_ULSCH_MODULATION,
  VCD_SIGNAL_DUMPER_FUNCTIONS_UE_ULSCH_ENCODING,
  VCD_SIGNAL_DUMPER_FUNCTIONS_UE_ULSCH_SCRAMBLING,
  VCD_SIGNAL_DUMPER_FUNCTIONS_ENB_DLSCH_MODULATION,
  VCD_SIGNAL_DUMPER_FUNCTIONS_ENB_DLSCH_ENCODING,
  VCD_SIGNAL_DUMPER_FUNCTIONS_ENB_DLSCH_ENCODING_W,
  VCD_SIGNAL_DUMPER_FUNCTIONS_ENB_DLSCH_SCRAMBLING,

  /* MAC signals  */
  VCD_SIGNAL_DUMPER_FUNCTIONS_MACPHY_INIT,
  VCD_SIGNAL_DUMPER_FUNCTIONS_MACPHY_EXIT,
  VCD_SIGNAL_DUMPER_FUNCTIONS_ENB_DLSCH_ULSCH_SCHEDULER,
  VCD_SIGNAL_DUMPER_FUNCTIONS_FILL_RAR,
  VCD_SIGNAL_DUMPER_FUNCTIONS_TERMINATE_RA_PROC,
  VCD_SIGNAL_DUMPER_FUNCTIONS_INITIATE_RA_PROC,
  VCD_SIGNAL_DUMPER_FUNCTIONS_CANCEL_RA_PROC,
  VCD_SIGNAL_DUMPER_FUNCTIONS_GET_DCI_SDU,
  VCD_SIGNAL_DUMPER_FUNCTIONS_GET_DLSCH_SDU,
  VCD_SIGNAL_DUMPER_FUNCTIONS_RX_SDU,
  VCD_SIGNAL_DUMPER_FUNCTIONS_MRBCH_PHY_SYNC_FAILURE,
  VCD_SIGNAL_DUMPER_FUNCTIONS_SR_INDICATION,
  VCD_SIGNAL_DUMPER_FUNCTIONS_DLSCH_PREPROCESSOR,
  VCD_SIGNAL_DUMPER_FUNCTIONS_SCHEDULE_DLSCH, // schedule_ue_spec
  VCD_SIGNAL_DUMPER_FUNCTIONS_FILL_DLSCH_DCI,

  VCD_SIGNAL_DUMPER_FUNCTIONS_OUT_OF_SYNC_IND,
  VCD_SIGNAL_DUMPER_FUNCTIONS_UE_DECODE_SI,
  VCD_SIGNAL_DUMPER_FUNCTIONS_UE_DECODE_PCCH,
  VCD_SIGNAL_DUMPER_FUNCTIONS_UE_DECODE_CCCH,
  VCD_SIGNAL_DUMPER_FUNCTIONS_UE_DECODE_BCCH,
  VCD_SIGNAL_DUMPER_FUNCTIONS_UE_SEND_SDU,
  VCD_SIGNAL_DUMPER_FUNCTIONS_UE_GET_SDU,
  VCD_SIGNAL_DUMPER_FUNCTIONS_UE_GET_RACH,
  VCD_SIGNAL_DUMPER_FUNCTIONS_UE_PROCESS_RAR,
  VCD_SIGNAL_DUMPER_FUNCTIONS_UE_SCHEDULER,
  VCD_SIGNAL_DUMPER_FUNCTIONS_UE_GET_SR,
  VCD_SIGNAL_DUMPER_FUNCTIONS_UE_SEND_MCH_SDU,

  /* RLC signals  */
  VCD_SIGNAL_DUMPER_FUNCTIONS_RLC_DATA_REQ,
  // VCD_SIGNAL_DUMPER_FUNCTIONS_RLC_DATA_IND,
  VCD_SIGNAL_DUMPER_FUNCTIONS_MAC_RLC_STATUS_IND,
  VCD_SIGNAL_DUMPER_FUNCTIONS_MAC_RLC_DATA_REQ,
  VCD_SIGNAL_DUMPER_FUNCTIONS_MAC_RLC_DATA_IND,
  VCD_SIGNAL_DUMPER_FUNCTIONS_RLC_UM_TRY_REASSEMBLY,
  VCD_SIGNAL_DUMPER_FUNCTIONS_RLC_UM_CHECK_TIMER_DAR_TIME_OUT,
  VCD_SIGNAL_DUMPER_FUNCTIONS_RLC_UM_RECEIVE_PROCESS_DAR,

  /* PDCP signals  */
  VCD_SIGNAL_DUMPER_FUNCTIONS_PDCP_RUN,
  VCD_SIGNAL_DUMPER_FUNCTIONS_PDCP_DATA_REQ,
  VCD_SIGNAL_DUMPER_FUNCTIONS_PDCP_DATA_IND,
  VCD_SIGNAL_DUMPER_FUNCTIONS_PDCP_APPLY_SECURITY,
  VCD_SIGNAL_DUMPER_FUNCTIONS_PDCP_VALIDATE_SECURITY,

  /* RRC signals  */
  VCD_SIGNAL_DUMPER_FUNCTIONS_RRC_RX_TX,
  VCD_SIGNAL_DUMPER_FUNCTIONS_RRC_MAC_CONFIG,
  VCD_SIGNAL_DUMPER_FUNCTIONS_RRC_UE_DECODE_SIB1,
  VCD_SIGNAL_DUMPER_FUNCTIONS_RRC_UE_DECODE_SI,

  /* GTPV1U signals */
  VCD_SIGNAL_DUMPER_FUNCTIONS_GTPV1U_ENB_TASK,
  VCD_SIGNAL_DUMPER_FUNCTIONS_GTPV1U_PROCESS_UDP_REQ,
  VCD_SIGNAL_DUMPER_FUNCTIONS_GTPV1U_PROCESS_TUNNEL_DATA_REQ,

  /* UDP signals */
  VCD_SIGNAL_DUMPER_FUNCTIONS_UDP_ENB_TASK,

  /* MISC signals  */
  VCD_SIGNAL_DUMPER_FUNCTIONS_EMU_TRANSPORT,
  VCD_SIGNAL_DUMPER_FUNCTIONS_LOG_RECORD,
  VCD_SIGNAL_DUMPER_FUNCTIONS_ITTI_ENQUEUE_MESSAGE,
  VCD_SIGNAL_DUMPER_FUNCTIONS_ITTI_DUMP_ENQUEUE_MESSAGE,
  VCD_SIGNAL_DUMPER_FUNCTIONS_ITTI_DUMP_ENQUEUE_MESSAGE_MALLOC,
  VCD_SIGNAL_DUMPER_FUNCTIONS_ITTI_RELAY_THREAD,
  VCD_SIGNAL_DUMPER_FUNCTIONS_TEST,
  
  /*test*/
  VCD_SIGNAL_DUMPER_FUNCTIONS_TX01,
  VCD_SIGNAL_DUMPER_FUNCTIONS_TX02,
  VCD_SIGNAL_DUMPER_FUNCTIONS_TX03,
  VCD_SIGNAL_DUMPER_FUNCTIONS_TX04,
  
  VCD_SIGNAL_DUMPER_FUNCTIONS_WORK_TX01,
  VCD_SIGNAL_DUMPER_FUNCTIONS_WORK_TX02,
  VCD_SIGNAL_DUMPER_FUNCTIONS_WORK_TX03,
  VCD_SIGNAL_DUMPER_FUNCTIONS_WORK_TX04,
  
  //Before PUCCH/PUSCH
   VCD_SIGNAL_DUMPER_FUNCTIONS_BEFORE_PUCCH,
  //PUCCH/PUSCH
  VCD_SIGNAL_DUMPER_FUNCTIONS_PUCCH,
  VCD_SIGNAL_DUMPER_FUNCTIONS_PUSCH,
  //decoding_para
  VCD_SIGNAL_DUMPER_FUNCTIONS_decoding_para01,
  VCD_SIGNAL_DUMPER_FUNCTIONS_decoding_para02,
  VCD_SIGNAL_DUMPER_FUNCTIONS_decoding_para03,
  VCD_SIGNAL_DUMPER_FUNCTIONS_decoding_para04,
  VCD_SIGNAL_DUMPER_FUNCTIONS_decoding_para05,
  VCD_SIGNAL_DUMPER_FUNCTIONS_decoding_para06,
  VCD_SIGNAL_DUMPER_FUNCTIONS_decoding_para07,
  VCD_SIGNAL_DUMPER_FUNCTIONS_decoding_para08,
  VCD_SIGNAL_DUMPER_FUNCTIONS_decoding_para09,
  VCD_SIGNAL_DUMPER_FUNCTIONS_decoding_para10,
  VCD_SIGNAL_DUMPER_FUNCTIONS_decoding_para11,
  VCD_SIGNAL_DUMPER_FUNCTIONS_decoding_para12,
  VCD_SIGNAL_DUMPER_FUNCTIONS_decoding_para13,
  VCD_SIGNAL_DUMPER_FUNCTIONS_decoding_para14,
  VCD_SIGNAL_DUMPER_FUNCTIONS_decoding_para15,
  VCD_SIGNAL_DUMPER_FUNCTIONS_decoding_para16,
  //PHY TO MAC
  VCD_SIGNAL_DUMPER_FUNCTIONS_p2m,
  //viterbi
  VCD_SIGNAL_DUMPER_FUNCTIONS_viterbi01,
  VCD_SIGNAL_DUMPER_FUNCTIONS_viterbi02,
  VCD_SIGNAL_DUMPER_FUNCTIONS_viterbi03,
  VCD_SIGNAL_DUMPER_FUNCTIONS_viterbi04,
  VCD_SIGNAL_DUMPER_FUNCTIONS_viterbi05,
  VCD_SIGNAL_DUMPER_FUNCTIONS_viterbi06,
  VCD_SIGNAL_DUMPER_FUNCTIONS_viterbi07,
  VCD_SIGNAL_DUMPER_FUNCTIONS_viterbi08,
  VCD_SIGNAL_DUMPER_FUNCTIONS_viterbi09,
  VCD_SIGNAL_DUMPER_FUNCTIONS_viterbi10,
  VCD_SIGNAL_DUMPER_FUNCTIONS_viterbi11,
  VCD_SIGNAL_DUMPER_FUNCTIONS_viterbi12,
  VCD_SIGNAL_DUMPER_FUNCTIONS_viterbi13,
  VCD_SIGNAL_DUMPER_FUNCTIONS_viterbi14,
  VCD_SIGNAL_DUMPER_FUNCTIONS_viterbi15,
  VCD_SIGNAL_DUMPER_FUNCTIONS_viterbi16,
  //turbo
  VCD_SIGNAL_DUMPER_FUNCTIONS_turbo,
  VCD_SIGNAL_DUMPER_FUNCTIONS_turbo01,
  VCD_SIGNAL_DUMPER_FUNCTIONS_turbo02,
  VCD_SIGNAL_DUMPER_FUNCTIONS_turbo03,
  VCD_SIGNAL_DUMPER_FUNCTIONS_turbo04,
  
  VCD_SIGNAL_DUMPER_FUNCTIONS_WORK_turbo01,
  VCD_SIGNAL_DUMPER_FUNCTIONS_WORK_turbo02,
  VCD_SIGNAL_DUMPER_FUNCTIONS_WORK_turbo03,
  VCD_SIGNAL_DUMPER_FUNCTIONS_WORK_turbo04,
  //Other
  VCD_SIGNAL_DUMPER_FUNCTIONS_more_than_10RB,
  
  /* IF4/IF5 signals */
  VCD_SIGNAL_DUMPER_FUNCTIONS_SEND_IF4,
  VCD_SIGNAL_DUMPER_FUNCTIONS_RECV_IF4,
  VCD_SIGNAL_DUMPER_FUNCTIONS_SEND_IF5,
  VCD_SIGNAL_DUMPER_FUNCTIONS_RECV_IF5,

  VCD_SIGNAL_DUMPER_FUNCTIONS_LAST,
  VCD_SIGNAL_DUMPER_FUNCTIONS_END = VCD_SIGNAL_DUMPER_FUNCTIONS_LAST,
} vcd_signal_dump_functions;

typedef enum {
  VCD_SIGNAL_DUMPER_MODULE_FREE = 0,

  VCD_SIGNAL_DUMPER_MODULE_VARIABLES,
  VCD_SIGNAL_DUMPER_MODULE_FUNCTIONS,
  //     VCD_SIGNAL_DUMPER_MODULE_UE_PROCEDURES_FUNCTIONS,
  VCD_SIGNAL_DUMPER_MODULE_LAST,
  VCD_SIGNAL_DUMPER_MODULE_END = VCD_SIGNAL_DUMPER_MODULE_LAST,
} vcd_signal_dumper_modules;

typedef enum {
  VCD_FUNCTION_OUT,
  VCD_FUNCTION_IN,
  VCD_FUNCTION_LAST,
} vcd_signal_dump_in_out;

typedef enum {
  VCD_REAL, // REAL = variable
  VCD_WIRE, // WIRE = Function
} vcd_signal_type;

/*!
 * \brief Init function for the vcd dumper.
 * @param None
 */
void vcd_signal_dumper_init(char* filename);
/*!
 * \brief Close file descriptor.
 * @param None
 */
void vcd_signal_dumper_close(void);
/*!
 * \brief Create header for VCD file.
 * @param None
 */
void vcd_signal_dumper_create_header(void);
/*!
 * \brief Dump state of a variable
 * @param Name of the variable to dump (see the corresponding enum)
 * @param Value of the variable to dump (type: unsigned long)
 */
void vcd_signal_dumper_dump_variable_by_name(vcd_signal_dump_variables variable_name,
    unsigned long             value);
/*!
 * \brief Dump function usage
 * @param Name Function name to dump (see the corresponding enum)
 * @param State: either VCD_FUNCTION_START or VCD_FUNCTION_END
 */
void vcd_signal_dumper_dump_function_by_name(vcd_signal_dump_functions  function_name,
    vcd_signal_dump_in_out     function_in_out);

extern int ouput_vcd;

#if T_TRACER

#include "T.h"

#define VCD_SIGNAL_DUMPER_INIT(x)         /* nothing */
#define VCD_SIGNAL_DUMPER_CLOSE()         /* nothing */
#define VCD_SIGNAL_DUMPER_CREATE_HEADER() /* nothing */
#define VCD_SIGNAL_DUMPER_DUMP_VARIABLE_BY_NAME(var, val) T_VCD_VARIABLE(var, val)
#define VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(var, val) T_VCD_FUNCTION(var, val)

#else /* T_TRACER */

#if defined(ENABLE_VCD)
   #define VCD_SIGNAL_DUMPER_INIT(aRgUmEnT)                   vcd_signal_dumper_init(aRgUmEnT)
   #define VCD_SIGNAL_DUMPER_CLOSE()                          vcd_signal_dumper_close()
   #define VCD_SIGNAL_DUMPER_CREATE_HEADER()                  vcd_signal_dumper_create_header()
   #define VCD_SIGNAL_DUMPER_DUMP_VARIABLE_BY_NAME(vAr1,vAr2) vcd_signal_dumper_dump_variable_by_name(vAr1,vAr2)
   #define VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(vAr1,vAr2) vcd_signal_dumper_dump_function_by_name(vAr1,vAr2)
#else
   #define VCD_SIGNAL_DUMPER_INIT(aRgUmEnT)
   #define VCD_SIGNAL_DUMPER_CLOSE()
   #define VCD_SIGNAL_DUMPER_CREATE_HEADER()
   #define VCD_SIGNAL_DUMPER_DUMP_VARIABLE_BY_NAME(vAr1,vAr2)
   #define VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(vAr1,vAr2)
#endif

#endif /* T_TRACER */

#endif /* !defined (VCD_SIGNAL_DUMPER_H_) */

