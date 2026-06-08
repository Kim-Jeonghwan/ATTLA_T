/**********************************************************************
 Nexcom Co., Ltd.
 Filename         : pm_bissc_include.h
 Version          : 00.01
 Description      : BiSS-C 인코더 인터페이스 레퍼런스 라이브러리 프로토타입 (F28388D 포팅)
 Programmer       : Kim Jeonghwan
 Last Updated     : 2026. 06. 09. (F2838xD MCU 지원 및 CLB/SPI 채널 매핑 추가)
**********************************************************************/
//###########################################################################
//

// FILE: pm_bissc_include.h
//
// TITLE: BiSS-C encoder interface reference library prototypes
//
//###########################################################################
// $Copyright:
// Copyright (C) 2017-2026 Texas Instruments Incorporated
//     http://www.ti.com/ ALL RIGHTS RESERVED
// $
//###########################################################################

//###########################################################################
// Please note:
//    BiSS is a trademark of iC-Haus GmbH
//    Developers must obtain their own BiSS Interface license agreement.
//    Refer to: https://biss-interface.com/biss-interface-license-agreement/
//###########################################################################


#ifndef _C2000_PM_BISSC_INCLUDE_H_
#define _C2000_PM_BISSC_INCLUDE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

//
// [doc-library-config-start]
//
// #define BISSC_ENCODER_HAS_CD_INTERFACE
//      Some encoders do not support the control-data CD interface.
//      0: the code associated with the control-data frames is not executed.
//      else: the code associated with control-data frames is executed.
//
// #define PM_BISSC_SPI  SPIA_BASE, SPIB_BASE, SPIC_BASE, SPID_BASE
//      Define which SPI peripheral is used.  This MUST match the instance
//      the CLB is clocking and be connected to the RS485 interface logic.
//
//      Note: The CLB output configuration and SPI configuration may need
//      to be modified if this selection is changed.  Refer to the
//      design guide for more information.
//
// #define PM_SPI_FIFO_MAX_LEVEL
//      This is the depth of the SPI FIFO. If you know the message returned
//      from the encoder will always use fewer levels it can be lowered.
//      This parameter is only used when reading the message out of the
//      SPI FIFO and can save cycles.
//
//  #define PM_BISSC_SPI_FIFO_WIDTH
//      Word size used by the SPI peripheral. You shouldn't have to change
//      this definition.
//
#define PM_BISSC_ENCODER_HAS_CD_INTERFACE   1U
#define PM_BISSC_SPI_FIFO_WIDTH            12U
#define PM_BISSC_SPI_FIFO_MAX_LEVEL        16U

//
// SPI selection specified here and the CLB output
// configuration, and the system control setup in the
// communications demo must match.  If one is changed,
// the other must be changed as well.
//
#if defined(_F2838X) || defined(F2838x_DEVICE)
    #define PM_BISSC_SPI           SPIB_BASE       // 보드 설계에 맞게 SPIB 로 변경
    #define PM_BISSC_INT_SPI_RX    INT_SPIB_RX     // 선택한 SPI 채널 인터럽트로 매핑
    #define PM_BISSC_CLK_GEN_BASE  CLB2_BASE       // 클럭 발생용 CLB 타일
    #define PM_BISSC_FSM_BASE      CLB1_BASE       // 상태머신 제어용 CLB 타일
#elif defined(_F28P65X) || defined(F28P65x_DEVICE)
    #define PM_BISSC_SPI           SPID_BASE
    #define PM_BISSC_INT_SPI_RX    INT_SPID_RX
    #define PM_BISSC_CLK_GEN_BASE  CLB3_BASE
    #define PM_BISSC_FSM_BASE      CLB4_BASE
#elif defined(_F28P55X) || defined(F28P55X_DEVICE)
    #define PM_BISSC_SPI           SPIA_BASE
    #define PM_BISSC_INT_SPI_RX    INT_SPIA_RX
    #define PM_BISSC_CLK_GEN_BASE  CLB2_BASE
    #define PM_BISSC_FSM_BASE      CLB1_BASE
#elif defined(_F28P551X) || defined(F28P551X_DEVICE)
    #define PM_BISSC_SPI           SPIA_BASE
    #define PM_BISSC_INT_SPI_RX    INT_SPIA_RX
    #define PM_BISSC_CLK_GEN_BASE  CLB2_BASE
    #define PM_BISSC_FSM_BASE      CLB1_BASE           
#else
    #error Device should match a predefined symbol in the build options.
#endif

//
// [doc-library-config-end]
//

