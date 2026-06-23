/**********************************************************************
    Nexcom Co., Ltd.
    Filename         : wizchip_conf.c
    Version          : 00.02
    Description      : WIZnet 이더넷 라이브러리 파일
    Programmer       : Kim Jeonghwan
    Last Updated     : 2026. 06. 16. (IPv6 및 미사용 I/O 인터페이스 관련 오류 수정)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 06. 16. - 전체 함수에 CSU/HAL 표준 주석 포맷 및 한글화 적용
 * 2026. 06. 16. - 정적시험 관련 IPv6, BUS/QSPI 미사용 I/O 제거 후 발생한 컴파일 에러 수정
 * 2026. 06. 15. - 정적시험 통과를 위한 타기종 및 미사용 TCP/IPv6 기능 전면 삭제
 */
#include <stddef.h>
#include "wizchip_conf.h"

#pragma diag_suppress 69
#pragma diag_suppress 1269

/*
@function    void wizchip_cris_enter(void)
@brief      크리티컬 섹션(인터럽트 비활성화) 진입을 위한 기본 콜백 함수입니다.
@param      void
@return     void
*/

void 	  wizchip_cris_enter(void)           {}

/*
@function    void wizchip_cris_exit(void)
@brief      크리티컬 섹션(인터럽트 활성화) 종료를 위한 기본 콜백 함수입니다.
@param      void
@return     void
*/

void 	  wizchip_cris_exit(void)          {}

/*
@function    void wizchip_cs_select(void)
@brief      WIZCHIP 칩 선택(CS Low)을 위한 기본 콜백 함수입니다.
@param      void
@return     void
*/

void 	wizchip_cs_select(void)            {}

/*
@function    void wizchip_cs_deselect(void)
@brief      WIZCHIP 칩 선택 해제(CS High)를 위한 기본 콜백 함수입니다.
@param      void
@return     void
*/

void 	wizchip_cs_deselect(void)          {}

/*
@function    void wizchip_spi_writebyte(uint8_t wb)
@brief      SPI 인터페이스에서 1바이트를 쓰기 위한 기본 콜백 함수입니다.
@param      wb: 전송할 1바이트 데이터
@return     void
*/

void 	wizchip_spi_writebyte(uint8_t wb) {}


/*
@function    uint8_t wizchip_spi_readbyte(void)
@brief      SPI 인터페이스에서 1바이트를 읽기 위한 기본 콜백 함수입니다.
@param      void
@return     읽어온 1바이트 데이터
*/
uint8_t wizchip_spi_readbyte(void)        {
    return 0;
}

/*
@function    void wizchip_spi_readburst(uint8_t* pBuf, uint16_t len)
@brief      SPI 인터페이스에서 여러 바이트(버스트)를 연속으로 읽기 위한 기본 콜백 함수입니다.
@param      pBuf: 수신된 데이터를 저장할 버퍼 포인터
@param      len: 읽어올 데이터 길이
@return     void
*/
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

/*
@function    void wizchip_spi_writeburst(uint8_t* pBuf, uint16_t len)
@brief      SPI 인터페이스에서 여러 바이트(버스트)를 연속으로 쓰기 위한 기본 콜백 함수입니다.
@param      pBuf: 전송할 데이터가 저장된 버퍼 포인터
@param      len: 전송할 데이터 길이
@return     void
*/
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

_WIZCHIP  WIZCHIP = {
    _WIZCHIP_IO_MODE_,
    {'W','6','1','0','0', 0, 0, 0}, // _WIZCHIP_ID_ (Fixed for TI C2000 Assembler bug)
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
            wizchip_spi_readbyte,
            wizchip_spi_writebyte,
            wizchip_spi_readburst,
            wizchip_spi_writeburst
        }
    }
};

static uint8_t    _DNS_[4];      // DNS server ip address
static ipconf_mode  _IPMODE_;      ///< IP configuration mode

