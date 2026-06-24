# ATTLA_T (초장사정 자동선회잠금장치) 시스템 아키텍처 및 기술 명세서

본 문서는 초장사정 자동선회잠금장치(ATTLA_T)의 TMS320F28388D 듀얼 코어 기반 펌웨어 및 시스템의 전체 아키텍처와 세부 기능 스펙을 종합적으로 정리한 시스템 설계서(Architecture)입니다. 하드웨어의 모든 핀맵, 물리 상수, 제어 주기, 스위칭 아키텍처 정보를 유실 없이 완벽하게 유지 관리하기 위해 작성되었습니다.

---

## 1. 시스템 개요 및 하드웨어 기본 설정

| 구분 | 상세 내용 |
| :--- | :--- |
| **타겟 장비** | 초장사정 자동선회잠금장치 (ATTLA_T) |
| **메인 MCU** | **TMS320F28388DPTPS** |
| **아키텍처** | **CPU1 + CM 듀얼 코어 구동 + CLA 보조** (CPU2 미사용) |
| **CPU1 역할** | 모터 제어(1x PWM 듀티/방향 제어), 전체 시스템 상태 머신 및 운용 제어 |
| **CM 역할** | W6100 이더넷 제어(SPI) 및 체계 연동 통신 전담 |
| **CLA 역할** | 아날로그 센싱(ADC) 데이터 고속 필터링 보조 연산 |

### 1.1 주요 동작 주파수 (Clock Specs)
| 항목 | 주파수 | 비고 |
| :--- | :--- | :--- |
| **CPU1 (C28x) Core** | 200 MHz | 메인 시스템 클럭 |
| **시스템 오실레이터** | 25 MHz | SIT2024BM-S2-33E-25.000000 사용 |
| **이더넷 크리스탈** | 25 MHz | ECS-250-8-37Q-RES-TR 사용 |
| **모터/스위칭 (PWM)** | 10 kHz | 100us 주기 (시스템 운용 핵심 제어 루프) |
| **DC/DC 컨버터** | 600 kHz / 330 kHz | DCM2322T50T2660T60 (600kHz), MGDD-21-N-C (330kHz) |

### 1.2 전원 공급망 (Power Supply)
- **전원 입력 (EMI & 보호)**: PLM74800DRRRQ1, LM5069MM-2 적용 (서지 억제 100V, 스파이크 억제 ±250V, 역전압 보호 -65V). TVS 다이오드(5.0SMDJ120A, 5.0SMDJ33CA)로 MIL-STD-1275E 규격 충족.
- **모터 구동 전원**: 18 ~ 32V 인가 (정격 24V), 최대 사용 전류 12.0A.
- **제어 전원 (3.3V/1.2V)**: 5V 외부 입력 ➡️ TPS70445PWP (고정밀 레귤레이터).
- **통신 전원 (3.3V)**: 체계 통신 안정성을 위해 ADP7158ARDZ-3.3-R7 적용 (독립 분리).

---

## 2. 모터 구동 및 제어부 (Motor & Drive)

### 2.1 모터 및 기구부 스펙
- **적용 모터**: **maxon EC 60** (24V, 200W)
  - 정격: 24V, 3240 rpm / 정지 토크: 2510 mNm, 정지 전류: 83.2 A
  - 최대 연속 토크: 540 mNm, 최대 연속 전류: 9.34 A
- **기구 구동부**: 기어비 44:1 (모터 44회전 = 기구 1행정)
- **제어 목표**: 
  - 위치 제어 범위 (Soft Limit): 0.0° ~ 15840.0° (멀티턴 44바퀴 한계)
  - 속도 제한: ±3240 RPM (제어기 한계치 및 과속 오류 기준 통일)
  - 위치 오차 ±1° 이내, 동작 제한 속도 1.5초 이내 (24V 인가 기준, 18V 인가 시 초과 허용).

