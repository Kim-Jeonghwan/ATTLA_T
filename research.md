# ATTLA_T 듀얼 채널 이더넷 아키텍처 반영 현황 조사 보고서 (Research Report)

## 1. 조사 목적
최근 업데이트된 `Architecture.md`의 시스템 구성도 내용(이더넷 통신망 분리)이 실제 펌웨어 소스코드(CPU1 및 CM 코어의 CSU, HAL, MAIN)에 올바르게 반영되어 있는지 교차 검증하고, 불일치하는 부분을 식별하여 향후 수정 및 리팩토링 방향을 수립하기 위함입니다.

### 업데이트된 아키텍처 기준 사항
- **CPU1 코어**: W6100 칩(SPI-A)을 제어하여 **노트북(분석 및 디버깅용) 통신망**을 담당합니다.
- **CM 코어**: 내부 EMAC 및 PHY 칩을 제어하여 **상위 제어기(화포통제컴퓨터) 체계 연동 통신망**을 전담합니다. (Boot Ack, CBIT, IBIT 등 시스템 ICD 프로토콜 수행)

---

## 2. 모듈별 상세 분석 결과

### [1] CM 코어 (ATTLA_T_CM) - 상위 제어기 연동망 (PHY/EMAC)
**결론: 아키텍처와 100% 일치하며 정상 구현됨 (수정 불필요)**

*   **`main_cm.c` (MAIN)**
    *   `Initial_Ethernet()` 및 `Ethernet_StateMachine()`을 주기적으로 호출하여 체계 연동망을 기동하고 통제합니다. **(일치)**
*   **`hal_Ethernet_cm.c` (HAL)**
    *   MII 인터페이스, 외부 25MHz 클럭, DP83822 PHY 설정 등 하드웨어 초기화 및 EMAC RX 인터럽트를 정상적으로 구성하고 있습니다. **(일치)**
*   **`csu_Ethernet_cm.c` (CSU)**
    *   상위 제어기와의 ICD(인터페이스 통제 문서) 규격에 따른 프로토콜(Boot Done, CBIT, IBIT 등)을 EMAC 패킷 전송 함수(`sendEthernetFrame`)를 통해 정상적으로 수행합니다.
    *   특히, IBIT 명령 수신 시 CPU1 코어로 `ibitClearReq` IPC 메시지를 보내 에러를 초기화하도록 위임하는 로직이 완벽히 연동되어 있습니다. **(일치)**

### [2] CPU1 코어 (ATTLA_T_CPU1) - 노트북 모니터링망 (W6100)
**결론: HAL과 MAIN은 정상이나, CSU 계층에서 심각한 아키텍처 불일치(로직 꼬임) 발생 (전면 수정 필요)**

*   **`main_cpu1.c` (MAIN)**
    *   CM 코어와의 IPC 통신(`isCmReady` 대기 등)이 잘 구성되어 있으며, `Ethernet_Init()`을 통해 W6100을 기동합니다.
    *   주기 태스크(`cycle_100ms`)에서 CM 코어로 상태 데이터(CBIT 송신용 `bitInformAll`)를 넘겨주거나, CM의 `ibitClearReq`를 받아 폴트 리셋을 수행하는 등 역할 분담이 정상적으로 이루어져 있습니다. **(일치)**
*   **`hal_Ethernet_cpu1.c` (HAL)**
    *   W6100 하드웨어를 제어하기 위해 SPI-A 콜백을 등록하고, 노트북 연결용 IP(`192.168.200.11`)를 세팅하는 등 HAL 기능이 의도에 맞게 구현되어 있습니다. **(일치)**
*   **`csu_Ethernet_cpu1.c` (CSU) 🚨 (수정 필요 대상)**
    *   **문제점**: 해당 파일은 W6100을 제어하고 있으나, 내부 로직은 노트북 모니터링용 프로토콜이 아닌 **CM 코어가 수행해야 할 체계 연동망 프로토콜(ICD - Boot Ack, PBIT/IBIT/CBIT 등)을 그대로 복사하여 수행**하고 있습니다.
    *   **증상**: `STATE_WAIT_BOOT_ACK` 상태를 운영하며 FC(화포통제컴퓨터) IP(`192.168.200.1`)로 묶여 있고, 심지어 자신이 만들어낸 `bitInformAll` 변수를 IPC 수신 버퍼(`pxDataCpu1ToCm`)에서 역으로 참조하는 등 복사-붙여넣기의 잔재가 강하게 남아 있습니다.

---

## 3. 수정 및 리팩토링 계획 (Action Items)

해당 리서치 결과를 바탕으로 다음 작업을 수행하여 아키텍처를 완벽하게 일치시켜야 합니다.

### 🔴 csu_Ethernet_cpu1.c 전면 리팩토링 (노트북 디버깅망 전용화)
1.  **기존 시스템 ICD 통신 로직 전면 삭제**:
    *   `STATE_WAIT_BOOT_ACK`, `ETH_CODE_BOOT_DONE`, `ETH_CODE_PBIT_REQ`, `ETH_CODE_IBIT_REQ`, CBIT 등 상위 제어기(FC) 전용 메시지 파싱 및 응답 로직을 모두 제거합니다.
2.  **모니터링/디버깅 프로토콜로 재구성**:
    *   W6100을 통한 통신은 노트북(PC UI)에서 모터의 상태(전류, 위치, 온도 등)를 실시간으로 확인하거나 단순한 디버깅 패킷(명령어)을 송수신하는 용도로만 동작하도록 `Ethernet_ParsePacket()` 및 `Ethernet_StateMachine()`을 단순화/재작성해야 합니다.
    *   타겟 IP 및 상태 머신(State Machine) 구조를 디버깅용 규격에 맞게 새롭게 정의해야 합니다.

*본 리서치 보고서를 확인하시고, **`csu_Ethernet_cpu1.c`의 수정(노트북 프로토콜 구현)에 대한 계획(Plan) 수립 및 코드 작성을 지시**해 주시면 즉시 작업을 진행하겠습니다.*
