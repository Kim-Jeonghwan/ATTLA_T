# ATTLA-T 펌웨어 CSU 상세 구현 보고서 (심층 분석)

이 문서는 ATTLA-T 펌웨어의 소프트웨어 CSCI 구조를 바탕으로 각 CSU(Control & Service Unit)가 코드 상에서 어떤 함수와 변수로 어떻게 정밀하게 구현되어 있는지 아주 상세하게 정리한 문서입니다. 

---

## 1. 시스템 초기화 CSC

### 1.1 CPU 부팅 및 초기화 CSU
- **관련 파일**: `csu_Control.c`
- **핵심 함수**:
  - `Control_Init()`: 전원 인가 직후 제어 상태를 관리하는 `xSysCtrl` 구조체 멤버들을 명시적으로 초기화합니다. 내부적으로 `Dio_Init()`을 호출합니다.
  - `Offset_Isr()`: 초기 구동 시 10,000번 호출되는 100us 타이머 인터럽트 함수입니다.
- **주요 변수**:
  - `xSysCtrl` (`stControlState` 구조체): `isOffsetCalibrated`, `isPbitComplete`, `offsetCount` 플래그 및 카운터를 포함합니다.
  - `PieVectTable.EPWM1_INT`: 동적 인터럽트 스위칭을 위해 타겟 벡터 주소를 교체(`&Pbit_Isr` -> `&MainControl_Isr`)할 때 조작하는 레지스터 변수입니다.
- **동작 상세**: 
  - `Offset_Isr`가 구동되면 1초간 실행되며, 완료 조건(`xSysCtrl.offsetCount >= 10000U`) 달성 시 `EALLOW` 권한을 열어 `EPWM1_INT` 벡터를 `Pbit_Isr`로 스위칭합니다. 이 동적 인터럽트 스위칭 기법을 통해 초기화 단계와 메인 제어 단계를 분리합니다.

### 1.2 전류센서 Offset 조정 CSU
- **관련 파일**: `csu_Control.c`, `csu_Adc.c`
- **핵심 함수**:
  - `Offset_Isr()`: ADC 원시 데이터를 누적합 연산합니다.
  - `Control_SaveOffsetToFram()`: 연산이 끝난 오프셋을 비휘발성 메모리(FRAM)에 저장합니다.
- **주요 변수**:
  - `adcRawData.isenMot`, `adcRawData.isenBrk`: 12비트 ADC 원시 측정값 변수.
  - `xSysCtrl.sumMot`, `xSysCtrl.sumBrk`: 10,000회의 변환 값을 1.5V 기준으로 스케일링 후 누적하는 `float32_t`형 변수.
  - `xAdc.isenMotOffset`, `xAdc.isenBrkOffset`: 최종적으로 10,000으로 나눈 평균 오프셋 값이 대입되는 변수.
- **동작 상세**:
  - 누적된 값을 평균 내어 `xAdc` 구조체에 대입한 후, 밀리암페어(mA) 단위 정수형으로 변환(`(uint16_t)(xAdc.isenMotOffset * 1000.0f)`)하여 FRAM의 `0x0000`, `0x0002` 주소에 상위/하위 바이트로 나누어 `Fram_WriteByte()`를 통해 저장합니다.

---

## 2. 자체 점검 CSC

### 2.1 초기점검 CSU (PBIT)
- **관련 파일**: `csu_Control.c`, `csu_Bit.c`
- **핵심 함수**:
  - `Pbit_Isr()`: `Offset_Isr` 종료 후 스위칭되는 두 번째 100us 초기화 전용 인터럽트 루틴.
  - `Bit_RunPBIT()`: 초기 점검 로직을 모아둔 래퍼 함수.
- **주요 변수**:
  - `xBit.faultFlagSet`: 전체 시스템 결함 유무를 판별하는 통합 플래그 변수 (0이면 정상).
  - `xSysCtrl.isPbitComplete`: PBIT 통과 여부를 나타내는 플래그 (1이면 통과).
- **동작 상세**:
  - `Bit_RunPBIT()` 내에서 `Bit_OvVoltage_Check()`, `Bit_OvTemperature_Check()`, `Bit_GateFault_Check()` 만을 1회 수행합니다. (초기이므로 과전류 검사는 제외). 
  - 에러가 없으면(`faultFlagSet == 0`) `xSysCtrl.isPbitComplete = 1`을 세팅하고, 인터럽트 벡터를 `&MainControl_Isr`로 최종 교체하여 메인 시스템 제어로 진입합니다.

