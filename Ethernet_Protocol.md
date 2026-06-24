# 🌐 ATTLA-T 이더넷 프로토콜 (페이로드 및 ACK 포맷)

본 문서는 ATTLA-T 체계의 UDP 이더넷 통신에서 사용되는 **페이로드(Payload) 메시지 포맷**, **ACK 메시지 포맷** 및 **통신망 가입/점검(BIT) 수행 절차** 규격을 정의합니다.
모든 데이터는 **Little Endian** 형식을 따릅니다.

---

## 1. 메시지 특성에 따른 ACK 정책
메시지의 성격(Command, Update, Request, Reflect)에 따라 ACK 요청 여부 및 응답 여부가 구분됩니다.

* **1. Update (갱신/설정)** : ACK 요청 안함 (X), 결과 응답 없음 (X)
* **2. Request (요청)**     : ACK 요청 안함 (X), 결과 응답 있음 (O)
* **3. Command (명령/구동)**: ACK 요청 함 (O), 결과 응답 있음 (O)
* **4. Reflect (주기송신)** : ACK 요청 안함 (X), 결과 응답 없음 (X) (예: BIT 결과)

---

## 2. 체계 연동통제안(ICD) ID 및 메시지 코드

### 2.1 장치 ID (Source / Destination ID)
- **화포통제컴퓨터 (FC) ID**: `0x01`
- **ATTLA-T (DSP) ID**: `0x02`

### 2.2 메시지 명령어 (Code) 테이블
| Code (Hex) | 매크로 명칭 | 설명 |
| :--- | :--- | :--- |
| `0x10` | `ETH_CODE_BOOT_DONE` | 망 가입 요청 (Boot Done) |
| `0x11` | `ETH_CODE_STATUS_REQ` | 상태 정보 요청 (100ms 통신상태 확인) |
| `0x12` | `ETH_CODE_PBIT_REQ` | PBIT(초기점검) 수행 요청 |
| `0x13` | `ETH_CODE_PBIT_REP` | PBIT 결과 응답 전송 |
| `0x14` | `ETH_CODE_IBIT_REQ` | IBIT(임의점검) 수행 요청 |
| `0x15` | `ETH_CODE_IBIT_REP` | IBIT 결과 응답 전송 |
| `0x16` | `ETH_CODE_CBIT_SET` | CBIT 전송주기 설정 요청 |
| `0x17` | `ETH_CODE_CBIT_REP` | CBIT 주기 점검 결과 송신 |
| `0x19` | `ETH_CODE_IBIT_DONE` | IBIT 수행 완료 통보 |
| `0x1A` | `ETH_CODE_IBIT_RES_REQ` | IBIT 결과 요청 |
| `0x1B` | `ETH_CODE_CBIT_STOP` | CBIT 전송 중지 요청 |
| `0x1C` | `ETH_CODE_CBIT_REQ` | CBIT 전송 시작 요청 |
| `0xFF` | `ETH_CODE_ACK` | ACK / NACK 응답 메시지 |

---

## 3. 연동 시나리오 및 절차 (PBIT, CBIT, IBIT)

### 3.1 PBIT (초기점검) 수행 및 통신망 가입
1. **망 가입 요청 (Update)**: DSP 부팅 후 `Boot Done` 메시지를 PC로 전송합니다. DSP는 PC로부터 ACK를 받을 때까지 500ms 주기로 반복 전송합니다.
2. **망 가입 완료 (Request)**: PC가 메시지를 수신하면 ACK를 전송하고, 100ms 주기로 `상태 정보 요청` 메시지를 전송합니다. 이때 PC의 UI에서는 `Boot Done`을 확인하여 '통신망 가입 완료'로 표시합니다. DSP는 상태 정보 요청을 받으면 `상태 정보 응답`을 수행하며, PC는 이를 수신하여 통신 상태를 표시합니다.
3. **PBIT 결과 획득 (Request)**: 처음 상태 정보 요청 응답이 1회 이루어진 직후, PC에서 `PBIT 결과 요청`을 전송합니다. DSP는 이를 수신하면 내부 PBIT 수행 후 `PBIT 결과 응답` 메시지로 전송하며, PC는 UI 상에 각 항목별(정상/오류) 결과를 표시합니다.

### 3.2 CBIT (주기점검) 수행 절차
*(CBIT 자체 점검은 모터 제어 주기마다 백그라운드에서 상시 진행됩니다.)*
1. **주기 설정 (Update)**: PC UI에 CBIT 전송 주기를 입력하고 설정 전송 버튼을 누르면, 설정된 주기(N초)를 담은 메시지가 DSP로 전송됩니다. DSP는 확인 후 주기 업데이트 및 ACK를 회신하며, PC는 ACK 수신 후 'CBIT 주기 업데이트 완료'를 UI에 표시합니다.
2. **결과 전송 시작 (Reflect)**: PC에서 'CBIT 결과 요청' 버튼을 누르면 메시지가 전송되고, DSP 내부의 CBIT 결과 전송 Flag가 켜집니다.
3. **주기적 결과 송신 (Reflect)**: 전송 Flag가 켜져 있으면, DSP는 설정된 N초 주기마다 자동으로 `CBIT 결과 응답` 메시지를 전송합니다.
4. **결과 전송 중지 (Reflect)**: PC에서 'CBIT 결과 요청 중지' 버튼을 누르면 메시지가 전송되고, DSP 내부의 CBIT 결과 전송 Flag가 꺼져 전송이 중지됩니다.

