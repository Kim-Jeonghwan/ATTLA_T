/**********************************************************************
 Nexcom Co., Ltd.
 Filename         : wizchip_conf.h
 Version          : 00.07
 Description      : WIZnet 이더넷 라이브러리 파일
 Programmer       : Kim Jeonghwan
 Last Updated     : 2026. 06. 16. (컴파일 에러 해결을 위한 IPv6 Enum/함수 선언 제거)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 16. - 컴파일 에러 해결을 위한 미사용 IPv6 Enum(CNS_DAD 등) 및 함수(wizchip_dad) 선언 완전 제거
 * 2026. 06. 16. - 중복된 #endif 제거하여 컴파일 오류 재수정
 * 2026. 06. 16. - 잘못 포함된 #endif 제거하여 컴파일 오류 수정
 * 2026. 06. 16. - 누락된 열거형 멤버(CN_SET_PREFER 등) 주석 추가
 * 2026. 06. 16. - 영문 주석 한국어 번역 및 모든 함수에 CSU/HAL 주석 표준 포맷 적용
 * 2026. 06. 16. - 누락되었던 구조체/열거형 주석 및 함수 주석 추가 보완 
 * 2026. 06. 16. - 타기종(W5x00) 호환성 매크로, 미사용 IO 모드, IPv6 구조체 전면 삭제 (정적시험 규격 준수)
 * 2026. 06. 15. - 정적시험 통과를 위한 타기종 및 미사용 TCP/IPv6 기능 전면 삭제
 */

#ifndef _WIZCHIP_CONF_H_
#define _WIZCHIP_CONF_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "device.h" // C2000용 uint8_t 제공
#include <stdint.h>

/**
    @brief WIZCHIP 모델 선택
*/
#define W6100 6100
#define W6300 6300

#ifndef _WIZCHIP_
#define _WIZCHIP_ W6100 // 6100, 6300
#endif

#define _WIZCHIP_IO_MODE_NONE_ 0x0000
#define _WIZCHIP_IO_MODE_SPI_ 0x0200 ///< SPI 인터페이스 모드

#define _WIZCHIP_IO_MODE_SPI_VDM_                                              \
  (_WIZCHIP_IO_MODE_SPI_ + 1) ///< 가변 길이 데이터를 위한 SPI 인터페이스 모드

/**
    @brief PHY는 @ref _PHYCR0_, _PHYCR1_을 통해 접근 가능합니다.
    @details 하드웨어 접근 방식을 제공합니다. 소프트웨어 차지 메모리가 적습니다.
*/
#define _PHY_IO_MODE_PHYCR_ 0x0000

/**
    @brief PHY는 MII 인터페이스의 MDC/MDIO 신호를 통해 접근 가능합니다.
    @details 소프트웨어 접근 방식을 제공합니다.
*/
#define _PHY_IO_MODE_MII_ 0x0010

/**
    @brief PHY 접근 모드 선택
    @details _PHY_IO_MODE_를 통해 PHY 접근 방식을 결정합니다.
*/
#define _PHY_IO_MODE_ _PHY_IO_MODE_MII_ //_PHY_IO_MODE_MII_

#define _WIZCHIP_ID_ "W6100\0"

/**
    @brief _WIZCHIP_ 인터페이스 모드를 정의합니다.
*/
#define _WIZCHIP_IO_MODE_ _WIZCHIP_IO_MODE_SPI_VDM_

typedef uint8_t iodata_t;   ///< IO 접근 단위 (버스 너비)
typedef int16_t datasize_t; ///< 송신 또는 수신 데이터 크기
#include "w6100.h"

#define _WIZCHIP_IO_BASE_ 0x00000000

#define _WIZCHIP_SOCK_NUM_ 8 ///< WIZCHIP의 독립적인 소켓 개수

