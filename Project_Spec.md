# ATTLA_T (초장사정 자동선회잠금장치) 시스템 전체 명세서 (Project Specification)

본 문서는 초장사정 자동선회잠금장치(ATTLA_T)의 TMS320F28388D 듀얼 코어(CPU1 + CM) 기반 펌웨어 및 PC 프로그램의 전체 아키텍처와 세부 기능 스펙을 정리한 문서입니다. 
기존 TMDSCNCD28388D_T 템플릿을 기반으로 실제 시스템 회로 및 요구사항에 맞게 재구성되었으며, 새로운 기능이 구현될 때마다 본 문서를 지속적으로 업데이트하여 시스템의 현재 상태를 완벽하게 추적합니다.

---

## 1. 하드웨어 및 시스템 기본 설정
- **타겟 장비** : 초장사정 자동선회잠금장치 (ATTLA_T)
- **메인 MCU** : TMS320F28388DPTPS (CPU1 + CLA 단독 구동 아키텍처 적용)
- **주요 동작 주파수** :
  - **CPU1 (C28x)** : **200 MHz**
  - **시스템 오실레이터** : 25 MHz
  - **모터드라이버 / MOSFET 스위칭 (PWM)** : 10 kHz
  - **DC/DC 컨버터** : 330 kHz / 600 kHz
- **아키텍처 역할 분담** :
  - **CPU1**: 모터 제어(FOC, PWM), W6100 이더넷 제어(SPI), 전체 시스템 상태 머신 단독 처리
  - **CLA**: 아날로그 센싱 필터링 보조 연산
  - *참고: CM 코어 및 CPU2는 사용하지 않으며 관련 초기화 및 IPC 로직은 제거됨.*
- **전원 공급 부** : 
  - 전원 입력단 (EMI & 보호): PLM74800DRRRQ1 및 LM5069MM-2를 적용하여 EMI 필터링 및 이상 전압 시 전원 스위칭(차단) 보호
  - 시스템 제어 전원: 외부 5V 입력 ➡️ 3.3V 및 1.2V 고정밀 전원 레귤레이팅
  - 모터 구동 전원: 18~32V 인가 (모터 정격 전압은 24V)
- **비휘발성 메모리 (FRAM)** : CY15B256Q-SXE (SPI-D 통신, 1MHz 속도 설정, 파라미터 및 에러 로그 등 데이터 저장 목적)
  - **사용 핀 (SPI-D)**: SIMO (GPIO 91), SOMI (GPIO 92), CLK (GPIO 93), CS (GPIO 94)
  - **SPI 설정**: Master 모드, Mode 3 (POL1PHA0), 8-bit Data
  - *참고: 페이지 쓰기(Page Write) 수행 시 안정성을 위해 10ms 블로킹 딜레이(Blocking Delay)가 적용되어 있습니다.*

---

## 2. 모터 드라이브 및 제어부 스펙
- **제어 대상 모터** : maxon EC 60 24V 200W 
  - 정격 전압: 24V, 정격 속도: 3240 rpm
  - 최대 연속 토크: 540 mNm, 최대 연속 전류: 9.34 A
  - 정지 토크: 2510 mNm, 정지 전류: 83.2 A
- **기구 구동부 스펙** : 기어비 44:1 (모터 44회전 = 기구 1행정)
- **제어 목표 및 성능** : 위치 오차 ±1° 이내, 동작 제한 속도 1.5초 이내 (반응시간 포함, 24V 인가 기준). 
  - *참고: 18V 인가 시에는 동작 시간 미달(1.5초 초과) 허용.*

## 3. 액추에이터 제어 (모터 및 브레이크)
- **모터 드라이버 (게이트 드라이버)** : DRV8343H-Q1 탑재
  - 제어 핀:
    - `DRV_ENABLE` (GPIO 2, Active High)
    - `DRV_DIR` (GPIO 3, Active High)
    - `DRV_nBRAKE` (사용하지 않음)
    - `DRV_nFAULT` (GPIO 10, 입력, Active Low)
  - 제어 인터페이스: SPI (직렬 통신)를 통한 드라이버 레지스터 설정 및 진단
  - 하드웨어 제어: PWM 입력을 통한 3상 인버터 구동
- **브레이크(Brake) 제어 회로** :
  - DSP 제어 핀: `DSP_BRAKE` (GPIO 37, Active High)
  - 제어 로직: DSP의 3.3V GPIO 제어 신호를 TLP293 광절연기(Optocoupler)를 통해 절연.
  - P-Channel MOSFET (SQJ431EP-T1_GE3) 구동 회로를 거쳐 24V 브레이크 동작.
