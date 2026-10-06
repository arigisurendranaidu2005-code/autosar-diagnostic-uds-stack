#ifndef UDS_PROTOCOL_H
#define UDS_PROTOCOL_H

#include <stdint.h>
#include <stdbool.h>

/* ISO 14229 Service Identifiers */
#define UDS_SID_DIAGNOSTIC_SESSION_CONTROL 0x10
#define UDS_SID_CLEAR_DTC_INFO             0x14
#define UDS_SID_READ_DTC_INFO              0x19
#define UDS_SID_READ_DATA_BY_ID            0x22
#define UDS_SID_SECURITY_ACCESS            0x27
#define UDS_SID_WRITE_DATA_BY_ID           0x2E
#define UDS_SID_NEGATIVE_RESPONSE          0x7F

/* Negative Response Codes (NRC) */
#define UDS_NRC_GENERAL_REJECT                      0x10
#define UDS_NRC_SERVICE_NOT_SUPPORTED               0x11
#define UDS_NRC_SUBFUNCTION_NOT_SUPPORTED           0x12
#define UDS_NRC_INCORRECT_MESSAGE_LENGTH            0x13
#define UDS_NRC_CONDITIONS_NOT_CORRECT              0x22
#define UDS_NRC_REQUEST_SEQUENCE_ERROR              0x24
#define UDS_NRC_SECURITY_ACCESS_DENIED              0x33
#define UDS_NRC_INVALID_KEY                         0x35

/* Diagnostic Sessions */
typedef enum {
    UDS_SESSION_DEFAULT = 0x01,
    UDS_SESSION_PROGRAMMING = 0x02,
    UDS_SESSION_EXTENDED = 0x03
} UdsSessionType_t;

typedef enum {
    UDS_SECURITY_LOCKED = 0,
    UDS_SECURITY_SEED_SENT = 1,
    UDS_SECURITY_UNLOCKED = 2
} UdsSecurityState_t;

typedef struct {
    uint8_t payload[256];
    uint16_t length;
} UdsMessage_t;

typedef struct {
    UdsSessionType_t current_session;
    UdsSecurityState_t security_state;
    uint32_t active_seed;
    uint32_t p2_timeout_ms;
} UdsServer_t;

void UDS_Init(UdsServer_t *server);
bool UDS_ProcessRequest(UdsServer_t *server, const UdsMessage_t *req, UdsMessage_t *resp);
void UDS_SendNegativeResponse(uint8_t rejected_sid, uint8_t nrc, UdsMessage_t *resp);
uint32_t UDS_CalculateSecurityKey(uint32_t seed);

#endif /* UDS_PROTOCOL_H */
