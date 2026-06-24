# 🌐 ATTLA-T 통신 및 인터페이스 명세서 (Ethernet 연동통제안)

본 문서는 초장사정 자동선회잠금장치(ATTLA-T) 프로젝트의 **TMS320F28388D (CM)** 코어 기반에서 동작하는 **이더넷(UDP) 프로토콜** 및 화포통제컴퓨터와의 **연동통제안 상태 머신**에 대한 모든 설정과 데이터 규격을 통합 정리한 스펙 문서입니다.

---

## 1. 이더넷 (UDP) 네트워크 하드웨어 및 기본 설정

| 항목 | DSP (ATTLA-T) | 체계 / PC (화포통제컴퓨터) |
| :--- | :--- | :--- |
| **IP 주소** | `192.168.200.10` | `192.168.200.1` (추정치/고정) |
| **MAC 주소** | `00:08:DC:11:22:33` | Auto-Learning |
| **Subnet Mask** | `255.255.255.0` | `255.255.255.0` |
| **Gateway** | `192.168.200.1` | - |
| **수신/송신 포트** | `5001` (`PORT_UDP_COM`) 고정 | `5001` (대상 포트 고정) |
| **프로토콜** | IPv4, UDP (Little Endian Payload) | IPv4, UDP |
| **물리 계층(칩셋)**| **W6100** (SPI-A, 10MHz 통신, **CM 코어 전담**) | - |
| **인터럽트** | `INTn` ➡️ GPIO 20 (`XINT1`, 하강 에지) | - |

> 💡 **비고**: SPI 클럭은 시스템 LSPCLK 최대 한계 초과 에러를 방지하기 위해 10MHz(`POL0PHA0`)로 최적화되어 있습니다. 하드웨어 미연결 또는 SPI 통신 불량 시 무한 루프 락업(Stuck) 방지를 위해 `g_isW6100Connected = 0` 처리되어 상태 머신이 동작하지 않습니다.

---

## 2. 메시지 패킷 기본 구조 (직렬화/역직렬화 적용)
- **Endianness**: Little Endian 준수
- **메모리 패킹 제약 및 해결방안**:
  - TI C28x (DSP) 코어는 최소 주소 단위가 16비트이므로 `#pragma pack(1)` 지원 불가.
  - 이를 해결하기 위해 `memcpy` 대신 송/수신 버퍼 배열에 8비트 Shift 연산을 수행하는 **직접 바이트 직렬화(Serialization)** 로직 적용.
  - 네트워크 회선 상으로는 정확히 12바이트 헤더 규격을 보장함.

### 2.1 공통 MSG Header 구조 (12 Bytes)
화포통제컴퓨터 $\leftrightarrow$ ATTLA-T 간 모든 UDP 패킷의 최상단 12바이트는 공통 헤더 규격을 사용합니다.

| Byte Offset | 필드명 | 크기 (Bytes) | 설명 |
| :--- | :--- | :--- | :--- |
| `[0~3]` | **Timestamp** | 4 | 화포통제컴퓨터 Tickcount (응답 시 그대로 복사 반환). 자발적 송신 시 `0x00000000` |
| `[4]` | **Source ID** | 1 | 송신 장치 ID. (ATTLA-T: `0x02`, 화포통제컴퓨터: `0x01` 임시) |
| `[5]` | **Dest ID** | 1 | 수신 목적지 장치 ID |
| `[6]` | **Code** | 1 | 메시지 명령어 코드 (하단 Code 테이블 참조) |
| `[7]` | **Request ACK** | 1 | `0xFF` (미요청), `0x01` (요청함). ACK 응답 시 `0x10`(정상), `0x11`(NACK) |
| `[8]` | **Priority** | 1 | 우선순위 (기본 `0x00`) |
| `[9]` | **Send Count** | 1 | 패킷 전송 횟수 (`1` ~ `4`). 재전송 시마다 카운트 1씩 증가 |
| `[10~11]`| **Data Length** | 2 | 뒤이어 오는 순수 Payload(Data)의 바이트 단위 길이 (Header 12B 및 Checksum 2B 제외) |

