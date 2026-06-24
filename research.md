# 이더넷 모듈(CPU1 / CM) 파일명 분리 및 리팩토링 조사 보고서

## 1. 개요 및 배경
현재 ATTLA_T 프로젝트의 듀얼 코어(CPU1, CM) 아키텍처 상, 양쪽 코어 폴더(`ATTLA_T_CPU1`, `ATTLA_T_CM`)의 `CSU`, `HAL` 계층에 **동일한 이름의 이더넷 소스/헤더 파일(`csu_Ethernet.c/h`, `hal_Ethernet.c/h`)** 이 존재하여 개발 및 유지보수 시 혼선을 야기할 우려가 있습니다.
이를 해결하기 위해 각 코어의 소속을 명확히 하는 식별자(`_cpu1`, `_cm`)를 파일명에 부여하는 리팩토링 작업을 위한 사전 조사를 진행했습니다.

---

## 2. 식별된 대상 파일 및 리네임(Rename) 계획

### 2.1 CPU1 코어 폴더 (`ATTLA_T_CPU1`)
| 기존 파일명 | 변경 대상 파일명 (To-Be) |
| :--- | :--- |
| `CSU/csu_Ethernet.c` | `CSU/csu_Ethernet_cpu1.c` |
| `CSU/csu_Ethernet.h` | `CSU/csu_Ethernet_cpu1.h` |
| `HAL/hal_Ethernet.c` | `HAL/hal_Ethernet_cpu1.c` |
| `HAL/hal_Ethernet.h` | `HAL/hal_Ethernet_cpu1.h` |

### 2.2 CM 코어 폴더 (`ATTLA_T_CM`)
| 기존 파일명 | 변경 대상 파일명 (To-Be) |
| :--- | :--- |
| `CSU/csu_Ethernet.c` | `CSU/csu_Ethernet_cm.c` |
| `CSU/csu_Ethernet.h` | `CSU/csu_Ethernet_cm.h` |
| `HAL/hal_Ethernet.c` | `HAL/hal_Ethernet_cm.c` |
| `HAL/hal_Ethernet.h` | `HAL/hal_Ethernet_cm.h` |

---

## 3. 코드 내 의존성(Dependencies) 및 변경 필요 항목

파일명 변경 시 소스 코드 내부에서도 참조하고 있는 내용(Include 구문, Header Guard 등)을 일관성 있게 수정해야 합니다. 

### 3.1 헤더 파일 (`#include` 수정)
- `main_cpu1.h`: `#include "csu_Ethernet.h"`, `#include "hal_Ethernet.h"` ➡️ `_cpu1.h`로 치환.
- `main_cm.h`: `#include "csu_Ethernet.h"`, `#include "hal_Ethernet.h"` ➡️ `_cm.h`로 치환.
- 각 이더넷 `.c` 파일들 상단의 자신의 `.h` 파일 `#include` 구문 업데이트.

### 3.2 헤더 가드 (Header Guard) 수정 (권장 사항)
헤더 파일 내부의 다중 포함 방지 매크로(`#ifndef`, `#define`)도 파일명에 맞추어 업데이트해야 합니다.
- (예) `CSU_ETHERNET_H_` ➡️ `CSU_ETHERNET_CPU1_H_` / `CSU_ETHERNET_CM_H_`
- (예) `HAL_ETHERNET_H_` ➡️ `HAL_ETHERNET_CPU1_H_` / `HAL_ETHERNET_CM_H_`

### 3.3 물리 파일명 주석 (Header Comment) 갱신
모든 `.c`, `.h` 파일 상단의 Nexcom 주석 템플릿(Header Comment) 내 `Filename`과 `Modification History` 항목을 룰에 맞게 업데이트(Version 증가 및 반영 이력 기재)해야 합니다.

---

## 4. 빌드 시스템(CCS IDE) 유의사항
- Eclipse 기반의 CCS(Code Composer Studio)는 `.cproject` 파일 내 명시적인 Exclude 설정이 없는 이상, 프로젝트 폴더 내 변경된 `.c` 파일들을 자동으로 재탐색하여 빌드 타겟에 포함시킵니다.
- 다만 가상 폴더 구조(Virtual Folders)에 종속된 경우 약간의 재로드가 필요할 수 있으며, 이 리팩토링 후에는 **사용자께서 IDE에서 명시적으로 Rebuild(빌드)를 수행하여 링킹 오류가 없는지 교차 검증**해야 합니다.

---
**결론:**
본 조사를 바탕으로 계획(Plan)이 승인되면, 시스템 쉘 커맨드(`mv` 또는 Powershell 명령어)를 통한 파일 리네임(Rename)과, 그에 종속된 소스 코드 상의 매크로/인클루드 부분 치환 작업을 100% 안전하게 진행할 준비가 되어 있습니다.
