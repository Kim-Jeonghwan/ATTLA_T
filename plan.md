# 함수명 접두어(csu_, hal_) 제거 리팩토링 계획

## 1. 개요
사용자의 지시 및 `GEMINI.md`의 **"프로젝트 계층 구조 (Project Architecture)"** 명명 규칙에 따라, `CSU` 및 `HAL` 계층의 파일명/모듈명에만 접두어를 허용하고 함수명/변수명에는 접두어(`csu_`, `hal_`) 사용을 제거하기 위한 코드 수정 계획입니다.

## 2. 수정 대상 함수 목록 (총 7건)
| 기존 함수명 | 변경될 함수명 | 수정 대상 파일 |
|---|---|---|
| `csu_Bit_RunPBIT` | `Bit_RunPBIT` | `csu_Bit.h`, `csu_Bit.c`, `csu_Control.c` |
| `csu_Bit_RunCBIT` | `Bit_RunCBIT` | `csu_Bit.h`, `csu_Bit.c`, `csu_Control.c` |
| `csu_Control_CalibrateCurrentOffset` | `Control_CalibrateCurrentOffset` | `csu_Control.h`, `csu_Control.c` |
| `csu_Control_SystemOperation` | `Control_SystemOperation` | `csu_Control.h`, `csu_Control.c`, `hal_Adc.c` |
| `hal_Led_InitGpio` | `Led_InitGpio` | `hal_Led.h`, `hal_Led.c`, `hal_DspInit.c` |
| `hal_Led_WritePin` | `Led_WritePin` | `hal_Led.h`, `hal_Led.c`, `csu_Led.c` |
| `hal_Led_TogglePin` | `Led_TogglePin` | `hal_Led.h`, `hal_Led.c`, `csu_Led.c` |

## 3. 작업 순서
1. **[CSU 계층 수정]**: `csu_Bit.h/c`, `csu_Control.h/c`에 선언 및 정의된 4개의 함수 이름 변경.
2. **[HAL 계층 수정]**: `hal_Led.h/c`에 선언 및 정의된 3개의 함수 이름 변경.
3. **[호출부 일괄 업데이트]**: 위 함수들을 호출하고 있는 `csu_Led.c`, `hal_Adc.c`, `hal_DspInit.c` 내부의 함수 호출명 일괄 변경.
4. **[헤더 주석 업데이트]**: **"1. Header Comment Auto-Update"** 룰에 따라 변경이 발생한 모든 파일의 최상단 주석을 업데이트하고 버전을 +0.01 올리며 Modification History 기록.

## 4. 검증 계획
- 사용자에게 `CCS Theia` 환경에서 컴파일(Build)을 수행하도록 안내하여 의존성 및 선언 오류가 없는지 최종 확인합니다.

> [!NOTE]
> 사용자가 `plan.md`의 내용을 확인하고 피드백을 주시거나 승인(수정 진행해)해주시면 즉시 실제 코드 수정을 시작하겠습니다.