- **모터 위치 검출** :
  - 홀 센서 (SN74HC9126PWR 버퍼 경유)
    - `HALL_A_IN` (GPIO 11)
    - `HALL_B_IN` (GPIO 12)
    - `HALL_C_IN` (GPIO 13)
  - **디지털 엔코더** : RLS 사 AksIM-2 (MB039DCC18MENT00)
    - **통신 방식**: BiSS-C (RS-422 기반 양방향 통신, 레지스터 R/W 지원)
    - **분해능**: 18-bit (1회전 싱글턴) + 16-bit (멀티턴 카운터)
    - **정확도**: ±0.004° ~ ±0.020°
    - **최대 동작 속도**: 10,000 RPM
    - **전원 사양**: 4.5V ~ 5.5V (통상 130mA 소모)
    - **온도 범위**: -40°C ~ +105°C (보관 및 작동)
    - **사용 핀 (SPI-B / CLB)**: 데이터 수신 SOMI (GPIO 25), 마스터 클럭 송신 MA (GPIO 26)
    - **통신 구현 및 특이사항 (pm_bissc 연동)**:
      - **적용 모듈**: TI `pm_bissc` 라이브러리를 포팅하여 `CLB2` 및 `SPIB` 모듈을 활용.
      - **제어 구조**: `main.c`의 1ms 고속 타이머 주기에서 `Encoder_UpdatePosition()`으로 위치를 획득하고, `Encoder_ProcessCDTasks()`로 레지스터 커맨드를 비동기(32주기/프레임) 처리.
      - ⚠️ **주의사항 (OUTPUT X-BAR 래치 충돌)**: 원본 `pm_bissc_source.c`의 초기화 로직은 CLB 리셋 시 클럭(MA)이 요동치는 것을 방지하기 위해 `XBAR_OUTPUT1`의 래치(Latch) 기능을 하드코딩하여 사용합니다. 현재 프로젝트의 클럭 핀(GPIO 26)은 `OUTPUTXBAR3`에 연결되어 있으므로 해당 래치 보호가 적용되지 않습니다. 초기 구동 또는 시스템 리셋의 찰나에 클럭 핀에 짧은 글리치(Glitch)가 발생할 수 있으며, 만약 이로 인해 엔코더 에러가 유발될 경우 향후 원본 `pm_bissc_source.c` 파일을 직접 수정하여 `XBAR_OUTPUT1`을 `XBAR_OUTPUT3`로 일괄 치환해야 합니다.
    - **엔코더 통신 프로토콜 핵심 스펙 (Register Map)**:
      - **데이터 포맷**: 16/32비트 다중 바이트 접근 시 **Big Endian** 적용 (가장 낮은 주소가 최상위 바이트 MSB)
      - **주소 체계**: 직접 접근(Direct access) `0x40~0x7F`, 뱅크 접근(Bank 0~87) `0x00~0x3F` (주소 `0x40`에 Bank 번호 기록 후 사용)
      - **주요 모니터링 레지스터 (Direct)**: 상태 및 에러(`0x4A-0x4B`), 온도(`0x4C-0x4D`), 신호 레벨(`0x4E-0x4F`), RPM(`0x50-0x51`)
      - **주요 설정 레지스터 (Bank 0)**: Position Offset(`0x00-0x03`), Filter Value(`0x04-0x07`), Multiturn Preset(`0x14-0x17`), Persistent Error(`0x2C-0x2D`)
      - **커맨드 시퀀스 (보안 해제 절차)**: 모든 제어/저장 명령은 Key 레지스터(`0x48`)에 `0xCD`를 기록한 후, Command 레지스터(`0x49`)에 특정 ASCII 코드를 전송하여 실행됨.
        - 파라미터 비휘발성 저장: `'c' (0x63)` (80ms 소요)
        - 멀티턴 프리셋 적용: `'m' (0x6D)`
        - 셀프 캘리브레이션 시작: `'A' (0x41)` (이후 0x52 상태 모니터링)
        - 공장 초기화(Factory Reset): `'r' (0x72)` (80ms 소요)
- **전류 센싱** : TMCS1108A4BQDBVRQ1 (모터 부하 전류 측정)

---

## 3. 외곽 인터페이스 및 제어부
- **브레이크 출력 제어** :
  - 제어: `DSP_BRAKE` 신호를 통한 TLP293 포토커플러(절연) ➡️ SPD15P10PLGBTMA1 P-Ch MOSFET 구동
  - 모니터링: 브레이크 해제 코일 상태 감시용 개별 TMCS1108A4BQDBVRQ1 전류 센서 적용 (고장 진단 정확성 향상)
