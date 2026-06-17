# 프로젝트 계층 구조 및 코딩 규칙 검토 보고서 (Research Report)

본 보고서는 `d:\Nexcom\Firmware\01_Project\04_ATTLA\ATTLA_T\ATTLA_T` 프로젝트의 `CSU`, `HAL`, `main` 폴더/파일들이 `GEMINI.md`에 명시된 아키텍처 및 코딩 룰을 올바르게 준수하고 있는지 전수 조사한 결과입니다.

## 1. 헤더 인클루드 규칙 (Header Inclusion Rules) 위반 사항
모든 커스텀 헤더 파일은 `main.h`에 전부 `#include` 되어야 하고, 개별 소스 파일(`.c`)은 자신의 이름과 동일한 단 하나의 헤더 파일(`.h`)만 `#include` 해야 합니다.

- **`csu_Ethernet.c`**
  - **위반 내용**: `#include "csu_Ethernet.h"` 외에 `#include "hal_Ethernet.h"`를 추가로 선언하고 있습니다.
  - **수정 방향**: `#include "hal_Ethernet.h"`를 제거하고 의존성은 `main.h`를 통해 해결되도록 해야 합니다.

- **외부/써드파티 라이브러리 파일 (`HAL` 폴더 내부)**
  - **대상 파일**: `wizchip_conf.h`, `wizchip_conf.c`, `w6100.h`, `w6100.c`, `socket.h`, `socket.c`, `easy28x_driverlib_v12.2.c` 등
  - **위반 내용**: `device.h`, `<stdint.h>`, `<stddef.h>`, `driverlib.h` 등 여러 헤더를 자체적으로 `#include` 하고 있습니다.
  - **수정 방향**: 커스텀 계층 파일이라면 `main.h`만 포함해야 합니다. 단, 이들이 외부 라이브러리/SDK라면 `SDK` 폴더로의 이전이 바람직합니다 (아래 2번 항목 참조).

## 2. 계층 구조 및 명명 규칙 (Project Architecture & Naming Convention) 위반 사항
CSU 및 HAL 모듈과 파일명에는 반드시 소문자 접두어(`csu_`, `hal_`)를 사용해야 하며, 함수나 변수명에는 접두어를 사용하지 말아야 합니다.

- **`HAL` 폴더 내 외부 라이브러리 파일 명명 및 위치 오류**
  - **대상 파일**: `w6100.c`, `w6100.h`, `socket.c`, `socket.h`, `wizchip_conf.c`, `wizchip_conf.h`, `easy28x_driverlib_v12.2.c`, `easy28x_driverlib_v12.2.h`
  - **위반 내용**: `hal_` 접두어가 전혀 사용되지 않았습니다.
  - **수정 방향**: 이 코드들은 제조사(WIZnet, TI/easyDSP)에서 제공한 드라이버/라이브러리 코드로 판단됩니다. 따라서 아키텍처 원칙에 따라 `HAL` 계층이 아닌 **`SDK` 계층으로 파일들을 이동**시키는 것이 맞습니다. 부득이 `HAL` 계층에 남겨야 한다면 `hal_` 접두어를 추가해야 합니다.

- **`csu_Ethernet.c` 내 함수명 접두어 사용 오류**
  - **위반 내용**: 함수명에 `csu_` 접두어가 사용되었습니다.
    - 예: `csu_Ethernet_Init()`, `csu_Ethernet_CalculateChecksum()`, `csu_Ethernet_SendMessage()`, `csu_Ethernet_StateMachine()` 등
  - **수정 방향**: "함수나 변수명에는 해당 접두어를 사용하지 마십시오"라는 규칙에 맞게, 접두어를 제외한 이름(예: `Ethernet_Init()`, `Ethernet_StateMachine()`)으로 리팩토링해야 합니다. (단, HAL 계층과 이름이 충돌하지 않도록 적절한 네이밍 필요)

## 3. 주석 및 수정 이력 (Modification History) 점검 결과
모든 파일은 템플릿 포맷에 맞춘 헤더 주석과 Modification History 블록을 최상단에 유지해야 합니다.

- **점검 결과: 양호 (PASS)**
  - `csu_Adc.c`, `hal_Adc.c`, `csu_Ethernet.c`, `hal_Ethernet.c`, `main.c`, `main.h` 등을 샘플링하여 확인한 결과, 지정된 헤더 주석 템플릿과 Modification History 양식이 정상적으로 반영되어 있습니다. 
  - Version 업데이트, Programmer명, 변경 날짜 기재 등 모두 원칙에 맞게 작성되어 있습니다.

## 4. 매크로 및 상수 선언 (Macro Constants) 점검 결과
매크로 상수는 `.c` 내부가 아닌 `.h` 모듈에 작성해야 합니다.

- **점검 결과: 양호 (PASS)**
  - 기존 코드 리뷰 및 이전 수정 내역(`ADC_SCALE_REF_VOLT` 적용 등)을 볼 때, 매직 넘버가 제거되고 주요 상수들이 헤더 파일(`.h`)로 분리 및 추상화되어 있는 등 규칙을 잘 따르고 있습니다.

---

**결론 및 다음 단계 (Conclusion & Next Steps)**
1. `HAL` 폴더 내의 WIZnet 및 외부 라이브러리(`socket`, `w6100`, `wizchip_conf`, `easy28x_...`)를 `SDK`로 이동할지, 아니면 파일명을 변경할지 결정이 필요합니다.
2. `csu_Ethernet.c` 등 함수명 내에 `csu_`가 포함된 부분과 부적절한 헤더 인클루드를 리팩토링해야 합니다.

**사용자님, 위 조사 결과를 바탕으로 코드 수정을 진행하기 위한 계획(Plan)을 작성할까요?**