### 2.2 모터 드라이버 (DRV8343)
- **적용 칩셋**: 스마트 게이트 드라이버 **DRV8343SPHPRQ1**
- **인버터 구성**: N-Channel MOSFET (IPTC007N06NM5ATMA1) 6개, 최대 전류 10.0 A. MAX14130/4131 절연 칩 적용.
- **제어 인터페이스 (SPI-B)**:
  - **핀맵**: `CLK` (GPIO 58), `nCS` (GPIO 59), `SIMO` (GPIO 60), `SOMI` (GPIO 61)
- **전용 제어 핀 (1x PWM 하드웨어 정류 배선)**:
  - `DRV_PWM` (`INHA`) (GPIO 0, EPWM1A, Active High) - 모터 속도/토크 제어용 10kHz PWM
  - `DRV_ENABLE` (GPIO 2, Active High) - DRV8343 활성화
  - `DRV_DIR` (`INHC`) (GPIO 3, Active High) - 모터 회전 방향 제어
  - `DRV_nBRAKE` (`INLC`) (GPIO 4, Active Low) - 모터 드라이버 제동 제어용 (1: Normal 구동, 0: 제동)
  - `DRV_nFAULT` (GPIO 10, 입력, Active Low) - 드라이버 하드웨어 폴트 감지

### 2.3 브레이크 (Brake) 제어 회로
- **동작 전류**: 최대 1.0 A
- **제어 핀**: `DSP_BRAKE` (GPIO 35, Active High - High 출력 시 브레이크 잠금 해제, Low 출력 시 기계적 잠금 유지)
- **로직 구현**: `csu_MotorCtrl.c` 내 `MotorCtrl_Run()`에서 모터가 정지 상태(`MOTOR_MODE_STOP`)일 경우 Low(잠금)를 출력하고, 구동 모드 진입 시 High(해제)를 출력하여 모터 기동 전 브레이크를 개방합니다.
- **회로 구성**: DSP(3.3V) ➡️ NPN TR(MMBT489LT1G) ➡️ TLP293(광절연) ➡️ P-Ch MOSFET(SPD15P10PLGBTMA1) ➡️ 24V 브레이크 구동. 역기전력 방어용 쇼트키 다이오드(V8PAM10HM3/I) 탑재.
- **모니터링**: 브레이크용 TMCS1126 개별 전류 센서를 통해 동작 및 고장 상태 진단.