/********************************************************
    SPI, SDIO, I2C 등을 위한 WIZCHIP 기본 I/F 콜백 함수 구조체
*********************************************************/
/**
    @ingroup DATA_TYPE
    @brief WIZCHIP을 위한 기본 I/F 콜백 함수 포인터 구조체
*/
typedef struct __WIZCHIP {
  uint16_t if_mode; ///< 호스트 인터페이스 모드
  uint8_t id[8];    ///< WIZCHIP 칩 ID (예: 6100)

  /**
      크리티컬 섹션 제어 콜백 함수 포인터
  */
  struct _CRIS {
    void (*_enter)(void); ///< 크리티컬 섹션 진입
    void (*_exit)(void);  ///< 크리티컬 섹션 종료
  } CRIS;

  /**
      WIZCHIP 칩 선택(CS) 제어 콜백 함수 포인터
  */
  struct _CS {
    void (*_select)(void);   ///< WIZCHIP 선택 (CS Low)
    void (*_deselect)(void); ///< WIZCHIP 해제 (CS High)
  } CS;

  /**
      인터페이스 IO 제어 콜백 함수 포인터
  */
  union _IF {
    /**
        SPI 인터페이스용
    */
    struct {
      uint8_t (*_read_byte)(void);
      void (*_write_byte)(uint8_t wb);
      void (*_read_burst)(uint8_t *pBuf, uint16_t len);
      void (*_write_burst)(uint8_t *pBuf, uint16_t len);
    } SPI;
  } IF;
} _WIZCHIP;

extern _WIZCHIP WIZCHIP;

/**
    @ingroup DATA_TYPE
    @brief ctlwizchip() 함수에서 사용하는 WIZCHIP 제어 명령 타입
*/
typedef enum {
  CW_SYS_LOCK, ///< 시스템 칩, PHY, 네트워크 정보 설정을 잠금(Lock) 처리합니다.
  CW_SYS_UNLOCK,  ///< 잠금을 해제(Unlock)합니다.
  CW_GET_SYSLOCK, ///< 칩 잠금 상태를 가져옵니다.

  CW_RESET_WIZCHIP, ///< WIZCHIP을 소프트웨어적으로 리셋합니다.
  CW_INIT_WIZCHIP,  ///< 소켓 버퍼 크기를 초기화합니다.
  CW_GET_INTERRUPT, ///< 인터럽트 발생 상태를 가져옵니다.
  CW_CLR_INTERRUPT, ///< 인터럽트를 클리어합니다.
  CW_SET_INTRMASK,  ///< 인터럽트 마스크를 설정합니다.
  CW_GET_INTRMASK,  ///< 설정된 인터럽트 마스크를 가져옵니다.
  CW_SET_INTRTIME,  ///< 인터럽트 보류(Pending) 시간을 설정합니다.
  CW_GET_INTRTIME,  ///< 인터럽트 보류 시간을 가져옵니다.
  CW_SET_IEN,       ///< 글로벌 인터럽트를 활성화합니다.
  CW_GET_IEN,       ///< 글로벌 인터럽트 상태를 가져옵니다.

  CW_GET_ID,  ///< WIZCHIP의 이름을 가져옵니다.
  CW_GET_VER, ///< TCP/IP 엔진의 버전을 가져옵니다.

  CW_SET_SYSCLK, ///< 시스템 클럭 주파수(100MHz/25MHz)를 설정합니다.
  CW_GET_SYSCLK, ///< 시스템 클럭 설정을 가져옵니다.

  CW_RESET_PHY,      ///< 내장 PHY 모듈을 리셋합니다.
  CW_SET_PHYCONF,    ///< PHY 동작 모드(속도, Duplex, 자동협상)를 설정합니다.
  CW_GET_PHYCONF,    ///< 설정된 PHY 동작 모드를 가져옵니다.
  CW_GET_PHYSTATUS,  ///< 이더넷 링크 연결 시 실제 적용된 PHY 상태를 가져옵니다.
  CW_SET_PHYPOWMODE, ///< PHY의 전력 동작 모드(정상/절전)를 설정합니다.
  CW_GET_PHYPOWMODE, ///< PHY의 전력 동작 모드를 가져옵니다.
  CW_GET_PHYLINK     ///< PHY 링크 연결 상태(ON/OFF)를 가져옵니다.
} ctlwizchip_type;

/**
    @ingroup DATA_TYPE
    @brief ctlnetwork() 함수에서 사용하는 네트워크 제어 명령 타입
*/
typedef enum {
  CN_SET_NETINFO, ///< IP 주소 등의 네트워크 정보를 설정합니다.
  CN_GET_NETINFO, ///< 네트워크 정보를 가져옵니다.
  CN_SET_NETMODE, ///< WOL, PPPoE 등의 네트워크 모드를 설정합니다.
  CN_GET_NETMODE, ///< 네트워크 모드를 가져옵니다.
  CN_SET_TIMEOUT, ///< 재시도 횟수, 시간 등의 네트워크 타임아웃을 설정합니다.
  CN_GET_TIMEOUT, ///< 네트워크 타임아웃 설정을 가져옵니다.
  CN_SET_PREFER,  ///< 소켓리스 통신의 선호도를 설정합니다.
  CN_GET_PREFER,  ///< 설정된 소켓리스 통신의 선호도를 가져옵니다.
} ctlnetwork_type;

