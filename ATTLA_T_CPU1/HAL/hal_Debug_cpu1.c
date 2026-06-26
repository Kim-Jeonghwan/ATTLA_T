/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : hal_Debug_cpu1.c
    Version          : 00.10
    Description      : 이더넷(W6100) 하드웨어 제어 로직 (디버그 통신망)
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 26. (모듈명 변경 및 디버깅 전용망 분리)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 26. - hal_Ethernet_cpu1 에서 hal_Debug_cpu1 으로 모듈명 변경
 * 2026. 06. 24. - 파일명 리팩토링 (_cpu1 분리)
 * 2026. 06. 23. - 모니터링 IP 변경: 192.168.200.11
 * 2026. 06. 15. - W6100용 SPI 통신 콜백 함수 이름을 spia_ 접두어를 포함한 이름으로 수정 반영
 * 2026. 06. 15. - Initial_W6100 반환값 처리 추가로 하드웨어 미연결 시 소켓 개방 스킵(무한루프 방지) 구현
 */

#include "hal_Debug_cpu1.h"

/*
@function    int8_t Initial_W6100(void)
@brief      W6100 하드웨어 초기화 및 IP/MAC 설정
@param      void
@return     int8_t (성공 0, 실패 -1)
*/
int8_t Initial_W6100(void)
{
    // 1. SPI Callbacks Registration (SPI-A 모듈 함수 연결)
    reg_wizchip_cs_cbfunc(spia_cs_sel, spia_cs_desel);
    reg_wizchip_spi_cbfunc(spia_read_byte, spia_write_byte, 0, 0);

    // 2. W6100 메모리 초기화 (TX/RX 버퍼 8개의 소켓에 각각 2KB씩 할당)
    uint8_t txsize[8] = {2, 2, 2, 2, 2, 2, 2, 2};
    uint8_t rxsize[8] = {2, 2, 2, 2, 2, 2, 2, 2};
    
    // 반환값이 0 미만이면 칩 연결 불량이거나 초기화 실패를 의미합니다.
    if (wizchip_init(txsize, rxsize) < 0) 
    {
        return -1; // 하드웨어 없음, 실패 반환
    }

    // 3. MAC 주소 및 고정 IP 설정 (IPv4)
    wiz_NetInfo netinfo = {
        .mac = {0x00, 0x08, 0xDC, 0x11, 0x22, 0x33}, // WIZnet OUI MAC
        .ip = {192, 168, 200, 11},
        .sn = {255, 255, 255, 0},
        .gw = {192, 168, 200, 1},
        .dns = {8, 8, 8, 8},
        .ipmode = NETINFO_STATIC_V4,
        .dhcp = NETINFO_STATIC
    };

    // IPv6 및 기타 필드 0 초기화를 위해 ctlnetwork 사용 가능
    wizchip_setnetinfo(&netinfo);

    // 하드웨어 통신 정상 여부 검증 (설정값 읽기 테스트)
    wiz_NetInfo check_info;
    wizchip_getnetinfo(&check_info);
    
    // 방금 설정한 IP 주소의 첫 번째 바이트(192)가 정상적으로 읽히지 않는다면
    // 하드웨어가 없거나 SPI 통신이 불량인 상태이므로 실패 반환
    if (check_info.ip[0] != 192)
    {
        return -1;
    }

    return 0; // 초기화 성공
}

// 하드웨어 연결 상태 플래그 (1: 정상 연결, 0: 미연결)
uint8_t isW6100Connected = 0;

/*
@function    void Debug_Init(void)
@brief      디버그 통신망, 소켓 개방 및 W6100 외부 인터럽트 등록
@param      void
@return     void
*/
void Debug_Init(void)
{
    // 1. W6100 하드웨어 설정 적용 및 연결 상태 확인
    if (Initial_W6100() < 0)
    {
        // 하드웨어가 연결되어 있지 않으면 이후의 소켓 개방 루틴을 무시하고 빠져나갑니다.
        isW6100Connected = 0;
        return; 
    }
    isW6100Connected = 1;
    
    // 2. UDP 소켓 개방
    socket(SOCK_UDP_DBG, Sn_MR_UDP, PORT_UDP_DBG, 0x00);
    
    // 3. W6100 INTn(GPIO 20) 핀을 XINT1 외부 인터럽트로 등록
    GPIO_setInterruptPin(20U, GPIO_INT_XINT1);
    
    // XINT1 극성 설정 (Falling edge: W6100 INTn은 Active Low)
    GPIO_setInterruptType(GPIO_INT_XINT1, GPIO_INT_TYPE_FALLING_EDGE);
    
    // PIE 인터럽트 등록
    Interrupt_register(INT_XINT1, isr_DebugExtInt);
}

/*
@function    __interrupt void isr_DebugExtInt(void)
@brief      W6100 외부 인터럽트 (XINT1) 서비스 루틴
@param      void
@return     __interrupt void
*/
__interrupt void isr_DebugExtInt(void)
{
    // 1. 수신 처리 및 응답 전송 로직 실행
    Debug_Process();
    
    // 2. PIE ACK (XINT1은 그룹 1에 속함)
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP1);
}

/*
@function    void Debug_Process(void)
@brief      W6100 인터럽트 발생 시 패킷 수신 및 파싱
@param      void
@return     void
@remark     XINT1 (isr_DebugExtInt) 인터럽트 루틴에서 호출됨
*/
void Debug_Process(void)
{
    uint8_t rx_buf[128];
    uint16_t rx_size;
    uint8_t dest_ip[4];
    uint16_t dest_port;
    uint8_t dest_addrlen = 4; // IPv4 address length
    
    // 1. 현재 소켓의 하드웨어 상태 레지스터(Sn_SR) 값을 읽어옵니다.
    uint8_t sn_sr = getSn_SR(SOCK_UDP_DBG);

    // 2. 소켓이 정상적으로 UDP 모드로 개방되어 있는 경우
    if (sn_sr == SOCK_UDP) 
    {
        // 3. 수신된 데이터(RX Payload)가 버퍼에 존재하는지 바이트 크기를 확인합니다.
        rx_size = getSn_RX_RSR(SOCK_UDP_DBG);

        if (rx_size > 0) 
        {
            if (rx_size > sizeof(rx_buf)) 
            {
                rx_size = sizeof(rx_buf);
            }

            recvfrom_W6x00(SOCK_UDP_DBG, rx_buf, rx_size, dest_ip, &dest_port, &dest_addrlen);
            
            // 수신 후 인터럽트 플래그 클리어 (W6100 하드웨어)
            setSn_IR(SOCK_UDP_DBG, 0xFF);
            
            // 수신된 패킷 파싱 로직 호출 (동적 IP 캡처를 위해 송신자 IP와 Port 전달)
            Debug_ParsePacket(rx_buf, rx_size, dest_ip, dest_port);
        }
    } 
    else if (sn_sr == SOCK_CLOSED) 
    {
        socket(SOCK_UDP_DBG, Sn_MR_UDP, PORT_UDP_DBG, 0x00);
    }
}
