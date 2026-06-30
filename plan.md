# ATTLA-T 리팩토링 및 수정 구현 계획 (Implementation Plan)

## 1. 개요
이 계획서는 `research.md`에서 도출된 8가지 항목에 대한 검토 결과를 바탕으로, 소스 코드 및 관련 문서의 불일치 사항을 정정하고 정적시험 기준 및 코드 컨벤션에 맞게 펌웨어를 수정하기 위한 작업 계획입니다.

## 2. 세부 작업 단계 (Tasks)

### [x] [단계 1] 마크다운 명세서 오기 정정 (문서 업데이트)
코드의 실제 구현 상태와 일치하도록 명세서의 파일명 오기를 정정합니다.
- **수정 대상 1**: `Architecture.md`
  - 4.1절: `csu_Debug_cpu1.c` 모듈 -> `csu_Debug.c` 모듈로 수정
  - 7.1절: `stEthControl xEthCtrl` 변수 설명에서 `csu_Ethernet.h` -> `csu_Ethernet_cm.h`로 수정 (-> 확인 결과 `Ethernet_Specification.md`의 오기로 파악되어 수정 완료)
- **수정 대상 2**: `Ethernet_Specification.md`
  - 3장 및 5장: `csu_Ethernet.c`, `csu_Ethernet.h` -> `csu_Ethernet_cm.c`, `csu_Ethernet_cm.h`로 수정

### [단계 2] 모든 함수 주석 스캔 및 보완 (코드 포맷팅)
모든 `CSU`, `HAL`, `main` 파일들에 대해 함수 정의부를 스캔하여 규칙(Doxygen 포맷)에 맞는 주석이 누락된 함수를 찾아냅니다.
- **작업 내용**:
  - Python 스크립트 또는 정규식을 통해 `csu_`, `hal_`, `main_` 파일 내의 모든 함수 선언/정의 상단에 `/*` 블록이나 `@function` 주석이 있는지 전수 조사.
  - 주석이 누락된 함수가 발견되면, 해당 함수의 기능(`@brief`), 매개변수(`@param`), 반환값(`@return`)을 파악하여 표준 양식에 맞게 주석 추가.
  - 파일 수정 시 `GEMINI.md` 규칙에 따라 상단의 `Modification History` 및 버전을 업데이트.

### [단계 3] 정적시험 기준 (DAPA SCR-G) 위반 사항 리팩토링
국방 규격(무기체계 소프트웨어 코딩규칙) 및 소스코드 품질 메트릭에 부합하도록 소스 코드를 리팩토링합니다.
- **작업 내용**:
  - **단일 Return 원칙 (Single Exit Point)**: 함수 내에 `return` 문이 여러 개인 경우, 결과 변수를 하나로 통일하여 마지막에 한 번만 `return` 하도록 수정.
  - **방어적 프로그래밍 (default/else 강제)**: 모든 `switch` 문에 `default:` 블록 추가, `if-else if` 구문에 최종 `else` 예외 처리 블록 추가.
  - **초기화 누락 방지 (CWE-457)**: 선언만 되고 초기화되지 않은 지역 변수들에 대해 선언 시점 명시적 초기화(`= 0`, `= 0.0f` 등) 적용.
  - **복잡도 완화 (Cyclomatic Complexity)**: 한 함수 내에서 조건/분기가 20개를 초과하거나 `들여쓰기(Depth)`가 6단계를 넘는 방대한 함수(예: 통신 파싱 로직 등)가 발견될 경우, 하위 함수로 분할(Refactoring).

### [단계 4] 컴파일 오류 확인 및 안전성 검증
에이전트는 코드 수정 시 문맥 유실이 발생하지 않도록 치환 도구(multi_replace_file_content)를 엄격히 사용하며, 수정이 완료된 후 사용자가 CCS IDE를 통해 빌드를 수행하여 문제가 없음을 확인할 수 있도록 안내합니다.

## 3. 사용자 확인 및 승인 요청 (User Review Required)
- `csu_Debug.c` 파일명을 `csu_Debug_cpu1.c`로 파일 자체를 변경하는 대신, 문서(`Architecture.md`)를 실제 파일명에 맞게 수정하는 방향으로 진행하고자 합니다. 이에 동의하십니까?
- 정적시험 기준 수정을 위해 전체 파일을 스캔하여 꽤 많은 부분을 일괄 리팩토링하게 됩니다. 진행해도 괜찮겠습니까?
- **계획에 동의하시면 "진행해줘"라고 입력해 주십시오.**
