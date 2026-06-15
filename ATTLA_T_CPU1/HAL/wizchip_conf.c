/**********************************************************************
 Nexcom Co., Ltd.
 Filename         : wizchip_conf.c
 Version          : 00.01
 Description      : WIZnet 이더넷 라이브러리 파일
 Programmer       : Kim Jeonghwan
 Last Updated     : 2026. 06. 15. (정적시험용 코드 다이어트: 미사용 기능 삭제)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 15. - 정적시험 통과를 위한 타기종 및 미사용 TCP/IPv6 기능 전면 삭제
 */
#include <stddef.h>
//

#include "wizchip_conf.h"

/////////////
/////////////

/**
    @brief Default function to enable interrupt.
    @note This function help not to access wrong address. If you do not describe this function or register any functions,
    null function is called.
*/
//void 	  wizchip_cris_enter(void)           {};
void 	  wizchip_cris_enter(void)           {}

/**
    @brief Default function to disable interrupt.
    @note This function help not to access wrong address. If you do not describe this function or register any functions,
    null function is called.
*/
//void 	  wizchip_cris_exit(void)          {};
void 	  wizchip_cris_exit(void)          {}

/**
    @brief Default function to select chip.
    @note This function help not to access wrong address. If you do not describe this function or register any functions,
    null function is called.
*/
//void 	wizchip_cs_select(void)            {};
void 	wizchip_cs_select(void)            {}

/**
    @brief Default function to deselect chip.
    @note This function help not to access wrong address. If you do not describe this function or register any functions,
    null function is called.
*/
//void 	wizchip_cs_deselect(void)          {};
void 	wizchip_cs_deselect(void)          {}

/**
    @brief Default function to read in direct or indirect interface.
    @note This function help not to access wrong address. If you do not describe this function or register any functions,
    null function is called.
*/
//uint8_t wizchip_bus_readbyte(uint32_t AddrSel) { return * ((volatile uint8_t *)((ptrdiff_t) AddrSel)); }
iodata_t wizchip_bus_readdata(uint32_t AddrSel) {
    return * ((volatile iodata_t *)((ptrdiff_t) AddrSel));
}

/**
    @brief Default function to write in direct or indirect interface.
    @note This function help not to access wrong address. If you do not describe this function or register any functions,
    null function is called.
*/
//void 	wizchip_bus_writebyte(uint32_t AddrSel, uint8_t wb)  { *((volatile uint8_t*)((ptrdiff_t)AddrSel)) = wb; }
void 	wizchip_bus_writedata(uint32_t AddrSel, iodata_t wb)  {
    *((volatile iodata_t*)((ptrdiff_t)AddrSel)) = wb;
}
#if 1
// 20231103 taylor
/**
    @brief Default function to read @ref iodata_t buffer by using BUS interface
    @details @ref wizchip_bus_read_buf() provides the default read @ref iodata_t data as many as <i>len</i> from BUS of @ref _WIZCHIP_.
    @param AddrSel It specifies the address of register to be read.
    @param buf It specifies your buffer pointer to be saved the read data from @ref _WIZCHIP_.
    @param len It specifies the data length to be read from @ref _WIZCHIP_.
    @param addrinc It specifies whether the address is increased by every read operation or not.\n
          0 : Not Increased \n
          1 : Increased
    @return void
    @note It can be overwritten with your function or register your functions by calling @ref reg_wizchip_bus_cbfunc().
    @sa wizchip_bus_write_buf()
*/
void wizchip_bus_read_buf(uint32_t AddrSel, iodata_t* buf, int16_t len, uint8_t addrinc) {
    uint16_t i;
    if (addrinc) {
        addrinc = sizeof(iodata_t);
    }
    for (i = 0; i < len; i++) {
        *buf++ = WIZCHIP.IF.BUS._read_data(AddrSel);
        AddrSel += (uint32_t) addrinc;
    }
}

/**
    @brief Default function to write @ref iodata_t buffer by using BUS interface.
    @details @ref wizchip_bus_write_buf() provides the default write @ref iodata_t data as many as <i>len</i> to BUS of @ref _WIZCHIP_.
    @param AddrSel It specifies the address of register to be written.
    @param buf It specifies your buffer pointer to be written to @ref _WIZCHIP_.
    @param len It specifies the data length to be written to @ref _WIZCHIP_.
    @param addrinc It specifies whether the address is increased by every write operation or not.\n
          0 : Not Increased \n
          1 : Increased
    @return void
    @note It can be overwritten with your function or register your functions by calling @ref reg_wizchip_bus_cbfunc().
    @sa wizchip_bus_read_buf()
*/
void wizchip_bus_write_buf(uint32_t AddrSel, iodata_t* buf, int16_t len, uint8_t addrinc) {
    uint16_t i;
    if (addrinc) {
        addrinc = sizeof(iodata_t);
    }
    for (i = 0; i < len ; i++) {
        WIZCHIP.IF.BUS._write_data(AddrSel, *buf++);
        AddrSel += (uint32_t)addrinc;
    }

}
#endif