### 2.2 메시지 Code (명령어) 테이블
| Code 값 | 매크로 명칭 | 설명 및 페이로드 데이터 |
| :--- | :--- | :--- |
| `0x10` | `ETH_CODE_BOOT_DONE` | 망 가입 요청 (부팅 완료 보고) |
| `0x11` | `ETH_CODE_HEARTBEAT` | 상태 정보 (100ms 주기 교환). Payload[0]: 270V 전원 인가 상태 |
| `0x12` | `ETH_CODE_PBIT_REQ` | PBIT(초기점검) 수행 요청 |
| `0x13` | `ETH_CODE_PBIT_REP` | PBIT 결과 응답 전송 (Payload[0~3]: 4B BIT Result Bitmask) |
| `0x14` | `ETH_CODE_IBIT_REQ` | IBIT(임의점검) 수행 요청 |
| `0x15` | `ETH_CODE_IBIT_REP` | IBIT 결과 응답 전송 (Payload[0~3]: 4B BIT Result Bitmask) |
| `0x16` | `ETH_CODE_CBIT_SET` | CBIT 전송주기 설정 요청. Payload[0~1]: N초 (주기) |
| `0x17` | `ETH_CODE_CBIT_REP` | CBIT 주기 점검 결과 송신 (Payload[0~3]: 4B BIT Result Bitmask) |
| `0x18` | `ETH_CODE_POWER_270V`| 270VDC 구동 전원 인가 통보 |
| `0x19` | `ETH_CODE_IBIT_DONE` | IBIT 수행 완료 통보 (DSP -> PC) |
| `0x1A` | `ETH_CODE_IBIT_RES_REQ` | IBIT 결과 요청 (PC -> DSP) |
| `0x1B` | `ETH_CODE_CBIT_STOP` | CBIT 전송 중지 요청 (PC -> DSP) |
| `0xFF` | `ETH_CODE_ACK` | ACK(수신 확인) 응답 메시지 |

### 2.3 Checksum (체크섬) 생성 규칙
- **위치**: 전체 패킷(Header + Data)의 맨 마지막에 위치 (크기: 2 Bytes).
- **연산식**: Checksum 2바이트 공간을 제외한 앞선 모든 바이트 배열 데이터를 덧셈 연산하여 누적합을 구한 뒤, 그 결과의 **최하위 2바이트 (Little Endian)** 값을 사용.
- **검증**: 수신 시 계산된 체크섬과 일치하지 않으면 NACK(`ETH_ACK_INFO_CS_ERR`, `0x0001`)를 전송함.

### 2.4 ACK (응답 / NACK) 패킷 규격
패킷 수신 성공/실패 여부를 응답하는 전용 메시지.
- **총 길이**: 18 Bytes (Header 12 + Data 4 + Checksum 2)
- **Data 필드 상세 (4B)**:
  - `[12~13]` **Code Info** (2B) : 수신한 원본 메시지의 대상 Code 값
  - `[14~15]` **Ack Info** (2B) : `0x0000` (정상/OK), `0x0001` (Checksum Error)

---

## 3. 이더넷 상태 머신 파이프라인 (State Machine)

`csu_Ethernet.c` 내의 `Ethernet_StateMachine()` 함수에 의해 **CM 코어의 100ms(10Hz) 주기** 백그라운드 태스크에서 폴링 구동됩니다.
> Rx 파싱의 경우, 100us 모터 제어 주기와의 간섭을 막기 위해 W6100 `INTn` 외부 인터럽트(XINT1)를 통해 비동기로 파싱 및 ACK 리턴(`csu_Ethernet_Rx_ISR` 성격)을 즉각 수행합니다.