//*****************************************************************************
//
// Shift and mask values to extract the CDS bits from the SPI FIFO
// The CDS bit will always be in the first buffer received, rdata[0]
//
// _BITS is the number of bits in the specified field
// _S is the shift required to right justify the field
// _M indicates the mask to extract the bits
//
//     <--------BISSC_SPI_FIFO_WIDTH------------->
//     START : CDS  :        data
//       1      1      BISSC_RDATA_DATA_BITS      bits
//
//*****************************************************************************
#define PM_BISSC_CD_CDS_BITS               1U
#define PM_BISSC_CD_START1_BITS            1U
#define PM_BISSC_CD_CTS_ID_ADDR_BYTES      2U
#define PM_BISSC_CD_RDATA_DATA_BITS  ( PM_BISSC_SPI_FIFO_WIDTH                 \
                                    - PM_BISSC_CD_START1_BITS                  \
                                    - PM_BISSC_CD_CDS_BITS)
#define PM_BISSC_CD_RDATA_START_S    (PM_BISSC_CD_RDATA_DATA_BITS              \
                                    - PM_BISSC_CD_CDS_BITS)
#define PM_BISSC_CD_RDATA_START_M    ((1U << PM_BISSC_CD_START1_BITS) - 1U)
#define PM_BISSC_CD_RDATA_CDS_S      PM_BISSC_CD_RDATA_DATA_BITS
#define PM_BISSC_CD_RDATA_CDS_M      ((1U << PM_BISSC_CD_CDS_BITS) - 1U)

//
// Declare CRC table for BiSS-C CRC calculations
//
#define PM_BISSC_SCD_CRCTABLE_SIZE      256U
#define PM_BISSC_CD_CRCTABLE_SIZE       256U
extern uint16_t bisscCRCtableSCD[PM_BISSC_SCD_CRCTABLE_SIZE];
#if PM_BISSC_ENCODER_HAS_CD_INTERFACE
extern uint16_t bisscCRCtableCD[PM_BISSC_CD_CRCTABLE_SIZE];
#endif

//
// [doc-cd-struct-start]
//*****************************************************************************
//
// Control-data frame parameters and states
//
//*****************************************************************************
enum cdState                   // Control data state machine
{
    PM_BISSC_CD_NEW_REQUEST,   // New read or write request pending
    PM_BISSC_CD_TRANSMITTING,  // Read or write is in progress
    PM_BISSC_CD_COMPLETE,      // Read or write has completed
    PM_BISSC_CD_PARSED,        // CD data has been parsed
    PM_BISSC_CD_WAIT,          // BiSS required wait between CD frames
    PM_BISSC_CD_NONE
};


//
// The cdStatus will remain until either the application clears it (sets it to
// PM_BISSC_CD_PASS) or a new CD Request is made.
//
// PASS                     No error detected
// WRITE_FAIL               Write access failure. The data echoback (received
//                          data) did not match the data transmitted
// CRC_FAIL:                Calculated CRC did not match received CRC
// READ_WRITE_INVALID       The combination of R+W is invalid per the
//                          protocol specification
// READ_WRITE_UNSUPPORTED   Unsupported feature. Likely the encoder requires
//                          wait-states to access the register
//
enum cdStatus
{
    PM_BISSC_CD_PASS,
    PM_BISSC_CD_WRITE_FAIL,
    PM_BISSC_CD_WRITE_PASS,
    PM_BISSC_CD_READ_FAIL,
    PM_BISSC_CD_READ_PASS,
    PM_BISSC_CD_CRC_FAIL,
    PM_BISSC_CD_READ_WRITE_INVALID,
    PM_BISSC_CD_READ_WRITE_UNSUPPORTED
};

typedef struct  PM_bissc_cdStruct{
    enum cdState cdState;      // State machine current state
    enum cdStatus cdStatus;    // Pass or Fail status
    uint32_t  cdmStream;       // Least significant bit is next CM to transmit
    uint32_t  cdsStream;       // CDS stream without the CRC
    uint16_t  crcBits;         // Size of CRC polynomial
    uint16_t  crcPoly;         // CRC polynomial without the leading 1
    uint16_t  crcStart;        // CRC initial value
} PM_bissc_cdStruct;

typedef PM_bissc_cdStruct * PM_bissc_cdStruct_Handle;

//
// [doc-cd-struct-end]
//

//
// [doc-encoder-struct-start]
//*****************************************************************************
//
// Encoder information parsed from the single-cycle-data.
//
//*****************************************************************************
typedef struct PM_bissc_encoderStruct
{
    uint32_t  positionMT;   // Multi-turn position
    uint32_t  positionST;   // Single-turn position
    uint64_t  position;     // MT::ST position
    uint16_t  error;        // Error bit
    uint16_t  warning;      // Warning bit
} PM_bissc_encoderStruct;

typedef PM_bissc_encoderStruct * PM_bissc_encoderStruct_Handle;

