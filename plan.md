# ATTLA_T 모터 제어 소프트 리미트(Soft Limit) 적용 계획서

사용자의 요청에 따라 기계적 구동 범위를 제한하고, 속도 및 전류 지령의 한계치를 오류 점검(BIT) 임계치와 동일하게 맞추는 구현 계획입니다.

## 1. 구현 목표 및 파라미터 설계
- **위치 제한 (Position Limit)**
  - 범위: `0.0f` ~ `15840.0f` (기계각 44바퀴 × 360도)
  - 방법: 목표 위치 지령(`targetPosition`)을 PID 제어기에 인가하기 전에 범위 내로 클램핑(Clamping)합니다.
- **속도 제한 (Speed Limit)**
  - 범위: `-3500.0f` ~ `3500.0f` RPM (과속 오류 기준과 동일)
  - 방법: 위치 제어기(`posPid`)의 출력 제한(Limit) 값을 기존 ±3000.0에서 ±3500.0으로 상향 조정하여 최대 속도 명령을 제한합니다.
- **전류 제한 (Current Limit)**
  - 범위: `-10.0f` ~ `10.0f` A (과전류 오류 기준과 동일)
  - 방법: 속도 제어기(`speedPid`)의 출력 제한(Limit) 값을 ±10.0A로 설정하여 하드웨어 제어 전류를 제한합니다. (이미 기존 코드에 10.0A로 반영되어 있으므로 유지/확인합니다)

## 2. 파일별 수정 상세 계획

### 2.1 [MODIFY] `csu_MotorCtrl.h`
- 안전 제한용 매크로 상수 추가.
  ```c
  // --- 모터 제어 소프트 리미트 (Soft Limits) ---
  #define LIMIT_POS_MIN       0.0f        // 기구부 최소 각도 (0도)
  #define LIMIT_POS_MAX       15840.0f    // 기구부 최대 각도 (44바퀴 * 360도)
  
  #define LIMIT_SPEED_MAX     3500.0f     // 최대 동작 속도 (RPM)
  #define LIMIT_SPEED_MIN     -3500.0f    // 최소 동작 속도 (RPM)
  
  #define LIMIT_CURRENT_MAX   10.0f       // 최대 동작 전류 (A)
  #define LIMIT_CURRENT_MIN   -10.0f      // 최소 동작 전류 (A)
  
  // 클램핑 매크로 유틸리티
  #define CLAMP_F32(x, min, max)  (((x) < (min)) ? (min) : (((x) > (max)) ? (max) : (x)))
  ```

### 2.2 [MODIFY] `csu_MotorCtrl.c`
- **`MotorCtrl_Init()` 함수 수정**:
  - `posPid` 초기화 시 출력 제한(Max/Min)을 ±3500.0f로 변경.
  - `speedPid` 초기화 시 출력 제한(Max/Min)을 ±10.0f로 명시적 변수화.
  ```c
  PID_Init(&speedPid, 0.5f, 0.01f, 0.0f, 0.001f, LIMIT_CURRENT_MAX, LIMIT_CURRENT_MIN);
  PID_Init(&posPid, 1.0f, 0.0f, 0.0f, 0.004f, LIMIT_SPEED_MAX, LIMIT_SPEED_MIN);
  ```

- **`MotorCtrl_Run()` 함수 수정**:
  - 위치 제어 연산 루프 직전에 `targetPosition`을 `CLAMP_F32`를 통해 제한 범위 내에 묶어둡니다.
  ```c
  // 위치 명령 소프트 리미트 적용
  xMotorCtrl.targetPosition = CLAMP_F32(xMotorCtrl.targetPosition, LIMIT_POS_MIN, LIMIT_POS_MAX);
  speedCmd = PID_Calculate(&posPid, xMotorCtrl.targetPosition, xMotorCtrl.currentPosition);
  ```

## 3. 검증 계획
- 속도 및 전류 한계치가 PID 제어기의 포화(Saturation) 특성으로 잘 동작하는지 검토합니다.
- `Project_Spec.md`의 소프트 리미트 관련 내용을 `0도 ~ 15840도 (44바퀴)`로 업데이트합니다.

> 위 계획에 따라 위치(0~44바퀴), 속도(±3500RPM), 전류(±10A)의 소프트웨어 제약(Limit)을 구현하고자 합니다. 승인해 주시면 즉시 코드에 적용하겠습니다!