/**
    @brief Default function to read in SPI interface.
    @note This function help not to access wrong address. If you do not describe this function or register any functions,
    null function is called.
*/
//uint8_t wizchip_spi_readbyte(void)        {return 0;};
uint8_t wizchip_spi_readbyte(void)        {
    return 0;
}

/**
    @brief Default function to write in SPI interface.
    @note This function help not to access wrong address. If you do not describe this function or register any functions,
    null function is called.
*/
//void 	wizchip_spi_writebyte(uint8_t wb) {};
void 	wizchip_spi_writebyte(uint8_t wb) {}

/**
    @brief Default function to burst read in SPI interface.
    @note This function help not to access wrong address. If you do not describe this function or register any functions,
    null function is called.
*/
//void 	wizchip_spi_readburst(uint8_t* pBuf, uint16_t len) 	{};
#if 1
// 20231018 taylor
void 	wizchip_spi_readburst(uint8_t* pBuf, uint16_t len) {
    uint16_t i;
    for (i = 0; i < len; i++) {
        *pBuf++ = WIZCHIP.IF.SPI._read_byte();
    }
}
#else
void 	wizchip_spi_readburst(uint8_t* pBuf, uint16_t len) 	{}
#endif

/**
    @brief Default function to burst write in SPI interface.
    @note This function help not to access wrong address. If you do not describe this function or register any functions,
    null function is called.
*/
//void 	wizchip_spi_writeburst(uint8_t* pBuf, uint16_t len) {};
#if 1
// 20231018 taylor
void 	wizchip_spi_writeburst(uint8_t* pBuf, uint16_t len) {
    uint16_t i;
    for (i = 0; i < len; i++) {
        WIZCHIP.IF.SPI._write_byte(*pBuf++);
    }
}
#else
void 	wizchip_spi_writeburst(uint8_t* pBuf, uint16_t len) {}
#endif
#if 1   //teddy 240122

/**
    @brief Default function to read in QSPI interface.
    @note This function help not to access wrong address. If you do not describe this function or register any functions,
    null function is called.
*/
void wizchip_qspi_read(uint8_t opcode, uint16_t addr, uint8_t* pBuf, uint16_t len) {}

/**
    @brief Default function to write in QSPI interface.
    @note This function help not to access wrong address. If you do not describe this function or register any functions,
    null function is called.
*/
void wizchip_qspi_write(uint8_t opcode, uint16_t addr, uint8_t* pBuf, uint16_t len) {}

#endif
/**
    @\ref _WIZCHIP instance
*/
//
//            Replace the assignment of struct members with the assingment of array
//
/*
    _WIZCHIP  WIZCHIP =
      {
      .id                  = _WIZCHIP_ID_,
      .if_mode             = _WIZCHIP_IO_MODE_,
      .CRIS._enter         = wizchip_cris_enter,
      .CRIS._exit          = wizchip_cris_exit,
      .CS._select          = wizchip_cs_select,
      .CS._deselect        = wizchip_cs_deselect,
      .IF.BUS._read_byte   = wizchip_bus_readbyte,
      .IF.BUS._write_byte  = wizchip_bus_writebyte
    //    .IF.SPI._read_byte   = wizchip_spi_readbyte,
    //    .IF.SPI._write_byte  = wizchip_spi_writebyte
      };
*/
_WIZCHIP  WIZCHIP = {
    _WIZCHIP_IO_MODE_,
    {'W','6','1','0','0','\0','\0','\0'}, // _WIZCHIP_ID_ (Fixed for TI C2000 Assembler bug)
    {
        wizchip_cris_enter,
        wizchip_cris_exit
    },
    {
        wizchip_cs_select,
        wizchip_cs_deselect
    },
    {
        {
            //wizchip_bus_readbyte,
            //wizchip_bus_writebyte
            wizchip_bus_readdata,
            wizchip_bus_writedata
        },

    }
};

static uint8_t    _DNS_[4];      // DNS server ip address
static uint8_t      _DNS6_[16];
static ipconf_mode  _IPMODE_;      ///< IP configuration mode

void reg_wizchip_cris_cbfunc(void(*cris_en)(void), void(*cris_ex)(void)) {
    if (!cris_en || !cris_ex) {
        WIZCHIP.CRIS._enter = wizchip_cris_enter;
        WIZCHIP.CRIS._exit  = wizchip_cris_exit;
    } else {
        WIZCHIP.CRIS._enter = cris_en;
        WIZCHIP.CRIS._exit  = cris_ex;
    }
}

void reg_wizchip_cs_cbfunc(void(*cs_sel)(void), void(*cs_desel)(void)) {
    if (!cs_sel || !cs_desel) {
        WIZCHIP.CS._select   = wizchip_cs_select;
        WIZCHIP.CS._deselect = wizchip_cs_deselect;
    } else {
        WIZCHIP.CS._select   = cs_sel;
        WIZCHIP.CS._deselect = cs_desel;
    }
}

