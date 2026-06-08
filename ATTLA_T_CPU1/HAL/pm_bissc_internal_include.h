//###########################################################################
//
// FILE: pm_bissc_internal_include.h
//
// TITLE: BiSS-C encoder interface internal prototypes
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

#ifndef _C2000_PM_BISSC_INTERNAL_INCLUDE_H_
#define _C2000_PM_BISSC_INTERNAL_INCLUDE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "clb_config.h"


//
// [doc-library-bissc-command-format-start]
//
//*****************************************************************************
//
// Length, in bits, of various portions of the control data master (CDM) frame
// _S is the shift required to right justify the field
// _M indicates the mask to extract the bits
//
//
// CDM:                        Addr
//  START1 : CTS : ID : ADDR : CRC1  R : W : START2 : Data :  CRC2 :  STOP
//     1      1    3     7      4    1   1     1       8        4      1   bits
//    <---------------- Read -------------------><---- 12 0s + STOP  --->
//    <-------------------------- Write -------------------------------->
//    <-------------------------- 32 bits ------------------------------>
//
//*****************************************************************************
#define PM_BISSC_CD_BITS               32U
#define PM_BISSC_CD_START1_BITS        1U
#define PM_BISSC_CD_START1_S           31U
#define PM_BISSC_CD_START1_M           1U
#define PM_BISSC_CD_CTS_BITS           1U
#define PM_BISSC_CD_CTS_S              30U
#define PM_BISSC_CD_CTS_M              1U
#define PM_BISSC_CD_ID_BITS            3U
#define PM_BISSC_CD_ID_S               27U
#define PM_BISSC_CD_ID_M               0x0007U
#define PM_BISSC_CD_ADDR_BITS          7U
#define PM_BISSC_CD_ADDR_S             20U
#define PM_BISSC_CD_ADDR_M             0x007FU
#define PM_BISSC_CD_ID_ADDR_BITS       10U
#define PM_BISSC_CD_CTS_ID_ADDR_S      20U
#define PM_BISSC_CD_CTS_ID_ADDR_M      0x07FFU
#define PM_BISSC_CD_ADDR_CRC_BITS      4U
#define PM_BISSC_CD_ADDR_CRC_S         16U
#define PM_BISSC_CD_ADDR_CRC_M         0x000FU
#define PM_BISSC_CD_RW_BITS            2U
#define PM_BISSC_CD_RW_S               14U
#define PM_BISSC_CD_RW_M               0x003U
#define PM_BISSC_CD_START2_BITS        1U
#define PM_BISSC_CD_START2_S           13U
#define PM_BISSC_CD_START2_M           0x0001U
#define PM_BISSC_CD_DATA_BITS          8U
#define PM_BISSC_CD_CRC2_BITS          4U
#define PM_BISSC_CD_CRC2_S             1U
#define PM_BISSC_CD_CRC2_M             0x000FU
#define PM_BISSC_CD_RWS_S              13U
#define PM_BISSC_CD_RWS_M              0x0007U
#define PM_BISSC_CD_DATA_BITS          8U
#define PM_BISSC_CD_DATA_S             5U
#define PM_BISSC_CD_DATA_M             0x00FFU
#define PM_BISSC_CD_DATA_CRC_BITS      4U
#define PM_BISSC_CD_DATA_CRC_S         1U
#define PM_BISSC_CD_DATA_CRC_M         0x000FU
#define PM_BISSC_CD_STOP_BITS          1U
#define PM_BISSC_CD_STOP_S             0U
#define PM_BISSC_CD_STOP_M             0x0001U

//*****************************************************************************
//
// Defines to set or check if a bit is set (high) or cleared (low)
//
//*****************************************************************************
#define PM_BISSC_CD_START_SET          1UL
#define PM_BISSC_CD_START_CLEAR        0UL
#define PM_BISSC_CD_CTS_SET            1UL
#define PM_BISSC_CD_CTS_CLEAR          0UL
#define PM_BISSC_CD_STOP_SET           1UL

//*****************************************************************************
//
// Defines to compare bits against
//
//*****************************************************************************
#define PM_BISSC_CD_ID                 0U
#define PM_BISSC_CD_READ               0b10U    // R+W for read
#define PM_BISSC_CD_WRITE              0b01U    // R+W for write
#define PM_BISSC_CD_RW_INVALID         0b11U    // Both R & W high is not valid
#define PM_BISSC_CD_READ_START         0b101U   // R+W+S for read
#define PM_BISSC_CD_WRITE_START        0b011U   // R+W+S for write
#define PM_BISSC_CD_RW_UNSUPPORTED     0b110U

