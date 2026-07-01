/**********************************************************************
 Nexcom Co., Ltd.
 Filename         : hal_Ramfuncs.h
 Version          : 00.02
 Description      : RAM 빌드 전용 Ramfuncs 더미 심볼 정의 모듈
 Programmer       : Kim Jeonghwan
 Last Updated     : 2026. 07. 01. (헤더 버전 동기화 및 템플릿 유지)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 07. 01. - 헤더 버전 동기화 및 템플릿 유지 (코딩 규칙 적용)
 * 2026. 06. 23. - main.h -> main_cpu1.h 인클루드 명칭 리팩토링
 * 2026. 06. 11. - 주석 표준화 및 레거시 코드 정리
 * 2026. 06. 04. - 최초 작성 (RAM 빌드 시 링커 미정의 심볼 에러 해소 목적)
 * 2026. 06. 04. - 중복 정의 해결을 위해 변수 실체를 hal_Ramfuncs.c로 이관 및 extern 선언으로 변경
 */

#ifndef HAL_RAMFUNCS_H
#define HAL_RAMFUNCS_H

/* ************************** [[   include   ]]  *********************************************************** */
#include "main_cpu1.h"

/* ************************** [[   define    ]]  *********************************************************** */

/* ************************** [[  enum/struct ]]  *********************************************************** */

/* ************************** [[   global    ]]  *********************************************************** */
#ifndef _FLASH
/*
 * RAM 빌드 전용: RamfuncsLoad/Run 더미 심볼 extern 선언
 */
extern Uint16 RamfuncsLoadStart; /* .TI.ramfunc 섹션 Flash 로드 시작 주소 (RAM 빌드 미사용) */
extern Uint16 RamfuncsLoadEnd;   /* .TI.ramfunc 섹션 Flash 로드 종료 주소 (RAM 빌드 미사용) */
extern Uint16 RamfuncsLoadSize;  /* .TI.ramfunc 섹션 Flash 로드 크기     (RAM 빌드 미사용) */
extern Uint16 RamfuncsRunStart;  /* .TI.ramfunc 섹션 RAM 실행 시작 주소  (RAM 빌드 미사용) */
extern Uint16 RamfuncsRunEnd;    /* .TI.ramfunc 섹션 RAM 실행 종료 주소  (RAM 빌드 미사용) */
extern Uint16 RamfuncsRunSize;   /* .TI.ramfunc 섹션 RAM 실행 크기       (RAM 빌드 미사용) */
#endif /* #ifndef _FLASH */

/* ************************** [[  function   ]]  *********************************************************** */

#endif  // #ifndef HAL_RAMFUNCS_H
