# 리미트 스위치 로직 및 고장 판단 구현 계획 (Implementation Plan)

## 1. 목표 (Goal Description)
이전에 조사한 리서치 내용을 바탕으로, `csu_LimitSwitch` 모듈을 신규 추가하여 리미트 스위치의 설정 데이터(오차 범위, 매핑 등)와 상태 판단 로직을 캡슐화합니다. 제어 루프 중 즉시 고장을 판단하고 모터 제어 모드를 에러 상태(`MOTOR_MODE_FAULT_STOP`)로 전환하도록 시스템을 구현합니다.

## 2. 제안되는 변경 사항 (Proposed Changes)

---

### 모터 제어 및 고장 감지 로직 분리 (CSU 계층)

#### [NEW] `ATTLA_T_CPU1/CSU/csu_LimitSwitch.h`
- `LimitSwitchFaultCode_t` 열거형 정의 (단선 및 위치 불일치 등 4가지 에러 코드)
- `stLimitSwitchConfig` 구조체 정의 (목표 1/2 위치, 오차 허용치, 스위치 매핑 번호)
- `stLimitSwitchState` 구조체 정의 (에러 유무 플래그 및 현재 발생한 에러 코드)
- `LimitSwitch_Init()`, `LimitSwitch_CheckFaults()` 함수 원형 선언

#### [NEW] `ATTLA_T_CPU1/CSU/csu_LimitSwitch.c`
- `xLimitSwitchConfig`, `xLimitSwitch` 전역 인스턴스 할당 및 초기화
- `LimitSwitch_Init()` 구현: 임의의 기본값으로 위치 및 공차, 매핑 변수 초기화
- `LimitSwitch_CheckFaults()` 구현:
  1. `xDio.limit1No == xDio.limit1Nc` 단선 고장 판별
  2. `xDio.limit2No == xDio.limit2Nc` 단선 고장 판별
  3. `xMotorCtrl.currentPosition`을 바탕으로 Target 1/2 근처(`nearTolerance` 이내) 판별 후, 스위치 1/2 매핑에 맞춰 NO 신호와 논리적 일치 여부 비교 (위치 불일치 에러 세팅)

#### [MODIFY] `ATTLA_T_CPU1/CSU/csu_MotorCtrl.h`
- `MotorControlMode_t` 열거형에 `MOTOR_MODE_FAULT_STOP` 추가

#### [MODIFY] `ATTLA_T_CPU1/CSU/csu_MotorCtrl.c`
- 최상단에서 `csu_LimitSwitch.h`의 전역 상태를 참조.
- `MotorCtrl_Run()` 내부:
  - 위치 피드백(`MotorCtrl_UpdateFeedback()`) 갱신 직후, `LimitSwitch_CheckFaults()` 호출.
  - 리턴된 상태가 `isFaultActive == true` 라면, 강제로 `xMotorCtrl.mode = MOTOR_MODE_FAULT_STOP` 으로 모드 덮어쓰기.
  - `else if (xMotorCtrl.mode == MOTOR_MODE_FAULT_STOP)` 블록 추가하여, 안전 정지 시퀀스(듀티 0.0f, 브레이크 핀 0U 출력) 실행 (현재 `MOTOR_MODE_STOP`과 동일하나 추후 시퀀스 추가를 위한 공간 확보).

#### [MODIFY] `ATTLA_T_CPU1/CSU/csu_Control.c`
- `Control_Init()` 내부에서 `LimitSwitch_Init()` 호출 추가.

#### [MODIFY] `ATTLA_T_CPU1/main.h`
- 하단 `#include` 영역에 `#include "csu_LimitSwitch.h"` 추가.

---

## 3. 검토 요청 사항 (User Review Required)

> [!IMPORTANT]
> **초기 위치 설정 데이터 확인**  
> `LimitSwitch_Init()` 내에서 설정될 임시 타겟 위치(`targetPos1`, `targetPos2`)와 범위(`nearTolerance1`, `nearTolerance2`) 값의 권장 초기값이 있다면 피드백 부탁드립니다. (없으면 기본값 0, 15840 및 오차 100 등으로 임의 초기화합니다.)

> [!NOTE]
> `plan.md` 파일을 통해 수정 방향을 확인하시고, **승인(또는 추가 메모)** 해 주시면 즉시 코드 수정을 시작하겠습니다.