- **모터 구동 및 제어 연산 방식 (`csu_MotorCtrl`)**:
  - **제어 주기**: 100us (EPWM1 인터럽트 기반 동적 주기 제어)
  - **제어 모드 (`MotorControlMode_t`)**: `MOTOR_MODE_STOP`, `MOTOR_MODE_SPEED_CTRL`, `MOTOR_MODE_POS_CTRL`, `MOTOR_MODE_FAULT_STOP` (고장 정지 모드 추가)
  - **고장 감지 연동 (Fail-Safe)**: 
    - 리미트 스위치 결함 감지(`LimitSwitch_CheckFaults()`) 또는 내장 테스트(BIT) 결함 플래그(`xBit.faultFlagSet == 1U`) 발생 시 즉각 `MOTOR_MODE_FAULT_STOP` 모드로 강제 전환.
    - 고장 정지 모드 진입 시, 모터 출력(PWM) 즉시 0.0f 차단, 기계적 브레이크 잠금 유지, 그리고 모든 제어기(위치/속도/전류)의 PID 적분항(`integral`)을 0으로 완전 초기화하여 에러 해제 후 재구동 시 Windup(급발진) 현상을 원천 방지함.
  - **상태 관리 구조체**: `stMotorCtrlState xMotorCtrl` (내부에 목표/현재 위치 및 속도 변수 포함)
  - **위치 피드백 연산 (`xMotorCtrl.currentPosition`)**:
    - `xEncoder.position` (34비트 오프셋 보정 완료 위치) × `MOTOR_SCALE_POS_DEG` (0.001373291f, 360/2^18) = 기계각(Degree)
  - **정밀 속도 피드백 연산 (`xMotorCtrl.currentSpeedRpm`)**:
    - 기존 100us 미분 시 발생하는 이산 노이즈를 억제하기 위해 **1ms 분주(Decimation)** 로직 적용 (`DECIMATION_SPEED_CTRL`).
    - 연산식: `posDiff * MOTOR_SCALE_SPEED_RPM` (166.6667f, (1/0.001)*60/360)
  - **PID 제어 루프 (3-Stage Cascade) 및 제약 (Soft Limit)** (`csu_Pid` 범용 제어기 적용):
    - **위치 지령 클램핑**: 체계 명령 또는 목표 위치(`targetPosition`)는 `LIMIT_POS_MIN` (0.0f) ~ `LIMIT_POS_MAX` (15840.0f, 44바퀴 × 360°) 범위 내로 강제 제한됨.
    - **위치 제어기 (`posPid`)**: **PD 제어** 적용. dt=0.005s (`PID_POS_DT`, 5ms 분주 `DECIMATION_POS_CTRL`: 5U, 5ms / 1ms) / 출력 제한 `LIMIT_SPEED_MAX` (±3240.0 RPM)
    - **속도 제어기 (`speedPid`)**: **PI-IP 혼합 제어** 적용(`Ks` 계수 연동). dt=0.001s (`PID_SPD_DT`, 1ms 분주 `DECIMATION_SPEED_CTRL`: 10U, 1ms / 100us) / 출력 제한 `LIMIT_CURRENT_MAX` (±9.34 A)
    - **전류 제어기 (`currPid`)**: **PI 제어** 적용. dt=0.0001s (`PID_CURR_DT`) / 출력 제한 `-MOTOR_DUTY_MAX` ~ `+MOTOR_DUTY_MAX` (±100.0f %) (**부호 연산 기반 4상한 제어**)
    - **제어 파라미터 전역 튜닝 (`xPidGain`)**: 실시간 제어 튜닝을 위해 3단 제어기의 파라미터(Kp, Ki, Kd, Ks)가 매크로 하드코딩에서 벗어나 `xPidGain` 전역 구조체(`xPidGain.pos.Kp`, `xPidGain.spd.Ki` 등)로 통합 관리되어 매 제어 루프 반영됨.
    - **제어 순서**: 위치 지령 ➡️ `posPid` ➡️ 목표 속도 ➡️ `speedPid` ➡️ 목표 전류량 ➡️ `currPid` (절댓값 변환 없이 부호 유지) ➡️ 최종 목표 Duty 도출.
  - **출력 인가 및 영점 교차 처리 (`MotorCtrl_SetOutput`)**:
    - 전류 제어기를 통해 도출된 최종 부호 있는 `duty` 값(-100.0% ~ 100.0%)은 분리 없이 출력 함수로 인가됨.
    - 함수 내부에서 부호(+/-)를 판별하여 `DRV_DIR` 방향 핀을 전환(`MotorDriver_SetDir`)하고, 하드웨어 PWM 레지스터에만 절댓값으로 변환 인가(`Epwm_SetMotorDuty_1x`)함. 이를 통해 모터의 역방향 제동 및 영점 교차(Zero-Crossing) 구간에서 오차 왜곡이나 꺾임 현상(Fold-back) 없는 매끄러운 제어를 실현.

---

## 3. 정밀 위치 및 상태 센싱 (Sensing)

### 3.1 모터 위치 검출 (Encoder & Hall)
- **홀 센서 (하드웨어 정류 연동)**: 
  - `HALL_A` (`INLA`, GPIO 11), `HALL_B` (`INHB`, GPIO 12), `HALL_C` (`INLB`, GPIO 13)
  - 홀 센서 신호가 DSP로 입력됨과 동시에 DRV8343의 INLx/INHx 핀으로 직결되어 **1x PWM 모드의 하드웨어 기반 6-Step Commutation**을 수행하도록 설계됨. (SN74HCS126PWR 버퍼 경유)
