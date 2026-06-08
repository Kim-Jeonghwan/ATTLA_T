/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : hal_W6100.c
    Version          : 00.00
    Description      : W6100 이더넷 컨트롤러 하드웨어 제어
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 08. (주석 템플릿 일괄 적용)
**********************************************************************/

#include "hal_W6100.h"

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
