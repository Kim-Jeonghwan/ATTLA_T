# ATTLA_T 시스템 Built-In Test (BIT) 결함 감지 및 고장 방어 종합 조사 보고서

본 보고서는 `myresearch.md`에 정의된 고장 진단 항목들과 TDU 사업 코드(`csu_Bit.c`, `csu_MotorCtrl_M1_MSC.c`)의 분석 결과를 종합적으로 연계하여, `ATTLA_T` 프로젝트의 하드웨어 사양에 맞춤 설계한 통합 자체점검(BIT) 설계 문서입니다.

---

## 1. 개요 및 룰 준수 사항
- **공통 규칙 준수**: 
  - **임베디드 펌웨어 직접 수정 금지**: 본 보고서는 구현을 위한 조사 결과 및 설계 방안을 추천하며, 사용자의 승인이 완료되기 전까지는 실제 소스 코드(`*.c`, `*.h`)에 수정을 가하지 않습니다.
  - **드라이버립(Driverlib) 우선 사용**: 레지스터 직접 접근(`GpioDataRegs` 등)을 배제하고 TI C2000 Driverlib API인 `GPIO_readPin()` 등을 활용하여 하드웨어를 제어하고 조회하도록 설계합니다.
  - **한국어 주석 및 한글 대화**: 모든 설계 설명 및 코드 내 주석 제안은 한국어로 일관성 있게 작성합니다.
  - **계획 수립 지연**: 사용자의 요청에 따라 `plan.md` 작성(Plan 단계) 및 코드 수정은 수행하지 않고 리서치 분석 단계만 수행합니다.

---

## 2. 세부 BIT 진단 항목별 조사 및 구현 설계

### 2.1 과전류 진단 (모터 및 브레이크)
- **대상 및 임계치**: 모터 전류(`xAdc.isenMotLpf`) 기준 10.0A, 브레이크 전류(`xAdc.isenBrkLpf`) 기준 1.5A
- **구현 현황**: 
  - `csu_Bit.c` 내에 100us 마다 누적 필터링을 수행하는 `Bit_OvCurrent_Check()`가 기구현되어 있습니다.
  - 100ms(`BIT_CNT_REF_100MS = 1000U`) 동안 임계치 초과 시 각각 `faultOvCurrMot`, `faultOvCurrBrk` 고장 플래그를 세팅합니다.

### 2.2 과전압 진단 (입력 전압 및 브레이크 전압)
- **대상 및 임계치**: 입력 28V 전압(`xAdc.vsen28VLpf`) 기준 32.0V, 브레이크 전압(`PM_n24V`)
- **구현 현황**:
  - 입력 28V 전압은 `32.0f` 임계치를 바탕으로 100ms 필터링이 구현되어 있으나, **브레이크 전압(`PM_n24V`) 감시는 누락** 상태입니다.
- **개선 설계안**:
  - `hal_DspInit.c` 분석 결과 GPIO 40번 핀이 `PM_n24V` (Active Low, 1: 정상, 0: 이상) 입력으로 매핑되어 있음을 확인하였습니다.
  - 드라이버립 API인 `GPIO_readPin(40U)`을 적용하여 `0` (Active Low) 상태가 100ms 동안 유지될 경우 `xBit.faultOvVoltBrk = 1U`로 결함을 세팅하는 100ms 누적 필터 로직을 추가합니다.

### 2.3 게이트 오류 진단 (DRV_nFAULT)
- **대상**: 모터 드라이버 DRV8343 하드웨어 폴트 감지 (`DRV_nFAULT`)
- **구현 현황**:
  - 레지스터 직접 제어로 작성된 `GpioDataRegs.GPADAT.bit.GPIO10`을 감시하도록 되어 있습니다.
- **개선 설계안**:
  - 공통 룰을 준수하기 위해 드라이버립 API인 `GPIO_readPin(10U)`을 호출하여 Active Low(`0`) 일 때 `xBit.faultDrv8343nFault = 1U`로 결함을 세팅하는 방식으로 보완합니다.

### 2.4 모터 과속 보호
- **대상 및 임계치**: 모터 속도 피드백 `xMotorCtrl.currentSpeedRpm` 절대값 기준 3500.0 RPM
- **구현 설계안**:
  - 실시간 속도의 절대값이 3500.0 RPM 임계 범위를 초과하여 100ms 동안 유지될 경우 `xBit.faultOverSpeed = 1U` 결함 플래그를 셋하고, `xBit.informAll` 비트맵의 해당 비트를 활성화하는 감시 로직을 설계합니다.