| 상태 명칭 | 100ms 단위 상태 머신 동작 요약 |
| :--- | :--- |
| `STATE_BOOTING` | 28V 제어전원 인가 및 초기화 후 망 가입 대기 단계. 즉시 다음 상태로 천이함. |
| `STATE_WAIT_BOOT_ACK`| 망 가입을 위해 **500ms 단위**로 화포통제컴퓨터로 `ETH_CODE_BOOT_DONE` 패킷을 전송하고, ACK 응답 대기 상태. 정상 수신 시 `STATE_JOINED`로 천이. |
| `STATE_JOINED` | 망 가입 완료 및 운용 단계. **100ms 단위**로 `ETH_CODE_HEARTBEAT` 패킷을 자발적으로 교환. 이때 270V 구동 전원 인가 상태(`Power270VStatus`)를 Payload 첫 바이트에 실어 전송. |

### 3.1 타임아웃 및 재전송 (Fail-Safe) 로직
1. **ACK 대기 타임아웃**:
   - ACK를 요청한 패킷을 전송한 후, 100ms 이내에 응답이 없으면 즉각 재전송을 시도함.
   - 이때 패킷 헤더의 `Send_Count`를 1씩 증가시킴.
2. **망 이탈(통신 두절) 롤백 규칙**:
   - 한 패킷의 최대 전송 횟수는 **총 4회** (최초 1회 + 재전송 3회).
   - 4회까지 재전송했음에도 응답이 없거나, 링크 상태(`STATE_JOINED`)에서 100ms Heartbeat 패킷을 **연속 50회(5초)** 수신하지 못한 경우 통신 두절(`STATE_COMM_LOSS`) 조건 충족.
   - **조치**: W6100 소켓(0번)을 강제 리셋(닫고 재오픈)하고, 즉시 `STATE_WAIT_BOOT_ACK` 모드로 상태를 롤백하여 망 가입 단계부터 재개함.

---

## 4. 제어 변수 및 데이터 구조 (`csu_Ethernet.h`)

체계 통신 상태 모니터링을 위해 디버그 창(CCS Expressions)에서 활용되는 주요 글로벌 변수 구조체(`xEthCtrl`) 정보입니다.

```c
typedef struct {
    EthState_e  State;              // 현재 망 가입 상태 (0: BOOTING, 1: WAIT_BOOT_ACK, 2: JOINED, 3: COMM_LOSS)
    uint32_t    LastRecvTimestamp;  // 화포통제컴퓨터가 보낸 최신 Timestamp 백업
    uint16_t    TickCount100ms;     // 100ms 틱 카운터
    uint16_t    TimeoutCount;       // 미수신 통신 두절 카운터 (Max 50)
    uint16_t    RetryCount;         // 패킷 재전송 횟수 (Max 4)
    uint16_t    WaitAckTimer;       // ACK 수신 대기 타이머 카운터
    
    uint16_t    CbitPeriodSec;      // 수신된 CBIT N초 전송 주기
    uint16_t    CbitTimer100ms;     // CBIT 주기 확인용 타이머
    uint8_t     IbitInProgress;     // IBIT 수행 진행 상태
    uint8_t     Power270VStatus;    // 270V 구동 전원 유무 상태 플래그
    
    uint8_t     WaitAckCode;        // 현재 ACK를 대기 중인 명령어 Code 기록
    uint8_t     TxBuffer[256];      // 재전송용 패킷 버퍼
    uint16_t    TxSize;             // 재전송용 패킷 크기
} stEthControl;

extern stEthControl xEthCtrl;
extern uint8_t g_isW6100Connected;  // 하드웨어 미연결 시 예외 처리 플래그 (1: 정상)
```

> **💡 개발자 및 Antigravity 가이드**
> - 본 명세서는 ATTLA-T(초장사정 자동선회잠금장치) 체계의 최신 이더넷 및 연동통제안 로직을 포괄합니다.
> - PC 시뮬레이터(C# 등) 또는 제어 소프트웨어 제작 시 1, 2장의 규격에 맞춰 UDP 소켓 통신 모듈을 작성하고 패킷 구조체를 동일하게 패킹(Packing)하여 사용하십시오.
> - CBIT의 점검 결과 데이터가 추가로 정의되면 `Ethernet_SendMessage(ETH_CODE_CBIT_REP, ...)` 함수의 매개변수에 관련 상태 데이터 페이로드를 조립해 주어야 합니다.
