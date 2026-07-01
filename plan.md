# [Goal Description]
디버거(CCS)에서 실시간으로 한계값 및 튜닝 파라미터를 변경할 수 있도록, `csu_Bit`, `csu_LimitSwitch`, `csu_MotorCtrl` 모듈에 분산된 `#define` 매크로 상수들을 전역 변수(구조체) 형태로 전환합니다. (추천된 Option 1 방식 적용)

## User Review Required
> [!IMPORTANT]
> 1. 매크로 상수가 변수로 변경되므로 RAM 공간이 약간(수십 바이트 수준) 추가로 사용됩니다.
> 2. PID 계수 및 타겟 이동을 위한 변수는 이미 `xPidGain`, `xMotorCtrl` 구조체에 구현되어 있으므로 별도로 수정하지 않고, 디버거에서 바로 사용할 수 있도록 둡니다.

## Open Questions
- 각 구조체 이름으로 `xBitLimit`, `xLimitSwitchLimit`, `xMotorCtrlLimit` 를 사용하는 것이 괜찮으신가요?

## Proposed Changes

### CSU 모듈 리팩토링 (매크로 상수 -> 구조체 변환)

#### [MODIFY] [csu_Bit.h](file:///d:/Nexcom/Firmware/01_Project/04_ATTLA/ATTLA_T/ATTLA_T/ATTLA_T_CPU1/CSU/csu_Bit.h)
- 기존 `BIT_LIMIT_...` 매크로 상수들을 주석 처리 또는 삭제.
- `stBitLimit` 구조체 정의 (과전류, 스톨, 과속, 타이머 기준값 등을 멤버로 포함).
- `extern stBitLimit xBitLimit;` 선언 추가.

#### [MODIFY] [csu_Bit.c](file:///d:/Nexcom/Firmware/01_Project/04_ATTLA/ATTLA_T/ATTLA_T/ATTLA_T_CPU1/CSU/csu_Bit.c)
- `Bit_Init()` 함수에서 `xBitLimit` 구조체 멤버들을 기존 매크로 상수 값으로 초기화.
- 함수 내부 로직에서 매크로 상수를 사용하던 부분을 모두 `xBitLimit.xxx` 형태로 치환.

#### [MODIFY] [csu_LimitSwitch.h](file:///d:/Nexcom/Firmware/01_Project/04_ATTLA/ATTLA_T/ATTLA_T/ATTLA_T_CPU1/CSU/csu_LimitSwitch.h)
- `LIMIT_OFFSET_COUNT` 등의 매크로 상수 주석 처리.
- `stLimitSwitchLimit` 구조체 정의.
- `extern stLimitSwitchLimit xLimitSwitchLimit;` 선언 추가.

#### [MODIFY] [csu_LimitSwitch.c](file:///d:/Nexcom/Firmware/01_Project/04_ATTLA/ATTLA_T/ATTLA_T/ATTLA_T_CPU1/CSU/csu_LimitSwitch.c)
- `LimitSwitch_Init()` 함수에서 `xLimitSwitchLimit` 초기화.
- 로직에서 매크로 상수를 구조체 멤버로 치환.

#### [MODIFY] [csu_MotorCtrl.h](file:///d:/Nexcom/Firmware/01_Project/04_ATTLA/ATTLA_T/ATTLA_T/ATTLA_T_CPU1/CSU/csu_MotorCtrl.h)
- 속도/전류/위치 리미트, 브레이크 딜레이 등의 매크로 상수 주석 처리.
- `stMotorCtrlLimit` 구조체 정의.
- `extern stMotorCtrlLimit xMotorCtrlLimit;` 선언 추가.

#### [MODIFY] [csu_MotorCtrl.c](file:///d:/Nexcom/Firmware/01_Project/04_ATTLA/ATTLA_T/ATTLA_T/ATTLA_T_CPU1/CSU/csu_MotorCtrl.c)
- `MotorCtrl_Init()` 함수에서 `xMotorCtrlLimit` 초기화.
- 제어 로직 내부의 매크로 상수들을 구조체 멤버로 치환.

## Verification Plan
### Manual Verification
- 사용자가 CCS 디버거에서 코드를 빌드하고 실행한 뒤, Expressions 창에 `xBitLimit`, `xLimitSwitchLimit`, `xMotorCtrlLimit` 변수를 추가하여 값을 모니터링 및 실시간으로 변경 가능한지 확인.
- `xPidGain`과 `xMotorCtrl` 의 내부 멤버를 수정하여 모터가 정상적으로 반응(디버그 구동)하는지 확인.
