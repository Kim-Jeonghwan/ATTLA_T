/**********************************************************************
 Nexcom Co., Ltd.
 Filename         : hal_Ramfuncs.c
 Version          : 00.01
 Description      : RAM 빌드 전용 Ramfuncs 더미 심볼 실제 정의 모듈
 Programmer       : Kim Jeonghwan
 Last Updated     : 2026. 07. 01. (초기화 구문 상세 한글 주석 추가)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 07. 01. - 변수 및 초기화 구문 상세 한글 주석 추가 (코딩 규칙 적용)
 * 2026. 06. 11. - 주석 표준화 및 레거시 코드 정리
 * 2026. 06. 11. - 파일 생성 및 기본 구조 작성
 */


/* ************************** [[   include  ]]  *********************************************************** */
#include "hal_Ramfuncs.h"

/* ************************** [[   global   ]]  *********************************************************** */
#ifndef _FLASH
/* RAM 빌드 전용: Ramfuncs Load/Run 더미 심볼 실제 정의 */
Uint16 RamfuncsLoadStart = 0U; /* .TI.ramfunc 섹션 Flash 로드 시작 주소 (RAM 빌드 미사용, 더미 0 초기화) */
Uint16 RamfuncsLoadEnd   = 0U; /* .TI.ramfunc 섹션 Flash 로드 종료 주소 (RAM 빌드 미사용, 더미 0 초기화) */
Uint16 RamfuncsLoadSize  = 0U; /* .TI.ramfunc 섹션 Flash 로드 크기     (RAM 빌드 미사용, 더미 0 초기화) */
Uint16 RamfuncsRunStart  = 0U; /* .TI.ramfunc 섹션 RAM 실행 시작 주소  (RAM 빌드 미사용, 더미 0 초기화) */
Uint16 RamfuncsRunEnd    = 0U; /* .TI.ramfunc 섹션 RAM 실행 종료 주소  (RAM 빌드 미사용, 더미 0 초기화) */
Uint16 RamfuncsRunSize   = 0U; /* .TI.ramfunc 섹션 RAM 실행 크기       (RAM 빌드 미사용, 더미 0 초기화) */
#endif /* #ifndef _FLASH */
