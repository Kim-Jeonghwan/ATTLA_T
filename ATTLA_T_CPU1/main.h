/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : main.h
    Version          : 00.00
    Description      : 코어 시스템 통합 헤더 파일
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 08. (주석 템플릿 일괄 적용)
**********************************************************************/

#ifndef MAIN_H
#define MAIN_H

/* ************************** [[   include  ]]  *********************************************************** */
/* 표준 라이브러리 */
#include <string.h>
#include <math.h>
#include <stdint.h>
#include <stdbool.h>

/* Driverlib 및 Device 기본 정의 */
// _DUAL_HEADERS가 선언되어 있어야 두 방식을 병행 가능합니다.
#include "driverlib.h"
#include "device.h"
#include "memcfg.h"
#ifndef MEMCFG_GSRAMMASTER_CM
#define MEMCFG_GSRAMMASTER_CM    2
#endif

/* Bit-field 헤더 포함 */
// Uint16, Uint32 타입 및 비트필드 레지스터 구조체(AdcaRegs, GpioDataRegs 등) 제공
#include "f28x_project.h"

/* easyDSP Library */
#include "easy28x_driverlib_v12.2.h"

/* Biss-C Library */
#include "pm_bissc_include.h"
#include "pm_bissc_internal_include.h"

/* W6100 Library */
#include "socket.h"
#include "w6100.h"
#include "wizchip_conf.h"

/* HAL Library */
#include "hal_Adc.h"
#include "hal_Common.h"
#include "hal_DspInit.h"
#include "hal_Encoder.h"
#include "hal_EpwmTimer.h"    /* EPWM1 기반 2ms 타이머 */
#include "hal_Ramfuncs.h"
#include "hal_Sci.h"
#include "hal_Spi.h"
#include "hal_Timer.h"
#include "hal_W6100.h"

/* CSU Library */
#include "csu_Adc.h"
#include "csu_Encoder.h"
#include "csu_Epwm.h"
#include "csu_Ethernet.h"
#include "csu_Fram.h"
#include "csu_Led.h"
#include "csu_SciPc.h"


/* ************************** [[   define   ]]  *********************************************************** */
//typedef uint8_t   Uint8; 



/* ************************** [[   enum or struct   ]]  *************************************************** */



/* ************************** [[   global   ]]  *********************************************************** */



/* ************************** [[  function  ]]  *********************************************************** */
// DSP program entry point
void main(void);


#endif	// #ifndef MAIN_H