//void reg_wizchip_bus_cbfunc(uint8_t(*bus_rb)(uint32_t addr), void (*bus_wb)(uint32_t addr, uint8_t wb))
void reg_wizchip_bus_cbfunc(iodata_t(*bus_rb)(uint32_t addr), void (*bus_wb)(uint32_t addr, iodata_t wb)) {
    while (!(WIZCHIP.if_mode & _WIZCHIP_IO_MODE_BUS_));
    /*
        if(!bus_rb || !bus_wb)
        {
        WIZCHIP.IF.BUS._read_byte   = wizchip_bus_readbyte;
        WIZCHIP.IF.BUS._write_byte  = wizchip_bus_writebyte;
        }
        else
        {
        WIZCHIP.IF.BUS._read_byte   = bus_rb;
        WIZCHIP.IF.BUS._write_byte  = bus_wb;
        }
    */
    if (!bus_rb || !bus_wb) {
        WIZCHIP.IF.BUS._read_data   = wizchip_bus_readdata;
        WIZCHIP.IF.BUS._write_data  = wizchip_bus_writedata;
    } else {
        WIZCHIP.IF.BUS._read_data   = bus_rb;
        WIZCHIP.IF.BUS._write_data  = bus_wb;
    }
}
#if 1
// 20231103 taylor
void reg_wizchip_busbuf_cbfunc(void(*busbuf_rb)(uint32_t AddrSel, iodata_t* pBuf, int16_t len, uint8_t addrinc), void (*busbuf_wb)(uint32_t AddrSel, iodata_t* pBuf, int16_t len, uint8_t addrinc)) {
    while (!(WIZCHIP.if_mode & _WIZCHIP_IO_MODE_BUS_));
    /*
        if(!bus_rb || !bus_wb)
        {
        WIZCHIP.IF.BUS._read_byte   = wizchip_bus_readbyte;
        WIZCHIP.IF.BUS._write_byte  = wizchip_bus_writebyte;
        }
        else
        {
        WIZCHIP.IF.BUS._read_byte   = bus_rb;
        WIZCHIP.IF.BUS._write_byte  = bus_wb;
        }
    */
    if (!busbuf_rb || !busbuf_wb) {
        WIZCHIP.IF.BUS._read_data_buf   = wizchip_bus_read_buf;
        WIZCHIP.IF.BUS._write_data_buf  = wizchip_bus_write_buf;
    } else {
        WIZCHIP.IF.BUS._read_data_buf   = busbuf_rb;
        WIZCHIP.IF.BUS._write_data_buf  = busbuf_wb;
    }
}
#endif

void reg_wizchip_spi_cbfunc(uint8_t (*spi_rb)(void),
                            void (*spi_wb)(uint8_t wb),
                            void (*spi_rbuf)(uint8_t* buf, datasize_t len),
                            void (*spi_wbuf)(uint8_t* buf, datasize_t len)) {
    while (!(WIZCHIP.if_mode & _WIZCHIP_IO_MODE_SPI_));

    if (!spi_rb) {
        WIZCHIP.IF.SPI._read_byte      = wizchip_spi_readbyte;
    } else {
        WIZCHIP.IF.SPI._read_byte      = spi_rb;
    }
    if (!spi_wb) {
        WIZCHIP.IF.SPI._write_byte     = wizchip_spi_writebyte;
    } else {
        WIZCHIP.IF.SPI._write_byte     = spi_wb;
    }

    if (!spi_rbuf) {
        WIZCHIP.IF.SPI._read_burst  = wizchip_spi_readburst;
    } else {
        WIZCHIP.IF.SPI._read_burst  = spi_rbuf;
    }
    if (!spi_wbuf) {
        WIZCHIP.IF.SPI._write_burst = wizchip_spi_writeburst;
    } else {
        WIZCHIP.IF.SPI._write_burst = spi_wbuf;
    }
}

// 20140626 Eric Added for SPI burst operations
void reg_wizchip_spiburst_cbfunc(void (*spi_rb)(uint8_t* pBuf, uint16_t len), void (*spi_wb)(uint8_t* pBuf, uint16_t len)) {
    while (!(WIZCHIP.if_mode & _WIZCHIP_IO_MODE_SPI_));

    if (!spi_rb || !spi_wb) {
        WIZCHIP.IF.SPI._read_burst   = wizchip_spi_readburst;
        WIZCHIP.IF.SPI._write_burst  = wizchip_spi_writeburst;
    } else {
        WIZCHIP.IF.SPI._read_burst   = spi_rb;
        WIZCHIP.IF.SPI._write_burst  = spi_wb;
    }
}
#if 1 //teddy 240122
void reg_wizchip_qspi_cbfunc(void (*qspi_rb)(uint8_t opcode, uint16_t addr, uint8_t* pBuf, uint16_t len),
                             void (*qspi_wb)(uint8_t opcode, uint16_t addr, uint8_t* pBuf, uint16_t len)) {
    while (!(WIZCHIP.if_mode & _WIZCHIP_IO_MODE_SPI_QSPI_));

    if (!qspi_rb || !qspi_wb) {
        WIZCHIP.IF.QSPI._read_qspi   = wizchip_qspi_read;
        WIZCHIP.IF.QSPI._write_qspi  = wizchip_qspi_write;
    } else {
        WIZCHIP.IF.QSPI._read_qspi   = qspi_rb;
        WIZCHIP.IF.QSPI._write_qspi  = qspi_wb;
    }
}
#endif

