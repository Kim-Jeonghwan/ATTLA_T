# 매크로 상수의 디버그용 튜닝 변수 구조체화 리서치 (Research Report)

## 1. 개요 및 목적
사용자 요청(101 단축키 포함)에 따라 CPU1, CM 등 전체 프로젝트 파일 구조(CSU, HAL, main)를 다시 파악하였으며, 디버깅 과정에서 실시간으로 튜닝이 필요한 주요 매크로 상수들을 변수(구조체)화하기 위한 방안을 조사하였습니다.
목표는 기존에 `#define`으로 고정되어 있던 한계값(Limit)이나 지연시간 등을 CCS 디버거에서 실시간으로 변경 가능하도록 전역 변수 구조체(이름이 `x`로 시작)로 관리하는 것입니다.

## 2. 튜닝 대상 상수 분석 현황
소스 코드를 분석한 결과, 각 모듈에 분산된 튜닝 대상 매크로 상수들은 다음과 같습니다.

### 2.1. `csu_Bit.h` (BIT 임계값)
- **과전류/과열/과전압**: `BIT_LIMIT_OVC_MOT_MAX` (10A), `BIT_LIMIT_OVC_BRK_MAX` (1.5A), `BIT_LIMIT_OVT_BD_MAX` (80도), `BIT_LIMIT_OVV_28V_MAX` (32V)
- **스톨 판단**: `BIT_LIMIT_STALL_CURR_MIN` (5A), `BIT_LIMIT_STALL_RPM_LIMIT` (10RPM), `BIT_LIMIT_STALL_TIME_CNT` (1초)
- **과속 판단**: `BIT_LIMIT_SPEED_MOT_MAX` / `MIN` (3240 RPM), `BIT_LIMIT_OVS_TIME_CNT` (100ms)
- **타이머 필터**: `BIT_LIMIT_OVV_BRK_TIME_CNT` (100ms), `BIT_CNT_FILTER_REF` (100ms)

### 2.2. `csu_LimitSwitch.h` (리미트 스위치 감지 설정)
- **오프셋 및 데드존**: `LIMIT_OFFSET_COUNT` (1000.0f), `LIMIT_DEADZONE_COUNT` (100.0f)
- **오류 판단 지연**: `SENSOR_ERROR_TIME_MS` (50ms) -> `SENSOR_ERROR_TICK_100US`

### 2.3. `csu_MotorCtrl.h` (모터 제어 소프트 리미트 및 딜레이)
- **위치/속도/전류 리미트**: `LIMIT_POS_MAX`/`MIN`, `LIMIT_SPEED_MAX`/`MIN`, `LIMIT_CURRENT_MAX`/`MIN`, `LIMIT_CURRENT_RATIO`
- **브레이크 딜레이**: `BRAKE_RELEASE_DELAY_MS` (150ms), `BRAKE_ENGAGE_DELAY_MS` (100ms)
- **안전 인터락 타이머**: `SAFE_ZERO_SET_TICK_100US` (500ms)
- **듀티 최대치**: `MOTOR_DUTY_MAX` (100.0f)

### 2.4. PID 계수, 구동 모드, 타겟 위치
- **현황**: 분석 결과, 이미 전역 변수(구조체) 형태로 구현되어 있습니다.
  - **PID 계수**: `csu_MotorCtrl.h`에 `stPidGain xPidGain;` 형태로 존재하여 `xPidGain.pos.Kp`, `xPidGain.spd.Ki` 등을 실시간으로 수정할 수 있습니다.
  - **구동 모드 및 타겟 위치**: `csu_MotorCtrl.h`에 `stMotorCtrlState xMotorCtrl;` 형태로 존재하여 `xMotorCtrl.mode`, `xMotorCtrl.targetPosition`, `xMotorCtrl.targetSpeedRpm` 등을 디버거에서 수정하여 모터를 구동할 수 있습니다.

## 3. 구현 방향 제안 (Implementation Strategy)

이 상수들을 어떻게 구조체로 관리할지에 대한 두 가지 옵션을 제안합니다.

### Option 1: 각 모듈별 개별 튜닝 구조체 생성 [추천]
기존 모듈의 독립성을 유지하면서 튜닝 가능한 한계값들을 각 모듈의 구조체로 묶습니다.
- **csu_Bit.h**: `stBitLimit xBitLimit;` 구조체를 생성하고 내부 멤버로 `ovcMotMax`, `stallCurrMin` 등을 포함시킵니다.
- **csu_LimitSwitch.h**: `stLimitSwitchLimit xLimitSwitchLimit;` 구조체 생성 (`offsetCount`, `deadzoneCount` 등 포함).
- **csu_MotorCtrl.h**: `stMotorCtrlLimit xMotorCtrlLimit;` 구조체 생성 (`posMax`, `brakeReleaseDelayMs` 등 포함).
- **장점**: 모듈 간 의존성이 낮아 유지보수(객체지향 설계)에 유리합니다. 사용자는 CCS Expressions 뷰에서 `xBitLimit`, `xMotorCtrlLimit`, `xPidGain`, `xMotorCtrl` 등 관련된 변수들을 올려두고 실시간으로 조정할 수 있습니다.

### Option 2: 통합 디버그 튜닝 구조체 생성 (`xDebugTuning`)
튜닝 가능한 모든 한계값(모터, BIT, 리미트 등)을 하나의 거대한 디버그 구조체 `stDebugTuning xDebugTuning;` 에 몰아넣습니다.
- **장점**: CCS Expressions 창에 `xDebugTuning` 하나만 등록하여 모든 파라미터를 트리 구조로 한눈에 볼 수 있습니다.
- **단점**: 모듈 간 의존성이 높아지며(모든 모듈이 전역 튜닝 구조체 헤더를 인클루드해야 함), `csu_Debug.h` 쪽에 방대한 변수들이 집중됩니다.

## 4. 진행 여부 확인
추천해 드린 **Option 1(각 모듈별 개별 튜닝 구조체 생성)** 방식으로 진행하고자 합니다. PID 제어 및 타겟 이동을 위한 변수(`xPidGain`, `xMotorCtrl.targetPosition` 등)는 이미 존재하므로, 이를 제외한 위 분석된 매크로 상수들을 각 모듈의 초기화 함수(`_Init()`)에서 초기값을 할당받는 구조체로 전환하겠습니다.

이 방향에 동의하시거나, 혹은 Option 2(통합 구조체) 방식을 원하신다면 알려주십시오. 확인되는 대로 Plan 문서 작성 후 적용을 시작하겠습니다.
