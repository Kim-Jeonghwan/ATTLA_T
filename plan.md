# ATTLA_T 모터 전류 제어기(Current PID) 추가 구현 계획서

본 문서는 사용자의 "1x PWM 모드 하에서의 전류 제어기 구현 및 조사" 요청에 따라 작성된 구현 계획입니다.

## 1. 전류 제어 루프 도입 배경 및 아키텍처 조사
- **조사 배경**: 앞서 TDU 프로젝트 조사 결과 1x PWM 모드에서는 소프트웨어적인 전류 벡터 제어(FOC)가 생략되었음을 확인하였습니다. 그러나 사용자의 요청에 따라, 비록 3상 교류 전류 제어(FOC) 방식은 아니지만 **DC 링크 전류 크기를 제어하는 "토크/전류 크기(Magnitude) 제어 루프"**를 도입하여 전류 제어기를 구현하는 방안을 설계하였습니다.
- **제어 아키텍처 변경 (Cascade 연장)**:
  - 기존: 위치 제어 ➡️ 속도 제어 ➡️ **PWM Duty 직결**
  - 변경: 위치 제어 ➡️ 속도 제어 ➡️ **전류(토크) 제어** ➡️ PWM Duty 인가

## 2. 파일별 구현 상세 계획

### 2.1 [MODIFY] `csu_MotorCtrl.c`
- **PID 인스턴스 추가**: `PID_Controller_t currPid;` 전역 제어기 변수 추가.
- **`MotorCtrl_Init()` 함수 수정**:
  - `currPid` 초기화 추가. (출력 범위: `0.0f` ~ `100.0f` 절대 Duty 크기)
  - 기존 `speedPid`의 출력 범위를 Duty(±100)에서 전류(예: ±10.0A)로 변경.
    ```c
    // speedPid 출력은 전류(Torque) 지령치이므로 최대 ±10.0A 로 제한
    PID_Init(&speedPid, 0.5f, 0.01f, 0.0f, 0.0001f, 10.0f, -10.0f);
    // currPid 출력은 Duty 크기이므로 0.0 ~ 100.0% 로 제한
    PID_Init(&currPid, 2.0f, 0.05f, 0.0f, 0.0001f, 100.0f, 0.0f);
    ```
- **`MotorCtrl_Run()` 함수 수정**:
  - 속도 제어 및 위치 제어 모드에서 `speedPid`의 출력을 `duty`가 아닌 `currentCmd`로 취급합니다.
  - 전류 지령(`currentCmd`)과 측정 전류(`xAdc.isenMotLpf`)의 **절대값(크기)**을 추출하여 전류 PID(`currPid`)를 연산합니다.
  - 산출된 `dutyAbs`에 원래의 `currentCmd` 부호를 다시 씌워 `MotorCtrl_SetOutput(duty)`로 인가합니다.

    ```c
    float32_t currentCmd = PID_Calculate(&speedPid, xMotorCtrl.targetSpeedRpm, xMotorCtrl.currentSpeedRpm);
    
    // 1. 크기(Magnitude) 기반 전류 제어 연산
    float32_t currentCmdAbs = (currentCmd >= 0.0f) ? currentCmd : -currentCmd;
    float32_t currentFdbkAbs = (xAdc.isenMotLpf >= 0.0f) ? xAdc.isenMotLpf : -xAdc.isenMotLpf;
    
    // 2. 전류 PID (입력: 타겟 전류 크기, 피드백 전류 크기 -> 출력: 목표 Duty 크기)
    float32_t dutyAbs = PID_Calculate(&currPid, currentCmdAbs, currentFdbkAbs);
    
    // 3. 목표 전류의 부호(방향) 복원
    float32_t duty = (currentCmd >= 0.0f) ? dutyAbs : -dutyAbs;
    MotorCtrl_SetOutput(duty);
    ```

### 2.2 [MODIFY] `Project_Spec.md`
- **프로젝트 명세서 업데이트**: 전류 제어 루프가 1x PWM 구조에 맞춰 크기(Magnitude) 기반 전류 피드백 제어로 확장되었음을 명시하고, 제어 순서(Cascade) 설명을 업데이트합니다.

---

> 위 계획에 따라 직렬 제어(Cascade) 구조를 위치 ➡️ 속도 ➡️ 전류(토크) ➡️ Duty 순으로 확장하는 코드 수정을 진행하고자 합니다. 해당 방향으로 구현을 시작해도 될지 확인 부탁드립니다.
