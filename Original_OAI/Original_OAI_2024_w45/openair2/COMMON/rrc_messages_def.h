/*
 * Licensed to the OpenAirInterface (OAI) Software Alliance under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The OpenAirInterface Software Alliance licenses this file to You under
 * the OAI Public License, Version 1.1  (the "License"); you may not use this file
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

/*
 * rrc_messages_def.h
 *
 *  Created on: Oct 24, 2013
 *      Author: winckel
 */

//-------------------------------------------------------------------------------------------//
// Messages for RRC logging
#if defined(DISABLE_ITTI_XER_PRINT)
MESSAGE_DEF(RRC_DL_BCCH_MESSAGE,        MESSAGE_PRIORITY_MED_PLUS,  RrcDlBcchMessage,           rrc_dl_bcch_message)
MESSAGE_DEF(RRC_DL_CCCH_MESSAGE,        MESSAGE_PRIORITY_MED_PLUS,  RrcDlCcchMessage,           rrc_dl_ccch_message)
MESSAGE_DEF(RRC_DL_DCCH_MESSAGE,        MESSAGE_PRIORITY_MED_PLUS,  RrcDlDcchMessage,           rrc_dl_dcch_message)

MESSAGE_DEF(RRC_UE_EUTRA_CAPABILITY,    MESSAGE_PRIORITY_MED_PLUS,  RrcUeEutraCapability,       rrc_ue_eutra_capability)

MESSAGE_DEF(RRC_UL_CCCH_MESSAGE,        MESSAGE_PRIORITY_MED_PLUS,  RrcUlCcchMessage,           rrc_ul_ccch_message)
MESSAGE_DEF(RRC_UL_DCCH_MESSAGE,        MESSAGE_PRIORITY_MED_PLUS,  RrcUlDcchMessage,           rrc_ul_dcch_message)
#else
MESSAGE_DEF(RRC_DL_BCCH,                MESSAGE_PRIORITY_MED_PLUS,  IttiMsgText,                rrc_dl_bcch)
MESSAGE_DEF(RRC_DL_CCCH,                MESSAGE_PRIORITY_MED_PLUS,  IttiMsgText,                rrc_dl_ccch)
MESSAGE_DEF(RRC_DL_DCCH,                MESSAGE_PRIORITY_MED_PLUS,  IttiMsgText,                rrc_dl_dcch)
MESSAGE_DEF(RRC_DL_MCCH,                MESSAGE_PRIORITY_MED_PLUS,  IttiMsgText,                rrc_dl_mcch)

MESSAGE_DEF(RRC_UE_EUTRA_CAPABILITY,    MESSAGE_PRIORITY_MED_PLUS,  IttiMsgText,                rrc_ue_eutra_capability)

MESSAGE_DEF(RRC_UL_CCCH,                MESSAGE_PRIORITY_MED_PLUS,  IttiMsgText,                rrc_ul_ccch)
MESSAGE_DEF(RRC_UL_DCCH,                MESSAGE_PRIORITY_MED_PLUS,  IttiMsgText,                rrc_ul_dcch)
#endif

MESSAGE_DEF(RRC_STATE_IND,              MESSAGE_PRIORITY_MED,       RrcStateInd,                rrc_state_ind)

//-------------------------------------------------------------------------------------------//
// eNB: ENB_APP -> RRC messages
MESSAGE_DEF(RRC_CONFIGURATION_REQ,      MESSAGE_PRIORITY_MED,       RrcConfigurationReq,        rrc_configuration_req)
MESSAGE_DEF(NBIOTRRC_CONFIGURATION_REQ, MESSAGE_PRIORITY_MED,       NbIoTRrcConfigurationReq,   nbiotrrc_configuration_req)

// UE: NAS -> RRC messages
MESSAGE_DEF(NAS_KENB_REFRESH_REQ, MESSAGE_PRIORITY_MED, kenb_refresh_req_t, nas_kenb_refresh_req)
MESSAGE_DEF(NAS_CELL_SELECTION_REQ, MESSAGE_PRIORITY_MED, cell_info_req_t, nas_cell_selection_req)
MESSAGE_DEF(NAS_CONN_ESTABLI_REQ, MESSAGE_PRIORITY_MED, nas_establish_req_t, nas_conn_establi_req)
MESSAGE_DEF(NAS_UPLINK_DATA_REQ, MESSAGE_PRIORITY_MED, ul_info_transfer_req_t, nas_ul_data_req)
MESSAGE_DEF(NAS_DETACH_REQ, MESSAGE_PRIORITY_MED, nas_detach_req_t, nas_detach_req)
MESSAGE_DEF(NAS_DEREGISTRATION_REQ, MESSAGE_PRIORITY_MED, nas_deregistration_req_t, nas_deregistration_req)

MESSAGE_DEF(NAS_RAB_ESTABLI_RSP, MESSAGE_PRIORITY_MED, rab_establish_rsp_t, nas_rab_est_rsp)

MESSAGE_DEF(NAS_OAI_TUN_NSA, MESSAGE_PRIORITY_MED, nas_oai_tun_nsa_t, nas_oai_tun_nsa)

// UE: RRC -> NAS messages
MESSAGE_DEF(NAS_CELL_SELECTION_CNF, MESSAGE_PRIORITY_MED, cell_info_cnf_t, nas_cell_selection_cnf)
MESSAGE_DEF(NAS_CELL_SELECTION_IND, MESSAGE_PRIORITY_MED, cell_info_ind_t, nas_cell_selection_ind)
MESSAGE_DEF(NAS_PAGING_IND, MESSAGE_PRIORITY_MED, paging_ind_t, nas_paging_ind)
MESSAGE_DEF(NAS_CONN_ESTABLI_CNF, MESSAGE_PRIORITY_MED, nas_establish_cnf_t, nas_conn_establi_cnf)
MESSAGE_DEF(NAS_CONN_RELEASE_IND, MESSAGE_PRIORITY_MED, nas_release_ind_t, nas_conn_release_ind)
MESSAGE_DEF(NR_NAS_CONN_RELEASE_IND,    MESSAGE_PRIORITY_MED,       NRNasConnReleaseInd,        nr_nas_conn_release_ind)
MESSAGE_DEF(NAS_UPLINK_DATA_CNF, MESSAGE_PRIORITY_MED, ul_info_transfer_cnf_t, nas_ul_data_cnf)
MESSAGE_DEF(NAS_DOWNLINK_DATA_IND, MESSAGE_PRIORITY_MED, dl_info_transfer_ind_t, nas_dl_data_ind)

// xNB: realtime -> RRC messages
MESSAGE_DEF(RRC_SUBFRAME_PROCESS,       MESSAGE_PRIORITY_MED,       RrcSubframeProcess,         rrc_subframe_process)
MESSAGE_DEF(NRRRC_FRAME_PROCESS,        MESSAGE_PRIORITY_MED,       NRRrcFrameProcess,          nr_rrc_frame_process)

// eNB: RLC -> RRC messages
MESSAGE_DEF(RLC_SDU_INDICATION,         MESSAGE_PRIORITY_MED,       RlcSduIndication,           rlc_sdu_indication)
MESSAGE_DEF(NAS_PDU_SESSION_REQ, MESSAGE_PRIORITY_MED, nas_pdu_session_req_t, nas_pdu_session_req)