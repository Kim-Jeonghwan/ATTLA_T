# 📋 ATTLA_T 메인 초기화 시퀀스 및 제어 루프 아키텍처 개편 계획서

**작성 일자**: 2026. 06. 11.  
**대상 코어**: CPU1 Core (`ATTLA_T_CPU1`)  
**관련 문서**: `Project_Spec.md`, `소프트웨어 CSCI`, `소프트웨어 CSCI - 실행개념도`

---

## 1. 개요 (Overview)
제공해주신 실행개념도(Flowchart)와 CSCI 구조도를 기반으로 전체 시스템의 구동 시퀀스를 재설계합니다. 
가장 핵심적인 변경 사항은 제어 루틴의 실시간성을 보장하기 위해 **PWM Interrupt(100us 주기)** 를 시스템 제어의 심장(Heartbeat)으로 삼고, 모든 핵심 제어 파이프라인(시스템 운용 CSU)을 이 인터럽트 내에서 실행하는 것입니다. 
또한 이더넷 통신은 폴링(Polling) 방식에서 **외부 인터럽트(Ext. Interrupt)** 방식으로 전환합니다. 기존 `main()` 함수의 10ms, 100ms 루프는 삭제하지 않고 유지하며, 비실시간성(Low Priority) 태스크 처리에 활용합니다.

---

## 2. 기존 이더넷 동작 구조 및 개편 방향 (Ethernet Status & Plan)
- **현재 동작 방식 (Polling & Flag)**: 
  - 현재 이더넷(W6100) 수신/송신 로직은 `main()`의 `cycle_1ms()` 타이머 폴링 루프에서 `Ethernet_Process()` 함수를 호출하여 동작하고 있습니다.
  - 이와 별개로 송신의 경우, 기존 EPWM1 2ms 타이머 인터럽트(`isr_Epwm1Timer2ms`)에서 매 2ms마다 `flag_2ms_tx = 1`로 플래그를 세워주면, `Ethernet_Process()`에서 이 플래그를 감지하여 2ms 주기로 UDP 패킷을 송신하는 구조로 되어 있습니다.
  - 즉, 현재 **이더넷 인터럽트(Ext. Interrupt)는 구현되어 있지 않으며 메인 루프에 전적으로 의존**하고 있습니다.
- **개편 방향 (Ext. Interrupt 기반)**: 
  - 실행개념도에 맞추어 W6100 칩의 `INTn` 핀(GPIO 20)을 C28x의 외부 인터럽트(XINT)로 라우팅합니다.
  - 상위 체계(화포통제컴퓨터)로부터 데이터가 수신되거나 통신 이벤트가 발생하면 즉시 하드웨어 인터럽트가 발생하여 `화포통제컴퓨터 통신 CSU` 로직이 실행되도록 전면 수정합니다.

---

## 3. 상세 리팩토링 및 구현 계획 (Proposed Changes)

### 3.1 HAL: FRAM 딜레이시간 제거
#### [MODIFY] `hal_Fram.c`
- **변경 사항**: `Fram_PageWrite` 함수 등에 남아있는 10ms 블로킹 딜레이(`DELAY_US(10000u)`) 삭제. (FRAM Instant Write 특성 반영)

### 3.2 Main Application: 실행개념도 기반 초기화 시퀀스 재배치
#### [MODIFY] `main.c` / `main.h`
- 실행개념도에 맞추어 `main()`의 초기화 시퀀스를 다음과 같이 엄격한 블로킹 구조로 변경합니다:
  1. `DSP_Initialization()` (CPU 부팅 및 초기화 CSU)
  2. 인터럽트 기동 (`Interrupt_enable` 등을 호출하여 PWM Interrupt 100us 동작 시작)
  3. **전류센서 Offset 조정 대기**: `While Loop`를 돌며 PWM Interrupt 내에서 동작하는 Offset 조정이 완료될 때까지 대기(End?)
  4. **초기점검(PBIT) 대기**: `While Loop`를 돌며 PWM Interrupt 내에서 동작하는 PBIT가 완료될 때까지 대기(End?)
  5. `Ethernet Interrupt Enable`: W6100 외부 인터럽트 활성화
- **메인 유휴 루프 (`while(1)`)**:
  - 기존 10ms, 100ms 루프(cycle_10ms, cycle_100ms)는 구조를 그대로 유지하되, 내부 로직은 LED 점멸 등 덜 중요한 동작만 수행하도록 정리.

### 3.3 CSU & HAL: PWM 및 ADC 인터럽트 역할 분리 
#### [MODIFY] `hal_EpwmTimer.c` 및 `hal_Adc.c` 
- **ADC 인터럽트 (`AdcaIsr`)**:
  - 별도로 독립되어 동작합니다. EPWM1 SOC 트리거에 의해 변환이 완료되면 단순히 ADC RAW 데이터를 읽고 필터링하여 글로벌 변수 구조체에 갱신하는 역할만 담당합니다.
- **PWM 인터럽트 (`isr_Epwm1Timer100us`)**:
  - 주기를 100us(10kHz)로 정확히 설정합니다.
  - 실행개념도의 **시스템 운용 CSU** 역할을 수행합니다. 매 100us 마다 아래 모듈들을 순차적으로 호출(Call)합니다.
    1. `아날로그신호 입력 및 연산 CSU` (ADC 완료된 데이터를 가져와 물리량으로 변환)
    2. `이산신호 입력 CSU` (Limit Switch, Hall 등 갱신)
    3. `위치(각도) 정보 획득 및 처리 CSU` (BiSS-C / SSI 엔코더 값 갱신)
    4. `모터 드라이버 상태 정보 획득 및 처리 CSU`
    5. `주기점검(CBIT) CSU` (초기화 완료 이후 시스템 운용 중일 때만 동작)
    6. `모터 구동제어 CSU` (PID 위치/속도 제어 등)
    7. `데이터 저장 CSU` (필요시 FRAM 저장)

### 3.4 CSU: 외부 연동 통신 (이더넷 인터럽트)
#### [NEW / MODIFY] `csu_Ethernet.c` 등
- **변경 사항**:
  - `Ext. Interrupt` (XINT) 서비스 루틴을 신규 등록.
  - 인터럽트 발생 시 W6100 레지스터를 읽어 수신된 데이터를 파싱하고 응답하는 **화포통제컴퓨터 통신 CSU** 로직 구현.

---

## 4. 진행 가이드 (Next Steps)
실행개념도 구조에 맞게 ADC 인터럽트와 PWM 인터럽트(시스템 운용 CSU 담당)를 완벽히 분리하고, 이더넷 또한 타이머 폴링에서 외부 인터럽트(Ext. Interrupt) 방식으로 완전히 전환하는 계획을 세웠습니다.
본 계획(전류센서/PBIT 대기 루프 및 인터럽트 분배 구조)에 동의해주시면 바로 리팩토링 및 코드 구현을 시작하도록 하겠습니다.
