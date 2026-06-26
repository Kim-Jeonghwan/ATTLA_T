# 리팩토링 계획: 파일명 명명 규칙 통일 (hal_Timer_cm 및 CPU1 Debug)

## 작업 목표
1. CM 코어: `hal_Timer` 파일을 `hal_Timer_cm`으로 변경하여 명명 규칙 통일.
2. CPU1 코어: `csu_Debug_cpu1` 및 `hal_Debug_cpu1` 파일명에서 `_cpu1` 접미사를 제거하여 명명 규칙 통일.

## 1. CM 코어 (hal_Timer -> hal_Timer_cm) 변경 사항
### 파일 이름 변경
- `ATTLA_T_CM/HAL/hal_Timer.c` ➡️ `hal_Timer_cm.c`
- `ATTLA_T_CM/HAL/hal_Timer.h` ➡️ `hal_Timer_cm.h`

### 내부 참조 변경
- **hal_Timer_cm.h**: `#ifndef HAL_TIMER_CM_H` 로 헤더 가드 변경, 상단 주석 업데이트
- **hal_Timer_cm.c**: `#include "hal_Timer_cm.h"` 로 변경, 상단 주석 업데이트
- **main_cm.h**: `#include "hal_Timer_cm.h"` 로 변경, 상단 주석 업데이트

## 2. CPU1 코어 (Debug 모듈 _cpu1 제거) 변경 사항
### 파일 이름 변경
- `ATTLA_T_CPU1/HAL/hal_Debug_cpu1.c` ➡️ `hal_Debug.c`
- `ATTLA_T_CPU1/HAL/hal_Debug_cpu1.h` ➡️ `hal_Debug.h`
- `ATTLA_T_CPU1/CSU/csu_Debug_cpu1.c` ➡️ `csu_Debug.c`
- `ATTLA_T_CPU1/CSU/csu_Debug_cpu1.h` ➡️ `csu_Debug.h`

### 내부 참조 변경
- **hal_Debug.h**: 헤더 가드를 `#ifndef HAL_DEBUG_H` 로 변경, 상단 주석 업데이트
- **hal_Debug.c**: `#include "hal_Debug.h"` 로 변경, 상단 주석 업데이트
- **csu_Debug.h**: 헤더 가드를 `#ifndef CSU_DEBUG_H` 로 변경, 상단 주석 업데이트
- **csu_Debug.c**: `#include "csu_Debug.h"` 로 변경, 상단 주석 업데이트
- **main_cpu1.h**: `#include "hal_Debug.h"` 및 `#include "csu_Debug.h"` 로 변경, 상단 주석 업데이트

## 비고
- 각 모듈의 함수명(예: `Initial_TIMER`, `Debug_Init` 등)이나 변수명은 HAL 및 CSU 명명 규칙에 따라 원래 모듈명 접두어가 없었으므로 내부 로직의 변수/함수명 변경은 필요 없습니다.
- 모든 파일은 인코딩 변경(UTF-8) 없이 안전하게 파일명 및 인클루드 경로만 치환됩니다.
