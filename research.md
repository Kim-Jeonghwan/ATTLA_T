# 코드 구조 리팩토링 (헤더 파일 선언부 분리 및 매직 넘버 상수화) 조사 보고서

## 1. CSU (Control & Service Unit) 계층

### 1) csu_Control.c / .h
**현재 상태:**
- `csu_Control.c` 내부에 오프셋 보정을 위한 `static` 변수(`offsetCount`, `sumMot`, `sumBrk`)가 전역적으로 선언되어 있습니다.
- ADC 변환 상수인 `#define SCALE_ADC_3V (3.0f / 4096.0f)`가 소스 파일에 선언되어 있습니다.
**수정 방향:**
- `offsetCount`, `sumMot`, `sumBrk` 변수들을 `csu_Control.h`의 `stControlState` 구조체 멤버로 편입하여 은닉성과 응집도를 높이거나, `extern` 선언으로 헤더로 분리합니다. (상태 구조체 `xSysCtrl` 내부로 통합하는 방식 추천)
- `SCALE_ADC_3V` 매크로를 헤더로 이동합니다.

### 2) csu_Encoder.c / .h
**현재 상태:**
- `csu_Encoder.c`의 `Encoder_UpdatePosition` 함수 내부에 34비트 롤오버 상수 `0x400000000ULL`과, 18비트 해상도 변환 상수 `0.001373291015625f`가 하드코딩 되어 있습니다.
**수정 방향:**
- 상수들을 `csu_Encoder.h`에 직관적인 이름의 `#define` 매크로로 정의합니다.
  - 예: `#define ENC_ROLLOVER_34BIT   0x400000000ULL`
  - 예: `#define ENC_SCALE_18BIT_DEG  0.001373291015625f`

### 3) csu_MotorCtrl.c / .h
**현재 상태:**
- `csu_MotorCtrl.c` 내부에 위치 스케일 상수 `0.001373291f`, 속도 스케일 상수 `166.6667f`, 그리고 PWM Duty 출력 한도 `100.0f`가 매직 넘버로 사용되고 있습니다.
- `MotorCtrl_Init` 내의 PID 제어기 초기화용 Gain 값들(Kp, Ki, Kd)도 하드코딩되어 있습니다.
**수정 방향:**
- `csu_MotorCtrl.h`에 `#define`으로 각 PID 상수, Duty 제한치(Limit) 및 스케일 팩터들을 모두 선언하여 파라미터 튜닝을 용이하게 합니다.
  - 예: `#define MOTOR_DUTY_MAX 100.0f`

## 2. HAL (Hardware Abstraction Layer) 계층

### 1) hal_Adc.c / .h
**현재 상태:**
- `hal_Adc.c` 내에 이동 평균 필터 카운트 `#define DEFAULT_MAVE_COUNT 100u` 및 주파수 `#define DEFAULT_PWM_HZ 100000u`가 선언되어 있습니다.
**수정 방향:**
- 해당 매크로 선언들을 `hal_Adc.h`로 추출하여 이동합니다.

### 2) hal_Ethernet.c / .h
**현재 상태:**
- `hal_Ethernet.c` 내에 UDP 소켓 번호와 포트 번호 `#define SOCK_UDP_COM 0`, `#define PORT_UDP_COM 5001`이 선언되어 있습니다.
**수정 방향:**
- 소켓/포트 설정 매크로를 `hal_Ethernet.h`로 이동합니다.

### 3) hal_Sci.c / .h
**현재 상태:**
- `hal_Sci.c` 내에 SCI PC 통신용 GPIO 핀 번호와 핀 설정 매크로(`SCI_PC_GPIO_PIN_SCIA_RXD` 등 4개)가 선언되어 있습니다.
- 송수신용 큐 구조체인 `static stQsci xQueSCI_PC;`가 파일 레벨에 정적으로 선언되어 있습니다.
**수정 방향:**
- 통신 포트/핀 관련 매크로들을 `hal_Sci.h`로 이동합니다.
- (추가 옵션) 큐 구조체 변수의 선언부를 헤더로 이동하여 다른 모듈과의 연동성을 높일 수도 있습니다.

### 4) hal_Spi.c / .h
**현재 상태:**
- `hal_Spi.c` 내에 엔코더용 SPI GPIO 핀 번호 `#define ENCODER_SOMI_GPIC 51u`, `#define ENCODER_CLK_GPIC 52u`가 선언되어 있습니다.
**수정 방향:**
- 하드웨어 핀 맵핑을 명확히 하기 위해 핀 설정 매크로들을 `hal_Spi.h`로 이동합니다.

---
**조사 완료:** 위 내용을 기반으로 각 파일의 `.c` -> `.h` 헤더 분리 및 매직 넘버 리팩토링 구현을 시작할 수 있도록 준비했습니다.