### 2.5 엔코더 에러 & 워닝 진단 및 폴트 리셋
- **대상**: RLS AksIM-2 엔코더 하드웨어 프레임 감시
- **구현 설계안**:
  - `csu_Encoder.c`에서 파싱되는 `xEncoder.errBit`와 `xEncoder.warnBit` (둘 다 Active Low, 0: 이상)의 값을 활용하여 다음과 같이 폴트를 전파합니다.
    - **엔코더 에러**: `xEncoder.errBit == 0U` 일 때 즉시 `xBit.faultEncError = 1U` 발생.
    - **엔코더 워닝**: `xEncoder.warnBit == 0U` 일 때 즉시 `xBit.warnEncWarning = 1U` 발생.
  - `Bit_FaultReset()` 함수 내부에 신규로 추가될 고장 플래그(`faultOvVoltBrk`, `faultStall`, `faultOverSpeed`, `faultEncError`, `warnEncWarning` 등)와 이에 연동되는 누적 필터용 소프트웨어 카운터 변수들을 일괄 0U로 클리어하도록 확장 설계합니다.

---

## 3. 모터 스톨(Stall) 보호 기능 정밀 설계 (TDU 분석 반영)

TDU 사업의 속도 연산 주기 분석과 사용자의 구체적 요청 사항(1초 대기시간 및 5A 임계치)을 반영한 설계 사양입니다.

### 3.1 1ms 분주형 속도 측정 구조의 이식
- **이산화 오차 제어**:
  - 100us 제어 루프에서 단순 미분을 통해 속도를 계산하면 해상도 오차가 `22.88 RPM` 단위로 요동칩니다.
  - TDU 사업의 분주 방식을 차용하여 100us 인터럽트 내부에서 10회 카운트하여 **`1ms` 주기로만 속도를 차분 연산**하도록 변경합니다. (해상도 오차가 `2.288 RPM`으로 대폭 정밀화됨)
- **속도 갱신 제안식 (`csu_MotorCtrl.c`)**:
  ```c
  void MotorCtrl_UpdateFeedback_1ms(void)
  {
      static float32_t prevPos = 0.0f;
      static Uint16 speedCalcCnt = 0U;
      
      if (speedCalcCnt < 9U)
      {
          speedCalcCnt++;
      }
      else
      {
          xMotorCtrl.currentPosition = (float32_t)xEncoder.position * 0.001373291f; 
          float32_t posDiff = xMotorCtrl.currentPosition - prevPos;
          
          // 1ms 주기 기반 RPM 변환 (1/0.001 * 60 / 360 = 166.6667)
          xMotorCtrl.currentSpeedRpm = posDiff * 166.6667f;
          prevPos = xMotorCtrl.currentPosition;
          speedCalcCnt = 0U;
      }
  }
  ```

### 3.2 스톨 보호 판정 조건 및 파라미터 설계
TDU 경로의 감지 반응시간 규격과 지정된 전류 임계치를 기반으로 설계된 사양입니다:
- **감지 반응시간**: TDU 규격을 참고하여 **`1.0 초 (1000ms)`** 적용.
  - 100us 호출 주기 기준 누적 임계 카운트는 **`10000U`**가 됩니다.
  - 임계 카운트 상수: `#define BIT_LIMIT_STALL_TIME_CNT  10000U`
- **스톨 임계 전류**: 사용자 요청 사양에 따라 **`5.0 A`** 적용.
  - 모터 전류 피드백 `xAdc.isenMotLpf`의 절대값이 5.0A를 초과할 때 판정 조건에 진입합니다.
  - 임계 전류 상수: `#define BIT_LIMIT_STALL_CURR_MIN  5.0f`
- **스톨 판단 속도 한계**: **`10.0 RPM`** 미만.
  - 임계 속도 상수: `#define BIT_LIMIT_STALL_RPM_LIMIT 10.0f`

