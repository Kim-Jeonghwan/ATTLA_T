# 구현 계획 (Implementation Plan)

## 1. 개요
본 문서는 `GEMINI.md` 코딩 규칙 위반 사항을 바로잡기 위한 코드 수정 계획입니다.
사용자 요청에 따라 WIZnet 라이브러리(`w6100.c` 등) 및 `easy28x_driverlib` 관련 파일들은 수정 대상에서 제외하거나 접두어 없이 현 상태를 유지하며, 그 외 `csu_` 접두어 오용 및 중복 인클루드 문제를 해결합니다.

## 2. 세부 수정 계획

### 2.1. `csu_Ethernet.h` 및 `csu_Ethernet.c` 수정
- **헤더 인클루드 수정**
  - `csu_Ethernet.c` 파일 내에 불필요하게 선언된 `#include "hal_Ethernet.h"`를 제거합니다. (의존성은 `csu_Ethernet.h` -> `main.h` -> `hal_Ethernet.h`를 통해 자동 해결됨)
- **함수명 접두어(`csu_`) 제거** (함수 선언, 정의 및 내부 호출 모두 수정)
  - `csu_Ethernet_Init()` -> `Ethernet_ProtocolInit()` (HAL 계층의 `Ethernet_Init()`과의 이름 충돌을 피하기 위해 변경)
  - `csu_Ethernet_CalculateChecksum()` -> `Ethernet_CalculateChecksum()`
  - `csu_Ethernet_SendAck()` -> `Ethernet_SendAck()` (내부 static 함수)
  - `csu_Ethernet_SendMessage()` -> `Ethernet_SendMessage()`
  - `csu_Ethernet_StateMachine()` -> `Ethernet_StateMachine()`
  - `csu_Ethernet_ParsePacket()` -> `Ethernet_ParsePacket()`

### 2.2. `csu_Control.h` 및 `csu_Control.c` 수정
- **인터럽트 서비스 루틴(ISR) 함수명 접두어(`csu_`) 제거**
  - `csu_Offset_Isr()` -> `Offset_Isr()`
  - `csu_Pbit_Isr()` -> `Pbit_Isr()`
  - `csu_MainControl_Isr()` -> `MainControl_Isr()`
- **연관 코드 수정**
  - `#pragma CODE_SECTION`에 명시된 함수명 변경 적용
  - `PieVectTable.EPWM1_INT`에 ISR 함수 포인터를 할당하는 부분의 이름 변경 적용

### 2.3. `hal_Ethernet.c` 연동 수정
- 변경된 함수명 호출 반영: `csu_Ethernet_ParsePacket(...)` -> `Ethernet_ParsePacket(...)`

### 2.4. `main.c` 연동 수정
- 변경된 함수명 호출 및 ISR 등록 반영
  - `csu_Ethernet_Init()` -> `Ethernet_ProtocolInit()`
  - `csu_Ethernet_StateMachine()` -> `Ethernet_StateMachine()`
  - `Interrupt_register(INT_EPWM1, &csu_Offset_Isr);` -> `Interrupt_register(INT_EPWM1, &Offset_Isr);`

### 2.5. 파일 공통 적용 사항
- 수정되는 모든 파일(`.c`, `.h`)의 최상단 헤더 주석에서 **Version**을 `00.01`씩 증가시킵니다.
- **Modification History** 섹션에 금일 날짜로 수정 사항(명명 규칙 위반 리팩토링 및 헤더 인클루드 수정)을 한 줄로 기록합니다.

---

**사용자님, 위 계획(plan.md)을 검토해 주시고, 승인해 주시면 실제 코드 수정을 진행하겠습니다. 추가하실 제약 조건이나 수정할 이름이 있다면 메모를 남겨주세요.**
