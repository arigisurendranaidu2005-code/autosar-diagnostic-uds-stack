# AUTOSAR Unified Diagnostic Services (ISO 14229 UDS) Protocol Stack

[![Language: Embedded C](https://img.shields.io/badge/Language-Embedded%20C%20%2F%20C%2B%2B-blue.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
[![Standard: ISO 14229 / ISO 15765](https://img.shields.io/badge/Standard-ISO%2014229%20UDS-red.svg)](https://en.wikipedia.org/wiki/Unified_Diagnostic_Services)
[![Architecture: AUTOSAR Diagnostic](https://img.shields.io/badge/Architecture-AUTOSAR%20Classic%20DCM%20%2F%20DEM-orange.svg)](https://www.autosar.org/)
[![Tests: 100% Passing](https://img.shields.io/badge/Tests-100%25%20Passing-brightgreen.svg)](https://github.com/arigisurendranaidu2005-code/autosar-diagnostic-uds-stack)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

An automotive embedded software protocol stack implementing ISO 14229 Unified Diagnostic Services (UDS) for automotive Electronic Control Units (ECUs). Features Diagnostic Session Control, Seed-Key Security Access challenge-response, Read/Write Data by Identifier (DID), and Diagnostic Trouble Code (DTC) fault management aligned with Aptiv's **Connected Vehicle & AS&UX Data Connectivity Solutions**.

---

## 🏛️ Protocol Architecture & ISO 14229 Engine

```
       [ External OBD-II / UDS Diagnostic Client Tester ]
                              │
                              ▼ ISO 15765-2 / CAN 2.0B
   ┌─────────────────────────────────────────────────────────┐
   │         AUTOSAR Diagnostic Communication Manager (DCM)   │
   └──────────────────────────┬──────────────────────────────┘
                              │
         ┌────────────────────┼────────────────────┐
         │                    │                    │
         ▼                    ▼                    ▼
   ┌───────────┐        ┌───────────┐        ┌───────────┐
   │  Session  │        │  Security │        │ Data ID   │
   │  Control  │        │  Access   │        │  (DID)    │
   │ (SID 0x10)│        │ (SID 0x27)│        │ (SID 0x22)│
   └───────────┘        └───────────┘        └───────────┘
         │                    │                    │
         └────────────────────┼────────────────────┘
                              │
                              ▼
   ┌─────────────────────────────────────────────────────────┐
   │       Diagnostic Event Manager (DEM) Fault Memory       │
   │  • Diagnostic Trouble Code (DTC) Registry (SID 0x14/19) │
   │  • Freeze-Frame & Snapshot Storage                      │
   └─────────────────────────────────────────────────────────┘
```

---

## 💻 Implemented Automotive Services

| Service ID (SID) | Service Name | Description |
| :---: | :--- | :--- |
| **`0x10`** | `DiagnosticSessionControl` | Transitions between `DEFAULT`, `PROGRAMMING`, and `EXTENDED` sessions with $P_2 / P_2^*$ timing negotiation. |
| **`0x27`** | `SecurityAccess` | Two-stage cryptographic challenge-response authentication (`0x01` Request Seed $\rightarrow$ `0x02` Send Key) protecting ECU write permissions. |
| **`0x22`** | `ReadDataByIdentifier` | Reads standardized automotive parameters (`0xF190` VIN, `0x0100` Battery Voltage). |
| **`0x14`** | `ClearDiagnosticInformation` | Resets active and confirmed DTC fault codes across all vehicle diagnostic groups. |
| **`0x7F`** | `NegativeResponse` | Strict NRC handling (`0x11` ServiceNotSupported, `0x13` IncorrectMessageLength, `0x35` InvalidKey). |

---

## 🧪 Testing & Verification (100% Pass Rate)

```bash
# Compile and run test suite
gcc -Wall -Wextra -Iinclude src/uds_protocol.c src/dtc_manager.c tests/test_uds.c -o tests/test_uds.exe
.\tests\test_uds.exe
```

### Verified Test Cases:
- ✅ `test_session_control_positive_and_negative`: Validates positive session transitions and NRC rejection for unsupported sub-functions.
- ✅ `test_security_access_seed_key_flow`: Confirms locked state upon incorrect key and unlock upon valid seed transformation.
- ✅ `test_read_data_by_id_vin`: Verifies 17-byte ASCII vehicle identification number extraction.
- ✅ `test_dtc_manager_and_clear`: Validates DTC registration and memory purge routines.

---

## 🚀 Quickstart & Tester Execution

```bash
# Clone the repository
git clone https://github.com/arigisurendranaidu2005-code/autosar-diagnostic-uds-stack.git
cd autosar-diagnostic-uds-stack

# Compile and run client demo
make
.\uds_stack_demo.exe
```

---

## 📜 License
MIT License. Authored by Surendra Naidu Arigi. Aligned with Aptiv Connected Mobility & Diagnostics Standards.
