#include "uds_protocol.h"
#include "dtc_manager.h"
#include <string.h>

extern DtcManager_t g_dtc_manager;

void UDS_Init(UdsServer_t *server) {
    if (!server) return;
    memset(server, 0, sizeof(UdsServer_t));
    server->current_session = UDS_SESSION_DEFAULT;
    server->security_state = UDS_SECURITY_LOCKED;
    server->active_seed = 0x5A1C8E32;
    server->p2_timeout_ms = 50;
}

uint32_t UDS_CalculateSecurityKey(uint32_t seed) {
    /* Automotive proprietary Seed-to-Key pseudo-random transform */
    return ((seed ^ 0xDEADBEEF) + 0x12345678) ^ 0xA5A5A5A5;
}

void UDS_SendNegativeResponse(uint8_t rejected_sid, uint8_t nrc, UdsMessage_t *resp) {
    if (!resp) return;
    resp->payload[0] = UDS_SID_NEGATIVE_RESPONSE;
    resp->payload[1] = rejected_sid;
    resp->payload[2] = nrc;
    resp->length = 3;
}

bool UDS_ProcessRequest(UdsServer_t *server, const UdsMessage_t *req, UdsMessage_t *resp) {
    if (!server || !req || !resp || req->length == 0) return false;

    uint8_t sid = req->payload[0];

    switch (sid) {
        /* Service 0x10: DiagnosticSessionControl */
        case UDS_SID_DIAGNOSTIC_SESSION_CONTROL: {
            if (req->length < 2) {
                UDS_SendNegativeResponse(sid, UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp);
                return false;
            }
            uint8_t requested_session = req->payload[1];
            if (requested_session != UDS_SESSION_DEFAULT &&
                requested_session != UDS_SESSION_PROGRAMMING &&
                requested_session != UDS_SESSION_EXTENDED) {
                UDS_SendNegativeResponse(sid, UDS_NRC_SUBFUNCTION_NOT_SUPPORTED, resp);
                return false;
            }

            server->current_session = (UdsSessionType_t)requested_session;
            resp->payload[0] = sid + 0x40; // Positive Response SID = Request SID + 0x40
            resp->payload[1] = requested_session;
            resp->payload[2] = 0x00; // P2 Server Max high byte
            resp->payload[3] = 0x32; // P2 Server Max low byte (50 ms)
            resp->payload[4] = 0x01; // P2* Server Max high byte
            resp->payload[5] = 0xF4; // P2* Server Max low byte (500 ms)
            resp->length = 6;
            return true;
        }

        /* Service 0x27: SecurityAccess */
        case UDS_SID_SECURITY_ACCESS: {
            if (req->length < 2) {
                UDS_SendNegativeResponse(sid, UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp);
                return false;
            }
            uint8_t sub_fn = req->payload[1];

            if (sub_fn == 0x01) { // Request Seed
                server->security_state = UDS_SECURITY_SEED_SENT;
                resp->payload[0] = sid + 0x40;
                resp->payload[1] = 0x01;
                resp->payload[2] = (uint8_t)((server->active_seed >> 24) & 0xFF);
                resp->payload[3] = (uint8_t)((server->active_seed >> 16) & 0xFF);
                resp->payload[4] = (uint8_t)((server->active_seed >> 8) & 0xFF);
                resp->payload[5] = (uint8_t)(server->active_seed & 0xFF);
                resp->length = 6;
                return true;
            } else if (sub_fn == 0x02) { // Send Key
                if (server->security_state != UDS_SECURITY_SEED_SENT || req->length < 6) {
                    UDS_SendNegativeResponse(sid, UDS_NRC_REQUEST_SEQUENCE_ERROR, resp);
                    return false;
                }
                uint32_t received_key = ((uint32_t)req->payload[2] << 24) |
                                        ((uint32_t)req->payload[3] << 16) |
                                        ((uint32_t)req->payload[4] << 8) |
                                        (uint32_t)req->payload[5];
                uint32_t expected_key = UDS_CalculateSecurityKey(server->active_seed);

                if (received_key == expected_key) {
                    server->security_state = UDS_SECURITY_UNLOCKED;
                    resp->payload[0] = sid + 0x40;
                    resp->payload[1] = 0x02;
                    resp->length = 2;
                    return true;
                } else {
                    UDS_SendNegativeResponse(sid, UDS_NRC_INVALID_KEY, resp);
                    return false;
                }
            }
            UDS_SendNegativeResponse(sid, UDS_NRC_SUBFUNCTION_NOT_SUPPORTED, resp);
            return false;
        }

        /* Service 0x22: ReadDataByIdentifier */
        case UDS_SID_READ_DATA_BY_ID: {
            if (req->length < 3) {
                UDS_SendNegativeResponse(sid, UDS_NRC_INCORRECT_MESSAGE_LENGTH, resp);
                return false;
            }
            uint16_t did = ((uint16_t)req->payload[1] << 8) | req->payload[2];

            /* DID 0xF190: Vehicle Identification Number (VIN) */
            if (did == 0xF190) {
                resp->payload[0] = sid + 0x40;
                resp->payload[1] = (uint8_t)(did >> 8);
                resp->payload[2] = (uint8_t)(did & 0xFF);
                const char *vin = "APTIV2026MOBILITY1";
                memcpy(&resp->payload[3], vin, 17);
                resp->length = 3 + 17;
                return true;
            }
            /* DID 0x0100: ECU Battery Voltage */
            else if (did == 0x0100) {
                resp->payload[0] = sid + 0x40;
                resp->payload[1] = (uint8_t)(did >> 8);
                resp->payload[2] = (uint8_t)(did & 0xFF);
                resp->payload[3] = 0x32; // 12.5V (encoded in 250mV units)
                resp->length = 4;
                return true;
            }
            UDS_SendNegativeResponse(sid, UDS_NRC_CONDITIONS_NOT_CORRECT, resp);
            return false;
        }

        /* Service 0x14: ClearDiagnosticInformation */
        case UDS_SID_CLEAR_DTC_INFO: {
            DTC_ClearAll(&g_dtc_manager);
            resp->payload[0] = sid + 0x40;
            resp->length = 1;
            return true;
        }

        /* Default fallback */
        default:
            UDS_SendNegativeResponse(sid, UDS_NRC_SERVICE_NOT_SUPPORTED, resp);
            return false;
    }
}
