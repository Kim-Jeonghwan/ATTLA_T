/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : hal_Ethernet.c
    Version          : 00.03
    Description      : 이더넷(W6100) 하드웨어 제어 로직
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 15. (SPIA 전용 콜백 함수명 반영)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 15. - W6100용 SPI 통신 콜백 함수 이름을 spia_ 접두어를 포함한 이름으로 수정 반영
 * 2026. 06. 15. - Initial_W6100 반환값 처리 추가로 하드웨어 미연결 시 소켓 개방 스킵(무한루프 방지) 구현
 * 2026. 06. 12. - 소켓 및 포트 매크로 상수를 헤더(.h)로 이동 (글로벌 룰 적용)
 * 2026. 06. 11. - 주석 표준화 및 레거시 코드 정리
 * 2026. 06. 11. - 파일 생성 및 기본 구조 작성
 */


#include "hal_Ethernet.h"


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
        .ip = {192, 168, 200, 10},
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

/*
@function    void Ethernet_Init(void)
@brief      이더넷 통신망, 소켓 개방 및 W6100 외부 인터럽트 등록
@param      void
@return     void
*/
void Ethernet_Init(void)
{
    // 1. W6100 하드웨어 설정 적용 및 연결 상태 확인
    if (Initial_W6100() < 0)
    {
        // 하드웨어가 연결되어 있지 않으면 이후의 소켓 개방 루틴을 무시하고 빠져나갑니다.
        // 이를 통해 WIZnet 드라이버 내부의 무한 루프 블로킹을 근본적으로 방지합니다.
        return; 
    }
    
    // 2. UDP 소켓 개방
    socket(SOCK_UDP_COM, Sn_MR_UDP, PORT_UDP_COM, 0x00);
    
    // 3. W6100 INTn(GPIO 20) 핀을 XINT1 외부 인터럽트로 등록
    GPIO_setInterruptPin(20U, GPIO_INT_XINT1);
    
    // XINT1 극성 설정 (Falling edge: W6100 INTn은 Active Low)
    GPIO_setInterruptType(GPIO_INT_XINT1, GPIO_INT_TYPE_FALLING_EDGE);
    
    // PIE 인터럽트 등록
    Interrupt_register(INT_XINT1, isr_EthernetExtInt);
}

/*
@function    __interrupt void isr_EthernetExtInt(void)
@brief      W6100 외부 인터럽트 (XINT1) 서비스 루틴
@param      void
@return     __interrupt void
*/
__interrupt void isr_EthernetExtInt(void)
{
    // 1. 수신 처리 및 응답 전송 로직 실행
    Ethernet_Process();
    
    // 2. PIE ACK (XINT1은 그룹 1에 속함)
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP1);
}

/*
@function    void Ethernet_Process(void)
@brief      W6100 인터럽트 발생 시 패킷 수신 및 즉각 응답
@param      void
@return     void
@remark     XINT1 (isr_EthernetExtInt) 인터럽트 루틴에서 호출됨
*/
void Ethernet_Process(void)
{
    uint8_t rx_buf[128];
    uint16_t rx_size;
    uint8_t dest_ip[4];
    uint16_t dest_port;
    
    // 1. 현재 0번 소켓의 하드웨어 상태 레지스터(Sn_SR) 값을 읽어옵니다.
    uint8_t sn_sr = getSn_SR(SOCK_UDP_COM);

    // 2. 소켓이 정상적으로 UDP 모드로 개방되어 있는 경우
    if (sn_sr == SOCK_UDP) 
    {
        // 3. 수신된 데이터(RX Payload)가 버퍼에 존재하는지 바이트 크기를 확인합니다.
        rx_size = getSn_RX_RSR(SOCK_UDP_COM);

        if (rx_size > 0) 
        {
            if (rx_size > sizeof(rx_buf)) 
            {
                rx_size = sizeof(rx_buf);
            }

            recvfrom(SOCK_UDP_COM, rx_buf, rx_size, dest_ip, &dest_port);
            
            // 수신 후 인터럽트 플래그 클리어 (W6100 하드웨어)
            setSn_IR(SOCK_UDP_COM, 0xFF);
            
            // TODO: 수신된 패킷 파싱 로직 구현
            // parsePacket(rx_buf);
            
            // 데이터 수신 시, 즉각적으로 화포통제컴퓨터로 응답(Tx) 전송
            uint8_t tx_buf[18] = {0}; 
            
            tx_buf[0] = (uint8_t)xXmtSciPcMsg1.DspTemp;
            tx_buf[1] = (uint8_t)xXmtSciPcMsg1.IncNumber;
            tx_buf[2] = (uint8_t)xXmtSciPcMsg1.Status;
            
            // 수신된 IP와 Port로 즉시 응답 반환
            sendto(SOCK_UDP_COM, tx_buf, 18U, dest_ip, dest_port);
        }
    } 
    else if (sn_sr == SOCK_CLOSED) 
    {
        socket(SOCK_UDP_COM, Sn_MR_UDP, PORT_UDP_COM, 0x00);
    }
}