/*
@function    void reg_wizchip_cris_cbfunc(void (*cris_en)(void), void (*cris_ex)(void))
@brief      WIZCHIP 레지스터 읽기/쓰기 시 동시 접근을 막기 위한 크리티컬 섹션 콜백 함수를 등록합니다.
@param      cris_en: 크리티컬 섹션 진입(Enter) 콜백 포인터
@param      cris_ex: 크리티컬 섹션 종료(Exit) 콜백 포인터
@return     void
*/
void reg_wizchip_cris_cbfunc(void(*cris_en)(void), void(*cris_ex)(void)) {
    if (!cris_en || !cris_ex) {
        WIZCHIP.CRIS._enter = wizchip_cris_enter;
        WIZCHIP.CRIS._exit  = wizchip_cris_exit;
    } else {
        WIZCHIP.CRIS._enter = cris_en;
        WIZCHIP.CRIS._exit  = cris_ex;
    }
}

/*
@function    void reg_wizchip_cs_cbfunc(void (*cs_sel)(void), void (*cs_desel)(void))
@brief      통신을 시작하고 끝낼 때 호출되는 WIZCHIP 칩 선택(CS) 콜백 함수를 등록합니다.
@param      cs_sel: 칩 선택(CS Low) 콜백 포인터
@param      cs_desel: 칩 해제(CS High) 콜백 포인터
@return     void
*/
void reg_wizchip_cs_cbfunc(void(*cs_sel)(void), void(*cs_desel)(void)) {
    if (!cs_sel || !cs_desel) {
        WIZCHIP.CS._select   = wizchip_cs_select;
        WIZCHIP.CS._deselect = wizchip_cs_deselect;
    } else {
        WIZCHIP.CS._select   = cs_sel;
        WIZCHIP.CS._deselect = cs_desel;
    }
}

/*
@function    void reg_wizchip_spi_cbfunc(uint8_t (*spi_rb)(void), void (*spi_wb)(uint8_t wb), void (*spi_rbuf)(uint8_t *buf, uint16_t len), void (*spi_wbuf)(uint8_t *buf, uint16_t len))
@brief      SPI 통신을 통한 1바이트 및 버스트 단위 읽기/쓰기 콜백 함수를 등록합니다.
@param      spi_rb: 1바이트 읽기 함수 포인터
@param      spi_wb: 1바이트 쓰기 함수 포인터
@param      spi_rbuf: 버스트 데이터 읽기 함수 포인터
@param      spi_wbuf: 버스트 데이터 쓰기 함수 포인터
@return     void
*/
void reg_wizchip_spi_cbfunc(uint8_t (*spi_rb)(void),
                            void (*spi_wb)(uint8_t wb),
                            void (*spi_rbuf)(uint8_t* buf, uint16_t len),
                            void (*spi_wbuf)(uint8_t* buf, uint16_t len)) {
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

/*
@function    void reg_wizchip_spiburst_cbfunc(void (*spi_rb)(uint8_t *pBuf, uint16_t len), void (*spi_wb)(uint8_t *pBuf, uint16_t len))
@brief      SPI 버스트 읽기/쓰기 콜백 함수만 별도로 등록합니다.
@param      spi_rb: 버스트 읽기 함수 포인터
@param      spi_wb: 버스트 쓰기 함수 포인터
@return     void
*/
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



/*
@function    int8_t ctlwizchip(ctlwizchip_type cwtype, void *arg)
@brief      WIZCHIP 하드웨어(PHY 리셋, 인터럽트 제어 등)를 제어하는 통합 관리
함수입니다.
@param      cwtype: 실행할 명령의 타입 (ctlwizchip_type 참고)
@param      arg: 명령에 따라 요구되는 인자 구조체 포인터
@return     성공 시 0 반환, 잘못된 명령일 경우 -1 반환
*/

int8_t ctlwizchip(ctlwizchip_type cwtype, void* arg) {
    //teddy 240122
    uint8_t tmp = 0;
    if(arg != NULL) {
        tmp = *(uint8_t*) arg;
    }
    int8_t ret_tmp = 0;
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
        ret_tmp = wizphy_getphypmode();
        if (ret_tmp == -1) {
            return -1;
        }
        *(uint8_t*)arg = (uint8_t)ret_tmp;
        break;
    case CW_GET_PHYLINK:
        ret_tmp = wizphy_getphylink();
        if (ret_tmp == -1) {
            return -1;
        }
        *(uint8_t*)arg = (uint8_t)ret_tmp;
        break;
    default:
        return -1;
    }
    return 0;
}

