# CSU / HAL 계층 함수명 및 변수명 명명 규칙 조사 보고서

## 1. 개요
사용자의 지시에 따라 CSU, HAL, main 계층의 모든 소스 코드(.c) 및 헤더 파일(.h)을 대상으로, 함수명 및 변수명에 csu_ 또는 hal_ 접두어가 사용되었는지 전수 조사를 진행하였습니다.

**규칙 기준:** 모듈 및 파일명에만 소문자 csu_, hal_ 접두어를 사용해야 하며, 함수나 변수명에는 해당 접두어 사용을 엄격히 금지합니다. (단, 헤더 가드 등의 매크로는 대문자 HAL_, CSU_ 사용 허용)

## 2. 변수명 위반 사항 조사 결과
- 전체 코드베이스 검색 결과, csu_ 또는 hal_ 접두어가 포함된 **변수명은 발견되지 않았습니다.**

## 3. 함수명 위반 사항 조사 결과 (총 7건)
아래의 7개 함수들이 명명 규칙을 위반하여 소문자 접두어를 포함하고 있으므로, 접두어를 제거하는 수정이 필요합니다.

### [CSU 계층 위반 함수 - 4건]
1. csu_Bit_RunPBIT()
   - 위치: csu_Bit.c, csu_Bit.h, csu_Control.c
2. csu_Bit_RunCBIT()
   - 위치: csu_Bit.c, csu_Bit.h, csu_Control.c
3. csu_Control_CalibrateCurrentOffset()
   - 위치: csu_Control.c, csu_Control.h
4. csu_Control_SystemOperation()
   - 위치: csu_Control.c, csu_Control.h, hal_Adc.c

### [HAL 계층 위반 함수 - 3건]
1. hal_Led_InitGpio()
   - 위치: hal_Led.c, hal_Led.h, hal_DspInit.c
2. hal_Led_WritePin()
   - 위치: hal_Led.c, hal_Led.h, csu_Led.c
3. hal_Led_TogglePin()
   - 위치: hal_Led.c, hal_Led.h, csu_Led.c

## 4. 매크로 위반 여부 (위반 없음)
- HAL_MOTORDRIVER_SPI_CLK_PIN 등 대문자로 시작하는 매크로나, CSU_ADC_H 등의 헤더 가드는 명명 규칙(대문자 허용)에 부합하여 위반 사항이 아님을 확인하였습니다.

## 5. 개선 및 리팩토링 방안
위 7개의 함수를 선언, 정의, 호출하는 모든 소스 코드 영역에서 접두어를 제거하여 다음과 같이 수정합니다.

- csu_Bit_RunPBIT -> Bit_RunPBIT
- csu_Bit_RunCBIT -> Bit_RunCBIT
- csu_Control_CalibrateCurrentOffset -> Control_CalibrateCurrentOffset
- csu_Control_SystemOperation -> Control_SystemOperation
- hal_Led_InitGpio -> Led_InitGpio
- hal_Led_WritePin -> Led_WritePin
- hal_Led_TogglePin -> Led_TogglePin

> 사용자 승인 시, 이 내용을 바탕으로 plan.md를 작성하고 즉시 전체 코드 수정을 진행하겠습니다.

---

# 전역 변수 구조체(Struct) 관리 전환 조사 보고서

## 1. 개요
사용자의 지시에 따라 기존에 낱개로 흩어져 관리되던 전역 변수들(`extern` 선언)을 기능 단위의 구조체(`struct`)로 묶어 관리할 수 있도록 프로젝트 전체(CSU 계층 중심)를 조사하였습니다. 사용자가 제시한 엔코더(`Encoder`) 예시 코드를 참고하여, 각 모듈별로 산재해 있는 변수들을 파악하고 구조체화(struct-ify)하는 방안을 설계했습니다.

## 2. 모듈별 전역 변수 조사 및 구조체 변환 설계안

### 1) 엔코더 (Encoder) 모듈 `csu_Encoder.h / .c`
**기존 변수:** `encRawData`, `encOffset`, `encPosition`, `encAngleDeg`, `isEncError` 등 낱개 선언.
**구조체 적용 설계:** 사용자의 예시를 바탕으로 원시 데이터 파싱 결과와 어플리케이션 상태를 분리/통합하여 관리.
```c
typedef struct {
    // 1. 수신 원시 데이터 관련
    uint64_t fullFrame;   // SPI에서 수신된 64비트 전체 데이터 보관
    uint64_t rawPos;      // 파싱된 34비트 Position 원시값
    uint8_t  errBit;      // 파싱된 Error 비트 (Active Low)
    uint8_t  warnBit;     // 파싱된 Warning 비트
    uint8_t  crcRecv;     // 파싱된 수신 CRC (6비트)
    uint8_t  crcCalc;     // 자체 계산한 CRC (6비트)

    // 2. 어플리케이션 상태
    uint64_t offset;      // 제로셋 오프셋 값
    uint64_t position;    // 오프셋 적용 및 롤오버가 반영된 실시간 위치
    float32_t angleDeg;   // 360도 환산 기계각 (소수점 유지)
    bool      isValid;    // Error 비트 및 CRC 검증 결과에 따른 최종 유효성 (기존 isEncError 대체)
} stEncoderState;

extern stEncoderState xEncoder;
```