- **리미트 스위치 및 전원 모니터링 입력** : TLP293 포토커플러 절연 회로 적용 (GND 분리 및 유도성 노이즈, 서지 유입 차단)
  - 적용 부품: 오므론 D2VW-01-1M (PT+OT거리: 2.2mm)
  - 역할: 장치의 잠금(Lock) 및 풀림(Unlock) 상태를 하드웨어적으로 최종 인식
  - 상태 감지: DSP 입력 기준 NO(Normally Open, 확인 필요) 상태일 때 Low, 그 외의 경우 High로 인식
  - **입력 감시 (디지털 입력)** :
    - `nLIMIT1` : 리미트 스위치 1번 (GPIO 38, Active Low)
    - `nLIMIT2` : 리미트 스위치 2번 (GPIO 39, Active Low)
    - `PM_n24V` : 24V 브레이크/스위치 전원 감시 (GPIO 40, Active Low)
    - `CABLE_LOOP` : 체계 케이블 연결 체결 감시 (GPIO 46, Active Low)
- **상태 표시 및 외부 출력 제어 (LED/IO)** : 
  - **내부 상태 표시용 (G LED)**: PCB 내부에 장착된 단일 상태 표시 LED (DSP 내부 로직 구동 상태 모니터링, GPIO 30)
  - **외부 출력용 (NORMAL / FAULT)**: 외부 장비 연결/표시용. DSP 기준으로는 단순 제어용 IO 출력 핀으로 동작.
    - 구동 회로: MCP1415T 하이사이드 드라이버 2채널 적용
    - 제어 로직: DSP 핀 출력 기준 Low ➡️ 외부 ON(구동), High ➡️ 외부 OFF(차단)
    - `NORMAL` 출력 (`DSP_LED_nNORMAL` 핀, GPIO 31)
    - `FAULT` 출력 (`DSP_LED_nFAULT` 핀, GPIO 32)

---

## 4. 아날로그 센서 (ADC) 및 신호 처리 (정밀 스펙)
- **ADC 기준 및 트리거** : 
  - ADC 레퍼런스(VREFHI): **3.0V**
  - 트리거: **EPWM1 SOCA (10kHz, 100us)**
  - 필터: 전류/전압 측정값에 지수 이동 평균(EMA) 필터 적용 (Old 0.3 / Real 0.7 비율), 온도 측정은 (Old 0.9 / Real 0.1) 적용
- **입력 보호 회로** : 전 ADC 입력 채널(DSP 근처 배치)에 BAT54S 쇼트키 다이오드를 적용한 0~3.3V 클램핑(Clamping) 회로 탑재.
- **ADC Reference 감시 (`ADC_VSEN_REF`)** : 
  - 채널: **ADCB SOC1 (ADCINB1)**
  - REF3040AIDBZTG4 전용 레퍼런스 칩 (4.096V) 출력 후 OP-AMP 전압 분배를 통해 **2.048V**를 생성하여 ADC가 감시 (0~3V 범위 측정).
- **모터 및 브레이크 전류 센싱 (`ADC_ISEN_MOT`, `ADC_ISEN_BRK`)** : 
  - 채널: 모터 **ADCA SOC2 (ADCINA2)** / 브레이크 **ADCA SOC3 (ADCINA3)**
  - 센서: TMCS1108 (-24A ~ +24A 범위, 100mV/A, 기준 전압 2.5V)
  - OP-AMP (TLV9002) 증폭비: **0.619** (6.19k / 10k)
  - 10kHz Cut-off 로우패스 필터 적용 (스위칭 노이즈 차단)
  - 변환 스펙: -24A에서 +24A 사이 구간을 약 **0.01142V ~ 2.98262V** 로 변환하여 DSP ADC로 입력
- **입력 전압 모니터링 (`ADC_VSEN_28V`, `ADC_5VD`)** : 
  - 채널: 28V **ADCA SOC4 (ADCINA4)** / 5V **ADCA SOC5 (ADCINA5)**
  - 28V 변환 스펙: 50V 입력 시 약 **2.96451V** 로 측정
  - 5V 변환 스펙: 5V 입력 시 약 **2.5V** 로 측정
- **온도 센서 (`ADC_TSEN_BD`)** : MAX6605MXK+T 보드 내장 아날로그 온도 센서.
  - 채널: **ADCB SOC3 (ADCINB3)**
  - 변환 스펙: -55도에서 +125도 사이를 약 **0V ~ 2.142V** 로 측정

---

