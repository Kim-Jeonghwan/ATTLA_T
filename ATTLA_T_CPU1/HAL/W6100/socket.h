/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : socket.h
    Version          : 00.04
    Description      : WIZnet 이더넷 라이브러리 헤더 파일
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 16. (UDP 단일화 및 미사용 통신 로직 전면 삭제)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 16. - UDP 단일화 및 불필요한 통신 로직 전면 삭제
 * 2026. 06. 16. - 영문 주석 한국어 번역 및 CSU/HAL 주석 표준 포맷(@function 등) 적용
 * 2026. 06. 16. - 불필요한 주석 및 오버로딩 매크로(by_Lihan) 등 삭제
 * 2026. 06. 15. - 정적시험 통과를 위한 타기종 및 미사용 TCP/IPv6 기능 전면 삭제
 */

#ifndef _SOCKET_H_
#define _SOCKET_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "wizchip_conf.h"

#define SOCKET                uint8_t  ///< 레거시 드라이버를 위한 SOCKET 타입 정의

#define SOCK_OK               1        ///< 소켓 프로세스 결과가 정상(OK)입니다.
#define SOCK_BUSY             0        ///< 소켓이 작업을 처리 중입니다. (Non-block IO 모드에서만 유효)
#define SOCK_FATAL            -1000    ///< 소켓 프로세스에 치명적인 에러가 발생했습니다.

#define SOCK_ERROR            0
#define SOCKERR_SOCKNUM       (SOCK_ERROR - 1)     ///< 잘못된 소켓 번호
#define SOCKERR_SOCKOPT       (SOCK_ERROR - 2)     ///< 잘못된 소켓 옵션
#define SOCKERR_SOCKINIT      (SOCK_ERROR - 3)     ///< 소켓이 초기화되지 않았거나 Sn_MR_TCP일 때 SIPR이 0(Zero IP)입니다.
#define SOCKERR_SOCKCLOSED    (SOCK_ERROR - 4)     ///< 소켓이 예기치 않게 닫혔습니다.
#define SOCKERR_SOCKMODE      (SOCK_ERROR - 5)     ///< 소켓 작업에 대한 잘못된 소켓 모드입니다.
#define SOCKERR_SOCKFLAG      (SOCK_ERROR - 6)     ///< 잘못된 소켓 플래그
#define SOCKERR_SOCKSTATUS    (SOCK_ERROR - 7)     ///< 소켓 작업에 대한 잘못된 소켓 상태입니다.
#define SOCKERR_ARG           (SOCK_ERROR - 10)    ///< 잘못된 매개변수(인수)입니다.
#define SOCKERR_PORTZERO      (SOCK_ERROR - 11)    ///< 포트 번호가 0입니다.
#define SOCKERR_IPINVALID     (SOCK_ERROR - 12)    ///< 잘못된 IP 주소입니다.
#define SOCKERR_TIMEOUT       (SOCK_ERROR - 13)    ///< 타임아웃이 발생했습니다.
#define SOCKERR_DATALEN       (SOCK_ERROR - 14)    ///< 데이터 길이가 0이거나 버퍼 최대 크기보다 큽니다.
#define SOCKERR_BUFFER        (SOCK_ERROR - 15)    ///< 데이터 통신을 위한 소켓 버퍼가 충분하지 않습니다.

#define SOCKFATAL_PACKLEN     (SOCK_FATAL - 1)     ///< 잘못된 패킷 길이입니다. (치명적 에러)

/*
    - Sn_MR_MULTI : UDP 멀티캐스팅 지원
    - Sn_MR_BRDB  : 브로드캐스트 차단
    - Sn_MR_MC    : IGMP ver2, ver1
    - Sn_MR_SMB   : 요청된(Solicited) 멀티캐스트 차단
    - Sn_MR_UNIB  : 유니캐스트 차단
*/

/*
    SOCKET FLAG (소켓 플래그)
*/
/// UDP 모드에서 멀티캐스트 모드를 활성화합니다.
#define SF_MULTI_ENABLE      (Sn_MR_MULTI)

/// UDP 모드에서 브로드캐스트 패킷을 차단합니다.
#define SF_BROAD_BLOCK       (Sn_MR_BRDB)

#define SF_IGMP_VER2         (Sn_MR_MC)       ///< SF_MULTI_ENABLE을 포함한 UDP 모드에서 IGMP 버전 2를 선택합니다.   
#define SF_SOLICIT_BLOCK     (Sn_MR_SMB)      ///< UDP 모드에서 요청된 멀티캐스트 패킷을 차단합니다.

#define SF_UNI_BLOCK         (Sn_MR_UNIB)     ///< 멀티캐스트가 활성화된 UDP 모드에서 유니캐스트 패킷을 차단합니다. 

