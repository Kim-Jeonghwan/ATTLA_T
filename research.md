# 모터 드라이버(DRV8343) SPI 통신 및 초기화 설정 리서치 보고서

본 문서에서는 ATTLA 프로젝트의 CSU 및 HAL 계층 코드를 분석하여, 모터 드라이버(DRV8343)와의 SPI 통신을 위한 초기화 설정과 통신 메시지(Read/Write 프레임)가 구현된 위치와 상세 내용을 정리합니다.

## 1. SPI 통신 초기화 설정 (Initialization Settings)

모터 드라이버(DRV8343)의 하드웨어적인 SPI 초기화 및 핀 설정은 **HAL 계층**에 구현되어 있으며, 이를 호출하여 상태를 초기화하는 로직은 **CSU 계층**에 구현되어 있습니다.

### [HAL 계층] 하드웨어 초기화 (SPI-B)
* **파일 위치**: `ATTLA_T_CPU1/HAL/hal_MotorDriver.c`
* **함수명**: `MotorDriver_Init_Hardware(void)`
* **주요 설정 내용**:
  * **SPI 모듈**: **SPI-B** (`SPIB_BASE`) 사용
  * **통신 핀 할당**: 
    * GPIO 58: SPIB_CLK
    * GPIO 59: SPIB_STE (Chip Select)
    * GPIO 60: SPIB_SIMO (MOSI)
    * GPIO 61: SPIB_SOMI (MISO)
  * **SPI 통신 규격 (`SPI_setConfig`)**:
    * 1MHz 클럭 속도 (`1000000`)
    * 16비트 워드 크기 (`16`)
    * Master 모드 (`SPI_MODE_MASTER`)
    * Mode 1 통신 (`SPI_PROT_POL0PHA1` - Data on Falling Edge, Setup on Rising Edge)
  * **추가 하드웨어 제어**:
    * `DRV8343_REG_CONTROL_2` 레지스터를 조작하여 1x PWM 모드로 설정 (`DRV8343_CTRL2_PWM_MODE_1X`).
    * DRV_ENABLE (GPIO 2) 핀 설정.

### [CSU 계층] 어플리케이션 초기화 호출
* **파일 위치**: `ATTLA_T_CPU1/CSU/csu_MotorDriver.c`
* **함수명**: `MotorDriver_Init(void)`
* **주요 역할**: 
  * `MotorDriver_Enable(false)`로 핀 상태 초기화
  * `MotorDriver_Init_Hardware()` (HAL 함수) 호출을 통한 SPI 셋업
  * `MotorDriver_ClearFaults()` 호출로 에러 상태 해제

---

## 2. 통신 메시지 구조 (Communication Messages)

DRV8343 레지스터를 읽고 쓰기 위한 SPI 메시지 통신 프로토콜 구성(16비트 프레임 포맷)은 **HAL 계층**에서 래핑 함수로 제공하며, 실제 메시지를 활용하는 로직은 **CSU 계층**에서 수행합니다.

### [HAL 계층] 레지스터 Read/Write 통신 프로토콜 
* **파일 위치**: `ATTLA_T_CPU1/HAL/hal_MotorDriver.c`
* **Read 프레임 구성 (`MotorDriver_ReadReg`)**:
  * **포맷**: `Bit 15 = 1 (Read)`, `Bits 14:11 = 주소(Addr)`, `Bits 10:0 = Don't care`
  * **코드 구현**: `uint16_t txWord = (1U << 15) | ((addr & 0x0F) << 11);`
  * 수신된 16비트 데이터 중 하위 11비트(`rxWord & 0x07FF`)만 유효 데이터로 추출하여 반환합니다.
* **Write 프레임 구성 (`MotorDriver_WriteReg`)**:
  * **포맷**: `Bit 15 = 0 (Write)`, `Bits 14:11 = 주소(Addr)`, `Bits 10:0 = 데이터(Data)`
  * **코드 구현**: `uint16_t txWord = ((addr & 0x0F) << 11) | (data & 0x07FF);`

### [CSU 계층] 메시지 사용 예시
* **파일 위치**: `ATTLA_T_CPU1/CSU/csu_MotorDriver.c`
* **결함 해제 메시지 (`MotorDriver_ClearFaults`)**:
  * `DRV8343_REG_CONTROL_1` 레지스터를 읽고, Bit 0(CLR_FLT)을 1로 세트하여 다시 쓰는 메시지를 전송합니다.
* **상태 업데이트 메시지 (`MotorDriver_UpdateStatus`)**:
  * `DRV8343_REG_FAULT_STATUS_1` 레지스터 읽기 메시지를 전송하여 반환된 값으로 구조체(`xMotorDriver.faultStatus`)를 갱신합니다.
