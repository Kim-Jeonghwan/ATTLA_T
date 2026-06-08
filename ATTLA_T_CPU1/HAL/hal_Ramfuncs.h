/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : hal_Ramfuncs.h
    Version          : 00.00
    Description      : RAM 빌드 전용 Ramfuncs 더미 심볼 정의 모듈
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 08. (주석 템플릿 일괄 적용)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026.06.04 - 최초 작성 (RAM 빌드 시 링커 미정의 심볼 에러 해소 목적)
 * 2026.06.04 - 중복 정의 해결을 위해 변수 실체를 hal_Ramfuncs.c로 이관 및 extern 선언으로 변경
 */

#ifndef HAL_RAMFUNCS_H
#define HAL_RAMFUNCS_H

/* ************************** [[   include   ]]  *********************************************************** */
#include "main.h"

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
