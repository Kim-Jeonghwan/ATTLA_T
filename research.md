# CSU / HAL / main 계층 주석 현황 및 개선 방안 조사 보고서

## 1. 헤더 주석 및 이력 관리 규정 준수 현황
글로벌 룰(`RULE[user_global]`)에서 정의한 파일 최상단의 헤더 주석 템플릿과 `Modification History` 주석 규칙의 준수 여부를 검토했습니다.

### 1.1 개선이 필요한 사항 (공통)
- **헤더 이력 미반영**: 헤더 내의 `Last Updated` 내용과 `Version` 번호는 수정되어 있으나, 그 바로 아래 `Modification History` 목록에 수정 기록이 누적되어 있지 않거나 날짜가 맞지 않는 파일이 다수 식별되었습니다. (예: `main.h`, `csu_Led.h`, `csu_SciPc.h`, `hal_DspInit.h`, `hal_Ethernet.h`, `hal_MotorDriver.h` 등)
- **중복 이력 섹션**: `hal_Timer.c` 및 `hal_Timer.h` 파일의 경우, 표준 이력 테이블 외에 별도로 `수정 이력`, `변경 이력`이라는 레거시 주석 영역이 중복 선언되어 있어 제거가 권장됩니다.

---

## 2. 함수 및 프로토타입 주석 분석
각 계층의 소스 파일(`.c`) 및 헤더 파일(`.h`) 내 함수 주석의 품질, 누락 여부, 오타를 상세 조사하였습니다.

### 2.1 공통 발견 이슈: `@funtion` 오타
프로젝트 내의 거의 모든 Doxygen 스타일 주석에서 `@function`이 **`@funtion`**으로 오타가 나 있는 것을 발견했습니다.
- **대상 모듈**: `main.c`, `csu_Adc.c`, `csu_Control.c`, `csu_Led.c`, `csu_MotorCtrl.c`, `csu_PID.c`, `csu_SciPc.c`, `hal_Adc.c`, `hal_DspInit.c`, `hal_Epwm.c`, `hal_Ethernet.c`, `hal_Fram.c`, `hal_Sci.c`, `hal_Spi.c`, `hal_Timer.c`
- **조치 계획**: 주석 일괄 적용 시 `@funtion`을 `@function`으로 일제히 수정합니다.

### 2.2 헤더 파일(`.h`) 내 프로토타입 주석 누락 (전수 누락)
대부분의 헤더 파일에서 선언된 함수 프로토타입 앞에 Doxygen 주석이 완전히 누락되어 있거나, 형식에 맞지 않는 간단한 한 줄짜리 슬래시 주석만 기재되어 있습니다.
- **주석 전무 헤더**: `csu_Bit.h`, `csu_Control.h`, `csu_Encoder.h`, `csu_MotorCtrl.h`, `csu_MotorDriver.h`, `csu_PID.h`, `hal_Adc.h`, `hal_Encoder.h`, `hal_Epwm.h`, `hal_Ethernet.h`, `hal_Fram.h`, `hal_Led.h`, `hal_MotorDriver.h`, `hal_Sci.h`, `hal_Spi.h`
- **조치 계획**: 소스 파일의 함수 정의부 Doxygen 상세 주석을 헤더 파일의 프로토타입 선언부에도 형식을 맞춰 함께 반영해 줍니다.

### 2.3 소스 파일(`.c`) 내 함수 주석 누락 모듈
일부 핵심 모듈들의 경우 소스 파일 내부 함수 위에도 Doxygen 형태가 아닌, 단순히 함수 이름만 적힌 슬래시 주석이 있거나 주석이 아예 없는 경우가 많습니다.
- **`csu_Bit.c`**: `Bit_Init`, `Bit_OvCurrent_Check` 등 6개 함수 전원 주석 누락.
- **`csu_Control.c`**: `Control_Init` 주석 누락.
- **`csu_Encoder.c`**: `Encoder_Init`, `Encoder_LoadOffset` 등 5개 함수 전체 Doxygen 주석 누락.
- **`csu_MotorDriver.c`**: `MotorDriver_Init`, `MotorDriver_ClearFaults`, `MotorDriver_UpdateStatus` 3개 함수 Doxygen 주석 누락.
- **`hal_Encoder.c`**: `Encoder_Init_Hardware`, `Encoder_ReadSpiData` 2개 함수 Doxygen 주석 누락.
- **`hal_Led.c`**: `Led_InitGpio`, `Led_WritePin`, `Led_TogglePin` 3개 함수 Doxygen 주석 누락.
- **`hal_MotorDriver.c`**: `MotorDriver_Init_Hardware`, `MotorDriver_ReadReg`, `MotorDriver_WriteReg` 3개 함수 Doxygen 주석 누락.
- **`hal_Spi.c`**: `cs_sel`, `cs_desel`, `spi_read_byte`, `spi_write_byte` 4개 이더넷 연동 콜백 함수 주석 누락.