/*
 * -----------------------------------------------------------------------------
 * 함수명    : ctlwizchip
 * 역할      : W6100 하드웨어 전반(리셋, 메모리 크기 초기화 등)을 제어하기 위한 다목적 제어 함수입니다.
 * 매개변수  :
 *   - cwtype : 제어 명령 타입 (예: CW_RESET_WIZCHIP, CW_INIT_WIZCHIP)
 *   - arg    : 제어 명령에 사용될 인수 포인터
 * 반환값    : 성공 시 0, 실패 시 -1
 * -----------------------------------------------------------------------------
 */
int8_t ctlwizchip(ctlwizchip_type cwtype, void* arg) {
    //teddy 240122
    uint8_t tmp = *(uint8_t*) arg;
    uint8_t* ptmp[2] = {0, 0};
    switch (cwtype) {
        //teddy 240122
    case CW_SYS_LOCK:
        if (tmp & SYS_CHIP_LOCK) {
            CHIPLOCK();
        }
        if (tmp & SYS_NET_LOCK) {
            NETLOCK();
        }
        if (tmp & SYS_PHY_LOCK) {
            PHYLOCK();
        }
        break;
    case CW_SYS_UNLOCK:
        if (tmp & SYS_CHIP_LOCK) {
            CHIPUNLOCK();
        }
        if (tmp & SYS_NET_LOCK) {
            NETUNLOCK();
        }
        if (tmp & SYS_PHY_LOCK) {
            PHYUNLOCK();
        }
        break;
    case CW_GET_SYSLOCK:
        *(uint8_t*)arg = getSYSR() >> 5;
        break;
    case CW_RESET_WIZCHIP:
        wizchip_sw_reset();
        break;
    case CW_INIT_WIZCHIP:
        if (arg != 0) {
            ptmp[0] = (uint8_t*)arg;
            ptmp[1] = ptmp[0] + _WIZCHIP_SOCK_NUM_;
        }
        return wizchip_init(ptmp[0], ptmp[1]);
    case CW_CLR_INTERRUPT:
        wizchip_clrinterrupt(*((intr_kind*)arg));
        break;
    case CW_GET_INTERRUPT:
        *((intr_kind*)arg) = wizchip_getinterrupt();
        break;
    case CW_SET_INTRMASK:
        wizchip_setinterruptmask(*((intr_kind*)arg));
        break;
    case CW_GET_INTRMASK:
        *((intr_kind*)arg) = wizchip_getinterruptmask();
        break;
    case CW_SET_INTRTIME:
        setINTPTMR(*(uint16_t*)arg);
        break;
    case CW_GET_INTRTIME:
        *(uint16_t*)arg = getINTPTMR();
        break;
    case CW_GET_ID:
        ((uint8_t*)arg)[0] = WIZCHIP.id[0];
        ((uint8_t*)arg)[1] = WIZCHIP.id[1];
        ((uint8_t*)arg)[2] = WIZCHIP.id[2];
        ((uint8_t*)arg)[3] = WIZCHIP.id[3];
        ((uint8_t*)arg)[4] = WIZCHIP.id[4];
        ((uint8_t*)arg)[5] = WIZCHIP.id[5];
        ((uint8_t*)arg)[6] = 0;
        break;
#if 1
        // 20231017 taylor//teddy 240122
    case CW_GET_VER:
        *(uint16_t*)arg = getVER();
        break;
#endif
        //teddy 240122
    case CW_RESET_PHY:
        wizphy_reset();
        break;
    case CW_SET_PHYCONF:
        wizphy_setphyconf((wiz_PhyConf*)arg);
        break;
    case CW_GET_PHYCONF:
        wizphy_getphyconf((wiz_PhyConf*)arg);
        break;
    case CW_GET_PHYSTATUS:
#if 1
        // 20231012 taylor
#else
        wizphy_getphystat((wiz_PhyConf*)arg);
#endif
        break;
    case CW_SET_PHYPOWMODE:
        //teddy 240122
        wizphy_setphypmode(*(uint8_t*)arg);
        break;
        //teddy 240122
    case CW_GET_PHYPOWMODE:
        tmp = wizphy_getphypmode();
        if ((int8_t)tmp == -1) {
            return -1;
        }
        *(uint8_t*)arg = tmp;
        break;
    case CW_GET_PHYLINK:
        tmp = wizphy_getphylink();
        if ((int8_t)tmp == -1) {
            return -1;
        }
        *(uint8_t*)arg = tmp;
        break;
    default:
        return -1;
    }
    return 0;
}

/*
 * -----------------------------------------------------------------------------
 * 함수명    : ctlnetwork
 * 역할      : 네트워크 설정(IP/MAC 설정, 네트워크 모드 설정 등)을 제어하기 위한 다목적 함수입니다.
 * 매개변수  :
 *   - cntype : 네트워크 제어 명령 타입 (예: CN_SET_NETINFO, CN_GET_NETINFO)
 *   - arg    : 제어 명령에 사용될 인수 포인터
 * 반환값    : 성공 시 0, 실패 시 -1
 * -----------------------------------------------------------------------------
 */
