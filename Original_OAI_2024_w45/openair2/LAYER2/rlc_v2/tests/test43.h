/*
 * um test: test rlc_entity_um_discard_sdu
 * eNB and UE get some SDU, later on some are discarded
 */

TIME, 1,
    ENB_UM, 100000, 100000, 35, 10,
    UE_UM, 100000, 100000, 35, 10,
    ENB_SDU, 0, 10,
    ENB_SDU, 1, 10,
    ENB_SDU, 2, 10,
    ENB_SDU, 3, 10,
    ENB_PDU_SIZE, 23,
TIME, 2,
    ENB_DISCARD_SDU, 0,
    ENB_DISCARD_SDU, 2,
    ENB_DISCARD_SDU, 3,
    ENB_DISCARD_SDU, 1,
TIME, 10,
    UE_SDU, 0, 5,
    UE_SDU, 1, 5,
    UE_SDU, 2, 5,
    UE_SDU, 3, 5,
    UE_SDU, 4, 5,
    UE_SDU, 5, 5,
    UE_PDU_SIZE, 13,
TIME, 12,
    UE_DISCARD_SDU, 3,
    UE_DISCARD_SDU, 1,
    UE_DISCARD_SDU, 0,
    UE_DISCARD_SDU, 5,
    UE_DISCARD_SDU, 4,
    UE_DISCARD_SDU, 2,
TIME, 30,
    UE_SDU, 6, 5,
    UE_DISCARD_SDU, 6,
TIME, 31,
    UE_SDU, 7, 8,
TIME, -1