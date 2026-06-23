/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : w6100.c
    Version          : 00.03
    Description      : WIZnet 이더넷 라이브러리 파일
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 16. (UDP 단일화 및 미사용 통신 로직 전면 삭제)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 16. - UDP 단일화 및 불필요한 통신 로직 전면 삭제
 * 2026. 06. 16. - 미사용 BUS_INDIR 인터페이스 코드 삭제 및 Single Return 적용
 *               - 무한 루프 버그 수정 (WIZCHIP_READ_BUF 내 증감자 오류)
 * 2026. 06. 15. - 정적시험 통과를 위한 타기종 및 미사용 TCP/IPv6 기능 전면 삭제
 */

#include "w6100.h"

#define _WIZCHIP_SPI_VDM_OP_    0x00

#define _W6100_SPI_OP_          _WIZCHIP_SPI_VDM_OP_

/*
@function    void WIZCHIP_WRITE(uint32_t AddrSel, uint8_t wb)
@brief      W6100의 특정 레지스터 주소에 1바이트의 데이터를 씁니다. (SPI 또는 BUS 인터페이스 사용)
@param      AddrSel: 데이터를 쓸 W6100 레지스터의 주소
@param      wb: 기록할 1바이트 데이터
@return     void
*/
void WIZCHIP_WRITE(uint32_t AddrSel, uint8_t wb) {
    uint8_t tAD[4];
    tAD[0] = (uint8_t)((AddrSel & 0x00FF0000) >> 16);
    tAD[1] = (uint8_t)((AddrSel & 0x0000FF00) >> 8);
    tAD[2] = (uint8_t)(AddrSel & 0x000000ff);
    tAD[3] = wb;

    WIZCHIP_CRITICAL_ENTER();
    WIZCHIP.CS._select();

#if( (_WIZCHIP_IO_MODE_ == _WIZCHIP_IO_MODE_SPI_VDM_))
    tAD[2] |= (_W6100_SPI_WRITE_ | _W6100_SPI_OP_);
    if (!WIZCHIP.IF.SPI._write_burst) {
        WIZCHIP.IF.SPI._write_byte(tAD[0]);
        WIZCHIP.IF.SPI._write_byte(tAD[1]);
        WIZCHIP.IF.SPI._write_byte(tAD[2]);
        WIZCHIP.IF.SPI._write_byte(tAD[3]);
    } else {
        WIZCHIP.IF.SPI._write_burst(tAD, 4);
    }
#else
#error "Unknown _WIZCHIP_IO_MODE_ in W6100. !!!"
#endif

    WIZCHIP.CS._deselect();
    WIZCHIP_CRITICAL_EXIT();
}

/*
@function    uint8_t WIZCHIP_READ(uint32_t AddrSel)
@brief      W6100의 특정 레지스터 주소에서 1바이트의 데이터를 읽어옵니다.
@param      AddrSel: 데이터를 읽어올 W6100 레지스터 주소
@return     레지스터에서 읽어온 1바이트 값
*/
uint8_t WIZCHIP_READ(uint32_t AddrSel) {
    uint8_t ret;
    uint8_t tAD[3];
    tAD[0] = (uint8_t)((AddrSel & 0x00FF0000) >> 16);
    tAD[1] = (uint8_t)((AddrSel & 0x0000FF00) >> 8);
    tAD[2] = (uint8_t)(AddrSel & 0x000000ff);

    WIZCHIP_CRITICAL_ENTER();
    WIZCHIP.CS._select();

#if( (_WIZCHIP_IO_MODE_ ==  _WIZCHIP_IO_MODE_SPI_VDM_))
    tAD[2] |= (_W6100_SPI_READ_ | _W6100_SPI_OP_);
    if (!WIZCHIP.IF.SPI._read_burst || !WIZCHIP.IF.SPI._write_burst) {
        WIZCHIP.IF.SPI._write_byte(tAD[0]);
        WIZCHIP.IF.SPI._write_byte(tAD[1]);
        WIZCHIP.IF.SPI._write_byte(tAD[2]);
    } else {
        WIZCHIP.IF.SPI._write_burst(tAD, 3);
    }
    ret = WIZCHIP.IF.SPI._read_byte();
#else
#error "Unknown _WIZCHIP_IO_MODE_ in W6100. !!!"
#endif

    WIZCHIP.CS._deselect();
    WIZCHIP_CRITICAL_EXIT();
    return ret;
}