int8_t ctlnetwork(ctlnetwork_type cntype, void* arg) {

    switch (cntype) {
    case CN_SET_NETINFO:
        wizchip_setnetinfo((wiz_NetInfo*)arg);
        break;
    case CN_GET_NETINFO:
        wizchip_getnetinfo((wiz_NetInfo*)arg);
        break;
    case CN_SET_NETMODE:
        wizchip_setnetmode(*(netmode_type*)arg);
    case CN_GET_NETMODE:
        *(netmode_type*)arg = wizchip_getnetmode();
        break;
    case CN_SET_TIMEOUT:
        wizchip_settimeout((wiz_NetTimeout*)arg);
        break;
    case CN_GET_TIMEOUT:
        wizchip_gettimeout((wiz_NetTimeout*)arg);
        break;
        //teddy 240122
    case CN_SET_PREFER:
        setSLPSR(*(uint8_t*)arg);
        break;
    case CN_GET_PREFER:
        *(uint8_t*)arg = getSLPSR();
        break;
    default:
        return -1;
    }
    return 0;
}

void wizchip_sw_reset(void) {
    uint8_t gw[4], sn[4], sip[4];
    uint8_t mac[6];
    //teddy 240122
    uint8_t gw6[16], sn6[16], lla[16], gua[16];
    uint8_t islock = getSYSR();

    CHIPUNLOCK();

    getSHAR(mac);
    getGAR(gw);  getSUBR(sn);  getSIPR(sip); getGA6R(gw6); getSUB6R(sn6); getLLAR(lla); getGUAR(gua);
    setSYCR0(SYCR0_RST);
    getSYCR0(); // for delay

    NETUNLOCK();

    setSHAR(mac);
    setGAR(gw);
    setSUBR(sn);
    setSIPR(sip);
    setGA6R(gw6);
    setSUB6R(sn6);
    setLLAR(lla);
    setGUAR(gua);
    if (islock & SYSR_CHPL) {
        CHIPLOCK();
    }
    if (islock & SYSR_NETL) {
        NETLOCK();
    }
}

/*
 * -----------------------------------------------------------------------------
 * 함수명    : wizchip_init
 * 역할      : W6100의 모든 소켓에 대한 Tx/Rx 버퍼 메모리 할당 크기를 초기화합니다.
 *             배열 포인터가 NULL인 경우, 칩의 기본값으로 전체 버퍼 메모리를 균등하게 할당합니다.
 * 매개변수  :
 *   - txsize : 소켓별 Tx 버퍼 크기가 지정된 배열 포인터 (단위: KByte)
 *   - rxsize : 소켓별 Rx 버퍼 크기가 지정된 배열 포인터 (단위: KByte)
 * 반환값    : 버퍼 설정이 칩의 물리적 한계를 초과하면 -1, 성공하면 0
 * -----------------------------------------------------------------------------
 */
int8_t wizchip_init(uint8_t* txsize, uint8_t* rxsize) {
    int8_t i;
    int8_t tmp = 0;
    wizchip_sw_reset();
    if (txsize) {
        tmp = 0;
        for (i = 0 ; i < _WIZCHIP_SOCK_NUM_; i++) {
            tmp += txsize[i];

            if (tmp > 16) {
                return -1;
            }
        }
        for (i = 0 ; i < _WIZCHIP_SOCK_NUM_; i++) {
            setSn_TXBUF_SIZE(i, txsize[i]);
        }
    }

    if (rxsize) {
        tmp = 0;
        for (i = 0 ; i < _WIZCHIP_SOCK_NUM_; i++) {
            tmp += rxsize[i];
            if (tmp > 16) {
                return -1;
            }
        }
        for (i = 0 ; i < _WIZCHIP_SOCK_NUM_; i++) {
            setSn_RXBUF_SIZE(i, rxsize[i]);
        }
    }
    return 0;
}

void wizchip_clrinterrupt(intr_kind intr) {
    uint8_t ir  = (uint8_t)intr;
    uint8_t sir = (uint8_t)((uint16_t)intr >> 8);

    //teddy 240122
    int i;
    uint8_t slir = (uint8_t)((uint32_t)intr >> 16);
    setIRCLR(ir);
    for (i = 0; i < _WIZCHIP_SOCK_NUM_; i++) {
        if (sir & (1 << i)) {
            setSn_IRCLR(i, 0xFF);
        }
    }
    setSLIRCLR(slir);
    return;

    setIR(ir);
    //setSIR(sir);
    for (ir = 0; ir < 8; ir++) {
        if (sir & (0x01 << ir)) {
            setSn_IR(ir, 0xff);
        }
    }

}

intr_kind wizchip_getinterrupt(void) {
    uint8_t ir  = 0;
    uint8_t sir = 0;
    uint32_t ret = 0;

    ir  = getIR();
    sir = getSIR();

    ret = sir;
    ret = (ret << 8) + ir;
    //teddy 240122
    ret = (((uint32_t)getSLIR()) << 16) | ret;

    return (intr_kind)ret;
}

