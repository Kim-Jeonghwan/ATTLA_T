# ATTLA_T 하드웨어 변경 사항 반영 조사 보고서 (Research)

## 1. 이더넷 PHY IC 및 부품 변경 사양
### 1.1. CM 코어 이더넷 PHY IC
- **적용 부품**: DP83822HFRHBT
- **영향 범위**:
  - `Architecture.md` 문서 내 CM 코어 이더넷 제어부 명세(4.1절)에 해당 부품명 기입 필요.

### 1.2. 트랜스포머 및 클럭 발진기 (PHY IC)
- **트랜스포머**: HX1188NL (외부 커넥터 간 연결)
- **입력 클럭(XI)**: 25 MHz (발진기: SIT2024BM-S2-33E-25.000000)
- **영향 범위**:
  - `Architecture.md` 1.1 주요 동작 주파수에 기입 필요.

### 1.3. W6100 (디버그용 이더넷) 부품
- **트랜스포머**: HX1188NL
- **크리스탈**: ECS-250-8-37Q-RES-TR (25MHz)
- **영향 범위**:
  - `Architecture.md` W6100 사양(4.1절)에 부품 정보 추가 반영 필요.

## 2. GPIO 핀맵 할당 변경 (1차 그림 반영)
다음 핀들은 실제 코드가 위치한 `hal_DspInit.c` 및 `hal_Spi.c` 에 변경 반영이 필요합니다.

| 신호명 | 기존 GPIO | 변경 GPIO | 비고 |
| :--- | :--- | :--- | :--- |
| DSP_BRAKE | 35 | 74 | `hal_DspInit.c` 내 GPIO 초기화 핀 번호 변경 |
| DSP_nLIMIT1_NO | 36 | 75 (※임시 73) | TX_D0(75) 중복 문제로 평가보드 테스트 시에는 73으로 사용하고 주석 표기 요망 |
| DSP_nLIMIT1_NC | 37 | 76 | `hal_DspInit.c` GPIO 초기화 변경 |
| DSP_nLIMIT2_NO | 38 | 77 | `hal_DspInit.c` GPIO 초기화 변경 |
| DSP_nLIMIT2_NC | 39 | 78 | `hal_DspInit.c` GPIO 초기화 변경 |
| DSP_PM_n24V | 40 | 79 | `hal_DspInit.c` GPIO 초기화 변경 |
| DSP_nCABLE_LOOP | 46 | 80 | 기존 `DSP_CABLE_LOOP`에서 이름 변경 포함 |
| DSP_ENC_DATA | 51 | 70 | `hal_Spi.c` SPI-C SOMI 핀 매핑 변경 |
| DSP_ENC_CLK | 52 | 71 | `hal_Spi.c` SPI-C CLK 핀 매핑 변경 |
| DSP_DRV_SPIB_CLK | 58 | 65 | `DSP_SPIB_CLK` ➡️ `DSP_DRV_SPIB_CLK` (이름/핀 변경) |
| DSP_DRV_SPIB_nCS | 59 | 66 | `DSP_SPIB_nCS` ➡️ `DSP_DRV_SPIB_nCS` (이름/핀 변경) |
| DSP_DRV_SPIB_SIMO | 60 | 63 | `DSP_SPIB_SIMO` ➡️ `DSP_DRV_SPIB_SIMO` (이름/핀 변경) |
| DSP_DRV_SPIB_SOMI | 61 | 64 | `DSP_SPIB_SOMI` ➡️ `DSP_DRV_SPIB_SOMI` (이름/핀 변경) |

## 3. 이더넷 PHY 연결 GPIO 변경 (2차 그림 반영)
현재는 평가보드 테스트를 위해 **기존 GPIO 핀맵을 그대로 유지**하며, 향후 변경될 핀 번호를 주석으로 상세히 표기할 예정입니다.
대상 소스: `hal_DspInit.h` 내 `#define GPIO_PIN_ENET_...` 및 `hal_DspInit.c` `initEmacGpioPins()` 함수.

| 신호명 | 유지 GPIO (테스트용) | 예정 GPIO (주석에 반영) |
| :--- | :--- | :--- |
| ENET_CRS | 109 | 34 |
| ENET_COL | 110 | 35 |
| ENET_MDC | 105 | 42 |
| ENET_MDIO | 106 | 43 |
| ENET_RX_CLK | 111 | 49 |
| ENET_RX_DV | 112 | 50 |
| ENET_RX_ER | 113 | 51 |
| ENET_RX_D0 | 114 | 52 |
| ENET_RX_D1 | 115 | 53 |
| ENET_RX_D2 | 116 | 54 |
| ENET_RX_D3 | 117 | 55 |
| ENET_TX_EN | 118 | 56 |
| ENET_TX_CLK | 44 | 58 |
| ENET_TX_D0 | 75 | 59 |
| ENET_TX_D1 | 122 | 60 |
| ENET_TX_D2 | 123 | 61 |
| ENET_TX_D3 | 124 | 62 |
| ENET_RESET_N | 119 | 67 |
| ENET_PWDN_N | 108 | 68 |

## 4. 수행 계획 요약
위 조사 내용을 바탕으로 다음 단계에서 실제 펌웨어 코드 및 문서 수정을 진행할 계획입니다.
1. `Architecture.md` 업데이트: 주요 스펙에 부품명(DP83822HFRHBT, HX1188NL, 오실레이터 등) 반영.
2. `hal_DspInit.h`: 이더넷 PHY 핀 할당 주석을 새 예정 핀으로 업데이트 (기존 코드는 유지).
3. `hal_DspInit.c`: GPIO_35 ~ GPIO_46 입력/출력 핀 초기화 코드를 신규 핀(GPIO_73 ~ GPIO_80)으로 마이그레이션.
4. `hal_Spi.c`: 엔코더 및 모터 드라이버용 SPI 통신 핀을 매뉴얼 규격에 맞게 핀 MUX 세팅 변경.