- **디지털 앱솔루트 엔코더 (RLS AksIM-2)**:
  - **분해능**: 18-bit (싱글턴) + 16-bit (멀티턴 카운터) / 오차: ±0.004° ~ ±0.020°
  - **통신 방식**: **단방향 SPI-C 기반 SSI 통신** (BiSS-C 로직 완전 제거). SN65HVD30MDREP 차동 변환기 탑재.
  - **클럭 주파수**: **2.5 MHz** (SPI-C Baud Rate)
  - **사용 핀**: `SOMI` (GPIO 51, `ENCODER_SOMI_GPIC`), `CLK` (GPIO 52, `ENCODER_CLK_GPIC`)
  - **통신 타이밍 및 파싱 규칙**:
    - **초기 대기**: 하드웨어 초기화 단계에서 100ms 대기 로직 적용.
    - **통신 타임아웃 방어**: BiSS 타임아웃(13.5us) 회피용 SPI TX/RX FIFO 64-bit 수신 블로킹 프리.
    - **Dynamic Parsing**: 데이터 프레임 중 첫 '1' 비트를 능동적으로 Start 비트로 감지.
    - **CRC 규칙**: 다항식 0x43 (36비트 대상) 적용 검증.
    - **정밀 변환**: 제로셋 FRAM 저장 값 및 롤오버 상수(`ENC_ROLLOVER_34BIT`: 0x400000000ULL, 2^34) 뺄셈 및 오프셋 적용. 변환 계수 `ENC_SCALE_18BIT_DEG` (0.001373291f, 360 / 2^18).

### 3.2 아날로그 센서 (ADC) 
- **ADC 아키텍처 및 폴링 동기화**:
  - **Reference**: 3.0V (REF4132B30DBVRQ1). 내부 연산 시 `ADC_SCALE_REF_VOLT` 변환 상수 적용.
  - **Trigger**: EPWM1 SOCA (10kHz, 100us)
  - 기존 하드웨어 인터럽트 발동 구조에서 **EPWM1 기반 ISR 내 능동 폴링(Polling) 대기 방식**으로 아키텍처를 변경하여, 메인 타이머와 완전한 동기를 유지합니다.
- **채널 매핑 및 변환 스펙**:

| 센서 / 감시 항목 | 채널 할당 | 스펙 및 물리량 변환 계수 |
| :--- | :--- | :--- |
| **모터 전류** | ADCA SOC2 (A2) | TMCS1126 (100mV/A). 영점 1.49702V. 역산 계수: `16.1550888f` |
| **브레이크 전류** | ADCA SOC3 (A3) | TMCS1126. -2.4A ~ +2.4A (영점 1.49702V). 역산 계수: `1.6155088f` |
| **입력 28V 전압** | ADCA SOC4 (A4) | 50V 입력 시 약 2.96451V 변환. 전압 복원 계수: `16.86619f` |
| **시스템 5V 전압** | ADCA SOC5 (A5) | 5V 입력 시 약 2.500V 변환. 전압 복원 계수: `2.0f` |
| **Reference 전압** | ADCB SOC1 (B1) | REF3040 기반 2.048V 검증용. 변환 계수: `1.0f` |
| **보드 온도** | ADCB SOC3 (B3) | MAX6605 내부 온도 센서. -55℃ ~ +125℃. 수식: `(V_in * 84.033613f) - 55.0f` |

### 3.3 내장 테스트 (BIT) 결함 임계치 및 Fail-Safe (Fault Limits)
- **모터 과전류 (OVC_MOT)**: `10.0 A` 임계치.
- **브레이크 과전류 (OVC_BRK)**: `1.5 A` 임계치.
- **28V 과전압 (OVV_28V)**: `32.0 V` 임계치.
- **보드 과열 (OVT_BD)**: `80.0 ℃` 임계치.
- **모터 과속 (OVS_MOT)**: 모터 정격과 동일한 `3240.0 RPM` 임계치. (100ms 지연 필터)
- **모터 구속 (STALL)**: 전류가 `5.0 A` 초과이면서 속도가 `10 RPM` 미만으로 유지될 때. (기구적 끼임 사고 대비, 1.0초 지연 필터)
- **엔코더 및 드라이버 결함**: 통신 에러 및 DRV8343 하드웨어 폴트 감지 (디바운싱 후 즉각 반영).
  > 💡 **참고**: 각 고장 판정에는 일시적 노이즈에 의한 오탐지를 방지하기 위해 누적 지연 필터 카운터(예: `BIT_CNT_FILTER_REF` 100ms)가 적용되어 있으며, 최종 결함 확정 시 `xBit.faultFlagSet`이 세트되어 구동기가 즉시 차단(Fail-Safe)됩니다.

