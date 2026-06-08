# 📋 AksIM-2 엔코더 (BiSS-C) 연동 구현 상세 계획서

**작성 일자**: 2026. 06. 08.  
**대상 코어**: CPU1 Core (`ATTLA_T_CPU1`)
**적용 기술**: TI Position Manager (pm_bissc) + CLB (Configurable Logic Block)

---

## 1. 개요 및 구현 전략 (Strategy)
AksIM-2 엔코더와 F28388D MCU 간의 BiSS-C 통신을 구현합니다.
기존 하드웨어에는 `ENC_DATA` (GPIO 25)와 `ENC_CLK` (GPIO 26) 2가닥만 연결되어 있으나, BiSS-C 프로토콜은 마스터 클럭(MA)의 **펄스 폭(Duty Cycle) 변조**를 통해 양방향 제어 통신(Register R/W)을 수행하므로 물리적인 핀 2개만으로 완벽한 통신이 가능합니다. 

이러한 하드웨어적인 클럭 변조 및 수신 동기화는 TI의 `pm_bissc` 라이브러리와 내부 **CLB(Configurable Logic Block)**가 자동으로 처리합니다.

---

## 2. 사용자 확인 요망 (User Review Required) & Open Questions

> [!IMPORTANT]
> 1. **헤더 파일 수정 승인 (`pm_bissc_include.h`)**
>    - 이미 시도하신 대로, 해당 라이브러리는 최신 칩(`F28P65x`)용으로 작성되어 있어 타겟 칩 식별 구문이 필요합니다. 
>    - 추가해주신 코드를 `#elif defined(_F2838X)` 형태로 다듬어 문법 오류를 해결하고 진행하겠습니다. (승인 요망)
> 2. **GPIO 25, 26 기능 할당 (Pinmuxing)**
>    - GPIO 25: `SPIB_SOMI` (수신용)
>    - GPIO 26: `CLB_OUTPUT` (클럭 송신용)
>    - 이 핀들이 내부 X-BAR 라우팅을 거치게 되므로, 회로상 다른 용도와 충돌하지 않는지 교차 검증해 주셔야 합니다.

---

## 3. 리팩토링 및 파일 생성 상세 계획 (Proposed Changes)

### 3.1 SDK 라이브러리 수정 (HAL 종속 헤더)
#### [MODIFY] `pm_bissc_include.h`
- 칩 선택 매크로 문법 오류 수정.
- F28388D에 맞게 `SPIB_BASE`, `CLB1_BASE`, `CLB2_BASE` 할당 적용.

### 3.2 HAL (Hardware Abstraction Layer)
#### [NEW] `hal_Encoder.h` & `hal_Encoder.c`
- **역할**: 하드웨어 핀맵 설정 및 `pm_bissc` 라이브러리 초기화.
- **구현 내용**:
  - `hal_Encoder_Init()` 함수 구현:
    - `GPIO_setPinConfig()`를 사용하여 GPIO 25를 SPI 수신 핀으로 설정.
    - `GPIO_setPinConfig()` 및 Output X-BAR를 사용하여 GPIO 26을 CLB 출력 핀으로 설정.
    - `PM_bissc_setupPeriph()`, `PM_bissc_setFreq()` (통신 속도 설정), `PM_bissc_initParams()` 호출.

### 3.3 CSU (Control & Service Unit)
#### [NEW] `csu_Encoder.h` & `csu_Encoder.c`
- **역할**: AksIM-2 어플리케이션 기능 구현 (데이터시트의 주소 및 커맨드 시퀀스 래핑).
- **상수 정의 (매크로)**:
  - `ENC_REG_BANK_SEL (0x40)`
  - `ENC_REG_KEY (0x48)` / `ENC_KEY_UNLOCK (0xCD)`
  - `ENC_REG_CMD (0x49)` / CMD 종류 (`'c'`, `'m'`, `'A'`, `'r'`)
- **구현 내용**:
  - **위치 획득 루틴**: `csu_Encoder_UpdatePosition()`
    - EPWM 주기 또는 1ms 주기마다 호출.
    - `PM_bissc_setupSCDTransaction()`, `PM_bissc_startOperation()`으로 통신 개시.
    - 통신 완료 후 `PM_bissc_receivePosition()`을 호출하여 위치 파싱.
  - **백그라운드 통신 처리기**: `csu_Encoder_ProcessCDTasks()`
    - 매 주기 호출되어 `PM_bissc_doCDTasks()`를 실행. (32주기에 걸쳐 비동기적으로 1비트씩 파라미터 Read/Write 수행).
  - **제어 API (래퍼 함수)**:
    - `csu_Encoder_SaveParameters()`: 언락 ➡️ 저장 시퀀스 실행.
    - `csu_Encoder_StartCalibration()`: 언락 ➡️ 캘리브레이션 시퀀스 실행.
    - `csu_Encoder_ReadTemperature()`: 0x4C~0x4D Read 예약 및 합성(Big Endian 규칙 적용).

### 3.4 Main Application
#### [MODIFY] `main.c` / `main.h`
- `DSP_Initialization()` 호출 후 `hal_Encoder_Init()` 및 `csu_Encoder_Init()` 호출 추가.
- 고속 주기 루프(EPWM 인터럽트 또는 1ms 타이머) 내에 `csu_Encoder_UpdatePosition()` 및 `csu_Encoder_ProcessCDTasks()` 삽입.

---

## 4. 구현 단계 및 내일의 작업 가이드 (Next Steps)
1. 사용자가 위 "Open Questions"의 **헤더 수정 및 GPIO 25/26 충돌 여부**를 확인하고 승인합니다.
2. 에이전트(AI)가 `hal_Encoder` 모듈부터 생성하여 CLB와 SPI 핀 라우팅 코드를 작성합니다.
3. 에이전트(AI)가 데이터시트의 메모리 맵과 엔디안 규칙을 엄격하게 적용하여 `csu_Encoder` 모듈을 작성합니다.
4. 빌드 오류를 해결하고 메인 타이머 루프에 통신 태스크를 스케줄링하여 작업을 완료합니다.