/**
    @ingroup DATA_TYPE
    @brief 네트워크 서비스 제어 명령 타입
*/
typedef enum {
  CNS_ARP,   ///< 목적지 IP에 대한 ARP 요청을 수행합니다.
  CNS_PING  ///< 목적지 IP에 대한 PING 요청을 수행합니다.
} ctlnetservice_type;

/**
    @ingroup DATA_TYPE
    @brief 인터럽트 비트 종류 (매스크, 클리어 등에 사용됨)
*/
typedef enum {
  IK_PPPOE_TERMINATED = (1 << 0), ///< PPPoE 종료 인터럽트
  IK_DEST_UNREACH = (1 << 1),     ///< 목적지 도달 불가(Unreachable) 인터럽트
  IK_IP_CONFLICT = (1 << 2),      ///< IP 주소 충돌 인터럽트
  IK_WOL = (1 << 7),              ///< 매직 패킷(WOL) 수신 인터럽트
  IK_NET_ALL = (0x97),            ///< 모든 네트워크 관련 인터럽트

  IK_SOCK_0 = (1 << 8),      ///< 소켓 0 인터럽트
  IK_SOCK_1 = (1 << 9),      ///< 소켓 1 인터럽트
  IK_SOCK_2 = (1 << 10),     ///< 소켓 2 인터럽트
  IK_SOCK_3 = (1 << 11),     ///< 소켓 3 인터럽트
  IK_SOCK_4 = (1 << 12),     ///< 소켓 4 인터럽트
  IK_SOCK_5 = (1 << 13),     ///< 소켓 5 인터럽트
  IK_SOCK_6 = (1 << 14),     ///< 소켓 6 인터럽트
  IK_SOCK_7 = (1 << 15),     ///< 소켓 7 인터럽트
  IK_SOCK_ALL = (0xFF << 8), ///< 전체 소켓 인터럽트 통합

  IK_SOCKL_TOUT = (1UL << 16),   ///< 소켓리스 타임아웃 인터럽트
  IK_SOCKL_ARP4 = (1UL << 17),   ///< 소켓리스 IPv4 ARP 인터럽트
  IK_SOCKL_PING4 = (1UL << 18),  ///< 소켓리스 IPv4 PING 인터럽트
  IK_SOCKL_ALL = (0xFFUL << 16), ///< 소켓리스 인터럽트 통합

  IK_INT_ALL = (0x00FFFF97) ///< 칩의 모든 인터럽트 비트 통합
} intr_kind;

#define SYS_CHIP_LOCK (1 << 2) ///< 시스템 칩 설정 잠금
#define SYS_NET_LOCK (1 << 1)  ///< 네트워크 설정 잠금
#define SYS_PHY_LOCK (1 << 0)  ///< PHY 제어 잠금

#define SYSCLK_100MHZ 0 ///< 시스템 클럭 100MHz
#define SYSCLK_25MHZ 1  ///< 시스템 클럭 25MHz

#define PHY_MODE_MANUAL 0   ///< 수동 PHY 동작 모드 설정
#define PHY_MODE_AUTONEGO 1 ///< PHY 자동 협상 모드 설정
#define PHY_MODE_TE 2

#define PHY_CONFBY_HW 0   ///< 하드웨어 핀 설정에 의한 PHY 모드 적용
#define PHY_CONFBY_SW 1   ///< 소프트웨어 레지스터 설정에 의한 PHY 모드 적용
#define PHY_SPEED_10 0    ///< 속도 10Mbps
#define PHY_SPEED_100 1   ///< 속도 100Mbps
#define PHY_DUPLEX_HALF 0 ///< 반이중 (Half-Duplex)
#define PHY_DUPLEX_FULL 1 ///< 전이중 (Full-Duplex)
#define PHY_LINK_OFF 0    ///< 링크 단절 상태
#define PHY_LINK_ON 1     ///< 링크 연결 상태
#define PHY_POWER_NORM 0  ///< 정상 전력 모드
#define PHY_POWER_DOWN 1  ///< 절전(파워 다운) 모드

