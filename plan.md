# ATTLA-T 펌웨어 이더넷 모듈 리팩토링 구현 계획서 (plan.md)

이 문서는 CPU1과 CM 코어에서 이름 충돌과 혼동을 야기하는 이더넷 소스/헤더 파일들의 이름을 분리(`_cpu1`, `_cm`)하고, 관련 의존성을 100% 안전하게 치환하기 위한 구현 계획서입니다.

---

## 1. 파일명 변경 계획 (Terminal Rename)
운영체제 환경(Windows/Powershell)에 맞추어 `mv` 명령어를 통해 물리적 파일명을 먼저 변경합니다. (에이전트가 직접 터미널 명령어를 수행하며, 사용자의 승인창이 뜨면 수락해 주시면 됩니다.)

- **CPU1 코어**:
  - `ATTLA_T_CPU1/CSU/csu_Ethernet.c` ➡️ `csu_Ethernet_cpu1.c`
  - `ATTLA_T_CPU1/CSU/csu_Ethernet.h` ➡️ `csu_Ethernet_cpu1.h`
  - `ATTLA_T_CPU1/HAL/hal_Ethernet.c` ➡️ `hal_Ethernet_cpu1.c`
  - `ATTLA_T_CPU1/HAL/hal_Ethernet.h` ➡️ `hal_Ethernet_cpu1.h`

- **CM 코어**:
  - `ATTLA_T_CM/CSU/csu_Ethernet.c` ➡️ `csu_Ethernet_cm.c`
  - `ATTLA_T_CM/CSU/csu_Ethernet.h` ➡️ `csu_Ethernet_cm.h`
  - `ATTLA_T_CM/HAL/hal_Ethernet.c` ➡️ `hal_Ethernet_cm.c`
  - `ATTLA_T_CM/HAL/hal_Ethernet.h` ➡️ `hal_Ethernet_cm.h`

---

## 2. 코드 내부 의존성 교체 계획 (`multi_replace_file_content`)
파일명 변경이 완료되면, 변경된 각 파일 내부에 남아있는 구 파일명 문자열과 매크로를 국소 치환합니다. 원본 코드를 보존하는 `Replace` 도구만을 사용합니다.

### 2.1 CPU1 코어 대상 치환
1. **`main_cpu1.h`**:
   - `#include "csu_Ethernet.h"` ➡️ `#include "csu_Ethernet_cpu1.h"`
   - `#include "hal_Ethernet.h"` ➡️ `#include "hal_Ethernet_cpu1.h"`
2. **`csu_Ethernet_cpu1.c/h`**:
   - `.c` 파일 내: `#include "csu_Ethernet.h"` ➡️ `#include "csu_Ethernet_cpu1.h"`
   - `.h` 파일 내(매크로): `CSU_ETHERNET_H_` ➡️ `CSU_ETHERNET_CPU1_H_`
   - 두 파일 공통: 헤더 주석 내 `Filename`을 `csu_Ethernet_cpu1.c/h`로 수정하고 버전을 `00.01` 증가 및 수정 이력 추가.
3. **`hal_Ethernet_cpu1.c/h`**:
   - `.c` 파일 내: `#include "hal_Ethernet.h"` ➡️ `#include "hal_Ethernet_cpu1.h"`
   - `.h` 파일 내(매크로): `HAL_ETHERNET_H_` ➡️ `HAL_ETHERNET_CPU1_H_`
   - 두 파일 공통: 헤더 주석 내 `Filename` 수정, 버전 증가 및 수정 이력 추가.

### 2.2 CM 코어 대상 치환
1. **`main_cm.h`**:
   - `#include "csu_Ethernet.h"` ➡️ `#include "csu_Ethernet_cm.h"`
   - `#include "hal_Ethernet.h"` ➡️ `#include "hal_Ethernet_cm.h"`
2. **`csu_Ethernet_cm.c/h`**:
   - `.c` 파일 내: `#include "csu_Ethernet.h"` ➡️ `#include "csu_Ethernet_cm.h"`
   - `.h` 파일 내(매크로): `CSU_ETHERNET_H_` ➡️ `CSU_ETHERNET_CM_H_`
   - 두 파일 공통: 헤더 주석 내 `Filename` 수정, 버전 증가 및 수정 이력 추가.
3. **`hal_Ethernet_cm.c/h`**:
   - `.c` 파일 내: `#include "hal_Ethernet.h"` ➡️ `#include "hal_Ethernet_cm.h"`
   - `.h` 파일 내(매크로): `HAL_ETHERNET_H_` ➡️ `HAL_ETHERNET_CM_H_`
   - 두 파일 공통: 헤더 주석 내 `Filename` 수정, 버전 증가 및 수정 이력 추가.

---

## 3. 검증(Verification) 절차
- 에이전트는 자동 빌드를 수행하지 않으며, CCS 환경의 사용자가 직접 프로젝트를 열고 `Clean & Rebuild`를 수행하여 정상적으로 링크/컴파일되는지 최종 확인하도록 안내합니다.
- (필요 시 파일명 변경 후 CCS `.cproject` 트리에서 파일이 정상 갱신되었는지 확인을 요청합니다.)

---
사용자께서 본 `plan.md`를 확인하시고 승인 지시(진행 등)를 주시면, 우선적으로 Powershell `mv` 명령을 통한 파일명 리네임 작업을 시작하겠습니다.
