# CSU 및 HAL 계층 전면 상세 주석 추가 계획서 (CPU1, CM)

이 계획서는 펌웨어 구조(CSU, HAL) 내의 모든 함수, 구조체 멤버 변수, 그리고 초기화 로직에 GEMINI.md 코딩 규칙에 부합하는 상세한 한국어 주석을 일괄 추가하기 위한 세부 계획입니다. (W6100 모듈 제외)

## User Review Required
> [!IMPORTANT]
> - 본 작업은 50개가 넘는 헤더(`.h`)와 소스(`.c`) 파일을 수정하는 대규모 작업입니다.
> - 구조체와 함수의 기존 로직은 절대 수정하지 않고, 순수하게 주석만 추가 및 보강(치환 방식)합니다.
> - 각 파일의 최상단 버전 주석(Version)은 규칙에 따라 `00.01`씩 증가되며, Modification History 란에 작업 내역이 기록됩니다.
> - 파일 수가 많으므로, 단계별(예: CPU1 CSU -> CPU1 HAL -> CM CSU -> CM HAL)로 작업을 진행하게 됩니다.

## Open Questions
- 특별히 집중적으로 주석이 필요한(이해가 어려운) 특정 모듈이 있다면 말씀해 주시면 우선적으로 반영하겠습니다.

## Proposed Changes

### 주석 작성 표준 (GEMINI.md 기준)

**1. 구조체 변수 및 맴버 주석**
구조체 선언부(`.h` 또는 `.c`)에서 각 멤버 변수 옆에 단위와 역할을 명확히 한글로 명시합니다.
```c
typedef struct {
    float32_t currentSpeedRpm; // 현재 모터의 속도 (단위: RPM)
    uint16_t brakeTimerTick;   // 브레이크 상태 전이 대기 타이머 (100us 틱 단위)
} stMotorCtrlState;
```

**2. 초기화 함수 내 상세 주석**
각 모듈의 초기화 함수(예: `MotorCtrl_Init()`, `hal_Adc_Init()` 등) 내에서 변수 및 구조체를 초기화하는 구문마다 초기값 부여의 목적과 이유를 한글 주석으로 상세히 설명합니다.
```c
// 영점 설정 안전 인터락 타이머 초기화 (500ms 유지 목적)
xMotorCtrl.safeToZeroTimerTick = 0; 
```

**3. 함수 인터페이스 주석 (Doxygen 포맷)**
```c
/**
 * @brief      모터 제어기 초기화 및 기본 파라미터 셋업
 * @param      void
 * @return     void
 */
void MotorCtrl_Init(void);
```

---

### [Phase 1: CPU1 코어 CSU 계층 주석 추가]

**대상 모듈**:
- `csu_MotorCtrl`, `csu_Pid`, `csu_Encoder`, `csu_Control`, `csu_LimitSwitch` 등 핵심 제어 로직 모듈
- `csu_Adc`, `csu_Bit`, `csu_Debug`, `csu_Dio`, `csu_Ipc_cpu1`, `csu_Led`, `csu_MotorDriver`, `csu_SciPc`

**주요 변경 사항**:
- 각 모듈의 `.h` 파일에 선언된 상태 구조체, 제어 파라미터 구조체 멤버들에 상세 한글 주석 추가.
- `.c` 파일의 전역 변수 초기화 및 `_Init()` 함수 내부의 변수 초기화 과정 상세 주석 달기.
- 각 함수 선언 앞쪽 Doxygen 포맷 정비.

---

### [Phase 2: CPU1 코어 HAL 계층 주석 추가]

**대상 모듈**:
- `hal_Adc`, `hal_Common`, `hal_Debug`, `hal_DspInit`, `hal_Encoder`, `hal_Epwm`, `hal_Fram`, `hal_Ipc_cpu1`, `hal_MotorDriver`, `hal_Ramfuncs`, `hal_Sci`, `hal_Spi`, `hal_Timer`

**주요 변경 사항**:
- 하드웨어 레지스터 제어 및 Driverlib 래핑 함수의 목적 명시.
- 하드웨어 설정 변수 및 구조체(타이머 주기, 통신 보드레이트 설정용 구조체 등)의 각 필드 설명 보강.

---

### [Phase 3: CM 코어 계층 (CSU & HAL) 주석 추가]

**대상 모듈**:
- CM CSU: `csu_Ethernet_cm`, `csu_Ipc_cm`
- CM HAL: `hal_Ethernet_cm`, `hal_Ipc_cm`, `hal_Timer_cm`

**주요 변경 사항**:
- ARM Cortex-M 기반의 특성(예: SysTick, 바이트 주소 체계 등)을 반영한 주석 작성.
- 통신 버퍼(Tx/Rx) 구조체 및 IPC 데이터 교환용 구조체의 각 항목별 역할 명시.
- 공유 메모리(GSRAM) 접근 및 동기화를 위한 변수(예: `seqCount` 등)에 대한 동작 원리 설명 추가.

---

## Verification Plan

### 수동 검증 및 정적 검사
- `view_file` 및 `grep_search` 도구를 활용하여, 변경된 파일들이 UTF-8 인코딩을 유지하고 한글 깨짐이 없는지 확인합니다.
- 코드를 덮어쓰지 않고 정교하게 치환(replace) 방식으로 업데이트하여 원본 코드 로직이 100% 보존되었는지 검증합니다.
- 사용자에게 주석이 반영된 파일을 확인받고 빌드(IDE) 진행을 요청합니다.
