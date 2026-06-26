/**********************************************************************
   Nexcom Co., Ltd.
   Filename         : main_cm.h
   Version          : 00.02
   Description      : CM 코어 전역 헤더 관리 파일
   Programmer       : Kim Jeonghwan
   Last Updated     : 2026. 06. 26. (hal_Timer_cm 리팩토링 반영)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 26. - hal_Timer_cm 리팩토링 반영
 * 2026. 06. 24. - 이더넷 파일명 리팩토링 적용
 * 2026. 06. 23. - CM 코어 기동 및 동기화 구현을 위한 전역 헤더 생성
 */

#ifndef MAIN_CM_H
#define MAIN_CM_H

/* 표준 라이브러리 */
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* C28x 호환용 자료형 정의 */
typedef float float32_t;

/* CM Core Driverlib */
#include "driverlib_cm.h"
#include "cm.h"

/* HAL 계층 */
#include "hal_Ipc_cm.h"
#include "hal_Ethernet_cm.h"
#include "hal_Timer_cm.h"

/* CSU 계층 */
#include "csu_Ipc_cm.h"
#include "csu_Ethernet_cm.h"

#endif // MAIN_CM_H