/// 강제로 ARP 요청을 수행합니다. 데이터그램 모드에서는 데이터가 목적지로 전송되기 전에 강제로 ARP를 요청합니다.
#define SF_FORCE_ARP         (Sn_MR2_FARP)

#define SF_IO_NONBLOCK       (0x01 << 3)     ///< 소켓 Non-block IO 모드. socket() 함수의 매개변수로 사용됩니다.

/*
    UDP 패킷 정보
*/
#define PACK_COMPLETED       (1<<3)                ///< 수신된 패킷에서 읽은 데이터가 마지막임을 나타냅니다.
#define PACK_REMAINED        (1<<2)                ///< 수신된 패킷에 데이터가 남아있음을 나타냅니다.
#define PACK_FIRST           (1<<1)                ///< 수신된 패킷에서 읽은 데이터가 처음임을 나타냅니다.
#define PACK_NONE            (0x00)                ///< 패킷 정보가 없음을 나타냅니다.

/////////////////////////////
// SOCKET CONTROL & OPTION //
/////////////////////////////
#define SOCK_IO_BLOCK         0  ///< setsockopt()에서의 소켓 Block IO 모드
#define SOCK_IO_NONBLOCK      1  ///< setsockopt()에서의 소켓 Non-block IO 모드

/**
    @defgroup DATA_TYPE DATA TYPE
*/

/**
    @brief 소켓 인터럽트의 종류
*/
typedef enum {
    SIK_CONNECTED     = (1 << 0),    ///< 연결됨 (connected)
    SIK_DISCONNECTED  = (1 << 1),    ///< 연결 해제됨 (disconnected)
    SIK_RECEIVED      = (1 << 2),    ///< 데이터 수신됨 (data received)
    SIK_TIMEOUT       = (1 << 3),    ///< 타임아웃 발생 (timeout occurred)
    SIK_SENT          = (1 << 4),    ///< 송신 완료 (send ok)
    SIK_ALL           = 0x1F         ///< 모든 인터럽트 (all interrupt)
} sockint_kind;

/**
    @brief ctlsocket() 함수의 제어 명령 타입
*/
typedef enum {
    CS_SET_IOMODE,          ///< SOCK_IO_BLOCK 또는 SOCK_IO_NONBLOCK으로 소켓 IO 모드 설정
    CS_GET_IOMODE,          ///< 소켓 IO 모드 가져오기
    CS_GET_MAXTXBUF,        ///< TX 메모리에 할당된 소켓 버퍼 크기 가져오기
    CS_GET_MAXRXBUF,        ///< RX 메모리에 할당된 소켓 버퍼 크기 가져오기
    CS_CLR_INTERRUPT,       ///< sockint_kind를 사용하여 소켓의 인터럽트 지우기(Clear)
    CS_GET_INTERRUPT,       ///< 소켓 인터럽트 상태 가져오기

    CS_SET_PREFER,
    CS_GET_PREFER,
    CS_SET_INTMASK,
    CS_GET_INTMASK
} ctlsock_type;

/**
    @brief setsockopt() 또는 getsockopt()에서의 소켓 옵션 타입
*/
typedef enum {
    SO_FLAG,             ///< getsockopt()에서만 유효, 소켓 플래그 조회용
    SO_TTL,              ///< TTL 설정
    SO_TOS,              ///< TOS 설정
    SO_DESTIP,           ///< 목적지 IP 주소 설정
    SO_DESTPORT,         ///< 목적지 포트 번호 설정
    SO_SENDBUF,          ///< getsockopt() 전용. 소켓 TX 버퍼의 남은 여유 공간 크기 가져오기
    SO_RECVBUF,          ///< getsockopt() 전용. 소켓 RX 버퍼에 수신된 데이터 크기 가져오기
    SO_STATUS,           ///< getsockopt() 전용. 소켓 상태 가져오기
    SO_MODE,
    SO_REMAINSIZE,       ///< getsockopt() 전용. 남은 패킷 크기 가져오기
    SO_PACKINFO          ///< getsockopt() 전용. 패킷 정보 가져오기 (PACK_FIRST, PACK_REMAINED 등)
} sockopt_type;

/*
@function    int8_t socket(uint8_t sn, uint8_t protocol, uint16_t port, uint8_t flag)
@brief      소켓을 초기화하고 엽니다.
@param      sn: 소켓 번호 (0 ~ _WIZCHIP_SOCK_NUM_ 사이의 값이어야 함)
@param      protocol: 동작할 프로토콜 타입 (예: TCP, UDP, MACRAW)
@param      port: 바인딩할 포트 번호
@param      flag: 소켓 플래그 (SF_ETHER_OWN, SF_IGMP_VER2, SF_TCP_NODELAY, SF_MULTI_ENABLE, SF_IO_NONBLOCK 등)
@return     성공 시: 매개변수로 전달된 소켓 번호 'sn'
            실패 시: SOCKERR_SOCKNUM(잘못된 소켓 번호), SOCKERR_SOCKMODE(지원하지 않는 소켓 모드), SOCKERR_SOCKFLAG(잘못된 소켓 플래그)
*/
int8_t  socket(uint8_t sn, uint8_t protocol, uint16_t port, uint8_t flag);