---

## 4. 체계 통신 및 외부 인터페이스 (Comms & I/O)

### 4.1 체계 통신 (W6100 이더넷) 및 연동통제안 규격
- 하드웨어 TCP/IP 스택 기반 대용량 칩셋(W6100) 적용 (HX1188NL 절연). **(CM 코어 전담 제어)**
- **인터페이스 (SPI-A)**: `SIMO`(16), `SOMI`(17), `CLK`(18), `nCS`(19), `INTn`(20, XINT1 외부 인터럽트 수신), `RSTn`(21)
- **통신 프로토콜 기본 정책**: 
  - UDP 통신 단일 사용 (`SOCK_UDP_COM`: 0, 통신 포트: 5001)
  - **드라이버 경량화**: 정적 시험 복잡도 및 메모리 최적화를 위해 TCP, IPv6, MACRAW 등 미사용 통신 기능 코드를 전면 제거한 UDP 전용 드라이버 아키텍처 적용 완료.
  - 모든 데이터는 **Little Endian** 형식 준수
  - **체크섬 (Checksum)**: 메시지 맨 끝 2 Bytes 할당. 체크섬 필드를 제외한 모든 필드의 바이트(Byte) 단위 합산 결과 중 최하위 2 Bytes 적용.
- **메시지 패킷 구조 (직렬화/역직렬화 적용)**:
  - C28x 아키텍처의 1바이트=16비트 한계를 극복하기 위해 `sizeof()` 및 `memcpy` 대신 명시적인 바이트 직렬화(Serialization) 적용.
  - 패킷 송신 시 배열(TxBuffer)에 8비트씩 Shift 연산하여 `12 Bytes`의 헤더 규격 강제 준수.
  - **헤더부(12 Bytes)**: Timestamp(4), Source_ID(1), Dest_ID(1), Code(1), Request_ACK(1), Priority(1), Send_Count(1), Data_Length(2).
  - **ACK 응답 메시지 (18 Bytes 고정)**: Header(12 Bytes) + Data(4 Bytes) + Checksum(2 Bytes)
    - `ACK Data`: Code_Info(2B), Ack_Info(2B, 정상:0x00, 체크섬오류:0x01 등)
- **응답(ACK) 및 타임아웃/재전송 로직**:
  - ACK를 요청받은 경우 수신 후 100ms 이내에 리턴 (체크섬 오류 시 NACK 0x11 리턴)
  - 송신 후 100ms 이내 응답 없을 시 재전송 수행. 최초 1회 포함 **총 4회** (재전송 3회) 시도 (`Send_Count` 반영).
  - 4회 시도에도 응답이 없으면 통신 두절로 판단하고 소켓 리셋.
- **상태 머신 (통신망 가입 및 연동통제안 규격, `csu_Ethernet.c` 백그라운드 태스크 구현 완료)**:
  - **STATE_BOOTING**: 28V 제어전원 인가 및 초기화 후 즉각 망 가입 절차로 천이.
  - **STATE_WAIT_BOOT_ACK**: 망 가입을 위해 화포통제컴퓨터로 `BOOT_DONE` 500ms 주기 전송 및 응답 대기.
  - **STATE_JOINED**: 망 가입 완료 후, 100ms 주기로 상태정보(`ETH_CODE_STATUS_REQ`) 요청 수신 시 응답 수행.
  - **STATE_COMM_LOSS (통신 두절 롤백)**: 100ms 주기 상태 메시지가 **연속 50회(5초) 이상** 미응답 시 소켓을 닫고 리셋한 뒤 `STATE_WAIT_BOOT_ACK` 단계로 롤백.
- **CBIT / IBIT 제어 로직**:
  - CBIT(주기 점검)는 지정된 N초 단위로 100us ISR 내부에서 백그라운드로 송신. (ACK 미요청)
  - IBIT(지시 점검) 수행 시 CBIT가 일시 중단되며, IBIT 완료 보고 후 다시 CBIT를 재개함.