//
// [doc-encoder-struct-end]
//

//
// [doc-register-struct-start]
//*****************************************************************************
//
// Register access parameters used to specify address and access type
//
//*****************************************************************************
enum cdRegisterAccessType
{
    PM_BISSC_REGISTER_READ = 2U,    // R:W = 1 0 = read access
    PM_BISSC_REGISTER_WRITE = 1U    // R:W = 0 1 = write access
};

typedef struct PM_bissc_registerStruct
{
    uint16_t                    address;    // Address of register to access
    uint16_t                    rxData;     // Data read from the register
    uint16_t                    txData;     // Data to write to the register
    enum cdRegisterAccessType   accessType; // Specify read or write access
} PM_bissc_registerStruct;

typedef PM_bissc_registerStruct * PM_bissc_registerStruct_Handle;

//
// [doc-register-struct-end]
//

//
// [doc-scd-struct-start]
//*****************************************************************************
//
// Parameters needed to process single-cycle-data
//
//*****************************************************************************
typedef struct PM_bissc_scdStruct {
    volatile bool  dataReady;   // false: not ready. true: ready
    uint16_t  crcBits;          // Size of polynomial
    uint16_t  crcPoly;          // Polynomial
    uint16_t  crcStart;         // CRC initial value
    uint16_t  crc_M;            // Mask value to extract the CRC
    uint16_t  spiFIFOWidth;     // SPI FIFO word size
    uint16_t  spiFIFOMask;      // Mask for a SPI FIFO word
    uint16_t  spiFIFOLevel;     // SPI FIFO depth
    uint16_t  numDataClocks;    // Number of clocks required to receive SCD
    uint16_t  positionSTBits;   // Number of single-turn bits
    uint16_t  positionST_S;     // Shift to extract ST bits
    uint16_t  positionMTBits;   // Number of multi-turn bits
    uint16_t  positionMT_S;     // Shift to extract MT bits
    uint16_t  positionBits;     // Total position bits (MT + ST)
    uint32_t  positionMT_M;     // Mask to extract MT bits
    uint32_t  positionST_M;     // Mask to extract ST bits
    uint64_t  position_M;       // Mask to extract the total position
    uint64_t  posErrWarnBits;   // Number of error and warn bits
} PM_bissc_scdStruct;

typedef PM_bissc_scdStruct * PM_bissc_scdStruct_Handle;

//
// [doc-scd-struct-end]
//

//
// [doc-scd-rx-array-start]
//*****************************************************************************
//
// This array is loaded with the received single-cycle-data from
// the encoder during the SPI interrupt routine.
//
//*****************************************************************************
extern volatile uint32_t scdRxData[PM_BISSC_SPI_FIFO_MAX_LEVEL];

//
// [doc-scd-rx-array-end]
//

#define PM_BISSC_PASS                   0
#define PM_BISSC_FAIL                   0xFFFF

#define PM_BISSC_SCD_CRC_FAIL           0xFFFC
#define PM_BISSC_DATA_INVALID           0xFFFA


//*****************************************************************************
//! \defgroup BISSC_API_INIT Initialization Functions
//!
//! \brief These are the library functions used to initialize the
//! configurable logic block (CLB), SPI, and CRC lookup tables.
//!
//! @{
//*****************************************************************************
//*****************************************************************************
//! \brief Initialize the CRC Lookup Table
//!
//! Generate a look-up table of 256 entries for a given CRC polynomial
//! with specified number of bits.
//!
//! \param nBits        number of bits of the given polynomial
//! \param polynomial   polynomial used for the CRC calculations
//! \param pTable       pointer to the CRC table
//!
//! \return None.
//!
//*****************************************************************************
void PM_bissc_generateCRCTable(uint16_t nBits,
                                 uint16_t polynomial,
                                 uint16_t *pTable);

//*****************************************************************************
//! \brief Initialize parameters used to extract position and CRC
//!
//! This function initializes mask and shift values used to extract information
//! from the single-cycle data and control data.
//!
//! \note The following parameters must be initialized before calling
//!  this the function:
//!  * crcBits: Number of CRC bits defined by the utilized polynomial.
//!  * positionMTBits: Number of multi-turn bits supported by the encoder.
//!  * positionSTBits: Number of single-turn bits supported by the encoder.
//!
//! This function also initializes the CRC lookup tables.
//!
//! \param scdParams   Single-cycle data Parameters initialized by the function
//! \param cdParams    Control data Parameters initialized by the function
//! \return none
//!
//! \sa PM_bissc_receivePosition
//! \sa PM_bissc_doCDTasks
//
//*****************************************************************************
void PM_bissc_initParams(PM_bissc_scdStruct_Handle scdParams,
                         PM_bissc_cdStruct_Handle cdParams);