//
// [doc-library-bissc-command-format-end]
//

//
// [doc-library-bissc-single-cycle-data-start]
//
//*****************************************************************************
//
// _S is the shift required to right justify the field
// _M indicates the mask to extract the bits
//
// The number of multi-turn (MT) and single-turn (ST) position bits
// is often read out of the encoder on start-up. For this reason, the MT shift
// along with the MT and ST mask values are calculated at run-time.
//
// Shift and mask values to extract portions of the scd_raw data
//     ST_POSITION : MT_POSITION  : error :  warning
//           POSITION_BITS            1         1       bits
//
//*****************************************************************************
#define PM_BISSC_SCD_RAW_ERROR_S        0U
#define PM_BISSC_SCD_RAW_ERROR_M        1U
#define PM_BISSC_SCD_RAW_WARNING_S      1U
#define PM_BISSC_SCD_RAW_WARNING_M      1U
#define PM_BISSC_SCD_RAW_POSITION_S     2U
#define PM_BISSC_SCD_RAW_ST_POSITION_S  2U

//*****************************************************************************
//
// Number of bits for each field within a single-cycle-data frame
//
//*****************************************************************************
#define PM_BISSC_SCD_ERROR_BITS          1U
#define PM_BISSC_SCD_WARNING_BITS        1U
#define PM_BISSC_SCD_START_BITS          1U
#define PM_BISSC_SCD_CDS_BITS            1U

//*****************************************************************************
//
// Defines to set or check if a bit is set (high) or cleared (low)
// Note: CDM is transmitted inverted
//
// The CDM is controlled by the CPU through the GPREG input
//
//*****************************************************************************
//
//! [BISSC_ENUM_CDMVALUE_SNIPPET]
//

enum cdmValue
{
    PM_BISSC_SCD_CDM_ONE = 0U,      // CDM = 1
    PM_BISSC_SCD_CDM_ZERO = 1U,     // CDM = 0
    PM_BISSC_SCD_CDM_DEFAULT = 1U,  // CDM default = 0
};

//
//! [BISSC_ENUM_CDMVALUE_SNIPPET]
//

//
// GPREG bit definitions
//
#define PM_BISSC_START_OPERATION_BIT    0U
#define PM_BISSC_CDM_CONTROL_BIT        7U

//
// [doc-library-bissc-single-cycle-data-end]
//

//*****************************************************************************
//! \defgroup BISSC_API_PRIVATE Private Functions
//!
//! Internal functions used by the BiSS-C encoder interface library
//!
//! @{
//*****************************************************************************

//*****************************************************************************
//
//! Generates CRC tables for a specified polynomial
//!
//! \param nBits -      Number of bits of the given polynomial
//! \param polynomial - Polynomial used for CRC calculations
//! \param pTable -     Pointer to the CRC lookup table
//!
//! This function generates table of 256 entries for a given CRC polynomial
//! with specified number of bits (nBits)
//! Generated tables are stored at the address specified by pTable.
//!
//! \return None.
//
//*****************************************************************************
void bissc_generateCRCTable(uint16_t nBits,
                            uint16_t polynomial,
                            uint16_t *pTable);

//*****************************************************************************
//!
//! \brief Configure the CLB for a Transaction
//!
//! Configures the CLB counters and R0 register in order to generate
//! the required number of MA clocks and SPI clocks for the transaction.
//!
//! \param spiClocks    The number of SPI clocks required to receive the
//!                     single-cycle-data into the RX FIFO. This includes
//!                     any additional clock cycles needed to trigger
//!                     the SPI RX interrupt.
//! \param dataClocks   The number of MA clocks required to receive the
//!                     single-cycle-data.
//!
//! \return None.
//!
//*****************************************************************************
static void bissc_configureCLB(uint16_t spiClocks,
                               uint16_t dataClocks)
                               __attribute__((ramfunc));

//*****************************************************************************
//!
//! \brief Configure the SPI FIFO depth
//!
//! Configures the SPI RX FIFO width and depth as required for the
//! transaction.
//!
//! \param widthFIFO is the word length
//! \param depthFIFO is the RX FIFO interrupt level
//!
//! \note The SPI TX FIFO is not used by BiSS-C
//!
//! \return None.
//!
//*****************************************************************************
static void bissc_configureSPILength(uint16_t widthFIFO,
                                     uint16_t depthFIFO)
                                      __attribute__((ramfunc));