### 4.2 디버깅 및 장거리 통신
- **easyDSP**: SCI-A (`SCI_PC_GPIO_PIN_SCIA_RXD`: GPIO 28, `SCI_PC_GPIO_PIN_SCIA_TXD`: GPIO 29), 115200bps. `PonRST` 신호를 통해 전원/DSP 리셋 동기화.
- **RS-422 통신**: ISOW7843DWE 절연형 트랜시버 탑재. 장거리 차동 신호 송수신 지원.

### 4.3 디지털 입출력 (Digital I/O)
- **이산신호 디바운싱**: 100us 제어 주기 기반으로 1ms(10 카운트, `DIO_CNT_DEBOUNCE_REF`) 연속 동일 상태 유지 시에만 값이 갱신되는 대칭형 소프트웨어 디바운싱 필터(`csu_Dio`)를 적용하여 기계적 바운싱 및 전기적 노이즈를 원천 차단합니다.

| 구분 | 신호명 (핀) | 핀 방향 및 특징 |
| :--- | :--- | :--- |
| **입력 (센서)** | `nLIMIT1_NO` (36), `nLIMIT1_NC` (37) | 리미트 스위치 1번 (Active Low) |
| **입력 (센서)** | `nLIMIT2_NO` (38), `nLIMIT2_NC` (39) | 리미트 스위치 2번 (Active Low) |
| **입력 (감시)** | `PM_n24V` (40) | 24V 주 전원 감시 (Active Low) |
| **입력 (감시)** | `CABLE_LOOP` (46) | 외부 케이블 연결 체결 감시 (Active Low) |
| **출력 (상태)** | `DSP_LED_nNORMAL` (31) | 외부 시스템 Normal 상태 표시 (Low ➡️ ON) |
| **출력 (상태)** | `DSP_LED_nFAULT` (32) | 외부 시스템 Fault 상태 표시 (Low ➡️ ON) |
| **출력 (LED)** | `LED_G` (30) | 보드 내부 DSP 동작 상태 점멸용 |

### 4.4 비휘발성 메모리 (FRAM)
- **적용 칩셋**: CY15B256Q-SXE
- **인터페이스 (SPI-D)**: `SIMO`(91), `SOMI`(92), `CLK`(93), `CS`(94) / 1MHz 속도.
- **특징**: 데이터 로깅 및 오프셋 저장용. Instant Write 특성 활용 무지연 엑세스.
- **데이터 로깅 연동 (`csu_Control`)**: 초기 1초 오프셋 보정이 완료된 직후(`csu_Offset_Isr`), 산출된 모터 및 브레이크 전류 오프셋을 FRAM 주소(0x0000 ~ 0x0003)에 자동 백업합니다.

### 4.5 내부 SPI 통신 규격 (Internal SPI Protocols)
프로젝트 내의 모든 내부 SPI 통신망은 `hal_Spi.c` 모듈 한 곳에서 통합 관리되며, 각 타겟 칩셋의 데이터시트 및 대역폭 요구사항에 맞춰 다음과 같이 개별 프로토콜(POL, PHA)과 보레이트가 최적화되어 있습니다. (TI C2000 기준)

| 통신 모듈 | 채널 | 모드 및 속도 | 동작 방식 및 근거 |
| :--- | :--- | :--- | :--- |
| **W6100 (이더넷)** | **SPI-A** | `POL0PHA0`<br>10 MHz | 클럭 Idle Low, 상승 에지 수신 (SPI Mode 0). 초기 20MHz로 기획되었으나, 시스템 LSPCLK 최대 한계(50MHz / 4 = 12.5MHz) 초과로 인한 Driverlib 초기화 에러(ASSERT)를 방지하기 위해 가장 안정적인 10MHz로 속도를 수정 적용함. |
| **DRV8343 (모터)** | **SPI-B** | `POL0PHA1`<br>1 MHz | 클럭 Idle Low, 하강 에지 출력 / 상승 에지 수신. 레지스터 설정용이므로 안정성 위주의 1MHz 통신. DRV8343 권장 SPI 타이밍 규격(하강 에지 출력) 충족. |
| **SSI 엔코더** | **SPI-C** | `POL1PHA0`<br>2.5 MHz | 클럭 Idle High, 첫 하강 에지에서 데이터 출력 시작 / 상승 에지 수신. 절대각 SSI 프로토콜의 표준 통신 스펙 규격을 정밀하게 따름. |
| **FRAM (메모리)** | **SPI-D** | `POL1PHA0`<br>1 MHz | 클럭 Idle High, 상승 에지 수신 (SPI Mode 3). FRAM 칩 규격을 충족하며, 파라미터 백업 등 소량 데이터 저장을 위해 노이즈 내성을 극대화한 1MHz로 설정. |

