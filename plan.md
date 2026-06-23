# 코딩 규칙 및 CSU/HAL 구조 불일치 리팩토링 계획 (Implementation Plan) - [모든 작업 완료]

본 문서는 ATTLA-T 프로젝트의 `CSU`, `HAL`, `main` 디렉토리 내에서 발견된 코딩 규칙 위반 및 계층 아키텍처 불일치 문제를 해결하기 위한 리팩토링 반영 계획입니다.

---

## 1. 개요 및 목표 [완료]
- **매크로 및 전역 변수 정의 위치 규칙 준수** [완료]
- **명명 규칙 일관성 확립** [완료]
- **외부 라이브러리/SDK 계층의 물리적 분리** [완료] (easyDSP는 SDK로 이관하고, W6100 라이브러리는 프로젝트 구조 유지를 위해 HAL에 보존)
- **정적 시험(DAPA SCR-G) 및 CWE 보안 취약점 제거** [완료]
- **헤더 주석 및 이력 업데이트** [완료]

---

## 2. 세부 작업 계획 (Proposed Changes) [완료]

### 2.1 매크로 및 전역 변수 위치 시정 [완료]
#### [MODIFY] [main.h](file:///d:/Nexcom/Firmware/01_Project/04_ATTLA/ATTLA_T/ATTLA_T/ATTLA_T_CPU1/main.h) [완료]
- `main.c`에 정의된 전역 변수 `FramTest`에 대한 외부 참조용 `extern uint16_t FramTest;` 선언을 global 영역에 추가 완료.

#### [MODIFY] [csu_SciPc.h](file:///d:/Nexcom/Firmware/01_Project/04_ATTLA/ATTLA_T/ATTLA_T/ATTLA_T_CPU1/CSU/csu_SciPc.h) [완료]
- `csu_SciPc.c`에 은폐 정의되었던 아래 매크로 상수 3종을 이관 선언 완료.
  ```c
  #define SCI_PC_SOF    0x7Eu
  #define SCI_PC_EOT    0x0Du
  #define SCI_PC_MSG1   0x10u
  ```

#### [MODIFY] [csu_SciPc.c](file:///d:/Nexcom/Firmware/01_Project/04_ATTLA/ATTLA_T/ATTLA_T/ATTLA_T_CPU1/CSU/csu_SciPc.c) [완료]
- 라인 23~25의 `#define` 매크로 정의 코드 삭제 완료.

---

### 2.2 변수 명명 일관성 수정 [완료]
#### [MODIFY] [hal_Ethernet.h](file:///d:/Nexcom/Firmware/01_Project/04_ATTLA/ATTLA_T/ATTLA_T/ATTLA_T_CPU1/HAL/hal_Ethernet.h) [완료]
- `extern uint8_t g_isW6100Connected;` 선언에서 헝가리안 접두어 `g_`를 배제한 `extern uint8_t isW6100Connected;`로 명칭 변경 완료.

#### [MODIFY] [hal_Ethernet.c](file:///d:/Nexcom/Firmware/01_Project/04_ATTLA/ATTLA_T/ATTLA_T/ATTLA_T_CPU1/HAL/hal_Ethernet.c) 및 기타 참조 파일들 [완료]
- `g_isW6100Connected` 변수가 쓰인 모든 참조부를 `isW6100Connected`로 치환 수정 완료. (`hal_Ethernet.c`, `main.c`는 미참조 상태 확인, `csu_Ethernet.c` 완료)

---

### 2.3 외부 라이브러리/SDK 파일 배치 이관 및 include 조정 [완료]
#### [DELETE/MOVE] `HAL/` 내 외부 라이브러리 파일 이동 [완료]
- W6100 라이브러리 파일(`socket.c/.h`, `w6100.c/.h`, `wizchip_conf.c/.h`)은 프로젝트 관리 편의성을 위해 **`HAL` 디렉토리에 그대로 보존**하기로 확정하였습니다.
- easyDSP 라이브러리 파일(`easy28x_driverlib_v12.2.c/.h`)만 **`SDK/easydsp/` 폴더로 이관** 완료하였습니다.

