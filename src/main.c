#include <stdio.h>
#include "uds_protocol.h"
#include "dtc_manager.h"

DtcManager_t g_dtc_manager;

void print_uds_hex(const char *prefix, const UdsMessage_t *msg) {
    printf("%s [%u bytes]: ", prefix, msg->length);
    for (uint16_t i = 0; i < msg->length; ++i) {
        printf("%02X ", msg->payload[i]);
    }
    printf("\n");
}

int main(void) {
    printf("========================================================\n");
    printf("   Aptiv AUTOSAR ISO 14229 UDS Diagnostic Stack        \n");
    printf("   Automotive Embedded Connected Vehicle Diagnostics    \n");
    printf("========================================================\n\n");

    UdsServer_t server;
    UDS_Init(&server);
    DTC_Init(&g_dtc_manager);

    UdsMessage_t req;
    UdsMessage_t resp;

    /* 1. Request Extended Diagnostic Session (0x10 0x03) */
    printf("[STEP 1] Tester -> DiagnosticSessionControl (Extended Session 0x03)\n");
    req.payload[0] = UDS_SID_DIAGNOSTIC_SESSION_CONTROL;
    req.payload[1] = UDS_SESSION_EXTENDED;
    req.length = 2;
    print_uds_hex("  REQ ", &req);
    UDS_ProcessRequest(&server, &req, &resp);
    print_uds_hex("  RESP", &resp);

    /* 2. Request Security Seed (0x27 0x01) */
    printf("\n[STEP 2] Tester -> SecurityAccess (Request Seed 0x01)\n");
    req.payload[0] = UDS_SID_SECURITY_ACCESS;
    req.payload[1] = 0x01;
    req.length = 2;
    print_uds_hex("  REQ ", &req);
    UDS_ProcessRequest(&server, &req, &resp);
    print_uds_hex("  RESP", &resp);

    /* 3. Send Calculated Security Key (0x27 0x02) */
    printf("\n[STEP 3] Tester -> SecurityAccess (Send Key 0x02)\n");
    uint32_t key = UDS_CalculateSecurityKey(server.active_seed);
    req.payload[0] = UDS_SID_SECURITY_ACCESS;
    req.payload[1] = 0x02;
    req.payload[2] = (uint8_t)((key >> 24) & 0xFF);
    req.payload[3] = (uint8_t)((key >> 16) & 0xFF);
    req.payload[4] = (uint8_t)((key >> 8) & 0xFF);
    req.payload[5] = (uint8_t)(key & 0xFF);
    req.length = 6;
    print_uds_hex("  REQ ", &req);
    UDS_ProcessRequest(&server, &req, &resp);
    print_uds_hex("  RESP", &resp);
    printf("  [SECURITY STATUS] ECU Security State: %s\n",
           server.security_state == UDS_SECURITY_UNLOCKED ? "UNLOCKED [ACTIVE]" : "LOCKED");

    /* 4. Read VIN (0x22 0xF1 0x90) */
    printf("\n[STEP 4] Tester -> ReadDataByIdentifier (DID: 0xF190 - Vehicle VIN)\n");
    req.payload[0] = UDS_SID_READ_DATA_BY_ID;
    req.payload[1] = 0xF1;
    req.payload[2] = 0x90;
    req.length = 3;
    print_uds_hex("  REQ ", &req);
    UDS_ProcessRequest(&server, &req, &resp);
    print_uds_hex("  RESP", &resp);

    return 0;
}
