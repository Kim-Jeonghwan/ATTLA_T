# ATTLA_T 하드웨어 변경 사항 반영 계획서 (Plan)

본 계획서는 `research.md`에서 조사된 내용(이더넷 부품 변경 및 GPIO 핀맵 재할당)을 펌웨어 프로젝트 전체에 안전하게 반영하기 위한 절차입니다.

## 1. 개요 및 반영 대상 (Overview & Target)
1. **`Architecture.md` 문서 업데이트**
   - CM 코어 이더넷 PHY IC 품번 (`DP83822HFRHBT`), 트랜스포머(`HX1188NL`), 크리스탈(`25MHz`, `SIT2024BM-S2-33E-25.000000`) 정보 기입.
   - W6100 사양에 트랜스포머(`HX1188NL`) 및 크리스탈(`ECS-250-8-37Q-RES-TR`) 정보 기입.
2. **`hal_DspInit.h` 내 PHY 핀 주석 업데이트**
   - 2차 그림에 명시된 PHY 핀 변경(예: ENET_CRS 109 -> 34) 내용을 `#define`의 임시 할당된 값은 유지하되, 주석으로 변경 예정 핀 번호를 상세히 기록.
3. **`hal_DspInit.c` 내 시스템 GPIO 초기화 코드 마이그레이션**
   - 기존 GPIO(35~46번 대) 입력/출력 설정 코드를 새로운 핀(74~80번 대)으로 치환.
   - 단, `DSP_nLIMIT1_NO`의 예정 핀 75번은 기존 `ENET_TX_D0` 테스트 핀(75)과 충돌하므로 **임시로 GPIO 73번으로 할당하고 주석에 명시**함.
4. **`hal_Spi.c` 내 SPI 통신 모듈 핀 변경**
   - `InitSpic()`: 엔코더 SPI-C 통신 핀 SOMI(51->70), CLK(52->71) 변경 적용.
   - `InitSpib()`: 모터 드라이버 SPI-B 통신 핀 CLK(58->65), STE(59->66), SIMO(60->63), SOMI(61->64) 이름 및 번호 일괄 변경.

## 2. 세부 구현 단계 (Implementation Steps)

- [x] `Architecture.md` 수정: 부품 사양(PHY IC, W6100 등) 업데이트 반영
- [x] `hal_DspInit.h` 수정: 이더넷 PHY MII 핀 매크로 우측 주석에 예정 변경 핀(예: `/* TBD: 변경 예정 34 */`) 추가
- [x] `hal_DspInit.c` 수정: `Init_GpioDin()`, `Init_GpioDout()` 내의 하드웨어 스위치 및 브레이크 제어 핀 맵 교체 (GPIO 73, 74, 76, 77, 78, 79, 80 설정)
- [x] `hal_Spi.c` 수정: SPI-B 및 SPI-C 초기화 함수 내의 패드 설정용 매크로(`GPIO_58_SPIB_CLK` -> `GPIO_65_SPIB_CLK` 등) 업데이트 및 주석 동기화
- [x] 컴파일 전 정적 시험(문법 등) 자체 검토 및 헤더 주석 갱신

## 3. 사용자 확인 필요 (Open Questions)
> [!IMPORTANT]
> - `DSP_nLIMIT1_NO` 핀을 임시로 GPIO 73에 연결하는 것 외에, 내부 PULL-UP 속성 등은 기존 설정(ASYNC, PULLUP 등)을 100% 동일하게 유지하면 되는지 확인 부탁드립니다.
> - 위 계획 내용이 정확하다면, **"구현 시작"** 또는 메모(수정)를 달아서 피드백을 주시면 즉시 코드 수정을 진행하겠습니다.
