/**********************************************************************
   Nexcom Co., Ltd.
   Filename         : hal_DspInit.h
   Version          : 00.07
   Description      : CPU1 마스터 초기화 헤더
   Programmer       : Kim Jeonghwan
   Last Updated     : 2026. 07. 01. (헤더 버전 동기화 및 템플릿 유지)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 07. 01. - 헤더 버전 동기화 및 템플릿 유지 (코딩 규칙 적용)
 * 2026. 06. 30. - GPIO 145 핀을 CM_ALIVE_LED에서 LED_nG로 명칭 변경 및 제어권 수정
 * 2026. 06. 26. - 시스템 제어 및 스위치 GPIO 핀 매직넘버 상수화 추가
 * 2026. 06. 26. - 이더넷 PHY 핀 할당 변경 예정 주석 추가
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
#define GPIO_PIN_ENET_PHY_RESET         119U  /* TBD: PHY 하드웨어 리셋 (Active-Low) (※변경 예정: 67) */
#define GPIO_PIN_ENET_PHY_PWDN_INT      108U  /* TBD: PHY Power Down / Interrupt (입력) (※변경 예정: 68) */

/* MII 데이터 및 클럭 GPIO 핀 MUX용 상수 */
#define GPIO_PIN_MII_TX_CLK             44U   /* TBD (※변경 예정: 58) */
#define GPIO_PIN_MII_TX_EN              118U  /* TBD (※변경 예정: 56) */
#define GPIO_PIN_MII_TX_D0              75U   /* TBD (※변경 예정: 59) */
#define GPIO_PIN_MII_TX_D1              122U  /* TBD (※변경 예정: 60) */
#define GPIO_PIN_MII_TX_D2              123U  /* TBD (※변경 예정: 61) */
#define GPIO_PIN_MII_TX_D3              124U  /* TBD (※변경 예정: 62) */

#define GPIO_PIN_MII_RX_CLK             111U  /* TBD (※변경 예정: 49) */
#define GPIO_PIN_MII_RX_DV              112U  /* TBD (※변경 예정: 50) */
#define GPIO_PIN_MII_RX_ERR             113U  /* TBD (※변경 예정: 51) */
#define GPIO_PIN_MII_RX_D0              114U  /* TBD (※변경 예정: 52) */
#define GPIO_PIN_MII_RX_D1              115U  /* TBD (※변경 예정: 53) */
#define GPIO_PIN_MII_RX_D2              116U  /* TBD (※변경 예정: 54) */
#define GPIO_PIN_MII_RX_D3              117U  /* TBD (※변경 예정: 55) */

#define GPIO_PIN_MII_MDC_CLK            105U  /* TBD (※변경 예정: 42) */
#define GPIO_PIN_MII_MDIO_DATA          106U  /* TBD (※변경 예정: 43) */
#define GPIO_PIN_MII_CRS                109U  /* TBD (※변경 예정: 34) */
#define GPIO_PIN_MII_COL                110U  /* TBD (※변경 예정: 35) */

/* 평가보드 nG LED 상태 확인용 GPIO 핀 매핑 */
#define GPIO_PIN_LED_nG                 145U  /* nG 상태 표시용 LED (CPU1 제어) */
#define GPIO_PIN_CM_ETH_LED             146U  /* TBD: CM 이더넷 활동(Rx/Tx) 상태 표시용 LED */

/* ==========================================================================
 * 시스템 모니터링 및 모터 제어 GPIO 핀 번호 정의
 * ========================================================================== */
/* 디지털 입력 핀 (센서 및 스위치) */
#define GPIO_PIN_DRV_nFAULT             10U
#define GPIO_PIN_HALL_A_IN              11U
#define GPIO_PIN_HALL_B_IN              12U
#define GPIO_PIN_HALL_C_IN              13U

#define GPIO_PIN_nLIMIT1_NO             73U   /* ※TX_D0 중복 방지 테스트 임시, 향후 75로 변경 */
#define GPIO_PIN_nLIMIT1_NC             76U
#define GPIO_PIN_nLIMIT2_NO             77U
#define GPIO_PIN_nLIMIT2_NC             78U
#define GPIO_PIN_PM_n24V                79U
#define GPIO_PIN_nCABLE_LOOP            80U

#define GPIO_PIN_W6100_INTn             20U

/* 디지털 출력 핀 (제어 및 LED) */
#define GPIO_PIN_DRV_ENABLE             2U
#define GPIO_PIN_DRV_DIR                3U
#define GPIO_PIN_DRV_nBRAKE             4U

#define GPIO_PIN_W6100_RSTn             21U
#define GPIO_PIN_LED_nNORMAL            31U
#define GPIO_PIN_LED_nFAULT             32U
#define GPIO_PIN_LED_ISR_TEST           34U
#define GPIO_PIN_DSP_BRAKE              74U

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