void wizchip_setinterruptmask(intr_kind intr) {
    uint8_t imr  = (uint8_t)intr;
    uint8_t simr = (uint8_t)((uint16_t)intr >> 8);

    setIMR(imr);
    setSIMR(simr);
    //teddy 240122
    uint8_t slimr = (uint8_t)((uint32_t)intr >> 16);
    setSLIMR(slimr);
}

intr_kind wizchip_getinterruptmask(void) {
    uint8_t imr  = 0;
    uint8_t simr = 0;
    uint32_t ret = 0;
    imr  = getIMR();
    simr = getSIMR();

    ret = simr;
    ret = (ret << 8) + imr;

    //teddy 240122
    ret = (((uint32_t)getSLIMR()) << 16) | ret;

    return (intr_kind)ret;
}

int8_t wizphy_getphylink(void) {
    int8_t tmp = PHY_LINK_OFF;

#if (_PHY_IO_MODE_ == _PHY_IO_MODE_PHYCR_)
    return (getPHYSR() & PHYSR_LNK);
#elif (_PHY_IO_MODE_ == _PHY_IO_MODE_MII_)
    if (wiz_mdio_read(PHYRAR_BMSR) & BMSR_LINK_STATUS) {
        return PHY_LINK_ON;
    }
    return PHY_LINK_OFF;
#endif

    return tmp;
}

int8_t wizphy_getphypmode(void) {
    int8_t tmp = 0;
#if (_PHY_IO_MODE_ == _PHY_IO_MODE_PHYCR_)
    if (getPHYCR1() & PHYCR1_PWDN) {
        return PHY_POWER_DOWN;
    }
#elif (_PHY_IO_MODE_ == _PHY_IO_MODE_MII_)
    if (wiz_mdio_read(PHYRAR_BMCR) & BMCR_PWDN) {
        return PHY_POWER_DOWN;
    }
#endif
    return PHY_POWER_NORM;
    return tmp;
}

void wizphy_reset(void) {
#if (_PHY_IO_MODE_ == _PHY_IO_MODE_PHYCR_)
    uint8_t tmp = getPHYCR1() | PHYCR1_RST;
    PHYUNLOCK();
    setPHYCR1(tmp);
    PHYLOCK();
#elif (_PHY_IO_MODE_ == _PHY_IO_MODE_MII_)
    wiz_mdio_write(PHYRAR_BMCR, wiz_mdio_read(PHYRAR_BMCR) | BMCR_RST);
    while (wiz_mdio_read(PHYRAR_BMCR) & BMCR_RST);
#endif
}

void wizphy_setphyconf(wiz_PhyConf* phyconf) {
#if (_PHY_IO_MODE_ == _PHY_IO_MODE_PHYCR_)
    uint8_t tmp = 0;
    if (phyconf->mode == PHY_MODE_TE) {
        setPHYCR1(getPHYCR1() | PHYCR1_TE);
        tmp = PHYCR0_AUTO;
    } else {
        setPHYCR1(getPHYCR1() & ~PHYCR1_TE);
        if (phyconf->mode == PHY_MODE_AUTONEGO) {
            tmp = PHYCR0_AUTO;
        } else {
            tmp |= 0x04;
            if (phyconf->speed  == PHY_SPEED_10) {
                tmp |= 0x02;
            }
            if (phyconf->duplex == PHY_DUPLEX_HALF) {
                tmp |= 0x01;
            }
        }
    }
    setPHYCR0(tmp);
#elif (_PHY_IO_MODE_ == _PHY_IO_MODE_MII_)
    uint16_t tmp = wiz_mdio_read(PHYRAR_BMCR);
    if (phyconf->mode == PHY_MODE_TE) {
        setPHYCR1(getPHYCR1() | PHYCR1_TE);
        setPHYCR0(PHYCR0_AUTO);
    } else {
        setPHYCR1(getPHYCR1() & ~PHYCR1_TE);
        if (phyconf->mode == PHY_MODE_AUTONEGO) {
            tmp |= BMCR_ANE;
        } else {
            tmp &= ~(BMCR_ANE | BMCR_DPX | BMCR_SPD);
            if (phyconf->duplex == PHY_DUPLEX_FULL) {
                tmp |= BMCR_DPX;
            }
            if (phyconf->speed == PHY_SPEED_100) {
                tmp |= BMCR_SPD;
            }
        }
        wiz_mdio_write(PHYRAR_BMCR, tmp);
    }
#endif
}

