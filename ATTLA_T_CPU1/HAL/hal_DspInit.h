/**********************************************************************
   Nexcom Co., Ltd.
   Filename         : hal_DspInit.h
   Version          : 00.03
   Description      : CPU1 마스터 초기화 헤더
   Programmer       : Kim Jeonghwan
   Last Updated     : 2026. 06. 23. (main.h -> main_cpu1.h 인클루드 명칭 리팩토링)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 23. - main.h -> main_cpu1.h 인클루드 명칭 리팩토링
 * 2026. 06. 23. - CM 코어 기동 및 DP83822 이더넷 PHY MII 핀 매핑 추가
 * 2026. 06. 11. - 주석 표준화 및 레거시 코드 정리
 */

#ifndef HAL_DSPINIT_H
#define HAL_DSPINIT_H

/* ************************** [[   include  ]]  *********************************************************** */
#include "main_cpu1.h"

/* ************************** [[   define   ]]  *********************************************************** */
/* ==========================================================================
 * [TBD] DP83822 PHY 칩 MII 연결용 DSP 할당 GPIO 핀 번호 정의
 * ⚠️ 아래 핀 매핑은 현재 회로 설계 미정으로 임시 할당된 값입니다.
 * ⚠️ 추후 회로도 확정 시 반드시 실제 연결에 맞게 변경해야 합니다.
 * ========================================================================== */
#define GPIO_PIN_ENET_PHY_RESET         119U  /* TBD: PHY 하드웨어 리셋 (Active-Low) */
#define GPIO_PIN_ENET_PHY_PWDN_INT      108U  /* TBD: PHY Power Down / Interrupt (입력) */

/* MII 데이터 및 클럭 GPIO 핀 MUX용 상수 */
#define GPIO_PIN_MII_TX_CLK             44U   /* TBD */
#define GPIO_PIN_MII_TX_EN              118U  /* TBD */
#define GPIO_PIN_MII_TX_D0              75U   /* TBD */
#define GPIO_PIN_MII_TX_D1              122U  /* TBD */
#define GPIO_PIN_MII_TX_D2              123U  /* TBD */
#define GPIO_PIN_MII_TX_D3              124U  /* TBD */

#define GPIO_PIN_MII_RX_CLK             111U  /* TBD */
#define GPIO_PIN_MII_RX_DV              112U  /* TBD */
#define GPIO_PIN_MII_RX_ERR             113U  /* TBD */
#define GPIO_PIN_MII_RX_D0              114U  /* TBD */
#define GPIO_PIN_MII_RX_D1              115U  /* TBD */
#define GPIO_PIN_MII_RX_D2              116U  /* TBD */
#define GPIO_PIN_MII_RX_D3              117U  /* TBD */

#define GPIO_PIN_MII_MDC_CLK            105U  /* TBD */
#define GPIO_PIN_MII_MDIO_DATA          106U  /* TBD */
#define GPIO_PIN_MII_CRS                109U  /* TBD */
#define GPIO_PIN_MII_COL                110U  /* TBD */

/* CM LED 상태 확인용 GPIO 핀 매핑 */
#define GPIO_PIN_CM_ALIVE_LED           145U  /* TBD: CM 코어 Alive 상태 표시용 LED */
#define GPIO_PIN_CM_ETH_LED             146U  /* TBD: CM 이더넷 활동(Rx/Tx) 상태 표시용 LED */

/* ************************** [[   enum or struct   ]]  *************************************************** */

/* ************************** [[   global   ]]  *********************************************************** */

/* ************************** [[  function  ]]  *********************************************************** */
/**
 * @brief      DSP 및 주변장치 초기화 수행의 진입점
 * @param      void
 * @return     void
 */
void System_Initialization(void);

#endif	// #ifndef HAL_DSPINIT_H