//*****************************************************************************
//! \brief Scale to generate the MA clock
//!
//! This function configures the CLB counters to generate
//! the specified MA clock and the SPI clock frequency.
//!
//! \param freqDiv Specified as BISSC_FREQ_DIVIDER in the example's bissc.h
//! \return none
//
//*****************************************************************************
void PM_bissc_setFreq(uint32_t freqDiv);

//*****************************************************************************
//! \brief Initialize the CLB tiles
//!
//! Initialize the logic of both CLB tiles. The logic configuration code is
//! generated by the library's .syscfg file.
//!
//! \return none
//
//*****************************************************************************
void PM_bissc_setupPeriph(void);

//
//! @} //end of addtogroup BISSC_API_INIT
//

//*****************************************************************************
//! \defgroup BISSC_API_RUN_COMMANDS Run-time Functions
//!
//! \brief Library functions used during run-time to start a transaction.
//!
//! @{
//*****************************************************************************

//*****************************************************************************
//! \brief Command data state-machine
//!
//! This state-machine processes the command data frame for read and write
//! of the encoder's registers. When command-data (CDM) is being sent, the
//! this function should be called by the CPU once every BiSS frame.
//!
//! During system initialization, the CPU sets the state-machine status to
//! PM_BISSC_CD_NONE.
//!
//! To initiate a read or write, the CPU writes the request to
//! bisscRegisterParams.address and bisscRegisterParams.accessType and
//! then sets the state machine status to PM_BISSC_CD_NEW_REQUEST
//!
//! \attention This example only implements single register read and write
//!  control frames (CTS = 1). The following features are not implemented in
//!  this release:
//!  * Command control frames where the CTS bit is 0
//!  * Wait states for read/write of registers. This case occurs when the
//!    encoder requires more time for a read or write access and holds
//!    the start bit low to extend the frame. When this is detected, the
//!    state machine will return PM_BISSC_READ_WRITE_UNSUPPORTED
//!
//! \param cdParams         Control data frame parameters
//! \param registerParams   Register address, accessType and data
//!
//! \return
//!  * cdParams             Updated with the status and cdm/cds streams
//!  * registerParams       Updated rxData
//!
//! \sa PM_bissc_initParams
//
//*****************************************************************************
void PM_bissc_doCDTasks(PM_bissc_cdStruct_Handle cdParams,
                        PM_bissc_registerStruct_Handle registerParams)
                        __attribute__((ramfunc));

//*****************************************************************************
//! \brief Halt the BiSS-C Subsystem
//!
//! This function clears the "START_OPERATION" signal into both
//! tiles
//!
//! \return none
//
//*****************************************************************************
void PM_bissc_haltOperation(void)
                            __attribute__((ramfunc));

//*****************************************************************************
//! \brief Parse the SCD frame
//!
//! This function extracts the following information from the
//! single-cycle-data frame:
//!
//!  * CDS: Control Data Slave bit
//!  * MT Position: Multi-Turn data
//!  * ST Position: Single-Turn data
//!  * Error
//!  * Warning
//!  * CRC: Cyclic Redundancy Check for position:error:warning
//!
//! \return
//!     * BISSC_PASS The received CRC matches the calculated CRC
//!     * BISSC_FAIL The received CRC does not match the calculated CRC
//!     * encoderInfo is updated with the encoder information
//
//*****************************************************************************
extern uint16_t PM_bissc_receivePosition(
                            PM_bissc_scdStruct_Handle scdParams,
                            PM_bissc_cdStruct_Handle cdParams,
                            PM_bissc_encoderStruct_Handle encoderInfo)
                            __attribute__((ramfunc));

//*****************************************************************************
//! \brief Setup a New BiSS-C Transaction
//!
//! This function sets up a new single-cycle transaction.
//!
//! \sa This function is called sometime before the PM_bissc_startOperation
//! function.
//!
//! \return none
//
//*****************************************************************************
void PM_bissc_setupSCDTransaction(PM_bissc_scdStruct_Handle scdParams)
                                __attribute__((ramfunc));

//*****************************************************************************
//! \brief Start the BiSS-C Transaction
//!
//! This function starts the transmission of the MA clock.
//!
//! \sa This function is called sometime after the single-cycle-data
//!  setup function: PM_bissc_setupSCDTransaction
//!
//! \return none
//
//*****************************************************************************
void PM_bissc_startOperation(PM_bissc_scdStruct_Handle scdParams,
                             uint32_t Freq_us)
                             __attribute__((ramfunc));


//
//! @} //end of addtogroup BISSC_API_RUN_COMMANDS
//

#ifdef __cplusplus
}

//
// extern "C"
//
#endif

//
// end of BISSC_SECONDARY_INCLUDE_H
//
#endif

