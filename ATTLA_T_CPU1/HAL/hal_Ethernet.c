/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : hal_Ethernet.c
    Version          : 00.01
    Description      : W6100 이더넷 컨트롤러 제어 및 소켓 통신(Ext. Interrupt)
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 11. (csu_Ethernet과 hal_W6100 병합 및 초기화 로직 캡슐화)
**********************************************************************/

#include "hal_Ethernet.h"
#define SOCK_UDP_COM 0
#define PORT_UDP_COM 5001


/*
@funtion    void Initial_W6100(void)
@brief      W6100 하드웨어 초기화 및 IP/MAC 설정
@param      void
@return     void
*/
void Initial_W6100(void)
{
    // 1. SPI Callbacks Registration (SPI-A 모듈 함수 연결)
    reg_wizchip_cs_cbfunc(cs_sel, cs_desel);
    reg_wizchip_spi_cbfunc(spi_read_byte, spi_write_byte, 0, 0);

    // 2. W6100 메모리 초기화 (TX/RX 버퍼 8개의 소켓에 각각 2KB씩 할당)
    uint8_t txsize[8] = {2, 2, 2, 2, 2, 2, 2, 2};
    uint8_t rxsize[8] = {2, 2, 2, 2, 2, 2, 2, 2};
    
    // W6100 메모리 초기화 (TX/RX 버퍼 8개의 소켓에 각각 2KB씩 할당)
    // 반환값이 0 미만이면 초기화 실패를 의미합니다.
    if (wizchip_init(txsize, rxsize) < 0) 
    {
        // 초기화 실패 시 시스템 에러 처리 로직을 추후 여기에 추가할 수 있습니다.
        return;
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
}

/*
@funtion    void Ethernet_Init(void)
@brief      이더넷 통신망, 소켓 개방 및 W6100 외부 인터럽트 등록
@param      void
@return     void
*/
void Ethernet_Init(void)
{
    // 1. W6100 하드웨어 설정 적용
    Initial_W6100();
    
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
@funtion    __interrupt void isr_EthernetExtInt(void)
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
@funtion    void Ethernet_Process(void)
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