---

## 5. 시스템 제어 파이프라인 아키텍처 (Dynamic Interrupt Switching)

새롭게 설계된 시스템 제어 파이프라인은 `main.c` 내에서의 CPU 블로킹 대기를 제거하고, 타이머 기반 하드웨어 인터럽트 벡터를 실시간으로 교체하는 **동적 인터럽트 스위칭 기법**을 도입하여 아래와 같이 완벽하게 분리 수행됩니다.

### ✅ 단계 0: 코어 간 부팅 동기화 (IPC Handshake)
- 전원 인가 직후, CPU1은 PBIT 진입 전에 CM 코어가 독자적으로 기동하여 이더넷(PHY)을 초기화하고 인터럽트 셋업을 마칠 때까지 기다립니다.
- CM 코어에서 `IPC_CMD_CM_BOOT_READY` 신호를 발송하면 CPU1이 이를 수신하여 본격적인 제어 파이프라인을 가동합니다.

### ✅ 단계 1: 오프셋 인터럽트 (`csu_Offset_Isr`)
- 최초 부팅 시 100us 마다 호출됩니다.
- 내부에서 `while()`을 통해 ADC 변환 완료를 폴링하여 1.5V 영점 전압 기준값을 정밀하게 수집합니다.
- `10,000`번 누적(1초간)하여 오프셋의 평균을 계산 및 고정한 후, 스스로 PIE 벡터 테이블을 교체하여 제어권을 PBIT 인터럽트로 넘깁니다.

### ✅ 단계 2: 초기 점검 인터럽트 (`csu_Pbit_Isr`)
- 오프셋 단계 완료 후 100us 마다 호출됩니다.
- 초기 과전압, 과열, 게이트 폴트 등 PBIT를 1회 점검 수행합니다. 이상이 없으면, 대기시간 없이 즉시 메인 제어루프 인터럽트로 제어권을 스위칭합니다.

### ✅ 단계 3: 메인 제어 인터럽트 (`csu_MainControl_Isr`)
- 모든 초기화가 끝난 후 시스템이 종료될 때까지 100us 주기로 영구적으로 호출되는 최종 제어 ISR입니다.
- 가이드라인에 따라 `Control_SystemOperation()` 함수를 통해 다음의 7단계 파이프라인으로 엄격히 순차 실행됩니다:
  1. **아날로그 신호 입력 및 연산** (`CalcAdcData`)
  2. **이산신호 입력** (`Dio_UpdateInput`)
  3. **위치(각도) 정보 획득** (`Encoder_UpdatePosition`)
  4. **모터 드라이버 상태 획득** (`MotorDriver_UpdateStatus`)
  5. **주기 점검** (`Bit_RunCBIT`)
  6. **모터 구동 제어** (`MotorCtrl_Run`)
  7. **데이터 저장** (`Control_SaveDataToFram`)

---

## 6. 글로벌 구조체 변수 파이프라인 (디버깅 및 제어 상태 참조용)

디버그(CCS Expressions/Watch 윈도우 등) 목적으로 참고할 수 있는, ATTLA_T 펌웨어 아키텍처 내 주요 글로벌 구조체 변수들입니다.
`volatile` 선언이 된 구조체(`xSysCtrl`, `xDio`)는 실시간 상태 변경이 바로 반영되므로 디버그에 유용합니다.

### 6.1 CSU (Control & Service Unit) 계층