### 2.2 주기점검 CSU (CBIT)
- **관련 파일**: `csu_Bit.c`, `csu_Control.c`
- **핵심 함수**:
  - `Bit_RunCBIT()`: `MainControl_Isr` 내에서 매 주기마다 호출되는 주기 점검 래퍼 함수.
  - `Bit_OvCurrent_Check()`, `Bit_MotorStall_Check()`, `Bit_MotorOverSpeed_Check()` 등: 각 항목별 결함 탐지 함수.
- **주요 변수**:
  - `xBit` (`stBitState` 구조체): `faultOvCurrMot`, `faultOvCurrBrk`, `faultStall`, `faultOverSpeed`, `faultEncError` 등의 개별 결함 플래그와 `informAll`(비트마스크 에러 정보)를 관리합니다.
  - `BitCnt_Mot`, `BitCnt_OvSpeed`, `xBit.stallCheckCnt`: 일시적 노이즈에 의한 오탐지를 막기 위해 에러 조건을 지속 확인하는 디바운싱 필터 카운터 변수들.
- **동작 상세**:
  - 예를 들어 모터 스톨 점검(`Bit_MotorStall_Check`)의 경우, 절대 전류값(`currentAbs`)이 `BIT_LIMIT_STALL_CURR_MIN`을 초과하고 속도(`speedAbs`)가 `BIT_LIMIT_STALL_RPM_LIMIT` 미만인 상태가 `BIT_LIMIT_STALL_TIME_CNT` 기간 동안 누적 유지될 때만 `xBit.faultStall = 1` 및 `xBit.informAll |= 0x00020000U`를 설정합니다.

### 2.3 임의점검 CSU (IBIT)
- **관련 파일**: `csu_Ethernet.c`
- **핵심 함수**:
  - `Ethernet_ParsePacket()`: 이더넷 수신 패킷을 분석하는 함수 내부 분기(switch-case).
- **주요 변수**:
  - `pHeader->Code`: 수신된 패킷 헤더의 명령어 코드 (`ETH_CODE_IBIT_REQ`).
  - `xEthCtrl.IbitInProgress`: IBIT 테스트가 진행 중임을 알리는 상태 플래그.
- **동작 상세**:
  - 체계로부터 `ETH_CODE_IBIT_REQ` 명령 수신 시 `xEthCtrl.IbitInProgress = 1`로 세팅 후, 현재는 임시 구현으로 즉시 `Ethernet_SendMessage()` 함수를 호출하여 `ETH_CODE_IBIT_REP`를 응답한 뒤 플래그를 클리어합니다.

---

## 3. 시스템 제어 CSC

### 3.1 아날로그 신호 입력 및 연산 CSU
- **관련 파일**: `csu_Adc.c`
- **핵심 함수**:
  - `CalcAdcData()`: 인터럽트 내에서 하드웨어 원시값을 물리량으로 변환하는 함수.
- **주요 변수**:
  - `adcRawData.vsen28v`, `vsen5vd`, `tsenBd`: 원시 디지털 변환 값 (12비트).
  - `xAdc` 구조체 필터 변수들: `vsen28VLpf`, `isenMotLpf` 등.
  - 상수: `ADC_SCALE_REF_VOLT`(스케일 팩터), `LPF_OLD_CV`, `LPF_REAL_CV`(EMA 필터 계수).
- **동작 상세**:
  - 원시값에 전압 스케일 상수(`ADC_SCALE_REF_VOLT`)를 곱해 `V_in`을 도출하고, 센서 사양에 맞는 역산 상수(`ADC_SCALE_ISEN_MOT` 등)를 적용해 물리량 `curr_val`을 구합니다.
  - 도출된 `curr_val`을 `xAdc.isenMotLpf = (LPF_OLD_CV * xAdc.isenMotLpf) + (LPF_REAL_CV * curr_val)` 와 같이 누적하는 방식으로 지수 이동 평균(EMA) 필터를 적용합니다.

### 3.2 이산신호 입력 CSU
- **관련 파일**: `csu_Dio.c`
- **핵심 함수**:
  - `Dio_UpdateInput()`: 매 주기 스위치 핀들을 스캔하는 함수.
  - `Dio_Debounce(uint16_t rawValue, volatile uint16_t* pFilteredValue, uint16_t* pCount)`: 대칭형 디바운싱 알고리즘을 수행하는 정적(Static) 함수.
