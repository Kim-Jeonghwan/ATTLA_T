# 📋 SPI-D 기반 FRAM (CY15B256Q) 연동 구현 계획서

**작성 일자**: 2026. 06. 08.  
**대상 코어**: CPU1 Core (`ATTLA_T_CPU1`)

---

## 1. 개요 및 목표 (Goal Description)
사용자의 제공 코드(`NC_SpiFRAM.c/h`)를 바탕으로 TI DSP(F28388D)의 **SPI-D** 모듈을 이용하여 외부 FRAM과 통신하는 기능을 구현합니다. 
구현 시 기존 프로젝트의 아키텍처 규칙(`CSU`, `HAL` 분리) 및 **TI C2000 Driverlib API 사용 표준**을 엄격하게 준수하며, 메인 루프에 미구현 처리된 테스트 코드를 삽입합니다.

---

## 2. 사용자 확인 요망 (User Review Required) & Open Questions

> [!IMPORTANT]
> 본격적인 코드 구현에 앞서 아래 사항에 대해 확인 부탁드립니다.

1. **SPI 통신 속도 (Baudrate)**
   - 참고 코드에서는 SPI 클럭을 `1MHz`로 설정했습니다. 사용하시는 FRAM(CY15B256Q)은 보통 더 높은 속도(예: 10MHz 이상)도 지원하는데, **1MHz**로 그대로 적용해도 괜찮을까요?
2. **블로킹 딜레이 유지 여부 (Blocking Delay)**
   - 참고 코드의 `FRAM_PageWrite` 함수를 보면 데이터 전송 후 `DELAY_US(10000u);` (10ms) 블로킹 코드가 있습니다. 메인 백그라운드 루프 관점에서는 10ms 동안 다른 작업이 지연될 수 있는데, 이 딜레이를 그대로 유지할까요?
3. **SPI Transmit 함수 구조**
   - 참고 코드는 레지스터 직접 접근(`SpidRegs.SPITXBUF`) 및 Timeout 방식으로 구현되어 있습니다. 프로젝트 규칙(Driverlib 우선)에 따라 `SPI_writeDataBlockingNonFIFO()` / `SPI_readDataBlockingNonFIFO()`를 이용하는 폴링 방식으로 리팩토링하여 적용하겠습니다.

---

## 3. 리팩토링 상세 수행 계획 (Proposed Changes)

### 3.1 HAL (Hardware Abstraction Layer)
#### [MODIFY] `hal_Spi.h`
- SPI-D 핀맵 매크로 추가 (SIMO: 91, SOMI: 92, CLK: 93)
- FRAM CS 핀 제어용 매크로 추가 (CS: 94)
- `hal_Spid_CsLow()`, `hal_Spid_CsHigh()` 함수 또는 매크로 선언

#### [MODIFY] `hal_Spi.c`
- `static void InitSpid(void)` 함수 구현 및 `Initial_SPI()`에 추가.
- GPIO 91~94 핀 설정 (Driverlib `GPIO_setPinConfig` 및 `GPIO_setPadConfig` 적용).
- SPI-D 모듈 초기화 (Master, Mode 3, 8-bit 통신).
- 1바이트 데이터 송수신을 위한 래퍼 함수(`hal_Spid_Transmit`) 추가 구현.

---

### 3.2 CSU (Control & Service Unit)
#### [NEW] `csu_Fram.h`
- FRAM 명령어(Opcode) 정의 (WREN, WRDI, RDSR, WRSR, READ, WRITE).
- 상태 레지스터 관련 매크로 추가.
- 상위 레벨 API 선언: `csu_Fram_Init()`, `csu_Fram_ReadByte()`, `csu_Fram_WriteByte()`, `csu_Fram_PageWrite()`.

#### [NEW] `csu_Fram.c`
- 상위 애플리케이션 계층의 논리를 담당.
- `csu_Fram_Init()` 내부에 초기화 및 상태 레지스터 설정 로직 구현.
- `csu_Fram_ReadByte()`, `csu_Fram_WriteByte()`, `csu_Fram_PageWrite()` 등 핵심 읽기/쓰기 로직을 HAL의 `hal_Spid_Transmit` 함수를 호출하여 구현. (참고 코드의 지연시간 및 흐름 유지)

---

### 3.3 Main Application
#### [MODIFY] `main.h`
- `#include "csu_Fram.h"` 구문 추가.

#### [MODIFY] `main.c`
- `DSP_Initialization()` 호출 후 `csu_Fram_Init()` 초기화 루틴 추가.
- `cycle_1000ms()` (1초 주기 루프) 내부에 참고 코드와 동일하게 `#if 0` 로 감싼 **미구현 상태의 FRAM 테스트 코드** 삽입.

---

## 4. 검증 계획 (Verification Plan)
- **정적 코드 리뷰**: `csu_` 와 `hal_` 명명 규칙 및 Driverlib API 적용 확인.
- **컴파일 점검**: 사용자가 직접 CCS에서 빌드하여 오류가 없는지 검증합니다.
- **테스트 코드**: 향후 사용자가 `main.c`의 `#if 0` 블록을 활성화하여 실제 하드웨어에서 FRAM 읽기/쓰기 동작을 확인할 수 있도록 합니다.

---

위 계획서(오픈 퀘스천 포함)를 검토해 보시고, 수정이 필요하거나 진행해도 좋다면 **승인(진행해)** 부탁드립니다!
