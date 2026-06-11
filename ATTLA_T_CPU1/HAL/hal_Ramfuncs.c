/**********************************************************************
 Nexcom Co., Ltd.
 Filename         : hal_Ramfuncs.c
 Version          : 00.00
 Description      : RAM 빌드 전용 Ramfuncs 더미 심볼 실제 정의 모듈
 Programmer       : Kim Jeonghwan
 Last Updated     : 2026. 06. 11. (주석 표준화 및 레거시 코드 정리)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 11. - 주석 표준화 및 레거시 코드 정리
 * 2026. 06. 11. - 파일 생성 및 기본 구조 작성
 */


/* ************************** [[   include  ]]  *********************************************************** */
#include "hal_Ramfuncs.h"

/* ************************** [[   global   ]]  *********************************************************** */
#ifndef _FLASH
/* RAM 빌드 전용: Ramfuncs Load/Run 더미 심볼 실제 정의 */
Uint16 RamfuncsLoadStart = 0U;
Uint16 RamfuncsLoadEnd   = 0U;
Uint16 RamfuncsLoadSize  = 0U;
Uint16 RamfuncsRunStart  = 0U;
Uint16 RamfuncsRunEnd    = 0U;
Uint16 RamfuncsRunSize   = 0U;
#endif /* #ifndef _FLASH */
