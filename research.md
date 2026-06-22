# ATTLA_T 리미트 스위치 로직 및 고장 감지 리서치 보고서

본 문서는 사용자의 요청에 따라 모터 이동 범위 감지 및 고장 판단을 위한 2개의 SPDT 리미트 스위치 적용 방안을 분석하고, 소프트웨어 구조상 미리 반영할 수 있는 요소들을 정리한 리서치 문서입니다.

---

## 1. 요구사항 분석

### 1.1. 스위치 하드웨어 특성 (SPDT)
- **신호명**: `LIMIT_SWITCH1_NO`, `LIMIT_SWITCH1_NC`, `LIMIT_SWITCH2_NO`, `LIMIT_SWITCH2_NC`
- **정상 상태**: NO 신호와 NC 신호는 항상 반대 값을 가져야 함 (NO = 1 이면 NC = 0).
- **고장 판단 1 (물리적 단선/고장)**: 각 스위치의 NO와 NC 값이 같으면(둘 다 0이거나 둘 다 1) 리미트 스위치 자체 고장으로 판단.

### 1.2. 스위치 및 목표 위치의 가변성 (데이터화)
- 1번 스위치와 2번 스위치의 설치 위치가 변경(Swap)될 수 있음.
- 목표 위치(Target 1, Target 2)와 리미트 스위치가 눌리는 범위(Tolerance/Threshold)가 고정되어 있지 않아 설정 데이터(파라미터)로 관리해야 함.
- 스위치가 눌리더라도 모터는 설정된 최종 목표 위치까지 추가로 이동해야 함. (즉, 리미트 스위치는 물리적 하드 스탑이 아니라, 목표 근방에 도달했음을 알리는 '소프트 리미트 진입' 센서로 작용함).

### 1.3. 위치-스위치 연동 고장 판단 로직
- **목표 1 (Target 1) 연동 (스위치 1 매핑 시)**
  - 모터가 목표 1 근처에 있음 & 스위치 1 NO != 1 ➡️ **고장 (정지)**
  - 모터가 목표 1에서 충분히 벗어남 & 스위치 1 NO == 1 ➡️ **고장 (정지)**
- **목표 2 (Target 2) 연동 (스위치 2 매핑 시)**
  - 모터가 목표 2 근처에 있음 & 스위치 2 NO != 1 ➡️ **고장 (정지)**
  - 모터가 목표 2에서 충분히 벗어남 & 스위치 2 NO == 1 ➡️ **고장 (정지)**

### 1.4. 정지 시퀀스
- 고장 감지 시 모터를 정지시키는 시퀀스(브레이크 타이밍, 모터 출력 차단 시간 등)는 현재 **미정**임.

---

## 2. 미리 반영 가능한 소프트웨어 구조 (사전 설계)

요구사항을 바탕으로 `csu_MotorCtrl` (또는 신규 `csu_LimitSwitch` 모듈)에 즉시 반영할 수 있는 구조적 설계안입니다.

### 2.1. 설정 데이터 (Configuration Data) 구조화
리미트 스위치 및 목표 위치의 범위를 데이터로 관리하기 위해 아래와 같은 구조체를 전역 변수로 정의할 수 있습니다.

```c
typedef struct {
    float32_t targetPos1;        // 목표 1 위치 (예: 최소 위치)
    float32_t targetPos2;        // 목표 2 위치 (예: 최대 위치)
    float32_t nearTolerance1;    // 목표 1 근처로 판단하는 범위 (오차 허용치)
    float32_t nearTolerance2;    // 목표 2 근처로 판단하는 범위 (오차 허용치)
    Uint16 mappedSwitchForTarget1; // 목표 1에 매핑된 스위치 번호 (1 또는 2)
    Uint16 mappedSwitchForTarget2; // 목표 2에 매핑된 스위치 번호 (1 또는 2)
} stLimitSwitchConfig;
```

### 2.2. 상태 및 고장 감지 구조화
주기적인 제어 루프에서 스위치의 상태와 위치를 평가하여 고장 유무를 판단하는 상태 관리 구조체입니다.