/*
@function    int8_t ctlnetwork(ctlnetwork_type cntype, void *arg)
@brief      WIZCHIP 네트워크 환경(IP, 서브넷, 타임아웃, 특수 모드 등)을
설정하거나 가져오는 통합 함수입니다.
@param      cntype: 실행할 명령의 타입 (ctlnetwork_type 참고)
@param      arg: 명령에 따라 요구되는 인자 구조체 포인터
@return     성공 시 0 반환, 잘못된 명령일 경우 -1 반환
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

/*
@function    void wizchip_sw_reset(void)
@brief      소프트웨어 레지스터 명령을 통해 WIZCHIP을 리셋합니다.
@param      void
@return     void
*/

void wizchip_sw_reset(void) {
    uint8_t gw[4], sn[4], sip[4];
    uint8_t mac[6];
    uint8_t islock = getSYSR();

    CHIPUNLOCK();

    getSHAR(mac);
    getGAR(gw);  getSUBR(sn);  getSIPR(sip);
    setSYCR0(SYCR0_RST);
    getSYCR0(); // for delay

    NETUNLOCK();

    setSHAR(mac);
    setGAR(gw);
    setSUBR(sn);
    setSIPR(sip);
    if (islock & SYSR_CHPL) {
        CHIPLOCK();
    }
    if (islock & SYSR_NETL) {
        NETLOCK();
    }
}

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

int8_t wizchip_init(uint8_t* txsize, uint8_t* rxsize) {
    uint8_t i;
    uint8_t tmp = 0;
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

/*
@function    void wizchip_clrinterrupt(intr_kind intr)
@brief      지정된 종류의 WIZCHIP 인터럽트를 클리어(해제)합니다.
@param      intr: 클리어할 인터럽트 종류를 지정 (intr_kind 값을 OR 연산하여 사용
가능)
@return     void
*/

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
}

/*
@function    intr_kind wizchip_getinterrupt(void)
@brief      칩셋에서 발생한 모든 인터럽트의 상태를 읽어옵니다.
@param      void
@return     현재 발생한 인터럽트들의 조합 값 (intr_kind의 OR 연산 결과)
*/

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

/*
@function    void wizchip_setinterruptmask(intr_kind intr)
@brief      외부 인터럽트 핀으로 신호를 발생시킬 인터럽트들을 마스크 설정합니다.
@param      intr: 마스킹을 허용할 인터럽트 종류들을 지정 (intr_kind 값을 OR
연산하여 사용)
@return     void
*/

void wizchip_setinterruptmask(intr_kind intr) {
    uint8_t imr  = (uint8_t)intr;
    uint8_t simr = (uint8_t)((uint16_t)intr >> 8);

    setIMR(imr);
    setSIMR(simr);
    //teddy 240122
    uint8_t slimr = (uint8_t)((uint32_t)intr >> 16);
    setSLIMR(slimr);
}

/*
@function    intr_kind wizchip_getinterruptmask(void)
@brief      현재 설정된 인터럽트 마스크 상태를 읽어옵니다.
@param      void
@return     현재 설정된 마스크 값 (intr_kind의 OR 연산 결과)
*/

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

/*
@function    int8_t wizphy_getphylink(void)
@brief      현재 이더넷 이더넷 케이블의 링크 연결 상태(UP/DOWN)를 읽어옵니다.
@param      void
@return     링크 연결됨(1), 링크 끊어짐(0) 반환
*/