/*
@function    void WIZCHIP_WRITE_BUF(uint32_t AddrSel, uint8_t* pBuf, uint16_t len)
@brief      지정한 메모리 버퍼의 데이터를 W6100의 레지스터(주로 Tx 버퍼)에 연속적으로 씁니다.
@param      AddrSel: 데이터 쓰기를 시작할 W6100의 주소
@param      pBuf: 쓸 데이터가 담긴 버퍼 포인터
@param      len: 기록할 데이터 길이 (바이트)
@return     void
*/
void WIZCHIP_WRITE_BUF(uint32_t AddrSel, uint8_t* pBuf, uint16_t len) {
    uint8_t tAD[3];
    uint16_t i = 0;

    tAD[0] = (uint8_t)((AddrSel & 0x00FF0000) >> 16);
    tAD[1] = (uint8_t)((AddrSel & 0x0000FF00) >> 8);
    tAD[2] = (uint8_t)(AddrSel & 0x000000ff);

    WIZCHIP_CRITICAL_ENTER();
    WIZCHIP.CS._select();

#if((_WIZCHIP_IO_MODE_ == _WIZCHIP_IO_MODE_SPI_VDM_))
    tAD[2] |= (_W6100_SPI_WRITE_ | _W6100_SPI_OP_);
    if (!WIZCHIP.IF.SPI._write_burst) {
        WIZCHIP.IF.SPI._write_byte(tAD[0]);
        WIZCHIP.IF.SPI._write_byte(tAD[1]);
        WIZCHIP.IF.SPI._write_byte(tAD[2]);
        for (i = 0; i < len; i++) {
            WIZCHIP.IF.SPI._write_byte(pBuf[i]);
        }
    } else {
        WIZCHIP.IF.SPI._write_burst(tAD, 3);
        WIZCHIP.IF.SPI._write_burst(pBuf, len);
    }
#else
#error "Unknown _WIZCHIP_IO_MODE_ in W6100. !!!!"
#endif

    WIZCHIP.CS._deselect();
    WIZCHIP_CRITICAL_EXIT();
}

/*
@function    void WIZCHIP_READ_BUF(uint32_t AddrSel, uint8_t* pBuf, uint16_t len)
@brief      W6100의 레지스터(주로 Rx 버퍼)에서 여러 바이트의 데이터를 연속적으로 읽어 버퍼에 저장합니다.
@param      AddrSel: 데이터 읽기를 시작할 W6100의 주소
@param      pBuf: 읽어온 데이터를 저장할 버퍼 포인터
@param      len: 읽어올 데이터 길이 (바이트)
@return     void
*/
void WIZCHIP_READ_BUF(uint32_t AddrSel, uint8_t* pBuf, uint16_t len) {
    uint8_t tAD[3];
    uint16_t i;
    tAD[0] = (uint8_t)((AddrSel & 0x00FF0000) >> 16);
    tAD[1] = (uint8_t)((AddrSel & 0x0000FF00) >> 8);
    tAD[2] = (uint8_t)(AddrSel & 0x000000ff);

    WIZCHIP_CRITICAL_ENTER();
    WIZCHIP.CS._select();

#if((_WIZCHIP_IO_MODE_ == _WIZCHIP_IO_MODE_SPI_VDM_))
    tAD[2] |= (_W6100_SPI_READ_ | _W6100_SPI_OP_);
    if (!WIZCHIP.IF.SPI._read_burst || !WIZCHIP.IF.SPI._write_burst) {
        WIZCHIP.IF.SPI._write_byte(tAD[0]);
        WIZCHIP.IF.SPI._write_byte(tAD[1]);
        WIZCHIP.IF.SPI._write_byte(tAD[2]);
        for (i = 0; i < len; i++) {
            pBuf[i] = WIZCHIP.IF.SPI._read_byte();
        }
    } else {
        WIZCHIP.IF.SPI._write_burst(tAD, 3);
        WIZCHIP.IF.SPI._read_burst(pBuf, len);
    }
#else
#error "Unknown _WIZCHIP_IO_MODE_ in W6100. !!!!"
#endif
    WIZCHIP.CS._deselect();
    WIZCHIP_CRITICAL_EXIT();
}