- **주요 변수**:
  - `xDio.limit1No`, `xDio.pm24V`, `xDio.cableLoop`, `xDio.hallA` 등: 1, 0으로 필터링 완료된 최종 상태 구조체.
  - `cnt_limit1No`, `cnt_pm24V` 등: 디바운싱 연속 누적을 기록하는 정적(static) 카운터 변수.
- **동작 상세**:
  - `GPIO_readPin(36U)` 등을 통해 원시 핀 상태(`rawValue`)를 읽고, 현재 구조체 상태(`*pFilteredValue`)와 다르면 카운터(`*pCount`)를 1씩 증가시킵니다. 카운터가 `DIO_CNT_DEBOUNCE_REF`를 초과해야만 비로소 `*pFilteredValue`를 갱신하는 노이즈 강인성 구조를 취하고 있습니다.

### 3.3 이산신호 출력 CSU
- **관련 파일**: `csu_MotorCtrl.c` (LED_nNOrmal 과 LED nFault 는 GPIO 설정만 완료, 제어 로직 미구현)
- **핵심 함수**:
  - `MotorCtrl_SetOutput(float32_t outputDuty)`: 모터 구동 모드에 따라 브레이크 해제 및 구동 출력을 결정하는 함수.
- **주요 변수**:
  - `xMotorCtrl.mode`: 현재 모터 상태 (`MOTOR_MODE_STOP`, `MOTOR_MODE_POS_CTRL` 등).
- **동작 상세**:
  - 상태표시등(nNormal, nFault) 제어 로직은 아직 미구현 상태이며, 현재는 GPIO 핀 초기화 설정만 되어 있습니다.
  - `MotorCtrl_Run` 함수 내부에서 `xMotorCtrl.mode`가 `MOTOR_MODE_STOP`일 경우 브레이크 핀(`GPIO_writePin(35U, 0U)`)을 제어하여 물리적으로 잠금 처리합니다.

### 3.4 위치(각도) 정보 획득 및 처리 CSU
- **관련 파일**: `csu_Encoder.c`
- **핵심 함수**:
  - `Encoder_UpdatePosition()`: SPI 수신 데이터를 파싱하고 각도로 환산하는 종합 함수.
  - `Encoder_CalcCrc6(uint64_t data36)`: 비트 시프트 및 XOR 연산을 통한 소프트웨어 CRC 계산.
- **주요 변수**:
  - `rawData64`: SPI-C를 통해 수신된 64비트 원시 데이터.
  - `xEncoder.rawPos`, `xEncoder.position`: 오프셋 적용 전/후 34비트 절대 위치 변수.
  - `xEncoder.angleDeg`: 기계각으로 환산된 실수형(`float32_t`) Degree 위치.
  - `startBitPos`: MSB에서 처음 나타난 '1'의 인덱스를 저장하는 동적 파싱 기준 변수.
- **동작 상세**:
  - `startBitPos`를 기점으로 34비트 위치(`extPos`), `errBit`, `warnBit`, 6비트 CRC를 추출합니다.
  - `Encoder_CalcCrc6` 함수에 36비트(Position+Error+Warning)를 넣어 0x43 다항식 연산 후 수신된 CRC와 일치하는지(`isValid = true`) 판별합니다.
  - 오프셋(`xEncoder.offset`)을 차감하고, 음수가 될 경우 34비트 롤오버 상수(`ENC_ROLLOVER_34BIT`)를 더하여 보정한 후 스케일 팩터(`ENC_SCALE_18BIT_DEG`)를 곱해 `xEncoder.angleDeg`를 산출합니다.

### 3.5 모터 드라이버 상태 정보 획득 및 처리 CSU
- **관련 파일**: `csu_MotorDriver.c`
- **핵심 함수**:
  - `MotorDriver_UpdateStatus()`: 상태 레지스터 폴링.
  - `MotorDriver_ClearFaults()`: 에러 레지스터 리셋 명령 송신.
- **주요 변수**:
  - `xMotorDriver.faultStatus`: DRV8343 내부 에러 코드가 저장되는 변수.
- **동작 상세**:
  - SPI-B 드라이버를 통해 `DRV8343_REG_FAULT_STATUS_1` 레지스터를 주기적으로 읽어 `faultStatus`를 갱신합니다. 에러 발생 시 `MotorDriver_ClearFaults()`를 통해 `DRV8343_REG_CONTROL_1`의 비트 0(`CLR_FLT`)을 세트하여 결함을 해제합니다.