//*****************************************************************************
//
//! \brief Extract bits from the received data
//!
//! \attention This function makes the following assumptions:
//! * Data is right justified in the global array named scdRxData.
//! * Data is ordered such that element 0 is the most-significant word
//! * Fetching bits beyond the size of the array is not checked
//!
//! \param numBitsToFetch Number of bits to extract and return.
//! \param numBitsParsed  How many bits were previously parsed. Corresponds
//!                       to the starting place for bit extraction.
//! \param bufferWidth    Word size of the SPI FIFO that captured the data.
//!
//! \return Bits extracted from the scdRxData array.
//!
//!
//! \verbatim
//!
//! In the following examples:
//!   - Bits to be fetched are marked "A"
//!   - Bits already parsed are marked as "P"
//!   - "P*" intersection of baseElement with numBitsParsed marks the
//!     last bit previously parsed and the start of data bits to extract.
//!
//! Case 1:
//! The bits to fetch (A) are contained across elements.
//!
//! |<-------  width = n+1 ----->|
//!  n n-1 n-2 ...  4  3  2  1  0  <-- bit in element
//!
//!  x  x   x  ...  x  x  x  x  x
//!  A  A   A  ...  A  x  x  x  x   RxData[baseElement+numElements-1]
//!  A  A   A  ...  A  A  A  A  A   RxData[baseElement+numElements-2]
//!  ....
//!  A  A   A  ...  A  A  A  A  A   RxData[baseElement+1]
//!  P  P*  A  ...  A  A  A  A  A   RxData[baseElement]
//!     ^---numBitsParsed
//!  P  P   P  ...  P  P  P  P  P
//!  ....
//!  P  P   P  ...  P  P  P  P  P   RxData[0]
//!
//! Case 2:
//! The bits to fetch (A) are contained within one element.
//!
//! |<-------  width = n+1 ----->|
//!  n n-1 n-2 ...  4  3  2  1  0  <-- bit in element
//!
//!  x  x   x  ...  x  x  x  x  x
//!  P  P*  A  ...  A  A  A  A  x   RxData[baseElement]
//!     ^   <--bitsToFetch -->|
//!     |
//!     -----numBitsParsed
//!
//! \endverbatim
//!
//
//*****************************************************************************
static uint64_t bissc_getBits(uint16_t numBitsToFetch,
                          uint16_t numBitsParsed,
                          uint16_t bufferWidth)
                        __attribute__((ramfunc));

//*****************************************************************************
//!
//! \brief Calculate a CRC using a lookup table
//!
//! \param crcStart     Initial value. This is typically 0.
//! \param nBitsPoly    The number of bits in the polynomial
//! \param msg          A pointer to the data to calculate the CRC for
//! \param crcTable     A pointer to the CRC lookup table
//! \param msgBytes     The number of 8-bit bytes in the message.
//!                     For example: If the message is 16-bits, then msgBytes
//!                     is 2. If the message uses a partial byte then round up.
//!                     For example: 18-bits then msgBytes = 3.
//!
//! \return The calculated CRC value.
//!
//*****************************************************************************
uint16_t bissc_getCRC(uint16_t crcStart,
                         uint16_t nBitsPoly,
                         uint16_t * msg,
                         uint16_t *crcTable,
                         uint16_t msgBytes)
                        __attribute__((ramfunc));

//*****************************************************************************
//!
//! \brief Reset the CLB
//!
//! Reset portions of the CLB after a transaction or to recover from a
//! failed transaction.
//!
//! \return None.
//!
//*****************************************************************************
static void bissc_resetCLB(void)
                           __attribute__((ramfunc));

//*****************************************************************************
//
//! \brief Set the CDM bit for the next SCD transaction
//!
//! \param value to be output during the BiSS timeout. The inverse of value
//! is interpreted to be the CDM bit by the encoder.
//!
//! \snippet this BISSC_ENUM_CDMVALUE_SNIPPET
//!
//! The value is sent to the CLB tiles through the CLB's GPREG.
//!
//! \return None.
//
//*****************************************************************************
static void bissc_setCDMBit(enum cdmValue value)
                        __attribute__((ramfunc));

//
//! @} //end of defgroup BISSC_API_PRIVATE
//

#ifdef __cplusplus
}
#endif

//
// end of ifndef BISSC_INTERNAL_INCLUDE_H
//
#endif

//*****************************************************************************
//
// End of file
//
//*****************************************************************************
