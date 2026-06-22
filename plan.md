# ATTLA_T 모터 제어 PID 및 주기 변경 구현 계획서

사용자님의 지시에 따라, **[옵션 A: 범용 모듈 `csu_PID` 코어 확장]** 방식을 기반으로 구현 계획을 수립하였습니다. 특히, **"위치/속도/전류 각 제어기의 변수를 다르게 독립된 전역변수로 분리하여 구현"**하라는 추가 지시사항을 반영하였습니다.

## 1. 구현 목표
- 범용 `csu_PID` 모듈을 수정하여 PI-IP 혼합 제어(Ks)를 지원하도록 확장합니다.
- 기존에 하드코딩 되어 있던 PID 계수들을, 위치, 속도, 전류 각각 독립된 `float32_t` 전역변수로 분리 선언합니다.
- 실시간 디버거(CCS/easyDSP)에서 변수값을 변경하면 즉시 제어기에 반영되도록 업데이트 로직을 추가합니다.
- 위치 제어 주기를 기존 4ms에서 **5ms**로 변경합니다.

---

## 2. 파일별 상세 수정 계획

### 2.1. `csu_MotorCtrl.h`
- 기존 `#define` 으로 선언된 Kp, Ki, Kd 매크로들을 삭제합니다.
- 위치 제어 분주비 매크로를 수정합니다: `#define DECIMATION_POS_CTRL 5U` (4ms -> 5ms)
- 5ms 주기에 맞춰 `#define PID_POS_DT 0.005f` 로 수정합니다.
- 제어기별 독립 전역변수를 `extern`으로 선언합니다:
  ```c
  // --- PID 파라미터 전역 변수 선언 ---
  // 위치 제어 (PD)
  extern float32_t M1_PosCtrl_Kp;
  extern float32_t M1_PosCtrl_Kd;

  // 속도 제어 (PI-IP)
  extern float32_t M1_SpdCtrl_Kp;
  extern float32_t M1_SpdCtrl_Ki;
  extern float32_t M1_SpdCtrl_Ks; // 0.0: IP, 1.0: PI, 0~1 사이 혼합

  // 전류 제어 (PI)
  extern float32_t M1_CurCtrl_Kp;
  extern float32_t M1_CurCtrl_Ki;
  ```

### 2.2. `csu_MotorCtrl.c`
- 최상단에 실제 전역변수를 선언하고 초기값을 할당합니다.
  ```c
  float32_t M1_PosCtrl_Kp = 1.0f;
  float32_t M1_PosCtrl_Kd = 0.0f;     // PD 제어의 D 요소

  float32_t M1_SpdCtrl_Kp = 0.5f;
  float32_t M1_SpdCtrl_Ki = 0.01f;
  float32_t M1_SpdCtrl_Ks = 0.25f;    // 기본 혼합 비율 (필요 시 수정 가능)

  float32_t M1_CurCtrl_Kp = 2.0f;
  float32_t M1_CurCtrl_Ki = 0.05f;
  ```
- `MotorCtrl_Init()` 함수에서 변경된 `PID_Init` 시그니처에 맞추어 `Ks` 값을 포함해 초기화를 수행합니다. (위치와 전류는 Ks=1.0f 고정으로 일반 PD/PI로 동작하게 함)
- `MotorCtrl_Run()` 함수의 최상단(제어 연산 직전)에, 디버거에서 값을 변경했을 때 즉각 반영되도록 파라미터 업데이트 코드를 삽입합니다:
  ```c
  // 디버거 실시간 튜닝 파라미터 갱신
  posPid.Kp = M1_PosCtrl_Kp;
  posPid.Kd = M1_PosCtrl_Kd;

  speedPid.Kp = M1_SpdCtrl_Kp;
  speedPid.Ki = M1_SpdCtrl_Ki;
  speedPid.Ks = M1_SpdCtrl_Ks;

  currPid.Kp = M1_CurCtrl_Kp;
  currPid.Ki = M1_CurCtrl_Ki;
  ```

### 2.3. `csu_PID.h`
- `PID_Controller_t` 구조체에 `float32_t Ks;` 멤버를 추가합니다.
- `PID_Init` 함수의 매개변수 리스트에 `ks` 를 추가합니다.

### 2.4. `csu_PID.c`
- `PID_Init()` 함수에서 `pid->Ks = ks;` 초기화 코드를 추가합니다.
- `PID_Calculate()` 함수 내부의 비례(P) 항 수식을, 사용자님의 코드를 일반화한 PI-IP 혼합 수식으로 덮어씌웁니다.
  ```c
  // [수정 전]
  // pOut = pid->Kp * error;
  
  // [수정 후 - PI/IP 혼합 일반화 식]
  // Ks = 1.0 (PI 제어): Kp * (setpoint - feedback) = Kp * error 로 기존과 동일.
  // Ks = 0.0 (IP 제어): Kp * (0 - feedback) = -Kp * feedback
  float32_t pOut = (error * pid->Kp * pid->Ks) - (feedback * pid->Kp * (1.0f - pid->Ks));
  ```
- 적분 항과 미분 항 로직, 그리고 안티와인드업(출력 클램핑 시 적분값 복원) 로직은 그대로 유지하여 완전한 범용성을 보존합니다.

---

**보고 및 대기:**
본 계획은 사용자님의 요구사항을 100% 충족하며 코드베이스의 안정성과 튜닝 편의성을 극대화합니다.
내용을 검토하신 후 **"계획대로 구현해 줘"** 라고 지시해 주시면, 즉시 소스 코드 파일들의 실제 수정을 진행하겠습니다!
