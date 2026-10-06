#include "dtc_manager.h"
#include <string.h>

void DTC_Init(DtcManager_t *mgr) {
    if (!mgr) return;
    memset(mgr, 0, sizeof(DtcManager_t));
}

bool DTC_SetFault(DtcManager_t *mgr, uint32_t dtc, uint8_t status_mask) {
    if (!mgr) return false;

    /* Check if already present */
    for (uint8_t i = 0; i < mgr->count; ++i) {
        if (mgr->records[i].dtc_code == dtc) {
            mgr->records[i].status_mask |= status_mask;
            mgr->records[i].occurrence_count++;
            return true;
        }
    }

    /* Add new */
    if (mgr->count < MAX_DTC_ENTRIES) {
        mgr->records[mgr->count].dtc_code = dtc;
        mgr->records[mgr->count].status_mask = status_mask;
        mgr->records[mgr->count].occurrence_count = 1;
        mgr->count++;
        return true;
    }
    return false;
}

void DTC_ClearAll(DtcManager_t *mgr) {
    if (!mgr) return;
    mgr->count = 0;
    memset(mgr->records, 0, sizeof(mgr->records));
}

uint8_t DTC_GetReport(const DtcManager_t *mgr, uint8_t *out_buffer, uint16_t max_len) {
    if (!mgr || !out_buffer || max_len < (mgr->count * 4)) return 0;

    uint16_t offset = 0;
    for (uint8_t i = 0; i < mgr->count; ++i) {
        /* Byte 0-2: DTC 24-bit code */
        out_buffer[offset++] = (uint8_t)((mgr->records[i].dtc_code >> 16) & 0xFF);
        out_buffer[offset++] = (uint8_t)((mgr->records[i].dtc_code >> 8) & 0xFF);
        out_buffer[offset++] = (uint8_t)(mgr->records[i].dtc_code & 0xFF);
        /* Byte 3: Status Mask */
        out_buffer[offset++] = mgr->records[i].status_mask;
    }
    return mgr->count;
}