### 3.6 모터 구동제어 CSU
- **관련 파일**: `csu_MotorCtrl.c`, `csu_PID.c`
- **핵심 함수**:
  - `MotorCtrl_Run()`: Multi-Rate 제어기가 포함된 메인 모터 루프.
  - `PID_Calculate(PID_Controller_t* pid, float32_t setpoint, float32_t feedback)`: PID 및 안티와인드업 알고리즘 수행.
- **주요 변수**:
  - `currPid`, `speedPid`, `posPid` (`PID_Controller_t` 구조체 인스턴스): 적분기(`integral`), 에러(`prevError`) 등을 보존하는 제어기.
  - `xPidGain` (`stPidGain` 전역 구조체): Kp, Ki, Kd, Ks 등 튜닝 파라미터를 통합하여 실시간으로 반영하는 구조체.
  - 카운터 분주 변수: `loop1msCnt`, `loopPosCtrlDivider` (동적 데시메이션 달성용).
- **동작 상세**:
  - **Fail-Safe 연동**: `LimitSwitch_CheckFaults()` 결함이나 `xBit.faultFlagSet` 발생 시, 즉시 `MOTOR_MODE_FAULT_STOP` 모드로 강제 전환하여 출력을 0으로 차단하고 기계적 브레이크 잠금을 수행합니다.
  - **위치 루프(분주 제어)**: `posPid`가 `xMotorCtrl.targetPosition`과 `currentPosition`의 편차를 계산해 목표 속도(`speedCmd`)를 출력합니다. (`DECIMATION_POS_CTRL` 매크로 주기 활용)
  - **속도 루프(1ms)**: `speedPid`가 1ms(`DECIMATION_SPEED_CTRL`)마다 분주 연산된 RPM을 피드백받아 목표 전류(`currentCmd`)를 출력합니다. (`Ks` 파라미터를 활용한 PI-IP 혼합 제어 지원)
  - **전류 루프(100us)**: 매 인터럽트마다 `currPid`가 목표 전류와 ADC의 `isenMotLpf`를 비교하여 최종 부호가 포함된 PWM `duty`(-100.0f ~ 100.0f)를 출력합니다. (절댓값 변환 없이 4상한 제어 달성)
  - 각 제어기는 `PID_Calculate` 내부에서 출력 범위를 클램핑하고, 오버플로우 초과분 만큼 `integral`을 차감하는 Anti-Windup 처리가 엄격히 적용되어 있습니다.

### 3.7 리미트 스위치 감시 및 안전 로직 CSU
- **관련 파일**: `csu_LimitSwitch.c`, `csu_LimitSwitch.h`
- **핵심 함수**:
  - `LimitSwitch_Init()`: 리미트 스위치 매핑 및 목표값 구조체 초기화.
  - `LimitSwitch_CheckFaults()`: 매 주기 스위치의 하드웨어적, 논리적 결함을 판별.
- **주요 변수**:
  - `xLimitSwitchConfig`: 각 스위치의 목표 위치(Target) 및 오차 허용치(Tolerance) 매핑 정보 보관.
  - `xLimitSwitch`: 스위치의 에러 여부(`isFaultActive`) 및 고장 코드(`faultCode`) 보관.
- **동작 상세**:
  - 1차적으로 `xDio.limit1No`와 `xDio.limit1Nc` 값을 비교하여 단선이나 하드웨어적 결함을 검출합니다.
  - 2차적으로 현재 모터 위치(`xMotorCtrl.currentPosition`)가 설정된 범위(Tolerance) 안에 진입했을 때, 매핑된 스위치가 정상적으로 닫혔는지(논리적 정합성)를 교차 검증하여 에러 플래그를 생성합니다.

### 3.8 시스템 운용 CSU
- **관련 파일**: `csu_Control.c`
- **핵심 함수**:
  - `MainControl_Isr()`: 100us 최상위 인터럽트 함수.
  - `Control_SystemOperation()`: 메인 제어 파이프라인.
- **설계 제약사항 및 해결방안 (C28x 16-bit 아키텍처 지원)**:
  - TI C28x 계열 DSP는 1 Byte가 16-bit 공간을 차지하여 `#pragma pack`을 통한 1바이트 구조체 패킹이 물리적으로 지원되지 않음.
  - 이를 해결하기 위해 네트워크 송/수신 시 `memcpy`를 배제하고 바이트 단위 비트 시프트 연산을 수행하는 **직렬화(Serialization)/역직렬화(Deserialization)** 헬퍼 함수를 자체 구현.
  - W6100 하드웨어로 데이터를 넘길 때 오차 없는 `12바이트(Header) + Payload` 형태의 정확한 패킷 전송을 보장.