/**
    @ingroup DATA_TYPE
    @brief 하드웨어 또는 소프트웨어에 의해 설정된 PHY의 실제 동작 상태 구조체
*/
typedef struct wiz_PhyConf_t {
  uint8_t by;     ///< H/W 핀 또는 S/W 레지스터로 설정되었는지 여부
  uint8_t mode;   ///< 수동 설정 또는 자동 협상 모드 여부
  uint8_t speed;  ///< 10Mbps 또는 100Mbps 속도
  uint8_t duplex; ///< 반이중 또는 전이중 모드
} wiz_PhyConf;

/**
    @ingroup DATA_TYPE
    @brief IP 주소 구성 모드 (DHCP 사용 여부)
*/
typedef enum {
  NETINFO_NONE = 0x00,      ///< DHCP를 사용하지 않음
  NETINFO_STATIC_V4 = 0x01, ///< IPv4 주소를 수동(정적)으로 설정함
  NETINFO_DHCP_V4 = 0x10    ///< DHCP 서버로부터 동적으로 IPv4 주소를 할당받음
} ipconf_mode;

/**
    @ingroup DATA_TYPE
    @brief DHCP 동작 모드
*/
typedef enum {
  NETINFO_STATIC = 1, ///< 정적 IP 설정
  NETINFO_DHCP        ///< 동적(DHCP) IP 설정
} dhcp_mode;

/**
    @ingroup DATA_TYPE
    @brief WIZCHIP 네트워크 정보 구성 구조체
*/
typedef struct wiz_NetInfo_t {
  uint8_t mac[6];     ///< 출발지(기기) MAC 주소
  uint8_t ip[4];      ///< 출발지 IPv4 주소
  uint8_t sn[4];      ///< 서브넷 마스크 주소
  uint8_t gw[4];      ///< 게이트웨이 IPv4 주소
  uint8_t dns[4];     ///< DNS 서버 IPv4 주소
  ipconf_mode ipmode; ///< IP 구성 모드 (정적/동적)
  dhcp_mode dhcp;     ///< DHCP 사용 여부 (1: 정적, 2: 동적)
} wiz_NetInfo;

/**
    @ingroup DATA_TYPE
    @brief 각종 네트워크 특수 모드 제어 플래그
*/
typedef enum {
  NM_IPB_V4 = (1 << 0), ///< IPv4 패킷 차단 모드
  NM_WOL = (1 << 2),    ///< Wake On Lan 활성화
  NM_MR_MASK = (0x05),  ///< NETMR 마스크

  NM_PPPoE = (1 << 8),       ///< PPPoE 모드
  NM_DHA_SELECT = (1 << 15), ///< 목적지 하드웨어 주소 강제 지정
  NM_MR2_MASK = (0x09 << 8), ///< NETMR2 마스크

  NM_PB4_ALL = (1UL << 16),      ///< 모든 IPv4 PING 요청 차단
  NM_TRSTB_V4 = (1UL << 17),     ///< IPv4 전송용 TCP RST 패킷 차단
  NM_PARP_V4 = (1UL << 18),      ///< IPv4 PING 응답 전 ARP 요청
  NM_UNRB_V4 = (1UL << 19),      ///< 목적지 도달 불가(Unreachable) 차단
  NM_NET4_MASK = (0x0FUL << 16), ///< NET4MR 마스크

  NM_MASK_ALL = (0x000F0905) ///< 전체 설정 마스크
} netmode_type;

/**
    @ingroup DATA_TYPE
    @brief WIZCHIP 패킷 재전송 타임아웃 구성 구조체
*/
typedef struct wiz_NetTimeout_t {
  uint8_t s_retry_cnt;    ///< 소켓 통신의 기본 재시도 횟수
  uint16_t s_time_100us;  ///< 소켓 통신의 재전송 시간 (100us 단위)
  uint8_t sl_retry_cnt;   ///< 소켓리스 통신의 재시도 횟수
  uint16_t sl_time_100us; ///< 소켓리스 통신의 재전송 시간 (100us 단위)
} wiz_NetTimeout;

/**
    @ingroup DATA_TYPE
    @brief 네트워크 서비스를 위한 IP 주소 보관 구조체
*/
typedef struct wiz_IPAddress_t {
  uint8_t ip[16]; ///< 주소 버퍼 배열
  uint8_t len;    ///< 사용되는 주소의 길이
} wiz_IPAddress;