/*
@function    uint16_t getSn_TX_FSR(uint8_t sn)
@brief      특정 소켓의 Tx 버퍼에 현재 전송할 수 있는 여유 공간(Free Size)을 확인합니다.
@param      sn: 확인할 소켓 번호
@return     버퍼의 여유 공간 (바이트 크기)
*/
uint16_t getSn_TX_FSR(uint8_t sn) {
    uint16_t prev_val = 0xFFFFU, val = 0;
    do {
        prev_val = val;
        val = WIZCHIP_READ(_Sn_TX_FSR_(sn));
        val = (val << 8) + WIZCHIP_READ(WIZCHIP_OFFSET_INC(_Sn_TX_FSR_(sn), 1));
    } while (val != prev_val);
    return val;
}

/*
@function    uint16_t getSn_RX_RSR(uint8_t sn)
@brief      특정 소켓의 Rx 버퍼에 현재 수신되어 대기 중인 데이터의 크기를 확인합니다.
@param      sn: 확인할 소켓 번호
@return     수신된 데이터의 크기 (바이트 크기)
*/
uint16_t getSn_RX_RSR(uint8_t sn) {
    uint16_t prev_val = 0xFFFFU, val = 0;
    do {
        prev_val = val;
        val = WIZCHIP_READ(_Sn_RX_RSR_(sn));
        val = (val << 8) + WIZCHIP_READ(WIZCHIP_OFFSET_INC(_Sn_RX_RSR_(sn), 1));
    } while (val != prev_val);
    return val;
}

/*
@function    void wiz_send_data(uint8_t sn, uint8_t *wizdata, uint16_t len)
@brief      어플리케이션의 데이터를 W6100 내부 Tx 버퍼 메모리로 실제로 복사(기록)합니다.
@param      sn: 데이터를 전송할 소켓 번호
@param      wizdata: 전송할 데이터가 담긴 버퍼 포인터
@param      len: 전송할 데이터 길이
@return     void
*/
void wiz_send_data(uint8_t sn, uint8_t *wizdata, uint16_t len) {
    uint16_t ptr = 0;
    uint32_t addrsel = 0;
    ptr = getSn_TX_WR(sn);
    addrsel = ((uint32_t)ptr << 8) + WIZCHIP_TXBUF_BLOCK(sn);
    WIZCHIP_WRITE_BUF(addrsel, wizdata, len);
    ptr += len;
    setSn_TX_WR(sn, ptr);
}

/*
@function    void wiz_recv_data(uint8_t sn, uint8_t *wizdata, uint16_t len)
@brief      W6100 내부 Rx 버퍼 메모리에 수신된 데이터를 어플리케이션의 버퍼로 복사(가져오기)합니다.
@param      sn: 수신할 소켓 번호
@param      wizdata: 데이터를 저장할 버퍼 포인터
@param      len: 읽어올 데이터 길이
@return     void
*/
void wiz_recv_data(uint8_t sn, uint8_t *wizdata, uint16_t len) {
    uint16_t ptr = 0;
    uint32_t addrsel = 0;
    if (len != 0) {
        ptr = getSn_RX_RD(sn);
        addrsel = ((uint32_t)ptr << 8) + WIZCHIP_RXBUF_BLOCK(sn);
        WIZCHIP_READ_BUF(addrsel, wizdata, len);
        ptr += len;
        setSn_RX_RD(sn, ptr);
    }
}

void wiz_recv_ignore(uint8_t sn, uint16_t len) {
    setSn_RX_RD(sn, getSn_RX_RD(sn) + len);
}

void wiz_delay_ms(uint32_t milliseconds) {
    uint32_t i;
    for (i = 0 ; i < milliseconds ; i++) {
        setTCNTRCLR(0xff);
        while (getTCNTR() < 0x0a) {}
    }
}

#if (_PHY_IO_MODE_ == _PHY_IO_MODE_MII_)
void wiz_mdio_write(uint8_t phyregaddr, uint16_t var) {
    setPHYRAR(phyregaddr);
    setPHYDIR(var);
    setPHYACR(PHYACR_WRITE);
    while (getPHYACR());
}

uint16_t wiz_mdio_read(uint8_t phyregaddr) {
    setPHYRAR(phyregaddr);
    setPHYACR(PHYACR_READ);
    while (getPHYACR());
    return getPHYDOR();
}
#endif