| 변수명 | 데이터 타입 | 역할 및 주요 멤버 변수 |
| :--- | :--- | :--- |
| **`xAdc`** | `stAdcState` | LPF(저주파 통과 필터) 적용 아날로그 센싱 값 및 영점 오프셋(Offset) 값 저장 |
| **`xBit`** | `stBitState` | 시스템 내장 테스트(BIT) 결과. 각 모듈의 고장(Fault), 경고(Warning) 플래그 통합 |
| **`xSysCtrl`** | `volatile stControlState` | 전체 시스템의 제어 상태(오프셋 캘리브레이션 완료 여부, PBIT 완료 여부 등) 플래그 관리 |
| **`xDio`** | `volatile stDioState` | 디바운싱 필터가 적용된 이산신호(리미트 스위치, 시스템 감시, 홀센서 상태 등) 상태 |
| **`xLimitSwitchConfig`**| `stLimitSwitchConfig`| 리미트 스위치 매핑, 타겟 위치 및 오차 허용치(Tolerance) 설정 데이터 |
| **`xLimitSwitch`** | `stLimitSwitchState` | 리미트 스위치 고장 판단(단선, 위치 불일치) 상태 및 에러 플래그 관리 |
| **`xEncoder`** | `stEncoderState` | 모터/기구부 앱솔루트 엔코더(SSI) 상태. (위치, 360도 환산 각도, 에러 상태 등) |
| **`xMotorCtrl`** | `stMotorCtrlState` | 모터 제어기의 현재 상태. (운전 모드, 목표/현재 속도(RPM), 목표/현재 위치 등) |
| **`xPidGain`** | `stPidGain` | 실시간 튜닝용 3단 계단식(위치/속도/전류) PID 제어기의 파라미터 게인 (Kp, Ki, Kd, Ks) 모음 |
| **`xMotorDriver`**| `stMotorDriverState` | DRV8343 모터 드라이버 IC의 하드웨어 결함(Fault) 상태 저장 |
| **`xRcvSciPcMsg1`**| `stRcvSciPcMsg1` | PC 또는 체계로부터 수신된 메시지 버퍼 및 파싱 구조체 |
| **`xXmtSciPcMsg1`**| `stXmtSciPcMsg1` | PC 또는 체계로 송신할 SCI 메시지 버퍼 구조체 |
| **`xLed`** | `stLedStatus` | 시스템 상태 표시용 LED(점멸 패턴 등) 제어 상태 |
| **`xEthCtrl`** | `stEthControl` | 이더넷 통신망 가입 상태 머신, 재전송 타이머, CBIT/IBIT 제어 구조체 |

### 6.2 HAL (Hardware Abstraction Layer) 계층

| 변수명 | 데이터 타입 | 역할 및 주요 멤버 변수 |
| :--- | :--- | :--- |
| **`adcRawData`** | `AdcRawData_t` | 인터럽트(ISR)에서 수집한 ADC 하드웨어 변환 결과(Raw Data) 버퍼 |
| **`xTimer`** | `stTimer` | 하드웨어 타이머(CPUTimer) 기반 이벤트 제어용 카운터/플래그 |
| **`xQueSCI_PC`** | `stQsci` | SCI-A (easyDSP/PC 통신) 비동기 송수신용 링 버퍼 큐 구조체 |

---

## 7. 미구현 및 추후 확정 예정 사항 (Pending Implementation & TBD)

현재 펌웨어 하드웨어 연동 및 기본 제어 파이프라인은 구성되었으나, 외부 체계 및 운용 요구사항 미확정으로 인해 다음 사항들은 아직 구현되지 않았습니다.

| 구분 | 상세 내용 |
| :--- | :--- |
| **LED 표시 로직** | 어떠한 시스템 상태(정상/오류/구동 등)에서 **LED 점등 및 점멸**을 수행할지에 대한 명확한 조건 미구현 |
| **홀센서 안전 제어** | 모터 상태 모니터링을 위한 홀센서 3상(A/B/C) 입력이 `xDio`로 수집되고 있으나, 이를 통한 구속(Stall) 감지 및 회전 방향 교차 검증 로직은 미구현 |

---

** 회로도 경로 D:\Nexcom\00_Work\삼현 - 초장사정 자동선회잠금장치\02_설계\03_회로도\2026.06.09_회로도 \ATTLA_CONTROL_REV0_1_260609
