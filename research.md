# ATTLA_T 시스템 이산신호 입력 및 FRAM 데이터 저장 로직 분석 및 리서치 보고서

## 1. 이산신호(DIO) 입력 처리 로직 분석

### 1.1 하드웨어 핀맵 및 요구사항
`Architecture.md`의 4.3 디지털 입출력 명세에 따라, 현재 제어기가 읽어들여야 하는 주요 이산 입력(GPIODIN) 핀은 다음과 같습니다.
* **리미트 스위치 1**: `nLIMIT1_NO` (GPIO 36), `nLIMIT1_NC` (GPIO 37) - Active Low
* **리미트 스위치 2**: `nLIMIT2_NO` (GPIO 38), `nLIMIT2_NC` (GPIO 39) - Active Low
* **시스템 감시**: `PM_n24V` (GPIO 40), `CABLE_LOOP` (GPIO 46), `DRV_nFAULT` (GPIO 10)

### 1.2 기존 및 타 프로젝트(TDU) 구현 분석
* **TDU 프로젝트**: `hal_Gpio_Drive.c`의 `Tdu_Din()` 함수에서 핀 입력을 폴링 방식으로 처리하고 있습니다. 이 때, 10kHz(100us) 제어 주기 기준으로 `DIN_CNT_REF_1MS` (10카운트 = 1ms) 동안 동일한 상태가 유지될 때만 유효한 신호로 판단하는 **디바운싱(Debouncing) 소프트웨어 필터**가 적용되어 노이즈를 방지하고 있습니다.
* **현재 ATTLA 프로젝트**: `csu_Bit.c`에서 `GPIO_readPin(40U)`, `GPIO_readPin(10U)`와 같이 TI Driverlib API를 사용해 핀 상태를 직접 읽어 결함 판정을 수행하고 있습니다. 또한 `csu_Control.c`의 `Control_SystemOperation()` 파이프라인 내부에 `updateDioInput();` 주석이 존재하여 100us 주기로 호출되도록 설계된 상태입니다.

### 1.3 구현 제안 방향 (CSU 계층)
* **모듈 분리**: 이산신호 관리를 전담하는 모듈을 생성하거나 `csu_Control.c`에 통합합니다. (예: 구조체 `stDioState xDio;` 및 함수 `Dio_UpdateInput()`)
* **디바운싱 로직 적용**: 리미트 스위치나 체결 감지 등 기계적 접점 신호는 바운싱(Bouncing)이 발생하므로, TDU의 사례를 벤치마킹하여 최소 1ms ~ 10ms 수준의 카운터 기반 필터를 적용해야 안정적입니다.
* **Driverlib 준수**: C2000Ware 표준인 `GPIO_readPin()` API를 사용하여 핀의 상태를 읽습니다.

---

## 2. FRAM 데이터 저장 기능(CSU) 분석

### 2.1 FRAM 하드웨어 및 HAL 인터페이스
* **스펙**: CY15B256Q-SXE (256Kbit SPI FRAM)이며, SPI-D (1MHz)를 통해 통신합니다. Instant Write 기능이 있어 무지연 엑세스가 가능합니다.
* **HAL 계층 확인**: `hal_Fram.c` 파일에 이미 `Fram_Init()`, `Fram_ReadByte()`, `Fram_WriteByte()`, `Fram_PageWrite()` 등의 저수준 SPI 엑세스 API가 완벽히 구현되어 있습니다.

### 2.2 구현 제안 방향 (CSU 계층)
* **데이터 관리 구조체**: 저장할 데이터 맵(어드레스 맵)을 관리하는 헤더와 상태 변수를 정의합니다.
* **`saveData()` 구현**: 100us 메인 인터럽트 주기마다 호출되므로, 매 주기 FRAM에 쓰는 것은 통신 병목을 유발할 수 있습니다. 상태가 변경되거나 일정 주기(예: 1초 혹은 이벤트 발생 시)마다 저장하도록 하는 스케줄러 로직이 필요합니다.
* **미리 구현 가능한 부분**: 
  1. `csu_Control.c` 의 `csu_Offset_Isr` 단계에서 1초간 측정한 초기 영점 조정 값(`isenMotOffset`, `isenBrkOffset`)을 FRAM 특정 주소에 자동 백업하는 기능.
  2. 에러가 발생하여 `xBit.faultFlagSet`이 활성화되었을 때, 고장 이력 로깅을 위해 결함 코드를 FRAM에 즉시 기록하는 기능.

---

## 3. 결론 및 향후 계획

사용자 지시에 따라 실제 코드 구현을 위한 사전 리서치를 마쳤습니다. 
1. **DIO 처리**: `GPIO_readPin()` 기반의 디바운싱 로직이 포함된 `Dio_UpdateInput()`을 구현하여 `stDioState` 구조체에 상태를 최신화하도록 개발할 수 있습니다.
2. **FRAM 제어**: `Fram_WriteByte()`를 활용하여, 오프셋 초기화 완료 시점이나 결함 발생 시 데이터를 로깅하는 구조적 기반(`saveData()`)을 `csu_Control.c`에 마련해 둘 수 있습니다.

사용자께서 승인해 주시거나 명시적으로 계획/구현 지시를 내려 주시면 즉시 `plan.md` 작성 후 구현을 시작하겠습니다.
