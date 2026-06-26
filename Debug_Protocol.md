# ATTLA_T 노트북 디버그 통신 프로토콜 명세서 (W6100 기반)

본 문서는 ATTLA_T 시스템의 CPU1 코어가 W6100 이더넷(SPI-A)을 통해 노트북(PC)과 연결되어 모니터링 및 상태 디버깅(Telemetry)을 수행하기 위한 UDP 프로토콜 규격을 정의합니다.

> [!NOTE]
> CM 코어에서 전담하는 체계 연동(FC) 통신 규격과는 분리된 독립적인 규격이며, 확장성을 고려하여 기존 12바이트 패킷 헤더 구조를 일부 차용합니다.

---

## 1. 통신 기본 설정

*   **물리 계층 (PHY)**: W6100 모듈 연결 (RJ-45)
*   **전송 계층**: UDP / IPv4
*   **통신 방식**: 
    *   **동적 IP 라우팅 (Dynamic Routing)**: DSP(CPU1)는 PC의 고정 IP를 미리 지정하지 않으며, PC에서 첫 패킷을 전송하면 해당 송신자의 IP와 Port를 내부적으로 캡처하여 응답 타겟으로 설정합니다.
*   **DSP(CPU1) 고정 IP 및 포트**:
    *   **IP Address**: `192.168.200.11`
    *   **Subnet Mask**: `255.255.255.0`
    *   **Gateway**: `192.168.200.1`
    *   **수신 포트 (RX Port)**: `5002`

---

## 2. 프로토콜 구조 (Packet Structure)

전체 패킷은 `[Header (12 Bytes)] + [Payload (Data_Length)] + [Checksum (2 Bytes)]` 로 구성됩니다.

### 2.1 패킷 헤더 (Header)

헤더는 체계 연동망과의 호환성을 고려하여 기존 포맷을 유지합니다. (총 12바이트, Little-Endian)

| Offset | 크기(Bytes) | 필드 명 (Field) | 설명 |
| :--- | :--- | :--- | :--- |
| 0 | 4 | `Timestamp` | PC의 Tick Count (필요시 사용, 기본값 0) |
| 4 | 1 | `Source_ID` | 송신 장치 ID (PC: `0x10`, DSP: `0x11`) |
| 5 | 1 | `Dest_ID` | 수신 목적지 ID (PC: `0x10`, DSP: `0x11`) |
| 6 | 1 | `Code` | 메시지 명령어 코드 (명령어 세트 참조) |
| 7 | 1 | `Request_ACK` | ACK 요청 플래그 (디버그망에서는 현재 미사용, `0xFF`) |
| 8 | 1 | `Priority` | 우선순위 (기본 `0x00`) |
| 9 | 1 | `Send_Count` | 전송 카운트 (기본 `0x01`) |
| 10 | 2 | `Data_Length` | 뒤따라오는 Payload 데이터의 바이트 크기 |

### 2.2 체크섬 (Checksum)

*   **크기**: 2바이트 (Little-Endian)
*   **계산 방식**: 헤더와 페이로드의 모든 바이트 값을 단순 누적 합산(`sum += byte`)하여 얻은 하위 16비트(`sum & 0xFFFF`) 값.
*   **위치**: 페이로드의 맨 마지막 (데이터 길이가 0인 경우 헤더 12바이트 바로 뒤인 13~14번째 바이트에 위치).

---

## 3. 명령어 세트 (Command Codes)

디버깅망에서 사용되는 `Code` 값의 정의입니다.

| Code | 매크로 명칭 | 방향 | Payload 크기 | 설명 |
| :--- | :--- | :--- | :--- | :--- |
| `0x20` | `DBG_CODE_TELEMETRY` | DSP ➡️ PC | 20 Bytes (가변) | **자동 텔레메트리 송신**: 100ms 주기로 DSP 상태 보고 |
| `0x21` | `DBG_CODE_REQ_STATE` | PC ➡️ DSP | 0 Bytes | **상태 강제 요청**: PC가 상태 데이터를 즉각 요구할 때 전송 |
| `0x22` | `DBG_CODE_CLR_FAULT` | PC ➡️ DSP | 0 Bytes | **폴트(에러) 초기화 명령**: DSP의 폴트 상태 리셋 |

---

## 4. 텔레메트리 (모니터링) 데이터 규격

`DBG_CODE_TELEMETRY` (`0x20`) 수신 시 Payload에 포함되는 디버그용 상태 구조체입니다.  
> [!IMPORTANT]
> 본 구조체는 초기 개발용(Dummy)으로, 향후 GUI 소프트웨어 및 세부 분석 항목이 확정됨에 따라 펌웨어 코드의 구조체(`stDbgTelemetry`) 수정과 함께 확장될 예정입니다.

| Payload Offset | 크기(Bytes) | 자료형 | 데이터 항목 (Variable) | 설명 |
| :--- | :--- | :--- | :--- | :--- |
| 0 | 4 | `uint32_t` | `systemTick` | DSP의 내부 100ms 누적 카운트 |
| 4 | 4 | `float` (※임시 32비트) | `currentA` | 모터 A상 전류 (현재 0.0f 더미) |
| 8 | 4 | `float` | `currentB` | 모터 B상 전류 (현재 0.0f 더미) |
| 12 | 4 | `float` | `currentC` | 모터 C상 전류 (현재 0.0f 더미) |
| 16 | 4 | `uint32_t` | `faultStatus` | BIT(진단) 및 에러 플래그 (xBit.informAll 매핑) |

---

## 5. 통신 시퀀스 특징

1.  **초기 상태 (대기)**: DSP 부팅 후 PC 측으로 선제적인 패킷을 전송하지 않습니다.
2.  **망 활성화 (Activation)**: 노트북(PC)에서 `0x21`(상태 요청) 또는 `0x22`(초기화) 명령 등 아무 UDP 패킷이나 DSP(`192.168.200.11:5002`)로 보내면, DSP는 해당 패킷의 IP를 **동적 타겟**으로 지정하여 활성화(`isActive = 1`) 상태가 됩니다.
3.  **자동 송신**: 활성화 상태가 되면 DSP는 타겟 IP를 향해 100ms 주기로 `DBG_CODE_TELEMETRY (0x20)` 패킷을 보냅니다.
4.  **통신 두절 (Timeout)**: PC로부터 5초(50 ticks) 이상 어떠한 패킷도 수신되지 않으면 DSP는 송신을 중단하고 다시 대기 상태로 전환됩니다.
