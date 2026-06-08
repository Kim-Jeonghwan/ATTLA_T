/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : csu_Ethernet.c
    Version          : 00.00
    Description      : 이더넷 통신 태스크 및 프로토콜 제어 로직
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 08. (주석 템플릿 일괄 적용)
**********************************************************************/

#include "csu_Ethernet.h"
#define SOCK_UDP_COM 0
#define PORT_UDP_COM 5001

uint8_t flag_2ms_tx = 0U;

/*
@funtion    void Ethernet_Init(void)
@brief      이더넷 통신망 및 소켓 초기화
@param      void
@return     void
*/
void Ethernet_Init(void)
{
    Initial_W6100();
    socket(SOCK_UDP_COM, Sn_MR_UDP, PORT_UDP_COM, 0x00);
}

/*
@funtion    void Ethernet_Process(void)
@brief      W6100 주기적 폴링 및 패킷 처리
@param      void
@return     void
@remark     1ms 또는 10ms 주기 태스크에서 호출
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
            // 수신 버퍼 크기를 초과하지 않도록 안전 장치를 마련합니다.
            if (rx_size > sizeof(rx_buf)) 
            {
                rx_size = sizeof(rx_buf);
            }

            // W6100 하드웨어 버퍼에서 실제 MCU 메모리(rx_buf)로 데이터를 복사하여 가져옵니다.
            recvfrom(SOCK_UDP_COM, rx_buf, rx_size, dest_ip, &dest_port);
            
            // TODO: 수신된 18바이트 패킷 파싱 로직 구현
            // if (rx_size == 18) {
            //    parsePacket(rx_buf);
            // }
        }
        
        // 4. 2ms 송신 주기 플래그 확인 후 모터 상태 데이터 (18바이트) 조립 및 전송
        if (flag_2ms_tx == 1U)
        {
            flag_2ms_tx = 0U; // 플래그 클리어
            
            uint8_t tx_buf[18] = {0}; // 18바이트 송신 버퍼 초기화
            
            // 임시 페이로드 구성 (추후 정확한 18바이트 패킷 구조체 맵핑 시 업데이트)
            // 인덱스 0~3: 온도 데이터 (Float -> Bytes 변환 등 필요시 추후 적용, 현재는 Dummy/하위 바이트)
            tx_buf[0] = (uint8_t)xXmtSciPcMsg1.DspTemp;
            tx_buf[1] = (uint8_t)xXmtSciPcMsg1.IncNumber;
            tx_buf[2] = (uint8_t)xXmtSciPcMsg1.Status;
            
            // 전송 대상 타겟 (임시 IP 192.168.1.100, Port 5001)
            uint8_t target_ip[4] = {192, 168, 1, 100};
            uint16_t target_port = 5001;

            sendto(SOCK_UDP_COM, tx_buf, 18U, target_ip, target_port);
        }
    } 
    // 5. 소켓이 닫혀있는 경우 (초기 상태 혹은 비정상 종료 시)
    else if (sn_sr == SOCK_CLOSED) 
    {
        // UDP 모드, 5001번 포트로 0번 소켓을 다시 개방합니다.
        socket(SOCK_UDP_COM, Sn_MR_UDP, PORT_UDP_COM, 0x00);
    }
}