### 2) 모터 제어 (MotorCtrl) 모듈 `csu_MotorCtrl.h / .c`
**기존 변수:** `currentMotorMode`, `targetSpeedRpm`, `targetPosition`, `currentSpeedRpm`, `currentPosition`
**구조체 적용 설계:**
```c
typedef struct {
    MotorControlMode_t mode;
    float32_t targetSpeedRpm;
    float32_t targetPosition;
    float32_t currentSpeedRpm;
    float32_t currentPosition;
} stMotorCtrlState;

extern stMotorCtrlState xMotorCtrl;
```

### 3) 모터 드라이버 (MotorDriver) 모듈 `csu_MotorDriver.h / .c`
**기존 변수:** `motorDriverFaultStatus`
**구조체 적용 설계:** 단일 변수지만 확장성을 고려하여 구조체화.
```c
typedef struct {
    uint16_t faultStatus;
} stMotorDriverState;

extern stMotorDriverState xMotorDriver;
```

### 4) 아날로그 센싱 (ADC) 모듈 `csu_Adc.h / .c`
**기존 변수:** `Isen_Mot_lpf`, `Isen_Brk_lpf`, `Vsen_28V_lpf`, `Vsen_5VD_lpf`, `Vsen_Ref_lpf`, `Tsen_Bd_lpf`, `Isen_Mot_Offset`, `Isen_Brk_Offset`
**구조체 적용 설계:** 센싱 데이터와 오프셋을 한데 묶어 가독성 및 캐시 효율 상승. (CamelCase 명명법으로 통일 제안)
```c
typedef struct {
    float32_t isenMotLpf;
    float32_t isenBrkLpf;
    float32_t vsen28VLpf;
    float32_t vsen5VDLpf;
    float32_t vsenRefLpf;
    float32_t tsenBdLpf;
    float32_t isenMotOffset;
    float32_t isenBrkOffset;
} stAdcState;

extern stAdcState xAdc;
```

### 5) 시스템 BIT(Built-In Test) 진단 상태 `csu_Bit.h / .c`
**기존 변수:** `Bit_Inform_all`, `BitStartFlag_Set`, `BitFaultFlag_Set`, `BitFault_OvCurr_Mot` 등 8개 이상.
**구조체 적용 설계:**
```c
typedef struct {
    Uint32 informAll;
    Uint16 startFlagSet;
    Uint16 faultFlagSet;
    Uint16 faultOvCurrMot;
    Uint16 faultOvCurrBrk;
    Uint16 faultOvTempBd;
    Uint16 faultOvVolt28V;
    Uint16 faultDrv8343nFault;
} stBitState;

extern stBitState xBit;
```

### 6) 시스템 상태 제어 (Control) 모듈 `csu_Control.h / .c`
**기존 변수:** `isOffsetCalibrated`, `isPbitComplete` (volatile)
**구조체 적용 설계:**
```c
typedef struct {
    uint16_t isOffsetCalibrated;
    uint16_t isPbitComplete;
} stControlState;

extern volatile stControlState xSysCtrl;
```

## 3. 구조체 변환에 따른 장점
1. **네임스페이스 및 충돌 방지**: `xEncoder.position`, `xAdc.isenMotLpf` 와 같이 변수명 앞에 모듈 객체명이 붙으므로 변수 이름 충돌을 원천 차단합니다.
2. **디버깅 가시성 극대화**: CCS 디버거에서 구조체 객체(`xEncoder`) 하나만 Watch 윈도우에 등록하면 내부의 모든 상태(Raw, Error, Position 등)를 계층적으로 한눈에 모니터링할 수 있습니다.
3. **통신 연동 용이성**: PC나 상위 제어기로 데이터를 쏠 때 `memcpy` 등을 활용해 구조체 통째로 쉽게 버퍼에 복사할 수 있어 IPC나 SCI 통신 관리가 편리해집니다.

> 사용자 승인 시, 이 내용을 바탕으로 `plan.md`를 업데이트(또는 새로 작성)하여 전체 코드의 구조체 마이그레이션(리팩토링)을 진행하겠습니다.
