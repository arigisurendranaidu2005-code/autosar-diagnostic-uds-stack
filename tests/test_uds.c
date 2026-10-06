#include <stdio.h>
#include <assert.h>
#include "uds_protocol.h"
#include "dtc_manager.h"

DtcManager_t g_dtc_manager;

void test_session_control_positive_and_negative(void) {
    UdsServer_t server;
    UDS_Init(&server);
    UdsMessage_t req, resp;

    /* Positive: Switch to Extended Session */
    req.payload[0] = UDS_SID_DIAGNOSTIC_SESSION_CONTROL;
    req.payload[1] = UDS_SESSION_EXTENDED;
    req.length = 2;
    assert(UDS_ProcessRequest(&server, &req, &resp) == true);
    assert(resp.payload[0] == 0x50); // 0x10 + 0x40
    assert(resp.payload[1] == UDS_SESSION_EXTENDED);
    assert(server.current_session == UDS_SESSION_EXTENDED);

    /* Negative: Invalid Subfunction */
    req.payload[1] = 0x99;
    assert(UDS_ProcessRequest(&server, &req, &resp) == false);
    assert(resp.payload[0] == UDS_SID_NEGATIVE_RESPONSE);
    assert(resp.payload[1] == UDS_SID_DIAGNOSTIC_SESSION_CONTROL);
    assert(resp.payload[2] == UDS_NRC_SUBFUNCTION_NOT_SUPPORTED);
    printf("PASS: test_session_control_positive_and_negative\n");
}

void test_security_access_seed_key_flow(void) {
    UdsServer_t server;
    UDS_Init(&server);
    UdsMessage_t req, resp;

    /* Step 1: Request Seed */
    req.payload[0] = UDS_SID_SECURITY_ACCESS;
    req.payload[1] = 0x01;
    req.length = 2;
    assert(UDS_ProcessRequest(&server, &req, &resp) == true);
    assert(resp.payload[0] == 0x67);
    assert(server.security_state == UDS_SECURITY_SEED_SENT);

    /* Step 2: Send Wrong Key */
    req.payload[1] = 0x02;
    req.payload[2] = 0x00; req.payload[3] = 0x00; req.payload[4] = 0x00; req.payload[5] = 0x00;
    req.length = 6;
    assert(UDS_ProcessRequest(&server, &req, &resp) == false);
    assert(resp.payload[2] == UDS_NRC_INVALID_KEY);
    assert(server.security_state != UDS_SECURITY_UNLOCKED);

    /* Step 3: Send Correct Key */
    uint32_t valid_key = UDS_CalculateSecurityKey(server.active_seed);
    req.payload[2] = (uint8_t)((valid_key >> 24) & 0xFF);
    req.payload[3] = (uint8_t)((valid_key >> 16) & 0xFF);
    req.payload[4] = (uint8_t)((valid_key >> 8) & 0xFF);
    req.payload[5] = (uint8_t)(valid_key & 0xFF);
    assert(UDS_ProcessRequest(&server, &req, &resp) == true);
    assert(resp.payload[0] == 0x67);
    assert(server.security_state == UDS_SECURITY_UNLOCKED);
    printf("PASS: test_security_access_seed_key_flow\n");
}

void test_read_data_by_id_vin(void) {
    UdsServer_t server;
    UDS_Init(&server);
    UdsMessage_t req, resp;

    req.payload[0] = UDS_SID_READ_DATA_BY_ID;
    req.payload[1] = 0xF1;
    req.payload[2] = 0x90; // DID: 0xF190 (VIN)
    req.length = 3;
    assert(UDS_ProcessRequest(&server, &req, &resp) == true);
    assert(resp.payload[0] == 0x62);
    assert(resp.length == 3 + 17);
    printf("PASS: test_read_data_by_id_vin\n");
}

void test_dtc_manager_and_clear(void) {
    DTC_Init(&g_dtc_manager);
    assert(DTC_SetFault(&g_dtc_manager, 0xC12001, 0x2F) == true);
    assert(g_dtc_manager.count == 1);

    UdsServer_t server;
    UDS_Init(&server);
    UdsMessage_t req, resp;

    req.payload[0] = UDS_SID_CLEAR_DTC_INFO;
    req.length = 1;
    assert(UDS_ProcessRequest(&server, &req, &resp) == true);
    assert(resp.payload[0] == 0x54);
    assert(g_dtc_manager.count == 0);
    printf("PASS: test_dtc_manager_and_clear\n");
}

int main(void) {
    printf("=== RUNNING AUTOSAR UDS DIAGNOSTIC STACK UNIT TESTS ===\n");
    test_session_control_positive_and_negative();
    test_security_access_seed_key_flow();
    test_read_data_by_id_vin();
    test_dtc_manager_and_clear();
    printf("=== ALL 4 UNIT TESTS PASSED SUCCESSFULLY (100%%) ===\n");
    return 0;
}
