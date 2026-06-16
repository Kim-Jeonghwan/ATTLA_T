# ATTLA_T 체계 연동 통제안 및 소프트웨어 아키텍처 반영 계획

본 문서는 명세서 상 '미구현'으로 분류된 이더넷 체계 연동 규격(UDP 프로토콜 및 상태 머신)과 100us 핵심 제어 루프의 CSU 실행 흐름을 코드에 반영하기 위한 상세 구현 계획입니다.

## 사용자 확인 필요
> **100us 핵심 제어 루프 (EPWM1 ISR) 변경 사항**
> 기존 `csu_MainControl_Isr()` 내에 포함되어 있던 주기점검(`Bit_RunCBIT`) 및 FRAM 저장 로직을 ISR에서 분리하여 백그라운드(`main.c`)로 이동시킵니다.
> 인터럽트 내에서의 무한 블로킹을 방지하기 위해 `Encoder_UpdatePosition()`과 `updateMotorDriverStatus()`는 비동기 처리 또는 타임아웃을 적용하여 실행 순서 3번, 4번에 배치합니다.

## 상세 변경 계획 (Proposed Changes)

### 1. 100us 핵심 제어 루프(ISR) 실행 순서 재배치 (`csu_Control.c`, `main.c`)
- **[MODIFY] `csu_Control.c`**:
  `csu_MainControl_Isr()` 함수의 내부 로직을 가이드에 명시된 6단계로 엄격히 재배치합니다.
  1. 아날로그 신호 입력 및 연산 CSU: `CalcAdcData();`
  2. 이산신호 입력 CSU: `Dio_UpdateInput();`
  3. 위치(각도) 정보 획득 및 처리 CSU: 주석 처리된 `Encoder_UpdatePosition();` 활성화.
  4. 모터 드라이버 상태 정보 획득 및 처리 CSU: DRV8343 폴트 확인 함수 호출 배치.
  5. 모터 구동 제어 CSU: `MotorCtrl_Run();`
  6. 시스템 운용 CSU: 전체 로직 판단을 위한 `Control_SystemOperation()` 배치.
  ※ 기존 포함되어 있던 `Bit_RunCBIT()` 및 `Control_SaveDataToFram()`은 제거.
- **[MODIFY] `main.c`**:
  제거된 `Bit_RunCBIT()`, `Control_SaveDataToFram()`, 그리고 `csu_Ethernet_StateMachine()`을 메인 백그라운드 유휴 루프(예: `cycle_100ms` 등)에서 비동기적으로 실행하도록 이관.

### 2. 브레이크 핀 제어 로직 보완 (`csu_MotorCtrl.c` 등)
- 브레이크 핀(GPIO 35)이 **Active High(High 출력 시 잠금 해제)**임을 코드에 명확히 반영합니다. `hal_DspInit.c`의 초기화(Low로 잠금 유지)는 정상이므로, 모터 제어 시작(모드 변경) 시 브레이크를 `High`로 출력하여 해제하는 로직을 모터 구동 모드 제어부(`csu_MotorCtrl.c` 또는 시스템 운용 제어부)에 추가합니다.

### 3. 체계 연동 통신망 상태 머신 및 전원 시퀀스 구현 (`csu_Ethernet.c/h`)
- **[MODIFY] `csu_Ethernet.c` / `csu_Ethernet.h`**:
  - 망 가입: 부팅 완료 시 `Boot Done` (500ms 주기) 전송 및 ACK 수신 대기 로직 완성.
  - Heartbeat: 망 가입 완료 시 100ms 주기로 상태정보(`ETH_CODE_HEARTBEAT`) 요청/응답 개시 및 무한 반복 통신 추가.
  - 통신 두절 예외 처리: 100ms 메시지가 연속 50회 미응답 시 소켓 리셋 후 망 가입 단계로 롤백하는 로직 검증.
  - 전원 시퀀스 제어: 270VDC 구동 전원 인가 메시지 수신 파싱 및 ACK 처리 후 상태 갱신.
  - IBIT / CBIT 연동: IBIT 요청 수신 시 CBIT를 중단하고, N초 간 IBIT 수행 후 IBIT Done 결과를 전송한 뒤 CBIT를 재개하는 시퀀스 로직 구현.

## 검증 계획 (Verification Plan)
1. **정적 분석 및 빌드**: TI Clang 컴파일러 에러 유무 확인, DAPA 무기체계 소프트웨어 코딩규칙 준수 검증.
2. **ISR 사이클 검증**: 100us 인터럽트 내부에서 지연(Blocking) 발생 여부와 순차적 실행 흐름 확인.
3. **UDP 상태 머신 로직 검증**: 망 가입(Boot Done) -> Heartbeat 지속 -> 타임아웃 발생 -> 망 가입 롤백의 State Transition Flow 정합성 검토.