---

## 3. 불필요한 주석 및 레거시 코드 조사 결과
소스 코드 내에 주석 처리된 채 잔존해 있어 가독성을 해치거나, 정적 분석에 불리하게 작용할 수 있는 불필요한 영역들을 발굴했습니다.

### 3.1 불필요한 선언 및 소멸된 코드 주석
- **`csu_Led.h`**: 
  - 34~35라인: `#define GPIO_LED_ERROR 146u` 주석 처리됨.
  - 46라인: `// eLED_ERROR = 146u,` 주석 처리됨.
  - 69라인: `// stLed ledError;` 주석 처리됨.
  - 108라인: `void updateGpioLed(void);` 프로토타입 선언이 존재하나, `.c` 내 구현부가 없어 불필요한 쓰레기 선언임.
- **`csu_Led.c`**:
  - 52~56라인: `ERROR LED (GPIO146) 설정` 관련 코드가 통째로 주석 처리되어 있음.
  - 76라인: `pLed[1] = &xLed.ledError;` 레거시 주석 처리.
- **`hal_Led.c`**:
  - 41~43라인 / 59~61라인: `case eLED_ERROR:` 분기 코드들이 전부 주석 처리된 레거시로 잔존.
- **`hal_Fram.c`**:
  - 139~149라인: `Fram_WriteDisable` 함수 정의와 Doxygen 주석 전체가 주석 처리됨.
  - 170~180라인: `Fram_ReadStatusRegister` 함수 정의와 Doxygen 주석 전체가 주석 처리됨.
  
### 3.2 디버깅 및 컴파일 제어 레거시 주석
- **`main.c`**: 
  - 79라인: `sendScia_SCI_PC(); // 디버깅용?` 모호한 주석 기재.
  - 166~170라인: `#if 0 ... #endif`로 ePWM 기반 테스트 코드가 비활성화되어 잔존.
- **`hal_Sci.c`**:
  - 270~279라인: `#if 1 ... #else ... #endif`를 이용해 기존 direct 블로킹 송신 함수인 `SCI_writeCharArray` 호출 코드가 주석 처리된 채 잔존.
- **`csu_Adc.h`**:
  - 51라인: `updateAdcData` 프로토타입 주석에 `(뼈대) - 더 이상 사용하지 않음`이라고 기재되어 있으나, 실제 `csu_Adc.c`에서 이 함수가 활성화되어 `updateDspTempSensor`를 호출하여 실시간 온도를 업데이트하고 있어 주석 내용이 왜곡됨.

---

## 4. 종합 개선 계획
1. **헤더 주석 통일**: 모든 `.c` 및 `.h` 파일 최상단의 버전을 표준 템플릿 형식으로 재점검하며, `Modification History`에 누락된 수정 사항 이력을 기입합니다.
2. **함수 Doxygen 양식 일치**:
   - 모든 함수의 주석을 `@function`으로 오타 수정 및 교정합니다.
   - 주석이 누락된 함수(CSU 및 HAL)들에 대해 매개변수(`@param`), 반환값(`@return`), 세부 동작 설명(`@remark`)을 갖춘 정식 Doxygen 주석을 전원 신규 추가합니다.
3. **헤더 파일 주석 추가**: 소스 파일의 상세 주석과 100% 매칭되는 설명 주석을 헤더 파일의 프로토타입 앞부분에도 정밀 추가하여 헤더만 보고도 API 사양을 파악할 수 있도록 개선합니다.
4. **레거시 및 불필요 주석 제거**: 위에 보고된 주석 처리되어 잔존하는 미사용 코드들(`ERROR LED` 잔재, 주석 처리된 함수, 컴파일 지시어 잔재 등)을 사용자 승인 하에 일괄 삭제하여 정적 신뢰성을 강화합니다.