/*
@function    int8_t close(uint8_t sn)
@brief      지정된 소켓을 닫습니다(Close).
@param      sn: 소켓 번호 (0 ~ _WIZCHIP_SOCK_NUM_ 사이의 값이어야 함)
@return     성공 시: SOCK_OK
            실패 시: SOCKERR_SOCKNUM(잘못된 소켓 번호)
*/
int8_t  close(uint8_t sn);

/*
@function    int32_t sendto_W6x00(uint8_t sn, uint8_t * buf, uint16_t len, uint8_t * addr, uint16_t port, uint8_t addrlen)
@brief      목적지 IP 주소와 포트 번호로 데이터그램을 전송합니다. (UDP 송신)
@param      sn: 소켓 번호
@param      buf: 전송할 데이터가 담긴 버퍼 포인터
@param      len: 전송할 데이터 길이 (바이트)
@param      addr: 목적지 IP 주소 변수 포인터
@param      port: 목적지 포트 번호
@param      addrlen: 주소의 길이 (IPv4의 경우 4)
@return     성공 시: 실제 전송된 데이터 크기
            실패 시: 에러 코드 반환 (SOCKERR_SOCKNUM, SOCKERR_TIMEOUT 등)
*/
int32_t sendto_W6x00(uint8_t sn, uint8_t * buf, uint16_t len, uint8_t * addr, uint16_t port, uint8_t addrlen);

/*
@function    int32_t recvfrom_W6x00(uint8_t sn, uint8_t * buf, uint16_t len, uint8_t * addr, uint16_t *port, uint8_t *addrlen)
@brief      상대방으로부터 데이터그램을 수신합니다. (UDP 수신)
@param      sn: 소켓 번호
@param      buf: 수신된 데이터를 저장할 버퍼 포인터
@param      len: 수신할 최대 데이터 길이
@param      addr: 상대방의 IP 주소가 저장될 변수 포인터
@param      port: 상대방의 포트 번호가 저장될 변수 포인터
@param      addrlen: 목적지 IP 주소의 바이트 길이
@return     성공 시: 실제 수신된 데이터 크기
            실패 시: 에러 코드 반환
*/
int32_t recvfrom_W6x00(uint8_t sn, uint8_t * buf, uint16_t len, uint8_t * addr, uint16_t *port,  uint8_t *addrlen);

/*
@function    int8_t ctlsocket(uint8_t sn, ctlsock_type cstype, void* arg)
@brief      소켓을 제어합니다. (IO 모드, 인터럽트 및 소켓 버퍼 정보 등을 제어 및 조회)
@param      sn: 소켓 번호
@param      cstype: 제어할 소켓 명령 타입 (ctlsock_type 참조)
@param      arg: 명령 타입(cstype)에 따라 결정되는 데이터 타입 및 값을 담은 포인터
@return     성공 시: SOCK_OK
            실패 시: SOCKERR_ARG(잘못된 매개변수)
*/
int8_t  ctlsocket(uint8_t sn, ctlsock_type cstype, void* arg);

/*
@function    int8_t setsockopt(uint8_t sn, sockopt_type sotype, void* arg)
@brief      소켓 옵션(TTL, MSS, TOS 등)을 설정합니다.
@param      sn: 소켓 번호
@param      sotype: 소켓 옵션 타입 (sockopt_type 참조)
@param      arg: 옵션 타입(sotype)에 따라 결정되는 설정값을 담은 포인터
@return     성공 시: SOCK_OK
            실패 시: 에러 코드 반환 (SOCKERR_SOCKNUM, SOCKERR_SOCKOPT 등)
*/
int8_t  setsockopt(uint8_t sn, sockopt_type sotype, void* arg);

/*
@function    int8_t getsockopt(uint8_t sn, sockopt_type sotype, void* arg)
@brief      소켓 옵션(플래그, TTL, MSS 등)을 읽어옵니다.
@param      sn: 소켓 번호
@param      sotype: 소켓 옵션 타입 (sockopt_type 참조)
@param      arg: 읽어온 값을 저장할 데이터 포인터
@return     성공 시: SOCK_OK
            실패 시: 에러 코드 반환
*/
int8_t  getsockopt(uint8_t sn, sockopt_type sotype, void* arg);

#ifdef __cplusplus
}
#endif

#endif   // _SOCKET_H_
