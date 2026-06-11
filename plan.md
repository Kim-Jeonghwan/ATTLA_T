# 전역 변수 구조체(Struct) 관리 전환 마이그레이션 계획

## 1. 목표
현재 각 모듈별로 산재되어 있는 낱개 전역 변수들(`extern` 변수)을 기능 단위의 구조체(`struct`)로 묶어 관리하도록 변경합니다.
이는 변수 네임스페이스 충돌을 방지하고, 디버깅 가시성을 높이며, 향후 통신 연동 시 데이터 캡슐화를 용이하게 하기 위함입니다.

## 2. 사용자 피드백 요청 (User Review Required)
> [!IMPORTANT]
> **구조체 변수명 승인 요청**
> 각 모듈 상태 구조체 인스턴스 이름은 접두어 규칙을 준수하여 소문자 `x`로 시작하는 형태(예: `xEncoder`, `xMotorCtrl`, `xAdc`, `xBit`, `xSysCtrl`)를 제안합니다. 혹시 선호하시는 다른 네이밍 룰(예: `g_Encoder` 등)이 있다면 의견을 남겨주세요.
>
> **엔코더 구조체 필드 승인 요청**
> 엔코더 구조체(`stEncoderState`)에 예시로 주신 대로 원시 데이터 필드(`errBit`, `warnBit`, `crcRecv` 등)를 잘게 쪼개어 추가했습니다. 이 필드 구성이 적합한지 검토 부탁드립니다.

## 3. 상세 수정 계획 (Proposed Changes)

---

### [csu_Encoder]
기존의 낱개 변수들을 제거하고 통합된 상태 구조체 `stEncoderState` 및 인스턴스 `xEncoder`를 정의합니다.

#### [MODIFY] csu_Encoder.h
- 기존 `extern uint64_t encRawData;` 등 제거
- `stEncoderState` 구조체 타입 선언 및 `extern stEncoderState xEncoder;` 선언 추가

#### [MODIFY] csu_Encoder.c
- 전역 변수 `stEncoderState xEncoder = {0};` 정의 추가
- `Encoder_UpdatePosition()`, `Encoder_SetZero()`, `Encoder_LoadOffset()` 등에서 `xEncoder.xxx`로 변수 참조 변경 (예: `encPosition` -> `xEncoder.position`)
- 통신 프레임 파싱 시 `xEncoder.fullFrame`, `xEncoder.errBit`, `xEncoder.crcRecv` 등에 개별 값을 저장하도록 로직 보강

---

### [csu_MotorCtrl]
모터 제어와 관련된 속도/위치 목표 및 현재 상태를 구조체화합니다.

#### [MODIFY] csu_MotorCtrl.h
- `MotorControlMode_t currentMotorMode;` 등을 `stMotorCtrlState` 구조체로 묶음
- `extern stMotorCtrlState xMotorCtrl;` 추가

#### [MODIFY] csu_MotorCtrl.c
- 구조체 초기화 및 내부 변수 접근 구문 일괄 변경

---

### [csu_MotorDriver]
모터 드라이버 진단 및 상태 정보를 관리합니다.

#### [MODIFY] csu_MotorDriver.h
- `uint16_t motorDriverFaultStatus;` 를 `stMotorDriverState` 에 포함
- `extern stMotorDriverState xMotorDriver;` 추가

#### [MODIFY] csu_MotorDriver.c
- 구조체 초기화 및 내부 변수 참조 변경

---

### [csu_Adc]
ADC 센싱 결과 필터 및 오프셋 변수를 구조체화합니다.

#### [MODIFY] csu_Adc.h
- `Isen_Mot_lpf` 등 8개의 변수를 `stAdcState` 로 묶음 (CamelCase 권장: `isenMotLpf` 등)
- `extern stAdcState xAdc;` 추가

#### [MODIFY] csu_Adc.c
- 센서 필터링 연산 및 오프셋 계산 구문에서 구조체 참조로 변경

---

### [csu_Bit & csu_Control]
BIT(진단) 및 시스템 제어 상태를 관리합니다.

#### [MODIFY] csu_Bit.h / csu_Bit.c
- `Bit_Inform_all`, `BitFaultFlag_Set` 등을 `stBitState xBit;` 로 통합
- 구조체 변수 참조 변경

#### [MODIFY] csu_Control.h / csu_Control.c
- `isOffsetCalibrated`, `isPbitComplete` 등을 `stControlState xSysCtrl;` 로 통합

---

### [main.c 및 기타 호출부]
#### [MODIFY] main.c
- 메인 루프에서 기존 전역 변수를 사용하는 로직을 구조체 참조(`xEncoder.position` 등)로 모두 업데이트

## 4. 검증 계획 (Verification Plan)
- 변경 후 `CCS Theia` 에서 전체 빌드(Rebuild All)를 수행하여 변수명 참조 에러 및 타입 미스매치 에러가 발생하지 않는지 확인합니다.
- `plan.md`가 승인되면, 지시하신 즉시 리팩토링 구현을 시작하고 `task.md`를 통해 진행률을 추적하겠습니다.