int8_t wizphy_getphylink(void) {
#if (_PHY_IO_MODE_ == _PHY_IO_MODE_PHYCR_)
    return (getPHYSR() & PHYSR_LNK);
#elif (_PHY_IO_MODE_ == _PHY_IO_MODE_MII_)
    if (wiz_mdio_read(PHYRAR_BMSR) & BMSR_LINK_STATUS) {
        return PHY_LINK_ON;
    }
    return PHY_LINK_OFF;
#endif
}

/*
@function    int8_t wizphy_getphypmode(void)
@brief      내장 PHY 모듈의 동작 파워 모드(절전/정상 모드) 상태를 읽어옵니다.
@param      void
@return     정상 파워 모드(0), 파워 다운 절전 모드(1) 반환
*/

int8_t wizphy_getphypmode(void) {
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
}

/*
@function    void wizphy_reset(void)
@brief      MDC/MDIO 또는 자체 통신 인터페이스를 통해 내장된 이더넷 PHY 모듈을
초기화합니다.
@param      void
@return     void
*/

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

/*
@function    void wizphy_setphyconf(wiz_PhyConf *phyconf)
@brief      내장된 이더넷 PHY의 속도(10/100) 및 통신 방식(Auto, Full/Half)
모드를 설정합니다.
@param      phyconf: 설정할 PHY 구성 정보 구조체 포인터
@return     void
*/

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

/*
@function    void wizphy_getphyconf(wiz_PhyConf *phyconf)
@brief      사용자가 설정한 이더넷 PHY 동작 모드 설정값을 가져옵니다. (실제 적용
상태와는 다를 수 있음)
@param      phyconf: 설정값을 반환받을 구조체 포인터
@return     void
*/

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

/*
@function    void wizphy_getphystat(wiz_PhyConf *phyconf)
@brief      링크가 연결된 경우 자동 협상 등에 의해 '실제' 결정된 PHY의 동작 속도
및 통신 모드 상태를 읽어옵니다.
@param      phyconf: 실제 적용 상태를 반환받을 구조체 포인터
@return     void
*/

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

/*
@function    void wizphy_setphypmode(uint8_t pmode)
@brief      이더넷 PHY 전력 동작 모드를 설정하여 칩의 클럭 소비 전력을
최소화합니다.
@param      pmode: 정상 동작 모드(PHY_POWER_NORM) 또는 절전 모드(PHY_POWER_DOWN)
@return     void
*/

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

/*
@function    int8_t wizchip_arp(wiz_ARP *arp)
@brief      소켓 없이 자체 기능(소켓리스)으로 목적지 IP에 대한 MAC 주소를
요청하는 ARP 프로토콜을 수행합니다.
@param      arp: 목적지 IP를 담고, 응답 시 목적지의 MAC 하드웨어 주소가 저장될
구조체 포인터
@return     성공 시 0(정상 MAC 획득), 타임아웃 등 실패 시 -1 반환
*/

int8_t wizchip_arp(wiz_ARP* arp) {
    uint8_t tmp;
    setSLDIP4R(arp->destinfo.ip);
    setSLCR(SLCR_ARP4);
    while (getSLCR());
    while ((tmp = getSLIR()) == 0x00);
    setSLIRCLR(~SLIR_RA);
    if (tmp & SLIR_ARP4) {
        getSLDHAR(arp->dha);
        return 0;
    }
    return -1;
}

/*
@function    int8_t wizchip_ping(wiz_PING *ping)
@brief      소켓 없이 특정 목적지에 대해 PING 요청(ICMP Echo Request)을 전송하여
네트워크 상태를 점검합니다.
@param      ping: 전송할 PING ID와 시퀀스 정보가 담긴 구조체 포인터
@return     성공 시 0(정상 Ping 응답 획득), 타임아웃 등 실패 시 -1 반환
*/

int8_t wizchip_ping(wiz_PING* ping) {
    uint8_t tmp;
    setPINGIDR(ping->id);
    setPINGSEQR(ping->seq);
    setSLDIP4R(ping->destinfo.ip);
    setSLCR(SLCR_PING4);
    while (getSLCR());
    while ((tmp = getSLIR()) == 0x00);
    setSLIRCLR(~SLIR_RA);
    if (tmp & SLIR_PING4) {
        return 0;
    }
    return -1;
}

