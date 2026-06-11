# CSU / HAL / main 계층 주석 표준화 및 레거시 주석 코드 삭제 계획

## 1. 목표
프로젝트 전반의 CSU, HAL, main 파일(.c, .h)을 대상으로, 글로벌 룰(`RULE[user_global]`/`GEMINI.md`)에 정의된 **주석 템플릿 표준을 100% 준수**하고, Doxygen 오타 교정 및 누락된 함수 주석을 전수 보완합니다. 또한 코드베이스에 주석 처리된 채 방치된 미사용 레거시 코드를 일괄 제거하여 코드 가독성과 정적 신뢰성을 강화합니다.

---

## 2. 사용자 검토 및 의사결정 필요 사항 (User Review Required)

> [!IMPORTANT]
> **1. 주석 처리된 미사용 레거시 코드 완전 삭제 여부**
> - `csu_Led.c / .h`, `hal_Led.c`에 남아있는 `ERROR LED (GPIO 146)` 관련 주석 처리 코드
> - `hal_Fram.c`에 함수 전체가 주석 처리되어 잔존하는 `Fram_WriteDisable()`, `Fram_ReadStatusRegister()`
> - `hal_Sci.c` 내에 `#if 1 ... #else ... #endif` 구문 속에 주석 처리된 `SCI_writeCharArray()`
> - **[추천]** 코드 가독성과 정적 분석 표준(CWE 및 무기체계 소프트웨어 코딩규칙) 준수를 위해, 위 주석 처리된 데드 코드를 **완전히 삭제**하는 것을 추천합니다. 동의하지 않으시면 이 계획서에 의견을 남겨주십시오.

> [!IMPORTANT]
> **2. `csu_Adc.h` 내 updateAdcData() 설명 교정**
> - 현재 `updateAdcData()` 프로토타입 주석에 `더 이상 사용하지 않음`이라고 기재되어 있으나, 실제 `csu_Adc.c`에서 이 함수는 온도를 갱신하는 용도로 엄연히 사용 중입니다. 이 주석을 `내부 온도 센서 실시간 데이터 수집 및 갱신`으로 교정하겠습니다.

---

## 3. 상세 수정 계획 (Proposed Changes)

### 3.1 공통 수정 사항 (전체 파일 적용)
- 모든 `.c` 및 `.h` 파일 최상단의 헤더 주석 템플릿 내 `Last Updated` 일자 및 상세 내용을 업데이트합니다.
- `Modification History`에 `2026. 06. 11. - 주석 표준화 및 레거시 코드 정리` 이력을 정식 추가합니다.
- 함수 정의부의 Doxygen 오타인 **`@funtion`을 `@function`으로 일괄 수정**합니다.

---

### 3.2 메인 계층 [main]
#### [MODIFY] [main.c](file:///d:/Nexcom/Firmware/01_Project/04_ATTLA/ATTLA_T/ATTLA_T/ATTLA_T_CPU1/main.c)
- `main`, `cycle_1ms` 등 5개 함수 Doxygen 주석 오타 교정
- line 79의 모호한 주석 `// 디버깅용?` 정비
- line 166-170의 `#if 0` 테스트 코드 주석 완전 삭제

#### [MODIFY] [main.h](file:///d:/Nexcom/Firmware/01_Project/04_ATTLA/ATTLA_T/ATTLA_T/ATTLA_T_CPU1/main.h)
- `main(void)` 프로토타입에 규격화된 Doxygen 설명 주석 추가
- `Modification History` 이력 매칭 최신화

---

### 3.3 어플리케이션 계층 [CSU]

#### [MODIFY] `csu_Adc.h`, `csu_Bit.h`, `csu_Control.h`, `csu_Encoder.h`, `csu_Led.h`, `csu_MotorCtrl.h`, `csu_MotorDriver.h`, `csu_PID.h`, `csu_SciPc.h`
- 헤더에 선언된 **모든 함수 프로토타입 앞에 상세 Doxygen 주석(기능, 매개변수, 반환값 설명)을 추가**합니다.
- `csu_Adc.h`: `updateAdcData` 설명 왜곡 교정
- `csu_Led.h`: `updateGpioLed` 미구현 프로토타입 제거, `ERROR LED` 주석 잔재 완전 삭제

