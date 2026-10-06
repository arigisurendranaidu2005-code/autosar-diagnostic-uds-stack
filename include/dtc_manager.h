#ifndef DTC_MANAGER_H
#define DTC_MANAGER_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_DTC_ENTRIES 16

/* Standard Automotive DTC Format (3 bytes) */
typedef struct {
    uint32_t dtc_code; // E.g., 0xC12001 (Brake Pressure Sensor Error)
    uint8_t status_mask; // E.g., 0x2F (TestFailed, ConfirmedDTC, WarningIndicatorRequested)
    uint8_t occurrence_count;
} DtcRecord_t;

typedef struct {
    DtcRecord_t records[MAX_DTC_ENTRIES];
    uint8_t count;
} DtcManager_t;

void DTC_Init(DtcManager_t *mgr);
bool DTC_SetFault(DtcManager_t *mgr, uint32_t dtc, uint8_t status_mask);
void DTC_ClearAll(DtcManager_t *mgr);
uint8_t DTC_GetReport(const DtcManager_t *mgr, uint8_t *out_buffer, uint16_t max_len);

#endif /* DTC_MANAGER_H */