## 5. 체계 통신 인터페이스
- **W6100 이더넷 (체계 통신)** : 
  - 하드웨어 TCP/IP 스택 내장, HX1188NL 절연 트랜스포머 경유.
  - CPU1에서 SPI A 통신을 통해 직접 제어 (WIZnet 공식 ioLibrary_Driver 포팅 적용).
  - **사용 핀 (SPI-A 제어)**: SIMO (GPIO 16), SOMI (GPIO 17), CLK (GPIO 18), nCS (GPIO 19), INTn (GPIO 20), RSTn (GPIO 21)
  - 통신 프로토콜: 기존 사양을 유지하여 UDP 프로토콜, 5001번 포트, 18바이트 페이로드 사용.
  - 수신(RX) 방식: 1ms 주기 타이머 폴링(Polling) 방식으로 안정성 확보 (W6100 인터럽트 대신).
- **디버깅 통신 (easyDSP)** : 
  - 통신 방식: SCI-A 기반 easyDSP 통신 탑재 (보드 상의 스위치 조작에 따라 10핀 커넥터로 직결하거나, RS-422 절연 트랜시버 ISOW7843DWE를 거쳐 외부 장비와 통신하도록 스위칭 가능)
  - 통신 속도(Baudrate): **115200 bps**
  - **사용 핀**: SCI-A RX (GPIO 28), SCI-A TX (GPIO 29)
  - **하드웨어 리셋 구조**: `PonRST` 신호는 easyDSP 리셋 핀, 내부 3.3V 레귤레이터(TPS70445PWP) 리셋 핀, 그리고 DSP `XRS_N` 핀과 공통으로 묶여있어 동작/리셋 시 완벽히 연동됨.
  - **Boot Mode 핀**: `nBOOT` 신호는 **GPIO 72**에 연결됨.
- **RS-422 직렬 통신** : 절연형 트랜시버 ISOW7843DWE 탑재. 노이즈에 강한 절연/차동 신호 기반 장거리 통신 지원.

## 7. 구현 현황 및 향후 추가 구현 예정 기능 (Implementation Status)

### ✅ 현재 구현 완료 (Implemented)
- **듀얼 코어 구조 제거**: CM 코어 및 IPC 동기화 루틴 제거, CPU1 단독 동작 구조 확립
- **W6100 (전송 구조 확립)**: 기존 CM 코어 IPC를 완전히 제거하고 10kHz ISR 타이머 플래그 기반으로 백그라운드 1ms 루프에서 18바이트 UDP 송신(임시 데이터) 연동 완료 (수신 파싱 제외)
- **easyDSP**: SCI-A (GPIO 28/29) 115200bps 통신 기반 디버깅 인터페이스 연동 완료
- **내부 상태 표시 (G LED)**: 보드 내부 구동 상태 모니터링용 G LED(GPIO 30) 토글 및 상태 제어 로직 구현 완료
- **ADC (10kHz 주기 획득)**: EPWM1 10kHz 트리거, 외부/내부 센서(전류, 전압, 온도) 선형 스케일링, EMA 필터 연동 완료
- **FRAM 연동**: SPI-D(1MHz) 기반 FRAM 제어 기능 연동 완료 (Driverlib 구조체 및 10ms 블로킹 지연시간 적용)
- **엔코더 (BiSS-C)**: CLB 및 SPI-B 연동 기반 BiSS-C 통신 구조체(pm_bissc) 포팅 및 1ms 주기의 백그라운드 태스크 연동 완료 (AksIM-2)
- **디지털 입력 (GPIO IN) 연동**: 홀 센서(A, B, C), 모터드라이버 nFAULT, 리미트 센서(nLIMIT1, nLIMIT2), 24V 전원 감시(PM_n24V), 케이블 체결 감시(CABLE_LOOP) 신호 핀 할당 및 설정 완료
- **디지털 출력 (GPIO OUT) 연동**: 브레이크 제어(DSP_BRAKE), 장비 표시용 상태 출력(LED_nNORMAL, LED_nFAULT) 핀 할당 및 기본 제어 상태 설정 완료

### ⏳ 미구현 / 보류 (Pending / On-Hold)
- **ENET (보류)**: 내부 EMAC 이더넷은 보류하고 외부 W6100을 우선적으로 사용

### 🚀 향후 구현 예정 (To Be Implemented)
- **모터 드라이버 제어**: DRV8343H SPI 통신 기반 초기화/진단, 3상 인버터 FOC 제어 알고리즘 및 PWM 생성
- **W6100 (상세 페이로드 연동)**: 18바이트 통신 데이터 구조체 맵핑 및 수신 패킷(제어 명령) 파싱 로직 구현
- **상태 머신 (State Machine)**: 시스템 전체 상태 제어 및 브레이크-모터 연동 시퀀스
