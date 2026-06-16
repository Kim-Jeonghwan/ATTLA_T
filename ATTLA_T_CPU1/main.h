/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : main.h
    Version          : 00.04
    Description      : 전역 헤더 관리 파일
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 16. (csu_Ethernet.h 인클루드 추가)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 16. - 이더넷 프로토콜 추가에 따른 csu_Ethernet.h 인클루드 추가
 * 2026. 06. 15. - 리팩토링에 따라 삭제된 hal_Led.h 인클루드 제거
 * 2026. 06. 12. - csu_Dio.h 인클루드 추가
 * 2026. 06. 12. - CM 및 IPC 관련 매크로 제거
 * 2026. 06. 11. - 주석 표준화 및 레거시 코드 정리
 * 2026. 06. 11. - 파일 생성 및 기본 구조 작성
 */


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

/* Bit-field 헤더 포함 */
// Uint16, Uint32 타입 및 비트필드 레지스터 구조체(AdcaRegs, GpioDataRegs 등) 제공
#include "f28x_project.h"

/* easyDSP Library */
#include "easy28x_driverlib_v12.2.h"

/* W6100 Library */
#include "socket.h"
#include "w6100.h"
#include "wizchip_conf.h"

/* HAL Library */
#include "hal_Adc.h"
#include "hal_Common.h"
#include "hal_DspInit.h"
#include "hal_Encoder.h"
#include "hal_Epwm.h"    /* EPWM1 기반 2ms 타이머 */
#include "hal_Fram.h"
#include "hal_MotorDriver.h"
#include "hal_Ramfuncs.h"
#include "hal_Sci.h"
#include "hal_Spi.h"
#include "hal_Timer.h"
#include "hal_Ethernet.h"

/* CSU Library */
#include "csu_Dio.h"
#include "csu_Adc.h"
#include "csu_Encoder.h"
#include "csu_Led.h"
#include "csu_Control.h"
#include "csu_MotorDriver.h"
#include "csu_MotorCtrl.h"
#include "csu_PID.h"
#include "csu_Bit.h"
#include "csu_SciPc.h"
#include "csu_Ethernet.h"


/* ************************** [[   define   ]]  *********************************************************** */
//typedef uint8_t   Uint8; 



/* ************************** [[   enum or struct   ]]  *************************************************** */



/* ************************** [[   global   ]]  *********************************************************** */



/* ************************** [[  function  ]]  *********************************************************** */
/**
 * @brief CPU1 코어 메인 엔트리 포인트 및 백그라운드 태스크 스케줄러
 * @param void
 * @return void
 */
void main(void);


#endif	// #ifndef MAIN_H