#### [MODIFY] [main.h](file:///d:/Nexcom/Firmware/01_Project/04_ATTLA/ATTLA_T/ATTLA_T/ATTLA_T_CPU1/main.h) [완료]
- easyDSP 라이브러리는 이관된 경로인 `SDK/easydsp/easy28x_driverlib_v12.2.h`로 갱신 완료하였습니다.
- W6100 라이브러리는 `HAL`에 유지되므로 기존 경로(`socket.h`, `w6100.h`, `wizchip_conf.h`)로 인클루드를 복구 완료하였습니다.

---

### 2.4 정적 시험 및 신뢰성 보안 취약점 보완 [완료]
#### [MODIFY] [csu_Pid.c](file:///d:/Nexcom/Firmware/01_Project/04_ATTLA/ATTLA_T/ATTLA_T/ATTLA_T_CPU1/CSU/csu_Pid.c) [완료]
- **CWE-369 (Divide by Zero) 해결**: `PID_Calculate` 함수에서 `pid->dt > 1.0e-6f` 일 때만 미분 분모 연산을 수행하도록 방어 조건문 추가 완료.
- **CWE-476 (Null Pointer Dereference) 해결**: `PID_Init` 및 `PID_Calculate` 함수 도입부에 포인터 인자 `pid`가 `NULL`인지 점검하는 코드 삽입 완료. (단일 리턴 구조 적용 완료)

#### [MODIFY] [hal_Common.h](file:///d:/Nexcom/Firmware/01_Project/04_ATTLA/ATTLA_T/ATTLA_T/ATTLA_T_CPU1/HAL/hal_Common.h) [완료]
- 브레이크 핀 및 모니터링용 LED 핀의 하드코딩 매직 넘버를 대체할 매크로 상수 추가 완료.
  ```c
  #define GPIO_PIN_MOTOR_BRAKE 35U  // 모터 브레이크 제어 핀 (Active High)
  #define GPIO_PIN_ALIVE_LED   34U  // 메인컨트롤 ISR 동작 상태 모니터링 LED 핀
  ```

#### [MODIFY] [csu_MotorCtrl.c](file:///d:/Nexcom/Firmware/01_Project/04_ATTLA/ATTLA_T/ATTLA_T/ATTLA_T_CPU1/CSU/csu_MotorCtrl.c) 및 [csu_Control.c](file:///d:/Nexcom/Firmware/01_Project/04_ATTLA/ATTLA_T/ATTLA_T/ATTLA_T_CPU1/CSU/csu_Control.c) [완료]
- 브레이크 핀 제어 함수 `GPIO_writePin(35U, ...)` 코드를 `GPIO_writePin(GPIO_PIN_MOTOR_BRAKE, ...)`으로 변경 완료.
- LED 토글 함수 `GPIO_togglePin(34U)` 코드를 `GPIO_togglePin(GPIO_PIN_ALIVE_LED)`으로 변경 완료.

---

### 2.5 헤더 주석 및 수정 이력 추가 [완료]
- 수정 대상 모든 소스 및 헤더 파일 상단에 위치한 파일 헤더 정보를 최신 수정 내용과 날짜에 맞춰 업데이트 완료.
- 수정 내용 요약을 `Modification History` 주석 블록에 순서대로 기재 완료.
  - **작성자**: `Kim Jeonghwan`
  - **일자**: `2026. 06. 23.`
  - **내용**: `(코딩 규칙 및 구조 불일치 사항 리팩토링 반영)`

---

## 3. 검증 계획 (Verification Plan) [완료]

### 3.1 컴파일 및 빌드 검증 [사용자 대기]
- 모든 리팩토링이 완료되면, 에이전트 직접 빌드가 불가능하므로 사용자에게 빌드를 요청합니다.
- 사용자는 **CCS(Code Composer Studio) Theia**를 통해 CPU1 프로젝트 빌드를 직접 수행하고 컴파일 에러나 링커 에러가 없는지 검증합니다.

### 3.2 정적 기준 충족 여부 확인 [완료]
- 정적 취약점 방어(널 포인터 및 0 나누기 방어 조건문)가 단일 리턴 룰을 유지하며 구조적으로 안전하게 적용되었는지 view_file 검증을 완료하였습니다.