```c
typedef enum {
    LS_FAULT_NONE = 0,
    LS_FAULT_SW1_BROKEN,        // 스위치1 NO == NC
    LS_FAULT_SW2_BROKEN,        // 스위치2 NO == NC
    LS_FAULT_POS1_MISMATCH,     // 목표 1 위치 조건과 스위치 상태 불일치
    LS_FAULT_POS2_MISMATCH      // 목표 2 위치 조건과 스위치 상태 불일치
} LimitSwitchFaultCode_t;

typedef struct {
    Uint16 sw1_NO;
    Uint16 sw1_NC;
    Uint16 sw2_NO;
    Uint16 sw2_NC;
    bool isFaultActive;
    LimitSwitchFaultCode_t faultCode;
} stLimitSwitchState;
```

### 2.3. 고장 판단 로직 (제어 루프 내 1ms 또는 5ms 주기 편입)
고장 판별 함수(예: `LimitSwitch_CheckFaults()`)를 만들어 모터의 현재 위치(`xMotorCtrl.currentPosition`)와 스위치 입력을 기반으로 아래 로직을 수행합니다.
1. `sw1_NO == sw1_NC` 또는 `sw2_NO == sw2_NC` 확인 -> 물리적 스위치 고장.
2. 현재 위치가 `targetPos1 ± nearTolerance1` 이내인지 판별하여 `isNearTarget1` (true/false) 계산.
   - `isNearTarget1` 인데 매핑된 스위치의 `NO != 1` 이면 `LS_FAULT_POS1_MISMATCH`.
   - `isNearTarget1` 이 아닌데(충분히 벗어남) 매핑된 스위치의 `NO == 1` 이면 `LS_FAULT_POS1_MISMATCH`.
3. Target 2에 대해서도 동일한 논리 연산 수행.

### 2.4. 정지 시퀀스를 위한 상태 머신 확장
정지 시퀀스의 세부 타이밍은 미정이지만, 고장이 발생했을 때 모터 제어 루프가 진입할 상태는 미리 정의해 둘 수 있습니다.
- `csu_MotorCtrl.h`의 `MotorControlMode_t` 열거형에 **`MOTOR_MODE_FAULT_STOP`** 상태를 추가합니다.
- 고장이 감지되면 제어기는 즉시 `MOTOR_MODE_FAULT_STOP` 모드로 전환되며, 추후 타이밍 스펙이 정해지면 이 모드 내부의 루틴(Switch-Case 또는 State Machine)에 브레이크 동작 및 모터 출력 0 처리 로직을 채워넣을 수 있습니다.

---

## 3. 구현 방향 추천 (Recommendation)

리미트 스위치 감지 로직은 하드웨어 종속성(GPIO 읽기)과 비즈니스 로직(위치 비교 및 고장 판단)이 혼재되어 있습니다. 이를 프로젝트 아키텍처 원칙(CSU, HAL 분리)에 맞게 구현하는 방안을 제안합니다.

1. **HAL (하드웨어 추상화 계층)**
   - `hal_Gpio.c` 등에 `LIMIT_SWITCH1_NO` 핀 등을 읽어오는 단순 추상화 함수(예: `HAL_ReadLimitSwitches()`)를 만듭니다. (핀 번호는 미정이면 매크로로 비워둘 수 있습니다.)

2. **CSU (로직 계층)**
   - `csu_MotorCtrl`의 구조가 이미 커지고 있으므로, 리미트 스위치의 관리와 고장 진단만을 전담하는 **`csu_LimitSwitch.c / .h` 모듈을 신규로 생성하는 것을 추천**합니다.
   - 이 모듈에서 앞서 설계한 `stLimitSwitchConfig` 및 상태를 관리하고, 1ms마다 갱신된 위치 데이터(`xMotorCtrl.currentPosition`)를 받아 고장 여부를 판독합니다.
   - 메인 모터 루프(`MotorCtrl_Run`)에서는 `LimitSwitch_CheckFaults()`가 `true`를 반환하면 모터 모드를 `MOTOR_MODE_FAULT_STOP`으로 강제 전환합니다.

---

**보고를 마칩니다.**

위 분석 내용을 바탕으로 **"제안된 구조대로 csu_LimitSwitch 모듈을 추가하여 계획(Plan)을 세워줘"** 또는 **"csu_MotorCtrl 내부에 포함해서 바로 구현해 줘"** 등 원하시는 방향을 알려주시면 상세 계획 수립이나 실제 코드 반영을 진행하겠습니다.