/**
    @ingroup DATA_TYPE
    @brief 소켓리스 ARP 프로세스를 위한 목적지 정보 보관 구조체
*/
typedef struct wiz_ARP_t {
  wiz_IPAddress destinfo; ///< ARP 요청을 전송할 목적지 IP 주소
  uint8_t dha[6]; ///< ARP 응답 수신 시 기록될 목적지의 하드웨어 MAC 주소
} wiz_ARP;

/**
    @ingroup DATA_TYPE
    @brief 소켓리스 PING 프로세스를 위한 정보 보관 구조체
*/
typedef struct wiz_PING_t {
  uint16_t id;            ///< PING 세션 ID
  uint16_t seq;           ///< PING 시퀀스 번호
  wiz_IPAddress destinfo; ///< PING을 요청할 목적지 IP 주소
} wiz_PING;

/*
@function    void reg_wizchip_cris_cbfunc(void (*cris_en)(void), void
(*cris_ex)(void))
@brief      WIZCHIP 레지스터 읽기/쓰기 시 동시 접근을 막기 위한 크리티컬 섹션
콜백 함수를 등록합니다.
@param      cris_en: 크리티컬 섹션 진입(Enter) 콜백 포인터
@param      cris_ex: 크리티컬 섹션 종료(Exit) 콜백 포인터
@return     void
*/
void reg_wizchip_cris_cbfunc(void (*cris_en)(void), void (*cris_ex)(void));

/*
@function    void reg_wizchip_cs_cbfunc(void (*cs_sel)(void), void
(*cs_desel)(void))
@brief      통신을 시작하고 끝낼 때 호출되는 WIZCHIP 칩 선택(CS) 콜백 함수를
등록합니다.
@param      cs_sel: 칩 선택(CS Low) 콜백 포인터
@param      cs_desel: 칩 해제(CS High) 콜백 포인터
@return     void
*/
void reg_wizchip_cs_cbfunc(void (*cs_sel)(void), void (*cs_desel)(void));

/*
@function    void reg_wizchip_spi_cbfunc(uint8_t (*spi_rb)(void), void
(*spi_wb)(uint8_t wb), void (*spi_rbuf)(uint8_t *buf, datasize_t len), void
(*spi_wbuf)(uint8_t *buf, datasize_t len))
@brief      SPI 통신을 통한 1바이트 및 버스트 단위 읽기/쓰기 콜백 함수를
등록합니다.
@param      spi_rb: 1바이트 읽기 함수 포인터
@param      spi_wb: 1바이트 쓰기 함수 포인터
@param      spi_rbuf: 버스트 데이터 읽기 함수 포인터
@param      spi_wbuf: 버스트 데이터 쓰기 함수 포인터
@return     void
*/
void reg_wizchip_spi_cbfunc(uint8_t (*spi_rb)(void), void (*spi_wb)(uint8_t wb),
                            void (*spi_rbuf)(uint8_t *buf, uint16_t len),
                            void (*spi_wbuf)(uint8_t *buf, uint16_t len));

/*
@function    void reg_wizchip_spiburst_cbfunc(void (*spi_rb)(uint8_t *pBuf,
uint16_t len), void (*spi_wb)(uint8_t *pBuf, uint16_t len))
@brief      SPI 버스트 읽기/쓰기 콜백 함수만 별도로 등록합니다.
@param      spi_rb: 버스트 읽기 함수 포인터
@param      spi_wb: 버스트 쓰기 함수 포인터
@return     void
*/
void reg_wizchip_spiburst_cbfunc(void (*spi_rb)(uint8_t *pBuf, uint16_t len),
                                 void (*spi_wb)(uint8_t *pBuf, uint16_t len));

/*
@function    int8_t ctlwizchip(ctlwizchip_type cwtype, void *arg)
@brief      WIZCHIP 하드웨어(PHY 리셋, 인터럽트 제어 등)를 제어하는 통합 관리
함수입니다.
@param      cwtype: 실행할 명령의 타입 (ctlwizchip_type 참고)
@param      arg: 명령에 따라 요구되는 인자 구조체 포인터
@return     성공 시 0 반환, 잘못된 명령일 경우 -1 반환
*/
int8_t ctlwizchip(ctlwizchip_type cwtype, void *arg);