#### [MODIFY] `csu_Bit.c`, `csu_Encoder.c`, `csu_MotorDriver.c`
- Doxygen 주석이 누락되거나 단순 슬래시로 작성되었던 모든 함수에 정식 Doxygen 주석을 작성합니다.
  - `csu_Bit.c` (`Bit_Init`, `Bit_OvCurrent_Check` 등 6개)
  - `csu_Encoder.c` (`Encoder_Init`, `Encoder_LoadOffset` 등 5개)
  - `csu_MotorDriver.c` (`MotorDriver_Init`, `MotorDriver_ClearFaults`, `MotorDriver_UpdateStatus` 3개)

#### [MODIFY] `csu_Control.c`
- `Control_Init` 주석 보완
- `Control_CalibrateCurrentOffset` 등의 `@funtion` 오타 수정
- 정적 분석 및 중괄호 줄바꿈 표준 준수를 위해 `Control_CalibrateCurrentOffset` 내의 `else` 블록 중괄호(`{}`) 누락 부분 정밀 보완

#### [MODIFY] `csu_Led.c`
- `ERROR LED` 관련 주석 처리된 코드 및 주석 배열 매핑 잔재 완전 삭제

---

### 3.4 하드웨어 추상화 계층 [HAL]

#### [MODIFY] `hal_Adc.h`, `hal_Encoder.h`, `hal_Epwm.h`, `hal_Ethernet.h`, `hal_Fram.h`, `hal_Led.h`, `hal_MotorDriver.h`, `hal_Sci.h`, `hal_Spi.h`, `hal_Timer.h`
- 헤더에 선언된 **모든 함수 프로토타입 앞에 상세 Doxygen 주석을 전원 추가**합니다.
- `hal_Common.h`: 데이터 변환 공용체 `onConv32`, `onConv16` 설명 상세화
- `hal_Timer.h`: 중복 선언된 레거시 `변경 이력` 주석 제거

#### [MODIFY] `hal_Encoder.c`, `hal_Led.c`, `hal_MotorDriver.c`
- Doxygen 주석이 누락되었던 다음 함수들에 규격 주석을 추가합니다.
  - `hal_Encoder.c` (`Encoder_Init_Hardware`, `Encoder_ReadSpiData`)
  - `hal_Led.c` (`Led_InitGpio`, `Led_WritePin`, `Led_TogglePin`)
  - `hal_MotorDriver.c` (`MotorDriver_Init_Hardware`, `MotorDriver_ReadReg`, `MotorDriver_WriteReg`)
- `hal_Led.c`: 주석 처리되어 방치된 `case eLED_ERROR:` 잔재 완전 삭제

#### [MODIFY] `hal_Ethernet.c`
- `Initial_W6100` 내 중복으로 쓰여있는 `W6100 메모리 초기화...` 주석 제거

#### [MODIFY] `hal_Fram.c`
- 주석 처리된 채 방치된 `Fram_WriteDisable()`, `Fram_ReadStatusRegister()` 코드 완전 삭제

#### [MODIFY] `hal_Sci.c`
- 송신 큐 제어 비활성 레거시 코드(`#if 1 ... #else ... #endif`) 영역 완전 삭제

#### [MODIFY] `hal_Spi.c`
- Doxygen 주석이 전혀 없던 이더넷 물리 핀 콜백 제어 함수군(`cs_sel`, `cs_desel`, `spi_read_byte`, `spi_write_byte`)에 상세 Doxygen 주석 추가

#### [MODIFY] `hal_Timer.c`
- 중복 선언된 레거시 `수정 이력` 주석 제거

---

## 4. 검증 및 빌드 계획 (Verification Plan)
- **에이전트 직접 빌드 금지 룰 준수**: 코드 수정 후 직접 터미널 빌드를 실행하지 않고, 사용자에게 안내합니다.
- **사용자 빌드 유도**: 모든 주석 정비가 완료되면 사용자가 `Code Composer Studio (CCS)` IDE 환경에서 전체 빌드(Rebuild All)를 진행하여 빌드가 에러 없이 완벽히 끝나는지 교차 검증을 수행합니다.