// IPv6 functions (DAD, SLAAC, Unsolicited NA, Prefix) removed for static analysis compliance

/*
@function    void wizchip_setnetinfo(wiz_NetInfo *pnetinfo)
@brief      WIZCHIP 레지스터에 IP 주소, 서브넷 마스크, 게이트웨이 등의 출발지
네트워크 정보를 설정합니다.
@param      pnetinfo: 설정할 네트워크 정보가 담긴 구조체 포인터
@return     void
*/

void wizchip_setnetinfo(wiz_NetInfo* pnetinfo) {
    uint8_t i = 0;
    setSHAR(pnetinfo->mac);
    setGAR(pnetinfo->gw);
    setSUBR(pnetinfo->sn);
    setSIPR(pnetinfo->ip);

    for (i = 0; i < 4; i++) {
        _DNS_[i]  = pnetinfo->dns[i];
    }

    _IPMODE_   = pnetinfo->ipmode;
}

/*
@function    void wizchip_getnetinfo(wiz_NetInfo *pnetinfo)
@brief      WIZCHIP 레지스터에 현재 설정된 출발지 네트워크 정보(IP, 서브넷 등)를
읽어옵니다.
@param      pnetinfo: 읽어온 네트워크 정보를 반환받을 구조체 포인터
@return     void
*/

void wizchip_getnetinfo(wiz_NetInfo* pnetinfo) {
    uint8_t i = 0;
    getSHAR(pnetinfo->mac);

    getGAR(pnetinfo->gw);
    getSUBR(pnetinfo->sn);
    getSIPR(pnetinfo->ip);

    for (i = 0; i < 4; i++) {
        pnetinfo->dns[i] = _DNS_[i];
    }

    pnetinfo->ipmode = _IPMODE_;
}

/*
@function    void wizchip_setnetmode(netmode_type netmode)
@brief      네트워크 모드(WOL 허용 여부, PING 응답 차단 여부 등)를 비트 마스크
형태로 설정합니다.
@param      netmode: 설정할 네트워크 모드의 비트 플래그 조합
@return     void
*/

void wizchip_setnetmode(netmode_type netmode) {
    uint32_t tmp = (uint32_t) netmode;
    setNETMR((uint8_t)tmp);
    setNETMR2((uint8_t)(tmp >> 8));
    setNET4MR((uint8_t)(tmp >> 16));
    setNET6MR((uint8_t)(tmp >> 24));
}

/*
@function    netmode_type wizchip_getnetmode(void)
@brief      현재 설정된 네트워크 모드(WOL, PING 차단 여부 등) 상태를 가져옵니다.
@param      void
@return     현재 설정된 네트워크 모드 반환
*/

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
@function    void wizchip_settimeout(wiz_NetTimeout *nettime)
@brief      네트워크 패킷 통신 실패 시 패킷을 재전송할 타임아웃 시간 단위 및
재시도 횟수를 설정합니다.
@param      nettime: 재전송 횟수 및 시간 값을 포함한 설정 구조체 포인터
@return     void
*/

void wizchip_settimeout(wiz_NetTimeout* nettime) {
    setRCR(nettime->s_retry_cnt);
    setRTR(nettime->s_time_100us);
    setSLRCR(nettime->sl_retry_cnt);
    setSLRTR(nettime->sl_time_100us);
}

/*
@function    void wizchip_gettimeout(wiz_NetTimeout *nettime)
@brief      설정된 네트워크 통신 재전송 타임아웃 정보(시간, 횟수)를 읽어옵니다.
@param      nettime: 읽어온 타임아웃 정보를 반환받을 구조체 포인터
@return     void
*/

void wizchip_gettimeout(wiz_NetTimeout* nettime) {
    nettime->s_retry_cnt   = getRCR();
    nettime->s_time_100us  = getRTR();
    nettime->sl_retry_cnt  = getSLRCR();
    nettime->sl_time_100us = getSLRTR();
}