void wizphy_getphyconf(wiz_PhyConf* phyconf) {
#if (_PHY_IO_MODE_ == _PHY_IO_MODE_PHYCR_)
    uint8_t tmp = 0;
    tmp = getPHYSR();
    if (getPHYCR1() & PHYCR1_TE) {
        phyconf->mode = PHY_MODE_TE;
    } else {
        phyconf->mode = (tmp & (1 << 5)) ? PHY_MODE_MANUAL : PHY_MODE_AUTONEGO ;
    }
    phyconf->speed  = (tmp & (1 << 4)) ? PHY_SPEED_10    : PHY_SPEED_100;
    phyconf->duplex = (tmp & (1 << 3)) ? PHY_DUPLEX_HALF : PHY_DUPLEX_FULL;
#elif (_PHY_IO_MODE_ == _PHY_IO_MODE_MII_)
    uint16_t tmp = 0;
    tmp = wiz_mdio_read(PHYRAR_BMCR);
    if (getPHYCR1() & PHYCR1_TE) {
        phyconf->mode = PHY_MODE_TE;
    } else {
        phyconf->mode   = (tmp & BMCR_ANE) ? PHY_MODE_AUTONEGO : PHY_MODE_MANUAL;
    }
    phyconf->duplex = (tmp & BMCR_DPX) ? PHY_DUPLEX_FULL   : PHY_DUPLEX_HALF;
    phyconf->speed  = (tmp & BMCR_SPD) ? PHY_SPEED_100     : PHY_SPEED_10;
#endif
}

void wizphy_getphystat(wiz_PhyConf* phyconf) {
    uint8_t tmp = 0;
    tmp = getPHYSR();
    if (getPHYCR1() & PHYCR1_TE) {
        phyconf->mode = PHY_MODE_TE;
    } else {
        phyconf->mode   = (tmp & (1 << 5))    ? PHY_MODE_MANUAL : PHY_MODE_AUTONEGO ;
    }
    phyconf->speed  = (tmp & PHYSR_SPD) ? PHY_SPEED_10    : PHY_SPEED_100;
    phyconf->duplex = (tmp & PHYSR_DPX) ? PHY_DUPLEX_HALF : PHY_DUPLEX_FULL;
}

void wizphy_setphypmode(uint8_t pmode) {
#if (_PHY_IO_MODE_ == _PHY_IO_MODE_PHYCR_)
    uint8_t tmp = getPHYCR1();
    if (pmode == PHY_POWER_DOWN) {
        tmp |= PHYCR1_PWDN;
    } else {
        tmp &= ~PHYCR1_PWDN;
    }
    setPHYCR1(tmp);
#elif (_PHY_IO_MODE_ == _PHY_IO_MODE_MII_)
    uint16_t tmp = 0;
    tmp = wiz_mdio_read(PHYRAR_BMCR);
    if (pmode == PHY_POWER_DOWN) {
        tmp |= BMCR_PWDN;
    } else {
        tmp &= ~BMCR_PWDN;
    }
    wiz_mdio_write(PHYRAR_BMCR, tmp);
#endif
}

int8_t wizchip_arp(wiz_ARP* arp) {
    uint8_t tmp;
    if (arp->destinfo.len == 16) {
        setSLDIP6R(arp->destinfo.ip);
        setSLCR(SLCR_ARP6);
    } else {
        setSLDIP4R(arp->destinfo.ip);
        setSLCR(SLCR_ARP4);
    }
    while (getSLCR());
    while ((tmp = getSLIR()) == 0x00);
    setSLIRCLR(~SLIR_RA);
    if (tmp & (SLIR_ARP4 | SLIR_ARP6)) {
        getSLDHAR(arp->dha);
        return 0;
    }
    return -1;
}

int8_t wizchip_ping(wiz_PING* ping) {
    uint8_t tmp;
    setPINGIDR(ping->id);
    setPINGSEQR(ping->seq);
    if (ping->destinfo.len == 16) {
        setSLDIP6R(ping->destinfo.ip);
        setSLCR(SLCR_PING6);
    } else {
        setSLDIP4R(ping->destinfo.ip);
        setSLCR(SLCR_PING4);
    }
    while (getSLCR());
    while ((tmp = getSLIR()) == 0x00);
    setSLIRCLR(~SLIR_RA);
    if (tmp & (SLIR_PING4 | SLIR_PING6)) {
        return 0;
    }
    return -1;
}

int8_t wizchip_dad(uint8_t* ipv6) {
    uint8_t tmp;
    setSLDIP6R(ipv6);
    setSLCR(SLCR_NS);
    while (getSLCR());
    while ((tmp = getSLIR()) == 0x00);
    setSLIRCLR(~SLIR_RA);
    if (tmp & SLIR_TOUT) {
        return 0;
    }
    return -1;
}

int8_t wizchip_slaac(wiz_Prefix* prefix) {
    uint8_t tmp;
    setSLCR(SLCR_RS);
    while (getSLCR());
    while ((tmp = getSLIR()) == 0x00);
    setSLIRCLR(~SLIR_RA);
    if (tmp & SLIR_RS) {
        prefix->len = getPLR();
        prefix->flag = getPFR();
        prefix->valid_lifetime = getVLTR();
        prefix->preferred_lifetime = getPLTR();
        getPAR(prefix->prefix);
        return 0;
    }
    return -1;
}

int8_t wizchip_unsolicited(void) {
    uint8_t tmp;
    setSLCR(SLCR_UNA);
    while (getSLCR());
    while ((tmp = getSLIR()) == 0x00);
    setSLIRCLR(~SLIR_RA);
    if (tmp & SLIR_TOUT) {
        return 0;
    }
    return -1;
}