### 3.3 IBIT (임의점검) 수행 절차
1. **IBIT 수행 요청 (Update)**: PC의 UI에서 IBIT 수행 시간(N초)을 설정한 뒤 '수행 요청' 버튼을 누르면 메시지가 전송됩니다. DSP는 ACK를 전송한 후 진행 중이던 CBIT 전송을 일시 중지하고, N초 동안 IBIT를 수행합니다. (수행 기간 중 누적된 오류 중 단 1번이라도 발생한 항목은 최종 오류로 판정)
2. **IBIT 완료 통보 (Update)**: N초 경과로 IBIT 결과가 획득되면, DSP가 `IBIT Done` 메시지를 PC로 전송합니다. PC는 ACK를 보내고 UI에 'IBIT 완료'를 표시합니다. DSP는 이 ACK를 수신하는 즉시 중단되었던 CBIT 결과 전송을 재개합니다.
3. **IBIT 결과 요청 (Request)**: PC에서 'IBIT 결과 요청' 버튼을 누르면 요청 메시지가 전송됩니다. DSP는 이를 받아 PC로 `IBIT 결과 응답` 메시지를 전송하며, PC는 수신된 데이터를 바탕으로 IBIT 결과를 UI에 표시합니다.

### 3.4 BIT 결과 분리 표시 요구사항
PC의 UI에서는 수신된 데이터가 **PBIT 결과**인지, **CBIT 결과**인지, **IBIT 결과**인지 사용자가 명확히 알 수 있도록 영역이나 상태를 분리하여 표시해야 합니다.

---

## 4. 페이로드 (Payload) 메시지 포맷 상세

### 4.1 점검(BIT) 결과 응답 페이로드
**대상 명령어**:
- `ETH_CODE_PBIT_REP` (0x13): 초기점검(PBIT) 결과 응답
- `ETH_CODE_IBIT_REP` (0x15): 임의점검(IBIT) 결과 응답
- `ETH_CODE_CBIT_REP` (0x17): 주기점검(CBIT) 결과 응답

**페이로드 크기**: `4 Bytes`

**포맷**: 32-bit `BIT Result Bitmask` (Little Endian)
| Byte Offset | 크기 | 설명 | 데이터 타입 |
| :--- | :--- | :--- | :--- |
| `[0~3]` | 4 | BIT 전체 결함 정보 (`informAll`) | `uint32_t` |

**Bitmask 상세 내용 (`xBit.informAll`)**:
- `Bit 8` (0x00000100) : 모터 과전류 (`faultOvCurrMot`)
- `Bit 9` (0x00000200) : 브레이크 과전류 (`faultOvCurrBrk`)
- `Bit 11` (0x00000800) : 보드 과열 (`faultOvTempBd`)
- `Bit 12` (0x00001000) : 28V 입력 과전압 (`faultOvVolt28V`)
- `Bit 13` (0x00002000) : 브레이크 입력 과전압 (`faultOvVoltBrk`)
- `Bit 16` (0x00010000) : 모터 드라이버(DRV8343) 결함 (`faultDrv8343nFault`)
- `Bit 17` (0x00020000) : 모터 구속/스톨 (`faultStall`)
- `Bit 18` (0x00040000) : 모터 과속 (`faultOverSpeed`)
- `Bit 19` (0x00080000) : 엔코더 에러 (`faultEncError`)
- `Bit 20` (0x00100000) : 엔코더 워닝 (`warnEncWarning`)

*(위 비트 중 1개라도 `1`로 세팅되면 해당 결함이 발생한 상태를 의미함)*

---

### 4.2 IBIT (임의점검) 수행 요청 페이로드
**대상 명령어**: `ETH_CODE_IBIT_REQ` (0x14)

**페이로드 크기**: `2 Bytes`

**포맷**: IBIT 수행 지정 시간 (초 단위)
| Byte Offset | 크기 | 설명 | 데이터 타입 |
| :--- | :--- | :--- | :--- |
| `[0~1]` | 2 | IBIT 수행 시간 (`durationSec`), 단위: 초 | `uint16_t` |

---

### 4.3 CBIT (주기점검) 전송주기 설정 페이로드
**대상 명령어**: `ETH_CODE_CBIT_SET` (0x16)

**페이로드 크기**: `2 Bytes`

**포맷**: CBIT 점검결과 전송 주기 (초 단위)
| Byte Offset | 크기 | 설명 | 데이터 타입 |
| :--- | :--- | :--- | :--- |
| `[0~1]` | 2 | CBIT 전송 주기 (`periodSec`), 단위: 초 | `uint16_t` |

---

## 5. ACK (응답 / NACK) 메시지 포맷 상세

**대상 명령어**: `ETH_CODE_ACK` (0xFF)

**전체 메시지 크기**: 18 Bytes 고정 (Header 12B + Payload 4B + Checksum 2B)

**페이로드 크기**: `4 Bytes`
패킷 수신 성공/실패 여부를 응답하는 전용 메시지입니다. ACK 페이로드는 공통 헤더 12바이트 직후에 위치합니다.

**포맷**:
| Payload Offset | 전체 프레임 Offset | 필드명 | 크기 | 설명 | 데이터 타입 |
| :--- | :--- | :--- | :--- | :--- | :--- |
| `[0~1]` | `[12~13]` | **Code Info** | 2 | 수신한 원본 메시지의 대상 명령어 Code 값 | `uint16_t` |
| `[2~3]` | `[14~15]` | **Ack Info** | 2 | 응답 상태 결과 코드 (아래 참조) | `uint16_t` |

**Ack Info (응답 상태 결과) 코드 정의**:
- `0x0000` (`ETH_ACK_INFO_OK`) : 정상 처리됨
- `0x0001` (`ETH_ACK_INFO_CS_ERR`) : 체크섬(Checksum) 오류 발생

---
*(문서 끝)*