- **주요 변수**:
  - `adcTimeout`: 무한루프(Stuck) 방지를 위한 100U 타임아웃 락 변수.
  - `isrAliveCounter`: 인터럽트 생존 검증용 카운터 변수 (5000 카운트 도달 시 토글).
- **동작 상세**:
  - `MainControl_Isr`에 진입하면 `ADC_getInterruptStatus()`를 타임아웃 기반으로 기다려 신뢰성을 확보한 뒤, 모든 센싱 데이터를 일괄 취득(`ADC_readResult`)합니다.
  - 이후 `Control_SystemOperation()`을 호출하여 `CalcAdcData -> Dio_UpdateInput -> Encoder_UpdatePosition -> MotorDriver_UpdateStatus -> Bit_RunCBIT -> MotorCtrl_Run` 순서의 결정론적(Deterministic) 제어 파이프라인을 실행합니다.

### 3.9 데이터 저장 CSU
- **관련 파일**: `csu_Control.c`, `csu_Encoder.c`
- **핵심 함수**:
  - `Encoder_SetZero()`, `Encoder_LoadOffset()`: 엔코더 영점 기록 및 호출.
  - `Fram_WriteByte()`, `Fram_ReadByte()`: 물리 메모리 조작 래퍼 함수 (HAL 기반).
- **주요 변수**:
  - `ENC_OFFSET_FRAM_ADDR` (매크로 상수): 엔코더 오프셋이 저장될 기준 메모리 번지(보통 0x0010 등).
  - `b` (로컬 바이트 변수): 64비트 변수를 8바이트로 쪼개어 쓰는 버퍼.
- **동작 상세**:
  - `Encoder_SetZero` 호출 시, `xEncoder.rawPos` 값을 8바이트로 시프트(`>> (i * 8)`)하여 FRAM에 1바이트씩 쪼개어 직렬화 기록합니다. 초기 구동 시 `Encoder_LoadOffset`에서 이를 거꾸로 결합하여 64비트 변수에 복원합니다.

---

## 4. 외부 연동 CSC

### 4.1 화포통제컴퓨터 통신 CSU
- **관련 파일**: `csu_Ethernet.c`
- **핵심 함수**:
  - `Ethernet_StateMachine()`: 상태 천이 및 타임아웃, 재전송 제어.
  - `Ethernet_SendMessage()`: 헤더 및 페이로드 조립, 센드카운트 적용, 송신.
  - `Ethernet_ParsePacket()`: 수신 버퍼 언패킹, 체크섬 및 ACK 로직.
- **주요 변수**:
  - `xEthCtrl.State`: 상태 머신 노드 관리 (`STATE_BOOTING`, `STATE_WAIT_BOOT_ACK`, `STATE_JOINED`).
  - `xEthCtrl.WaitAckCode`, `xEthCtrl.RetryCount`, `xEthCtrl.WaitAckTimer`: ACK 대기 및 재전송(최대 4회) 파라미터.
  - `xEthCtrl.Power270VStatus`: 체계로부터 270V 인가 메시지 수신 시 갱신되는 전원 플래그.
  - `g_isW6100Connected`: 하드웨어 링크 상태를 대변하는 글로벌 변수.
- **동작 상세**:
  - 100ms 주기로 백그라운드 태스크에서 동작하며 `g_isW6100Connected == 0`일 경우 함수를 즉시 `return`시켜 미연결 시 무한 락업 현상을 방어합니다.
  - 상태가 `STATE_WAIT_BOOT_ACK`일 때 500ms 주기로 `ETH_CODE_BOOT_DONE`을 전송하며 `WaitAckCode`를 세팅합니다.
  - 수신 ISR에서 `ETH_CODE_ACK` 메시지를 파싱하여 `Code_Info`와 내가 기다리는 코드가 일치하면 `State`를 `STATE_JOINED`로 천이시킵니다.
  - 송수신되는 모든 패킷(Header+Data)에 대해 맨 뒷단 2바이트를 제외한 모든 바이트를 더하는 방식의 `Ethernet_CalculateChecksum()` 연산을 수행하여 통신 무결성을 상호 검증합니다. 통신 두절(Timeout Limit 50회 연속 미수신 초과) 발생 시 Socket을 닫고 재오픈(`socket()`)하여 초기 망 가입 모드(`STATE_WAIT_BOOT_ACK`)로 롤백합니다.