/*
@function    int8_t ctlnetwork(ctlnetwork_type cntype, void *arg)
@brief      WIZCHIP 네트워크 환경(IP, 서브넷, 타임아웃, 특수 모드 등)을
설정하거나 가져오는 통합 함수입니다.
@param      cntype: 실행할 명령의 타입 (ctlnetwork_type 참고)
@param      arg: 명령에 따라 요구되는 인자 구조체 포인터
@return     성공 시 0 반환, 잘못된 명령일 경우 -1 반환
*/
int8_t ctlnetwork(ctlnetwork_type cntype, void *arg);

/*
@function    void wizchip_sw_reset(void)
@brief      소프트웨어 레지스터 명령을 통해 WIZCHIP을 리셋합니다.
@param      void
@return     void
*/
void wizchip_sw_reset(void);

/*
@function    int8_t wizchip_init(uint8_t *txsize, uint8_t *rxsize)
@brief      8개의 소켓에 대한 송신(TX) 버퍼와 수신(RX) 버퍼 크기를 할당하여
WIZCHIP을 초기화합니다.
@param      txsize: 각 소켓에 할당할 송신 버퍼 크기 배열 포인터. (NULL 전달 시
기본 2KB 할당)
@param      rxsize: 각 소켓에 할당할 수신 버퍼 크기 배열 포인터. (NULL 전달 시
기본 2KB 할당)
@return     성공 시 0 반환, 총 버퍼 메모리 한계를 초과하여 할당 실패 시 -1 반환
*/
int8_t wizchip_init(uint8_t *txsize, uint8_t *rxsize);

/*
@function    void wizchip_clrinterrupt(intr_kind intr)
@brief      지정된 종류의 WIZCHIP 인터럽트를 클리어(해제)합니다.
@param      intr: 클리어할 인터럽트 종류를 지정 (intr_kind 값을 OR 연산하여 사용
가능)
@return     void
*/
void wizchip_clrinterrupt(intr_kind intr);

/*
@function    intr_kind wizchip_getinterrupt(void)
@brief      칩셋에서 발생한 모든 인터럽트의 상태를 읽어옵니다.
@param      void
@return     현재 발생한 인터럽트들의 조합 값 (intr_kind의 OR 연산 결과)
*/
intr_kind wizchip_getinterrupt(void);

/*
@function    void wizchip_setinterruptmask(intr_kind intr)
@brief      외부 인터럽트 핀으로 신호를 발생시킬 인터럽트들을 마스크 설정합니다.
@param      intr: 마스킹을 허용할 인터럽트 종류들을 지정 (intr_kind 값을 OR
연산하여 사용)
@return     void
*/
void wizchip_setinterruptmask(intr_kind intr);

/*
@function    intr_kind wizchip_getinterruptmask(void)
@brief      현재 설정된 인터럽트 마스크 상태를 읽어옵니다.
@param      void
@return     현재 설정된 마스크 값 (intr_kind의 OR 연산 결과)
*/
intr_kind wizchip_getinterruptmask(void);

/*
@function    int8_t wizphy_getphylink(void)
@brief      현재 이더넷 이더넷 케이블의 링크 연결 상태(UP/DOWN)를 읽어옵니다.
@param      void
@return     링크 연결됨(1), 링크 끊어짐(0) 반환
*/
int8_t wizphy_getphylink(void);

/*
@function    int8_t wizphy_getphypmode(void)
@brief      내장 PHY 모듈의 동작 파워 모드(절전/정상 모드) 상태를 읽어옵니다.
@param      void
@return     정상 파워 모드(0), 파워 다운 절전 모드(1) 반환
*/
int8_t wizphy_getphypmode(void);

/*
@function    void wizphy_reset(void)
@brief      MDC/MDIO 또는 자체 통신 인터페이스를 통해 내장된 이더넷 PHY 모듈을
초기화합니다.
@param      void
@return     void
*/
void wizphy_reset(void);

/*
@function    void wizphy_setphyconf(wiz_PhyConf *phyconf)
@brief      내장된 이더넷 PHY의 속도(10/100) 및 통신 방식(Auto, Full/Half)
모드를 설정합니다.
@param      phyconf: 설정할 PHY 구성 정보 구조체 포인터
@return     void
*/
void wizphy_setphyconf(wiz_PhyConf *phyconf);

/*
@function    void wizphy_getphyconf(wiz_PhyConf *phyconf)
@brief      사용자가 설정한 이더넷 PHY 동작 모드 설정값을 가져옵니다. (실제 적용
상태와는 다를 수 있음)
@param      phyconf: 설정값을 반환받을 구조체 포인터
@return     void
*/
void wizphy_getphyconf(wiz_PhyConf *phyconf);

