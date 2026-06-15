# SPI 설정 통일화 및 매직 넘버 매크로 추출 계획

본 문서는 프로젝트 내 분산되어 있는 SPI 초기화 코드를 `hal_Spi.c`로 통일하고, 하드코딩된 매직 넘버(통신 속도, 핀 번호 등)를 `hal_Spi.h`의 매크로 상수로 추출하기 위한 리팩토링 계획입니다.

## 1. 현재 구조 분석
* **W6100 (SPI-A)**, **SSI Encoder (SPI-C)**, **FRAM (SPI-D)**: 
  * 초기화 로직이 `hal_Spi.c`의 `InitSpia()`, `InitSpic()`, `InitSpid()`에 구현되어 있습니다.
  * 통신 속도(20MHz, 2.5MHz, 1MHz)와 데이터 폭(8비트, 16비트), 핀 번호가 소스 코드 내에 하드코딩(매직 넘버)되어 있습니다.
* **Motor Driver (SPI-B)**: 
  * 초기화 로직이 `hal_MotorDriver.c`의 `MotorDriver_Init_Hardware()` 내부에 위치해 있습니다.
  * 관련 핀 매크로가 `hal_MotorDriver.h`에 정의되어 있습니다.

## 2. 변경 계획 (Refactoring Plan)

### [수정 1] `hal_Spi.h` 매크로 상수 정의
모든 SPI 모듈의 통신 설정값 및 핀 번호를 직관적인 매크로 상수로 정의합니다.

```c
// [ SPI-A : W6100 ]
#define SPIA_W6100_BAUDRATE        20000000u   // 20MHz
#define SPIA_W6100_DATA_WIDTH      8u
#define SPIA_W6100_CS_PIN          19u

// [ SPI-B : Motor Driver ]
#define SPIB_MOTOR_BAUDRATE        1000000u    // 1MHz
#define SPIB_MOTOR_DATA_WIDTH      16u
#define SPIB_MOTOR_CLK_PIN         58u
#define SPIB_MOTOR_STE_PIN         59u
#define SPIB_MOTOR_SIMO_PIN        60u
#define SPIB_MOTOR_SOMI_PIN        61u

// [ SPI-C : SSI Encoder ]
#define SPIC_SSI_BAUDRATE          2500000u    // 2.5MHz
#define SPIC_SSI_DATA_WIDTH        16u
// (기존 ENCODER_SOMI_GPIC, ENCODER_CLK_GPIC 매크로 유지 혹은 명칭 통일)

// [ SPI-D : FRAM ]
#define SPID_FRAM_BAUDRATE         1000000u    // 1MHz
#define SPID_FRAM_DATA_WIDTH       8u
// (기존 FRAM_CS_GPIO 매크로 유지 및 CLK, SIMO, SOMI 핀 91,92,93 추가 정의)
```

### [수정 2] `hal_Spi.c` 통합
* `hal_MotorDriver.c`에 있던 SPI-B 핀 설정 및 모듈 설정 코드를 `hal_Spi.c`의 `InitSpib()` 함수로 분리/이동합니다.
* `Initial_SPI()` 함수 내부에서 `InitSpib()` 도 함께 호출하도록 변경합니다.
* 코드 내 하드코딩된 숫자들을 모두 새로 정의한 매크로 상수로 교체합니다.

### [수정 3] `hal_MotorDriver.c` 및 `hal_MotorDriver.h` 수정
* `hal_MotorDriver.h`에서 SPI 핀 관련 매크로를 제거합니다.
* `hal_MotorDriver.c`의 `MotorDriver_Init_Hardware()` 함수 내 SPI 설정 부분을 제거합니다. (해당 함수에는 DRV_ENABLE, DIR 등 모터 전용 제어 핀 설정과 DRV8343 Wake-up 및 레지스터 설정 로직만 남깁니다.)

---
## 💬 사용자 확인 및 피드백 (User Review Required)
> [!IMPORTANT]
> SPI-B(모터 드라이버)의 초기화 로직을 `hal_Spi.c`로 완전히 이관하게 되면, 시스템 부팅 시 `Initial_SPI()`가 호출될 때 모든 SPI 모듈이 한 번에 초기화됩니다. 
> 이 방식이 설계 방향과 부합하는지, 그리고 매크로 명칭(예: `SPIA_W6100_BAUDRATE`)에 대한 선호하시는 네이밍 규칙이 있는지 검토 부탁드립니다. 이상이 없다면 구현을 지시해 주시면 실제 코드 수정을 진행하겠습니다.