### 3.3 스톨 보호 구현 알고리즘안 (`csu_Bit.c`)
```c
/**
 * @function Bit_MStall_Check
 * @brief    TDU 규격(1초 대기) 및 5A 임계치에 따른 모터 Stall(회전 고착) 판단 로직
 * @param    void
 * @return   void
 */
void Bit_MStall_Check(void)
{
    static Uint16 BitCnt_Stall = 0U;
    
    // 1. 모터 필터링 전류가 5.0A 초과이고, 1ms 갱신 속도가 10 RPM 미만인 상태 진단
    if (fabsf(xAdc.isenMotLpf) > BIT_LIMIT_STALL_CURR_MIN)
    {
        if (fabsf(xMotorCtrl.currentSpeedRpm) < BIT_LIMIT_STALL_RPM_LIMIT)
        {
            // TDU 규격을 연동한 1초(10000회) 누적 대기 필터
            if (BitCnt_Stall >= BIT_LIMIT_STALL_TIME_CNT)
            {
                xBit.faultStall = 1U;
                xBit.faultFlagSet = 1U;
                xBit.informAll |= 0x08000000U; // bit27 스톨 고장 비트 활성화
                BitCnt_Stall = 0U;
            }
            else
            {
                BitCnt_Stall++;
            }
        }
        else
        {
            // 정상 동작 회전 감지 시 점진적 카운터 차감 (감쇄 필터링)
            if (BitCnt_Stall > 0U)
            {
                BitCnt_Stall--;
            }
        }
    }
    else
    {
        // 정상 부하 상태일 때 점진적 카운터 차감
        if (BitCnt_Stall > 0U)
        {
            BitCnt_Stall--;
        }
    }
}
```

---

## 4. 자체점검(CBIT) 호출 배치 계획
- `csu_Control.c`의 `Bit_RunCBIT()` 함수 내에 `Bit_MStall_Check()`를 주기적으로 추가하여, 매 100us 마다 전류 상태와 속도를 검사할 수 있도록 설계합니다.

---

## 5. TDU 사업(참조 프로젝트)의 전류 제어(Current Control) 구현 조사 보고

사용자의 요청에 따라 `D:\Nexcom\Firmware\Kim\TDU 분석\3차 수정\TDU_CTRL\TDU` 경로의 프로젝트 코드를 분석하여 전류 제어의 구현 유무와 방식을 조사하였습니다.

### 5.1 TDU 사업의 `csu_MotorCtrl_M1_CC.c` 분석 결과
해당 모듈은 이름이 `M1_CC` (Current Control)로 되어 있으나, 실제 코드를 분석해 본 결과 **전류 제어(Current PID/FOC) 로직은 완전히 제거된 상태**입니다.
- **주요 주석 내용 확인**: 파일 최상단의 이력을 보면 `2026. 06. 09. (FOC 제거 및 DRV8343 단일 PWM/방향 제어 로직 적용)` 이라고 명시되어 있습니다.
- **실제 구현 로직 확인**: 내부의 `M1_CurrentControl()` 함수는 전류 피드백을 받아 비례적분(PID) 연산을 수행하는 것이 아니라, 상위 속도 제어기에서 넘겨준 듀티 명령치(`M1_MScValue_DutyCmd`)의 부호에 따라 방향(`Do_Drv_Dir`)을 설정하고 절대값을 바로 PWM Duty(`EPwm1Regs.CMPA.bit.CMPA`)로 인가하는 단순 통과(Pass-through) 로직만 구현되어 있습니다.

### 5.2 ATTLA_T 프로젝트와의 비교 및 반영 현황
- **하드웨어 1x PWM 구동 특성**: TDU와 ATTLA_T 모두 모터 드라이버(DRV8343)를 **1x PWM 모드**로 사용합니다. 이 모드에서는 홀 센서 입력을 통한 하드웨어적인 6-Step 정류(Commutation)가 칩 내부에서 자체적으로 이루어지므로, 소프트웨어적인 전류 벡터 제어(FOC)나 내부 전류 PID 루프가 필요하지 않습니다.
- **현재 ATTLA_T 구현 상태**:
  - `ATTLA_T` 프로젝트의 `csu_MotorCtrl.c`에 이미 구현된 `MotorCtrl_SetOutput()` 함수가 TDU의 `M1_CurrentControl()` 함수와 정확히 100% 동일한 역할을 수행하고 있습니다.
  - 즉, 위치 제어기(`posPid`) ➡️ 속도 제어기(`speedPid`) ➡️ **전류 제어기 생략(Duty 직결)** ➡️ `Epwm_SetMotorDuty_1x()` 의 제어 파이프라인이 현재 아키텍처(1x PWM)의 가장 최적화된 올바른 형태입니다.

**💡 종합 결론**:
참고 프로젝트(TDU)에서도 전류 제어(PID) 로직은 하드웨어 정류 모드(1x PWM) 도입에 따라 **제거**되었으며, 오직 속도 제어기의 출력(Duty)을 PWM에 직결하는 로직만 사용 중입니다. 현재 `ATTLA_T` 프로젝트는 이와 정확히 동일하게 속도 PID 출력을 `MotorCtrl_SetOutput()`을 통해 인가하도록 **올바르게 구현이 완료된 상태**입니다. 따라서 추가적인 전류 제어기(PID) 설계나 코딩은 필요하지 않습니다.