int8_t wizchip_getprefix(wiz_Prefix * prefix) {
    if (getSLIR() & SLIR_RA) {
        prefix->len = getPLR();
        prefix->flag = getPFR();
        prefix->valid_lifetime = getVLTR();
        prefix->preferred_lifetime = getPLTR();
        getPAR(prefix->prefix);
        setSLIRCLR(SLIR_RA);
    }
    return -1;
}

/*
 * -----------------------------------------------------------------------------
 * 함수명    : wizchip_setnetinfo
 * 역할      : MAC 주소, IP 주소, 서브넷 마스크, 게이트웨이 등 네트워크 기본 설정값을 W6100에 적용합니다.
 * 매개변수  :
 *   - pnetinfo : 적용할 네트워크 정보가 담긴 구조체 포인터 (wiz_NetInfo)
 * 반환값    : 없음
 * -----------------------------------------------------------------------------
 */
void wizchip_setnetinfo(wiz_NetInfo* pnetinfo) {
    uint8_t i = 0;
    setSHAR(pnetinfo->mac);
    setGAR(pnetinfo->gw);
    setSUBR(pnetinfo->sn);
    setSIPR(pnetinfo->ip);
    setGA6R(pnetinfo->gw6);
    setSUB6R(pnetinfo->sn6);
    setLLAR(pnetinfo->lla);
    setGUAR(pnetinfo->gua);

    for (i = 0; i < 4; i++) {
        _DNS_[i]  = pnetinfo->dns[i];
    }
    for (i = 0; i < 16; i++) {
        _DNS6_[i] = pnetinfo->dns6[i];
    }

    _IPMODE_   = pnetinfo->ipmode;
}

/*
 * -----------------------------------------------------------------------------
 * 함수명    : wizchip_getnetinfo
 * 역할      : 현재 W6100 칩에 설정되어 있는 네트워크 기본 설정값(IP, MAC 등)을 읽어옵니다.
 * 매개변수  :
 *   - pnetinfo : 정보를 저장할 구조체 포인터 (wiz_NetInfo)
 * 반환값    : 없음
 * -----------------------------------------------------------------------------
 */
void wizchip_getnetinfo(wiz_NetInfo* pnetinfo) {
    uint8_t i = 0;
    getSHAR(pnetinfo->mac);

    getGAR(pnetinfo->gw);
    getSUBR(pnetinfo->sn);
    getSIPR(pnetinfo->ip);

    getGA6R(pnetinfo->gw6);
    getSUB6R(pnetinfo->sn6);
    getLLAR(pnetinfo->lla);
    getGUAR(pnetinfo->gua);
    for (i = 0; i < 4; i++) {
        pnetinfo->dns[i] = _DNS_[i];
    }
    for (i = 0; i < 16; i++) {
        pnetinfo->dns6[i]  = _DNS6_[i];
    }

    pnetinfo->ipmode = _IPMODE_;
}

void wizchip_setnetmode(netmode_type netmode) {
    uint32_t tmp = (uint32_t) netmode;
    setNETMR((uint8_t)tmp);
    setNETMR2((uint8_t)(tmp >> 8));
    setNET4MR((uint8_t)(tmp >> 16));
    setNET6MR((uint8_t)(tmp >> 24));
}

netmode_type wizchip_getnetmode(void) {
    uint32_t ret = 0;
    ret = getNETMR();
    ret = (ret << 8)  + getNETMR2();
    ret = (ret << 16) + getNET4MR();
    ret = (ret << 24) + getNET6MR();
    return (netmode_type)ret;
}

// netmode_type wizchip_getnetmode(void)
// {
//    return (netmode_type) getMR();
// }

/*
 * -----------------------------------------------------------------------------
 * 함수명    : wizchip_settimeout
 * 역할      : W6100 칩의 재전송 타임아웃 시간(RTR)과 재전송 시도 횟수(RCR)를 설정합니다.
 * 매개변수  :
 *   - nettime : 설정할 재전송 타임아웃 관련 정보가 담긴 구조체 포인터
 * 반환값    : 없음
 * -----------------------------------------------------------------------------
 */
void wizchip_settimeout(wiz_NetTimeout* nettime) {
    setRCR(nettime->s_retry_cnt);
    setRTR(nettime->s_time_100us);
    setSLRCR(nettime->sl_retry_cnt);
    setSLRTR(nettime->sl_time_100us);
}

/*
 * -----------------------------------------------------------------------------
 * 함수명    : wizchip_gettimeout
 * 역할      : W6100 칩에 설정된 재전송 타임아웃 시간(RTR)과 재전송 시도 횟수(RCR)를 읽어옵니다.
 * 매개변수  :
 *   - nettime : 읽어온 정보를 저장할 구조체 포인터
 * 반환값    : 없음
 * -----------------------------------------------------------------------------
 */
void wizchip_gettimeout(wiz_NetTimeout* nettime) {
    nettime->s_retry_cnt   = getRCR();
    nettime->s_time_100us  = getRTR();
    nettime->sl_retry_cnt  = getSLRCR();
    nettime->sl_time_100us = getSLRTR();
}

