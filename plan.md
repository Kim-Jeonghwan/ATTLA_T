# 구현 계획 (Implementation Plan): 헤더 파일 선언부 분리 및 매직 넘버 상수화

본 계획은 새롭게 추가된 글로벌 코딩 룰인 **"모든 매크로 상수(`#define`)와 변수 선언은 헤더(`.h`)에 작성"** 규칙을 준수하여 작성되었습니다. 각 파일 쌍(`.c` / `.h`)에 대해 상단 버전(Version) 및 수정 이력(Modification History) 업데이트가 함께 진행됩니다.

## 1. CSU 계층 리팩토링 계획

### 1) csu_Control.c / .h
- **.c에서 제거:** 
  - `static uint16_t offsetCount;`
  - `static float32_t sumMot;`
  - `static float32_t sumBrk;`
  - `#define SCALE_ADC_3V (3.0f / 4096.0f)`
- **.h에 추가:**
  - `SCALE_ADC_3V` 매크로 정의 추가.
  - 제거된 3개의 `static` 변수를 `stControlState` 구조체(`xSysCtrl`)의 멤버 변수로 통합하여 상태 관리 응집도를 높이고 헤더를 통해 공유 가능하도록 선언.
- **수정 로직:** `csu_Control.c` 내에서 기존의 `offsetCount` 등 정적 변수를 사용하던 부분을 `xSysCtrl.offsetCount` 형태로 변경.

### 2) csu_Encoder.c / .h
- **.c에서 제거:** 매직 넘버 하드코딩된 값 `0x400000000ULL` 및 `0.001373291015625f`
- **.h에 추가:**
  - `#define ENC_ROLLOVER_34BIT   0x400000000ULL`
  - `#define ENC_SCALE_18BIT_DEG  0.001373291015625f`
- **수정 로직:** `Encoder_UpdatePosition()` 내에서 상수가 사용되던 부분을 새로 선언된 직관적인 매크로명으로 전부 대체.

### 3) csu_MotorCtrl.c / .h
- **.c에서 제거:** 
  - 엔코더 스케일 상수 `0.001373291f`, 속도 연산 스케일 상수 `166.6667f`
  - PID 한계/제한 매직넘버 `100.0f`
  - PID 초기화 Gain 상숫값들 (전류, 속도, 위치 제어기)
- **.h에 추가:**
  - `#define MOTOR_SCALE_POS_DEG    0.001373291f`
  - `#define MOTOR_SCALE_SPEED_RPM  166.6667f`
  - `#define MOTOR_DUTY_MAX         100.0f`
  - 전류 제어기 상수: `#define PID_CURR_KP 2.0f`, `KI 0.05f`, `KD 0.0f`, `DT 0.0001f` 등
  - 속도 제어기 상수: `#define PID_SPD_KP 0.5f` 등
  - 위치 제어기 상수: `#define PID_POS_KP 1.0f` 등
- **수정 로직:** `.c`의 로직 연산부와 `MotorCtrl_Init()` 함수에서 매크로 상수명을 호출하도록 일괄 변경.

## 2. HAL 계층 리팩토링 계획

### 1) hal_Adc.c / .h
- **.c에서 제거:** 
  - `#define DEFAULT_MAVE_COUNT 100u`
  - `#define DEFAULT_PWM_HZ 100000u`
- **.h에 추가:** 제거된 두 매크로를 헤더의 `#define` 영역으로 복사하여 글로벌하게 정의.

### 2) hal_Ethernet.c / .h
- **.c에서 제거:**
  - `#define SOCK_UDP_COM 0`
  - `#define PORT_UDP_COM 5001`
- **.h에 추가:** 해당 상수들을 헤더 파일 상단으로 이동.

### 3) hal_Sci.c / .h
- **.c에서 제거:**
  - RX/TX GPIO 핀 및 설정 매크로 4종 (`SCI_PC_GPIO_PIN_SCIA_RXD` 등)
  - `static stQsci xQueSCI_PC;` (정적 큐 변수 선언부)
- **.h에 추가:**
  - 제거된 핀 매크로 4종 추가.
  - `extern stQsci xQueSCI_PC;` 선언을 헤더에 명시적으로 추가하여 외부 접근성을 제공하고 선언을 분리.
- **수정 로직:** `.c`에서는 전역 선언 `stQsci xQueSCI_PC;` (static 키워드 제거)로 유지하여 헤더의 extern 선언과 연결.

### 4) hal_Spi.c / .h
- **.c에서 제거:**
  - `#define ENCODER_SOMI_GPIC 51u`
  - `#define ENCODER_CLK_GPIC 52u`
- **.h에 추가:** 핀 설정 매크로들을 헤더 파일로 이동.

---
**검토 요청 (User Review Required)**
1. `csu_Control.c`의 `static` 정적 변수들을 글로벌 `stControlState xSysCtrl` 구조체 멤버로 편입시키는 방안으로 진행해도 좋습니까? (추천 방식)
2. `hal_Sci.c`의 큐 구조체인 `xQueSCI_PC` 변수는 `.h` 파일에 `extern`으로 노출하여 사용하는 것이 의도에 부합합니까?

확인 후 **"승인"** 또는 **"구현해"** 라고 지시해 주시면 즉시 코드 변경 작업을 수행하겠습니다.
