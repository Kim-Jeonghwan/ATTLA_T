# 구현 계획 (Implementation Plan): ADC 폴링 전환 및 동적 인터럽트 스위칭

## 1. 목표 (Goal)
상무님의 지시사항 및 TDU 참조 코드 분석 결과에 따라, 다음 사항들을 구현합니다.
1. **ADC 변환 방식 변경**: 하드웨어 ADC 인터럽트 진입 방식에서 **EPWM1 인터럽트 내 ADC 완료 폴링(대기) 방식**으로 전환.
2. **동적 인터럽트 스위칭 구현**: 메인 백그라운드 루프에서의 블로킹 대기를 제거하고, `csu_Offset_Isr` ➡️ `csu_Pbit_Isr` ➡️ `csu_MainControl_Isr` 순서로 인터럽트 벡터를 동적으로 변경하여 100us 시퀀스를 제어.
3. **명세서 정리 및 이름 변경**: 기존 `Project_Spec.md`의 세부 내용(GPIO, 변수, 함수명 등)을 일체 유실 없이 유지하면서 문서를 보기 좋게 정리하고, 파일명을 변경하여 최신 구현 내용을 반영.

---

## 2. 사용자 피드백 요청 (User Review Required)
> [!NOTE]
> **명세서(Spec) 파일명 추천**
> "프로젝트스펙" 이라는 단어 대신, 소프트웨어 구조와 하드웨어 제어 규격을 모두 담고 있으므로 **`Architecture.md` (아키텍처)** 또는 **`SystemDesign.md` (시스템 설계서)** 를 추천합니다.
> 하지만 사용하기 가장 깔끔하고 짧은 단어를 원하신다면 **`Spec.md`** 로 변경하는 것도 좋습니다. 마음에 드시는 명칭을 알려주시면 해당 이름으로 정리하겠습니다.

> [!IMPORTANT]
> **PBIT 대기 및 검증 시간 질문**
> Offset은 10,000회(1초) 수행 후 PBIT로 넘어갑니다. PBIT 인터럽트(`csu_Pbit_Isr`)에서는 과전압/과열/게이트폴트를 점검하는데, **1회라도 정상이면 바로 메인 제어 루프로 넘어갈지**, 아니면 PBIT 단계에서도 **수백 ms 정도의 안정화(대기) 시간**을 가진 뒤 넘어갈지 결정이 필요합니다. (별도 지시가 없으시다면 1회 정상 확인 후 즉시 메인 제어로 스위칭하도록 구현하겠습니다.)

---

## 3. 제안된 코드 변경 사항 (Proposed Changes)

### 3.1. ADC 모듈 (HAL 계층)
#### [MODIFY] `hal_Adc.c` / `hal_Adc.h`
- `InitAdcModules()` 내의 `ADC_enableInterrupt(ADCA_BASE, ADC_INT_NUMBER1);` 구문을 제거하여 PIE로의 인터럽트 발생을 차단합니다. (인터럽트 플래그 세팅 로직은 폴링을 위해 유지)
- 기존의 `AdcaIsr()` 함수를 완전히 제거합니다.

### 3.2. EPWM 모듈 (HAL 계층)
#### [MODIFY] `hal_Epwm.c` / `hal_Epwm.h`
- `Initial_EpwmTimer()` 내에서 `EPWM1` 모듈이 `TBCTR_ZERO` 시점에 인터럽트를 발생시키도록 설정합니다.
  ```c
  EPWM_setInterruptSource(EPWM_TIMER1_BASE, EPWM_INT_TBCTR_ZERO);
  EPWM_enableInterrupt(EPWM_TIMER1_BASE);
  EPWM_setInterruptEventCount(EPWM_TIMER1_BASE, 1U);
  ```

### 3.3. 시스템 제어 모듈 (CSU 계층)
#### [MODIFY] `csu_Control.c` / `csu_Control.h`
- 신규 ISR 3종 세트를 선언하고 구현합니다. (함수는 RAM 공간인 `.TI.ramfunc`에 할당)
- **`csu_Offset_Isr()`**:
  1. ADC INT 플래그 `while` 대기 및 플래그 클리어
  2. ADC 데이터 리드 (`CalcAdcData()`)
  3. 전류/브레이크 오프셋 누적 연산
  4. 10,000 카운트(1초) 도달 시 평균 적용 및 `Interrupt_register(INT_EPWM1, &csu_Pbit_Isr);` 스위칭
- **`csu_Pbit_Isr()`**:
  1. ADC INT 플래그 대기 및 클리어
  2. ADC 데이터 리드
  3. `Bit_RunPBIT()` 수행. 이상 없을 경우 `Interrupt_register(INT_EPWM1, &csu_MainControl_Isr);` 스위칭
- **`csu_MainControl_Isr()`**:
  1. ADC INT 플래그 대기 및 클리어
  2. ADC 데이터 리드
  3. 메인 파이프라인 수행 (`Encoder_UpdatePosition()`, `Bit_RunCBIT()`, `MotorCtrl_Run()` 등)
- 기존에 `Control_SystemOperation()` 내부에 있던 Offset 캘리브레이션과 PBIT 대기 분기 로직은 제거하여 최적화합니다.

#### [MODIFY] `csu_Adc.c` / `csu_Adc.h`
- 오프셋 누적을 위해 1.5V를 읽는 과정에서 기존 EMA 필터를 거치기 전의 RAW 데이터(혹은 1차 선형 스케일링 된 데이터)를 직접 10,000번 합산할 수 있도록 로직을 다듬습니다.

### 3.4. 메인 루프 (Main)
#### [MODIFY] `main.c`
- 부팅 및 초기화(`System_Initialization`) 직후에 `Interrupt_register(INT_EPWM1, &csu_Offset_Isr);` 및 `Interrupt_enable(INT_EPWM1);`을 수행하여 최초 오프셋 인터럽트를 가동합니다.
- 메인 함수에 있던 다음의 폴링(블로킹) 대기 while 문들을 완전히 제거합니다. (인터럽트 동적 스위칭이 이를 대체함)
  ```c
  while(xSysCtrl.isOffsetCalibrated == 0U) {}
  while(xSysCtrl.isPbitComplete == 0U) {}
  ```

### 3.5. 명세서 (Documentation)
#### [NEW] `Spec.md` (또는 확정된 이름)
- 기존 `Project_Spec.md` 내용 전체 복사 후, 가독성 높은 헤더/표/강조 포맷팅 적용.
- 구현 완료될 "동적 인터럽트 스위칭 (Offset -> PBIT -> Main)" 및 "ADC 폴링 아키텍처" 내용을 스펙에 업데이트.
#### [DELETE] `Project_Spec.md`
- 기존 파일 삭제

---

## 4. 검증 계획 (Verification Plan)
- `main.c`에서 블로킹 대기 구문 삭제 후에도 펌웨어가 정상적으로 부팅되는지 확인.
- `csu_Offset_Isr`가 100us 간격으로 10,000회 정상 호출되며 1초간 지연되는지 검증.
- `csu_Pbit_Isr`를 거쳐 `csu_MainControl_Isr`로 ISR 벡터가 성공적으로 전환되는지(모터 구동 및 CBIT 진입 여부) 확인.
- ISR 내에서 ADC 폴링 `while` 문이 무한 루프(Deadlock)에 빠지지 않고 정상 탈출하는지 확인.
