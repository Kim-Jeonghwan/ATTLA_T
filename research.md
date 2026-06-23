# 🌐 ATTLA-T 프로젝트 코딩 규칙 및 구조(CSU/HAL) 불일치 조사 보고서

본 문서는 ATTLA-T 프로젝트의 `CSU`, `HAL`, `main` 디렉토리 내 소스 및 헤더 파일을 전수 조사하여, 명시된 코딩 규칙 및 CSU/HAL 3계층 구조 원칙과 일관되지 않거나 어긋나는 항목들을 정리한 보고서입니다.

---

## 1. 매크로 상수 및 전역 변수 정의 위치 위반 (Macro Constants & Declarations Rule)
> [!IMPORTANT]
> **준수 규칙**: 모든 매크로 상수(`#define`), 전역 변수, 스케일 팩터 등의 선언은 `.c` 소스 파일 내부가 아닌, 반드시 해당 모듈의 헤더 파일(`.h`)에 작성해야 합니다. 매직 넘버 사용을 지양하고 상수를 직관적인 이름으로 정의하여 유지보수성을 극대화하십시오.

### 1.1 `main.c` 내 전역 변수 누락 및 독단 정의
* **위치**: [main.c](file:///d:/Nexcom/Firmware/01_Project/04_ATTLA/ATTLA_T/ATTLA_T/ATTLA_T_CPU1/main.c#L31)
* **코드 내용**: `uint16_t FramTest = 0u;`
* **위반 세부사항**: `main.h` 등 헤더 파일에 `extern uint16_t FramTest;` 선언이 누락된 상태로 소스 파일 내부에 전역 변수가 정의되어 있습니다.

### 1.2 `csu_SciPc.c` 내 매크로 상수 `#define` 정의
* **위치**: [csu_SciPc.c](file:///d:/Nexcom/Firmware/01_Project/04_ATTLA/ATTLA_T/ATTLA_T/ATTLA_T_CPU1/CSU/csu_SciPc.c#L23-L25)
* **코드 내용**:
  ```c
  #define SCI_PC_SOF    0x7Eu
  #define SCI_PC_EOT    0x0Du
  #define SCI_PC_MSG1   0x10u
  ```
* **위반 세부사항**: PC 통신 프로토콜 관련 프레임 제어 매크로 상수가 `.c` 소스 파일 내부에 정의되어 은폐되어 있습니다. 해당 상수 정의는 모듈 헤더 파일인 [csu_SciPc.h](file:///d:/Nexcom/Firmware/01_Project/04_ATTLA/ATTLA_T/ATTLA_T/ATTLA_T_CPU1/CSU/csu_SciPc.h)로 이관해야 합니다.

---

## 2. 변수 명명 규칙 일관성 결여 및 네임스페이스 이탈 (Naming Convention Rule)
> [!IMPORTANT]
> **준수 규칙**: `HAL` 계층 모듈 및 파일명에만 소문자 `hal_` 접두어를 사용하며, 구조체 이름, 변수명, 함수명 등 어떠한 코드 내부 식별자에도 접두어(`hal_`, `csu_`)를 절대 사용해서는 안 됩니다.

### 2.1 `hal_Ethernet.h` 내 전역 변수 `g_isW6100Connected`
* **위치**: [hal_Ethernet.h](file:///d:/Nexcom/Firmware/01_Project/04_ATTLA/ATTLA_T/ATTLA_T/ATTLA_T_CPU1/HAL/hal_Ethernet.h#L38)
* **코드 내용**: `extern uint8_t g_isW6100Connected;`
* **위반 세부사항**: 
  - 타 모듈들(`xTimer`, `xLed`, `xAdc`, `xSysCtrl` 등)의 경우 상태를 나타내는 구조체 접두어 `x`를 붙여 정밀 제어되거나, 카멜케이스 단독(`adcRawData`)으로 선언되는 반면, 본 변수는 유일하게 헝가리안 표기법 접두어 `g_`가 붙어 있어 프로젝트 전반의 명명 일관성을 깨뜨립니다.
  - **개선 제안**: 이더넷 상태 구조체인 `xEthCtrl` 내의 멤버 변수로 편입시키거나, `isW6100Connected` 형태의 표준 카멜케이스 구조로 변경할 것을 권장합니다.

---

## 3. 외부 라이브러리 및 SDK 파일의 물리적 배치 (Project Architecture & Layering)
> [!IMPORTANT]
> **준수 규칙**: MCU의 주변장치와 맞닿아 제어하는 `HAL` 추상화 계층과 TI 제조사 혹은 칩셋 제조사에서 공식 제공하는 라이브러리(`SDK` 계층)는 명확히 물리적/논리적으로 분리 보존되어야 합니다.

### 3.1 `HAL` 디렉토리 내 외부 드라이버 라이브러리 혼재
* **위치**: `ATTLA_T_CPU1/HAL/` 디렉토리 하위
* **대상 파일**:
  - easyDSP 통신 드라이버: [easy28x_driverlib_v12.2.c](file:///d:/Nexcom/Firmware/01_Project/04_ATTLA/ATTLA_T/ATTLA_T/ATTLA_T_CPU1/HAL/easy28x_driverlib_v12.2.c), [easy28x_driverlib_v12.2.h](file:///d:/Nexcom/Firmware/01_Project/04_ATTLA/ATTLA_T/ATTLA_T/ATTLA_T_CPU1/HAL/easy28x_driverlib_v12.2.h)
  - Wiznet W6100 칩셋 드라이버: `socket.c/.h`, `w6100.c/.h`, `wizchip_conf.c/.h`
* **위반 세부사항**: 
  - 해당 파일들은 하드웨어 제어권을 통제하기 위해 외부 제조사(Wiznet, easyDSP)에서 제공하는 순수 라이브러리 및 드라이버 코드로, 논리상 **SDK 계층**에 속합니다. 
  - 그럼에도 불구하고 현재 하드웨어 추상화 계층인 `HAL` 디렉토리에 물리적으로 혼재되어 있어 계층 분리 원칙에 어긋납니다.
  - 또한, `hal_` 파일명 접두어 규칙(HAL 디렉토리 내 커스텀 파일은 반드시 접두어 사용)도 적용되지 않아 파일명 일관성이 저해됩니다.
  - **개선 제안**: [SDK](file:///d:/Nexcom/Firmware/01_Project/04_ATTLA/ATTLA_T/ATTLA_T/ATTLA_T_CPU1/SDK) 디렉토리 하위 또는 `External_Library` 폴더를 별도로 구성하여 이관 배치하는 것이 타당합니다.

---

## 4. 정적 시험 기준 및 보안 취약점 보완 항목 (Static Analysis Standards)
> [!IMPORTANT]
> **준수 규칙**: 신뢰성과 안전성을 보장하기 위해 방위사업청 무기체계 소프트웨어 코딩규칙(DAPA SCR-G) 및 CWE-658 C 언어 보안 취약점 기준(Divide by Zero 방지, Null Pointer 역참조 방지 등)을 전수 준수해야 합니다.

### 4.1 CWE-369 (Divide by Zero) 취약점 위험
* **위치**: [csu_Pid.c](file:///d:/Nexcom/Firmware/01_Project/04_ATTLA/ATTLA_T/ATTLA_T/ATTLA_T_CPU1/CSU/csu_Pid.c#L69)
* **코드 내용**: `dOut = pid->Kd * ((error - prevError) / pid->dt);`
* **위반 세부사항**: `dt`는 제어 주기를 나타내는 구조체 멤버 변수이나, 제어기 초기화 또는 변경 도중 실수로 `0.0f`가 입력되는 경우 **0으로 나누기(Divide by Zero)** 결함이 발생할 수 있는 취약성이 존재합니다. 
* **개선 제안**: 연산 수행 전 분모인 `pid->dt`가 `0`보다 큰 값인지 사전에 체크하거나 임계값을 넘는지 확인하는 방어 로직이 필요합니다.

### 4.2 CWE-476 (Null Pointer Dereference) 취약점 위험
* **위치**: [csu_Pid.c](file:///d:/Nexcom/Firmware/01_Project/04_ATTLA/ATTLA_T/ATTLA_T/ATTLA_T_CPU1/CSU/csu_Pid.c#L31), [csu_Pid.c](file:///d:/Nexcom/Firmware/01_Project/04_ATTLA/ATTLA_T/ATTLA_T/ATTLA_T_CPU1/CSU/csu_Pid.c#L54)
* **함수**: `PID_Init`, `PID_Calculate`
* **위반 세부사항**: 전달받은 `PID_Controller_t* pid` 구조체 포인터 인자를 대상으로 `NULL` 검사 없이 즉각 역참조(`pid->Kp` 등)하여 대입 또는 연산에 활용하고 있어 Null Pointer 참조 버그에 노출되어 있습니다.
* **개선 제안**: 함수 도입부에서 `if (pid == NULL)` 분기 처리를 통해 안전하게 리턴 처리하거나 방어하는 예외 처리 구문이 누락되어 있습니다.

### 4.3 매직 넘버 하드코딩 (Magic Number Avoidance)
* **위치**: 
  - [csu_MotorCtrl.c](file:///d:/Nexcom/Firmware/01_Project/04_ATTLA/ATTLA_T/ATTLA_T/ATTLA_T_CPU1/CSU/csu_MotorCtrl.c#L168) (브레이크 잠금/해제 GPIO 핀 35번 하드코딩)
  - [csu_Control.c](file:///d:/Nexcom/Firmware/01_Project/04_ATTLA/ATTLA_T/ATTLA_T/ATTLA_T_CPU1/CSU/csu_Control.c#L273) (동작 모니터링 토글 LED GPIO 핀 34번 하드코딩)
* **위반 세부사항**: GPIO 제어 대상 핀 번호(`35U`, `34U`)가 소스 코드 내부에 상수가 아닌 매직 넘버로 직접 기술되어 있어 하드웨어 핀 맵이 변경되거나 튜닝 시 관리에 취약합니다.
* **개선 제안**: [hal_Common.h](file:///d:/Nexcom/Firmware/01_Project/04_ATTLA/ATTLA_T/ATTLA_T/ATTLA_T_CPU1/HAL/hal_Common.h) 또는 모듈 헤더 파일에 `#define GPIO_PIN_MOTOR_BRAKE 35U`, `#define GPIO_PIN_ALIVE_LED 34U` 등으로 추상화하여 사용해야 합니다.