/*
@function    void wizphy_getphystat(wiz_PhyConf *phyconf)
@brief      링크가 연결된 경우 자동 협상 등에 의해 '실제' 결정된 PHY의 동작 속도
및 통신 모드 상태를 읽어옵니다.
@param      phyconf: 실제 적용 상태를 반환받을 구조체 포인터
@return     void
*/
void wizphy_getphystat(wiz_PhyConf *phyconf);

/*
@function    void wizphy_setphypmode(uint8_t pmode)
@brief      이더넷 PHY 전력 동작 모드를 설정하여 칩의 클럭 소비 전력을
최소화합니다.
@param      pmode: 정상 동작 모드(PHY_POWER_NORM) 또는 절전 모드(PHY_POWER_DOWN)
@return     void
*/
void wizphy_setphypmode(uint8_t pmode);

/*
@function    void wizchip_setnetinfo(wiz_NetInfo *pnetinfo)
@brief      WIZCHIP 레지스터에 IP 주소, 서브넷 마스크, 게이트웨이 등의 출발지
네트워크 정보를 설정합니다.
@param      pnetinfo: 설정할 네트워크 정보가 담긴 구조체 포인터
@return     void
*/
void wizchip_setnetinfo(wiz_NetInfo *pnetinfo);

/*
@function    void wizchip_getnetinfo(wiz_NetInfo *pnetinfo)
@brief      WIZCHIP 레지스터에 현재 설정된 출발지 네트워크 정보(IP, 서브넷 등)를
읽어옵니다.
@param      pnetinfo: 읽어온 네트워크 정보를 반환받을 구조체 포인터
@return     void
*/
void wizchip_getnetinfo(wiz_NetInfo *pnetinfo);

/*
@function    void wizchip_setnetmode(netmode_type netmode)
@brief      네트워크 모드(WOL 허용 여부, PING 응답 차단 여부 등)를 비트 마스크
형태로 설정합니다.
@param      netmode: 설정할 네트워크 모드의 비트 플래그 조합
@return     void
*/
void wizchip_setnetmode(netmode_type netmode);

/*
@function    netmode_type wizchip_getnetmode(void)
@brief      현재 설정된 네트워크 모드(WOL, PING 차단 여부 등) 상태를 가져옵니다.
@param      void
@return     현재 설정된 네트워크 모드 반환
*/
netmode_type wizchip_getnetmode(void);

/*
@function    void wizchip_settimeout(wiz_NetTimeout *nettime)
@brief      네트워크 패킷 통신 실패 시 패킷을 재전송할 타임아웃 시간 단위 및
재시도 횟수를 설정합니다.
@param      nettime: 재전송 횟수 및 시간 값을 포함한 설정 구조체 포인터
@return     void
*/
void wizchip_settimeout(wiz_NetTimeout *nettime);

/*
@function    void wizchip_gettimeout(wiz_NetTimeout *nettime)
@brief      설정된 네트워크 통신 재전송 타임아웃 정보(시간, 횟수)를 읽어옵니다.
@param      nettime: 읽어온 타임아웃 정보를 반환받을 구조체 포인터
@return     void
*/
void wizchip_gettimeout(wiz_NetTimeout *nettime);

/*
@function    int8_t wizchip_arp(wiz_ARP *arp)
@brief      소켓 없이 자체 기능(소켓리스)으로 목적지 IP에 대한 MAC 주소를
요청하는 ARP 프로토콜을 수행합니다.
@param      arp: 목적지 IP를 담고, 응답 시 목적지의 MAC 하드웨어 주소가 저장될
구조체 포인터
@return     성공 시 0(정상 MAC 획득), 타임아웃 등 실패 시 -1 반환
*/
int8_t wizchip_arp(wiz_ARP *arp);

/*
@function    int8_t wizchip_ping(wiz_PING *ping)
@brief      소켓 없이 특정 목적지에 대해 PING 요청(ICMP Echo Request)을 전송하여
네트워크 상태를 점검합니다.
@param      ping: 전송할 PING ID와 시퀀스 정보가 담긴 구조체 포인터
@return     성공 시 0(정상 Ping 응답 획득), 타임아웃 등 실패 시 -1 반환
*/
int8_t wizchip_ping(wiz_PING *ping);



#ifdef __cplusplus
}
#endif

#endif // _WIZCHIP_CONF_H_
