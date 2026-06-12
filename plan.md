# ATTLA_T 시스템 이산신호(DIO) 디바운싱 및 FRAM 저장 로직 구현 계획

본 계획서는 요구사항에 따라 1) 불필요한 GPIO 1 초기화 코드를 삭제하고, 2) 1ms 디바운싱 필터가 적용된 이산신호(DIO) 수집 모듈을 신규 구현하며, 3) FRAM을 통한 중요 데이터(오프셋 등) 저장 래퍼 모듈을 구현하는 방안을 설명합니다.

## User Review Required

> [!IMPORTANT]
> **구조 설계 승인 요청**
> 1. 이산신호 수집을 전담하는 모듈(`csu_Dio.c`, `csu_Dio.h`)을 신규 생성하여 독립적으로 관리하고자 합니다.
> 2. `csu_Bit.c`에 있던 기존 `GPIO_readPin(40U)`(24V 감시) 및 `GPIO_readPin(10U)`(DRV nFAULT) 직접 접근 로직을, 새롭게 구현될 디바운싱된 구조체 값(`xDio.pm24v`, `xDio.drvFault`)을 참조하도록 수정합니다.
> 3. 오프셋 값을 FRAM에 저장하기 위해 `csu_Control.c` 내에 `Control_SaveDataToFram()` 함수를 구현하고, 초기 오프셋 산출 완료 시(`csu_Offset_Isr`) 1회 저장하도록 구성합니다.

해당 아키텍처 및 변경 사항에 대해 승인해 주시면 즉시 구현을 진행하겠습니다.

## Proposed Changes

---

### 하드웨어 초기화 (HAL)

#### [MODIFY] [hal_DspInit.c](file:///d:/Nexcom/Firmware/01_Project/04_ATTLA/ATTLA_T/ATTLA_T/ATTLA_T_CPU1/HAL/hal_DspInit.c)
* `Init_GpioDin(void)` 함수 내부의 GPIO 1 입력 설정(GND 체크용) 관련 코드 3줄 삭제.
```c
    // GPIO 1: 입력 설정 (GND 체크용)
    // GPIO_setPinConfig(GPIO_1_GPIO1);
    // GPIO_setPadConfig(1u, GPIO_PIN_TYPE_PULLUP);
    // GPIO_setDirectionMode(1u, GPIO_DIR_MODE_IN);
```

---

### 이산신호 입력 처리 (CSU)

#### [NEW] `ATTLA_T_CPU1/CSU/csu_Dio.h`
* 이산신호 상태를 담는 구조체 `stDioState xDio` 선언.
* 핀 매크로 및 디바운싱 기준 카운트(`DIO_CNT_REF_1MS = 10U`) 매크로 정의.
* `Dio_Init()`, `Dio_UpdateInput()` 프로토타입 선언.

#### [NEW] `ATTLA_T_CPU1/CSU/csu_Dio.c`
* 100us 주기로 호출되는 `Dio_UpdateInput()` 구현.
* TDU 프로젝트의 디바운싱 방식을 적용하여, 각 입력 핀(GPIO 36, 37, 38, 39, 40, 46, 10)에 대해 10카운트(1ms) 동안 상태가 유지되었을 때만 `xDio` 구조체 변수를 갱신.

---

### 상태 진단 (CSU)

#### [MODIFY] [csu_Bit.c](file:///d:/Nexcom/Firmware/01_Project/04_ATTLA/ATTLA_T/ATTLA_T/ATTLA_T_CPU1/CSU/csu_Bit.c)
* `Bit_OvVoltage_Check()` 내부의 24V 브레이크 전압 감시를 `if (GPIO_readPin(40U) == 0U)` 에서 `if (xDio.pm24V == 0U)` 로 변경.
* `Bit_GateFault_Check()` 내부의 드라이버 폴트 감시를 `if (GPIO_readPin(10U) == 0U)` 에서 `if (xDio.drvFault == 0U)` 로 변경.
* (노이즈에 강인한 디바운싱 상태값을 기반으로 에러를 판별하도록 구조 개선)

---

### 메인 제어 및 FRAM (CSU)

#### [MODIFY] [csu_Control.c](file:///d:/Nexcom/Firmware/01_Project/04_ATTLA/ATTLA_T/ATTLA_T/ATTLA_T_CPU1/CSU/csu_Control.c)
* `Control_SystemOperation()` 함수 내 주석 처리된 `// updateDioInput();` 대신 `Dio_UpdateInput();` 호출 삽입.
* `// saveData();` 주석 위치에 FRAM 데이터 저장 함수 호출 또는 스케줄러 구현.
* FRAM 저장 래퍼 함수인 `Control_SaveDataToFram()` 신규 작성.
* `csu_Offset_Isr()`에서 오프셋 측정이 완료되어 `xSysCtrl.isOffsetCalibrated = 1U;`가 되는 시점에 `Control_SaveOffsetToFram()`을 호출하여 전류 오프셋을 FRAM의 특정 주소(예: 0x0000)에 기록.

#### [MODIFY] [csu_Control.h](file:///d:/Nexcom/Firmware/01_Project/04_ATTLA/ATTLA_T/ATTLA_T/ATTLA_T_CPU1/CSU/csu_Control.h)
* FRAM 저장 관련 함수(`Control_SaveOffsetToFram()`, `Control_SaveDataToFram()`) 프로토타입 선언.

---

## Verification Plan

### Manual Verification
1. **빌드 검증**: 사용자가 CCS에서 빌드하여 오류 없이 컴파일되는지 확인합니다.
2. **디버깅 모니터링**: 
   - easyDSP나 CCS 디버거를 통해 `xDio` 구조체에 리미트 스위치 등의 값이 1ms 지연 후 정확하게 갱신되는지 확인합니다.
   - GPIO 1이 더 이상 설정되지 않음을 레지스터 뷰(Registers View)에서 검증합니다.
   - 1초 경과 시 측정된 오프셋 값이 지정된 FRAM 주소에서 `Fram_ReadByte()`를 통해 다시 정상적으로 읽혀오는지 확인합니다.
