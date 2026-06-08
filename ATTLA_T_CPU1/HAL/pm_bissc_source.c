//###########################################################################
//
// FILE: pm_bissc_source.c
//
// TITLE: BiSS-C encoder interface reference library functions
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

#include "driverlib.h"
#include "pm_bissc_include.h"
#include "pm_bissc_internal_include.h"

void PM_bissc_setFreq(uint32_t freqDiv)
{

    //
    // Halt operation (clear the START_OPERATION bit and CDM bit)
    //
    CLB_setGPREG(PM_BISSC_FSM_BASE, 0);
    CLB_setGPREG(PM_BISSC_CLK_GEN_BASE, 0);

    //
    // Encoder clock (BiSS-C MA) frequency and duty control
    //
    CLB_writeInterface(PM_BISSC_CLK_GEN_BASE, CLB_ADDR_COUNTER_0_MATCH1,
                       freqDiv - 1UL);
    CLB_writeInterface(PM_BISSC_CLK_GEN_BASE, CLB_ADDR_COUNTER_0_MATCH2,
                        2UL * freqDiv - 1UL);

    //
    // CLB SPI clock frequency and duty control
    //
    CLB_writeInterface(PM_BISSC_CLK_GEN_BASE, CLB_ADDR_COUNTER_1_MATCH1,
                       freqDiv - 1UL);
    CLB_writeInterface(PM_BISSC_CLK_GEN_BASE, CLB_ADDR_COUNTER_1_MATCH2,
                       2UL * freqDiv - 1U);

    return;
}

void PM_bissc_setupPeriph()
{
    initBISSC_FSM_GEN_TILE(PM_BISSC_FSM_BASE);
    initBISSC_CLK_GEN_TILE(PM_BISSC_CLK_GEN_BASE);
}

void PM_bissc_startOperation(PM_bissc_scdStruct_Handle scdParams,
                             uint32_t freqDiv)
{
    scdParams->dataReady = false;

    //
    // Set counter start value in order to generate 50% duty
    // from the first clock
    //
    CLB_writeInterface(PM_BISSC_CLK_GEN_BASE, CLB_ADDR_COUNTER_0_LOAD,
                       freqDiv - 1U);

    //
    // Pull the START_OPERATION signal high for both CLB tiles
    //
    HWREGBITHW(PM_BISSC_FSM_BASE + CLB_LOGICCTL + CLB_O_GP_REG,
               PM_BISSC_START_OPERATION_BIT, 1);
    HWREGBITHW(PM_BISSC_CLK_GEN_BASE + CLB_LOGICCTL + CLB_O_GP_REG,
               PM_BISSC_START_OPERATION_BIT, 1);
}

void PM_bissc_haltOperation()
{
    //
    // Pull the START_OPERATION signal low for both CLB tiles
    //
    HWREGBITHW(PM_BISSC_FSM_BASE + CLB_LOGICCTL + CLB_O_GP_REG,
               PM_BISSC_START_OPERATION_BIT, 1);
    HWREGBITHW(PM_BISSC_CLK_GEN_BASE + CLB_LOGICCTL + CLB_O_GP_REG,
               PM_BISSC_START_OPERATION_BIT, 1);
}

//
// Internal functions
//

void bissc_resetCLB(void)
{
    //
    // Enable the XBAR latch to hold the BISSC_MA line high while
    // the CLB tiles are reset. This is the default idle state of MA.
    // The latch state (high) was specified in the syscfg initialization
    // code.
    //
    // Note: if the design is changed to use a CLB OUTPUTXBAR,
    // then use CLBOUTPUTXBAR_BASE as the first argument.
    //
    XBAR_setOutputLatchMode(OUTPUTXBAR_BASE, XBAR_OUTPUT1, true);

    //
    // For both tiles:
    // - Pull the START_OPERATION signal low
    // - Perform a soft reset using GLOBAL_EN
    // - Clear the counter registers
    // - Re-enable the tiles using GLOBAL_EN
    // - Once the tiles are re-enabled, release the OUTPUTXBAR latch.
    //   Now the CLB will control the level of MA.
    //
    CLB_setGPREG(PM_BISSC_FSM_BASE, 0);
    CLB_setGPREG(PM_BISSC_CLK_GEN_BASE, 0);
    EALLOW;
    HWREGH(PM_BISSC_FSM_BASE + CLB_LOGICCTL + CLB_O_LOAD_EN)
                &= ~CLB_LOAD_EN_GLOBAL_EN;
    HWREGH(PM_BISSC_CLK_GEN_BASE + CLB_LOGICCTL + CLB_O_LOAD_EN)
                &= ~CLB_LOAD_EN_GLOBAL_EN;
    EDIS;
    CLB_writeInterface(PM_BISSC_FSM_BASE, CLB_ADDR_COUNTER_0_LOAD, 0x0);
    CLB_writeInterface(PM_BISSC_FSM_BASE, CLB_ADDR_COUNTER_1_LOAD, 0x0);
    CLB_writeInterface(PM_BISSC_FSM_BASE, CLB_ADDR_COUNTER_2_LOAD, 0x0);
    CLB_writeInterface(PM_BISSC_CLK_GEN_BASE, CLB_ADDR_COUNTER_0_LOAD, 0x0);
    CLB_writeInterface(PM_BISSC_CLK_GEN_BASE, CLB_ADDR_COUNTER_1_LOAD, 0x0);
    CLB_writeInterface(PM_BISSC_CLK_GEN_BASE, CLB_ADDR_COUNTER_2_LOAD, 0x0);
    EALLOW;
    HWREGH(PM_BISSC_FSM_BASE + CLB_LOGICCTL + CLB_O_LOAD_EN)
                |= CLB_LOAD_EN_GLOBAL_EN;
    HWREGH(PM_BISSC_CLK_GEN_BASE + CLB_LOGICCTL + CLB_O_LOAD_EN)
                |= CLB_LOAD_EN_GLOBAL_EN;
    EDIS;
    XBAR_setOutputLatchMode(OUTPUTXBAR_BASE, XBAR_OUTPUT1, false);
}

void bissc_configureCLB(uint16_t spiClocks, uint16_t dataClocks)
{
    CLB_writeInterface(PM_BISSC_FSM_BASE, CLB_ADDR_COUNTER_2_MATCH1, spiClocks);
    CLB_writeInterface(PM_BISSC_FSM_BASE, CLB_ADDR_HLC_R0, dataClocks);
}

void bissc_configureSPILength(uint16_t widthFIFO, uint16_t depthFIFO)
{
    SPI_disableModule(PM_BISSC_SPI);
    EALLOW;
    HWREGH(PM_BISSC_SPI + SPI_O_CCR) = (HWREGH(PM_BISSC_SPI + SPI_O_CCR) &
                                 ~(SPI_CCR_SPICHAR_M)) | (widthFIFO - 1U);

    HWREGH(PM_BISSC_SPI + SPI_O_FFRX) = (HWREGH(PM_BISSC_SPI + SPI_O_FFRX) &
                                 (~SPI_FFRX_RXFFIL_M)) | depthFIFO;
    EDIS;
    SPI_resetRxFIFO(PM_BISSC_SPI);
    SPI_enableModule(PM_BISSC_SPI);
}

void PM_bissc_initParams(PM_bissc_scdStruct_Handle scdParams,
                         PM_bissc_cdStruct_Handle cdParams)
{
    scdParams->dataReady = false;

    scdParams->spiFIFOMask = (1U << scdParams->spiFIFOWidth) - 1U;

    scdParams->crc_M = (1U << scdParams->crcBits) - 1U;

    scdParams->positionMT_S =
       (PM_BISSC_SCD_RAW_POSITION_S + scdParams->positionSTBits);

    scdParams->positionMT_M =
       (1UL << scdParams->positionMTBits) - 1UL;

    scdParams->positionST_M =
       (1UL << scdParams->positionSTBits) - 1UL;

    scdParams->positionBits = scdParams->positionSTBits
            + scdParams->positionMTBits;

    scdParams->position_M =
       ((uint64_t)1UL << (uint64_t)scdParams->positionBits) - (uint64_t)1UL;

    scdParams->posErrWarnBits =
            scdParams->positionMTBits + scdParams->positionSTBits
       + PM_BISSC_SCD_ERROR_BITS + PM_BISSC_SCD_WARNING_BITS;

    scdParams->numDataClocks =
            scdParams->posErrWarnBits + scdParams->crcBits
       + PM_BISSC_SCD_START_BITS + PM_BISSC_SCD_CDS_BITS;

    cdParams->cdState = PM_BISSC_CD_WAIT;
    cdParams->cdmStream = 1;
    cdParams->cdsStream = 0;

    bissc_generateCRCTable(scdParams->crcBits,
                           scdParams->crcPoly,
                           bisscCRCtableSCD);

    #if defined(PM_BISSC_ENCODER_HAS_CD_INTERFACE)
    bissc_generateCRCTable(cdParams->crcBits,
                           cdParams->crcPoly,
                           bisscCRCtableCD);
    #endif
}


void PM_bissc_setupSCDTransaction(PM_bissc_scdStruct_Handle scdParams)
{
    uint16_t spiClocks;
    bissc_resetCLB();
    bissc_setCDMBit(PM_BISSC_SCD_CDM_DEFAULT);

    if(((scdParams->numDataClocks) % scdParams->spiFIFOWidth) == 0)
    {
        scdParams->spiFIFOLevel =
                    (scdParams->numDataClocks / scdParams->spiFIFOWidth);
    }
    else
    {
        scdParams->spiFIFOLevel =
                    (scdParams->numDataClocks / scdParams->spiFIFOWidth) + 1U;
    }
    spiClocks = (scdParams->spiFIFOLevel * scdParams->spiFIFOWidth);

    bissc_configureSPILength(scdParams->spiFIFOWidth, scdParams->spiFIFOLevel);
    bissc_configureCLB(spiClocks, scdParams->numDataClocks);
}

uint16_t PM_bissc_receivePosition(PM_bissc_scdStruct_Handle scdParams,
                                  PM_bissc_cdStruct_Handle cdParams,
                                  PM_bissc_encoderStruct_Handle encoderInfo)
{
    uint16_t scdBitsParsed;
    uint16_t checkCRC = PM_BISSC_FAIL;
    uint16_t status = PM_BISSC_FAIL;

    //
    // Track how many bits have been taken into account
    //
    // The START bit should be high. It is not parsed and checked as part
    // of this example.
    //
    scdBitsParsed = PM_BISSC_CD_START1_BITS;

    //
    // If a control-data frame is being transmitted, capture the last
    // received CDS bit from the encoder. This function assumes the CDS
    // bit will be in the lowest level FIFO buffer (first received)
    // which is bisscRxData[0]. This is a reasonable assumption since
    // CDS is the first bit received after the start bit.
    //
    // Shift the CDS bit into the cds_stream
    //
    if(cdParams->cdState == PM_BISSC_CD_TRANSMITTING)
    {
        cdParams->cdsStream = cdParams->cdsStream
                               << PM_BISSC_CD_CDS_BITS;
        cdParams->cdsStream |= (scdRxData[0]
                                >> PM_BISSC_CD_RDATA_CDS_S)
                                &  PM_BISSC_CD_RDATA_CDS_M;
    }
    scdBitsParsed+= PM_BISSC_CD_CDS_BITS;
    //
    // The location of the following bits depend on the resolution of the
    // encoder (how many multi-turn and single-turn bits) and the width of
    // the FIFO. Thus they are extracted from the buffer.
    // - tempPosErrWarn: position + error + warning bits
    // - scdRxCRC: received CRC for position + error + warning
    // - crcResult: calculated CRC for position + error + warning
    // - Compare the received CRC to the calculated CRC
    //
    uint16_t numBits = scdParams->posErrWarnBits;
    uint64_t tempPosErrWarn;
    tempPosErrWarn = bissc_getBits(numBits,
                                   scdBitsParsed,
                                   scdParams->spiFIFOWidth);
    scdBitsParsed += numBits;
    uint16_t scdRxCRC;
    scdRxCRC = bissc_getBits(scdParams->crcBits,
                             scdBitsParsed,
                             scdParams->spiFIFOWidth);

    uint16_t numBytes;
    //
    // Determine the number of 8-bit bytes based on the number of bits
    // (numBits/8). If numBits is not exactly divisible by 8, then one of the
    // last 3 bits in numBits will be set. If this is the case, add another
    // byte to account for the extra bits.
    //
    numBytes = (numBits / 8U);
    if(numBits & 0x7U) numBytes += 1U;
    checkCRC =  bissc_getCRC(scdParams->crcStart,
                                 scdParams->crcBits,
                                (uint16_t *)&tempPosErrWarn,
                                bisscCRCtableSCD, numBytes);

    //
    // If the calculated CRC matches the received CRC, populate the position,
    // error and warning information
    //
    // The CRC is transmitted inverted. Therefore, invert the received value
    // before the comparison
    //
    uint32_t shift;
    uint32_t mask = scdParams->crc_M;
    if(checkCRC == ((~scdRxCRC) & mask))
    {
        mask = PM_BISSC_SCD_RAW_ERROR_M;
        shift = PM_BISSC_SCD_RAW_ERROR_S;
        encoderInfo->error = (tempPosErrWarn >> shift) & mask;

        mask = PM_BISSC_SCD_RAW_WARNING_M;
        shift = PM_BISSC_SCD_RAW_WARNING_S;
        encoderInfo->warning = (tempPosErrWarn >> shift) & mask;

        mask = scdParams->positionST_M;
        shift = PM_BISSC_SCD_RAW_ST_POSITION_S;
        encoderInfo->positionST = (tempPosErrWarn >> shift) & mask;

        mask = scdParams->position_M;

        //
        // shift for MT:ST is the same as for ST
        //
        encoderInfo->position = (tempPosErrWarn >> shift) & mask;

        mask = scdParams->positionMT_M;
        shift = scdParams->positionMT_S;
        encoderInfo->positionMT = (tempPosErrWarn >> shift) & mask;

        status = PM_BISSC_PASS;
    }
    else
    {
        status = PM_BISSC_SCD_CRC_FAIL;
    }

    return(status);
}


void PM_bissc_doCDTasks(PM_bissc_cdStruct_Handle cdParams,
                        PM_bissc_registerStruct_Handle registerParams)
{
    static int16_t numCDBitsTx;
    static uint16_t waitCount = 1;

    switch(cdParams->cdState)
    {
        uint32_t calculatedCRC;
        uint16_t rxdRW;
        uint16_t rxdS;

        uint16_t tempCTSIDAddr;
        uint32_t tempCRC;
        //
        // New command data (CD) request
        //
        case PM_BISSC_CD_NEW_REQUEST:
            //
            // Initialize tempCTSIDAddr to CTS:ID:ADDR
            // - Set CTS bit
            // - Initialize the ID field
            // - Initialize the register address field
            // - Calculate the CRC for CTS:ID:ADDR
            // - Invert the CRC (CRC is transmitted inverted)
            //
            tempCTSIDAddr =  (PM_BISSC_CD_CTS_SET << PM_BISSC_CD_ID_ADDR_BITS);
            tempCTSIDAddr += (PM_BISSC_CD_ID << PM_BISSC_CD_ADDR_BITS);
            tempCTSIDAddr += registerParams->address;

            //
            // Create the CDM stream
            // START:CTS:ID:ADDR:CRC1:R:W:STOP1
            // - Set the START bit
            // - Initialize the CTS:ID:ADDR fields
            // - Calculate the CRC for CTS:ID:ADDR and invert the CRC.
            //   For BiSS-C, the CRC is transmitted inverted.
            // - Initialize the CRC1 field in the CDM stream
            //
            uint32_t tempCD;
            tempCD =  (PM_BISSC_CD_START_SET << PM_BISSC_CD_START1_S);
            tempCD += ((uint32_t)tempCTSIDAddr << PM_BISSC_CD_CTS_ID_ADDR_S);
            tempCRC = bissc_getCRC(cdParams->crcStart,
                                   cdParams->crcBits,
                                   &tempCTSIDAddr,
                                   bisscCRCtableCD,
                                   PM_BISSC_CD_CTS_ID_ADDR_BYTES);
            tempCRC = ~tempCRC & PM_BISSC_CD_ADDR_CRC_M;
            tempCD += (tempCRC << PM_BISSC_CD_ADDR_CRC_S);

            //
            // If the access is a read, set the R bit and clear W, set STOP1
            // This is the end of the CDM stream for a register read
            //
            if(registerParams->accessType == PM_BISSC_REGISTER_READ)
            {
                tempCD += (PM_BISSC_CD_READ << PM_BISSC_CD_RW_S);
            }
            else // Access is write
            {
                tempCD += (PM_BISSC_CD_WRITE << PM_BISSC_CD_RW_S);
                tempCRC = bissc_getCRC(cdParams->crcStart,
                                       cdParams->crcBits,
                                       &registerParams->txData,
                                       bisscCRCtableCD,
                                       PM_BISSC_CD_CTS_ID_ADDR_BYTES);
                tempCRC = ~tempCRC & PM_BISSC_CD_DATA_CRC_M;
                tempCD += (tempCRC << PM_BISSC_CD_DATA_CRC_S);
                tempCD += (registerParams->txData << PM_BISSC_CD_DATA_S);
            }

            //
            // - Set the START2 bit
            // - Flip the bit order so that bit 0 is the next CDM bit to
            //   be sent.
            //
            tempCD += (PM_BISSC_CD_START_SET << PM_BISSC_CD_START2_S);
            cdParams->cdmStream = __flip32(tempCD);
            cdParams->cdState = PM_BISSC_CD_TRANSMITTING;
            registerParams->rxData = PM_BISSC_DATA_INVALID;
            numCDBitsTx = PM_BISSC_CD_BITS;
            cdParams->cdStatus = PM_BISSC_CD_PASS;
            break;

        case PM_BISSC_CD_TRANSMITTING:
            //
            // A control-data frame is currently being transmitted
            // Get the next CDM bit
            // Set the CLB input to match the CDM bit value (inverted)
            // Shift out the bit used and decrement the count to send
            //

            if(numCDBitsTx > -1)
            {
                uint32_t tempCDMBit;
                tempCDMBit = (~cdParams->cdmStream) & 1U;
                bissc_setCDMBit((enum cdmValue)tempCDMBit);
                cdParams->cdmStream = cdParams->cdmStream >> 1U;
                numCDBitsTx--;
            }
            else
            {
                cdParams->cdState = PM_BISSC_CD_COMPLETE;
            }
            break;
        case PM_BISSC_CD_COMPLETE:
            //
            // A control-data frame has completed and is ready to be parsed
            // - extract the read + write + start bits from the CDS stream
            //
            //
            rxdRW = (cdParams->cdsStream >> PM_BISSC_CD_RW_S);
            rxdRW = rxdRW & PM_BISSC_CD_RW_M;
            rxdS = (cdParams->cdsStream >> PM_BISSC_CD_START2_S);
            rxdS = rxdS & PM_BISSC_CD_START2_M;

            //
            // Check for valid Read + Write bits. If these are valid,
            // check the start bit. If it is set then proceed to
            // extract the received CRC and received data. Calculate
            // the CRC of the data and compare it with the received CRC.
            //
            if((rxdRW == PM_BISSC_CD_WRITE || rxdRW == PM_BISSC_CD_READ)
                && rxdS == PM_BISSC_CD_START_SET)
            {
                uint16_t rxdCRC;
                uint16_t rxdData;

                rxdCRC = (cdParams->cdsStream >> PM_BISSC_CD_CRC2_S)
                          & PM_BISSC_CD_CRC2_M;

                rxdData = (cdParams->cdsStream >> PM_BISSC_CD_DATA_S)
                           & PM_BISSC_CD_DATA_M;

                calculatedCRC = PM_BISSC_CD_CRC2_M &
                                (~bissc_getCRC(cdParams->crcStart,
                                cdParams->crcBits,
                                (uint16_t *)&rxdData,
                                bisscCRCtableCD,
                                PM_BISSC_CD_CTS_ID_ADDR_BYTES));

                if(calculatedCRC == rxdCRC)
                {
                    //
                    // In the case of a PM_BISSC_CD_READ, the received data
                    // is the content of the register that was read.
                    //
                    if(rxdRW == PM_BISSC_CD_READ)
                    {
                        registerParams->rxData = rxdData;
                        cdParams->cdStatus = PM_BISSC_CD_READ_PASS;
                    }
                    //
                    // PM_BISSC_CD_WRITE: confirm the encoder data echoback
                    // (received data) matches the transmitted data. If it
                    // does not match, then the write failed.
                    //
                    else if(registerParams->txData == rxdData)
                    {
                        registerParams->rxData = rxdData;
                        cdParams->cdStatus = PM_BISSC_CD_WRITE_PASS;
                    }
                    else
                    {
                        cdParams->cdStatus = PM_BISSC_CD_WRITE_FAIL;
                    }
                }
                else
                {
                    cdParams->cdStatus = PM_BISSC_CD_CRC_FAIL;
                }
            }
            else
            {
                //
                // The encoder returns both R == 1 and W == 1 indicates an
                // error condition. An example might be attempting to access
                // an invalid register.
                //
                if(rxdRW == PM_BISSC_CD_RW_INVALID)
                {
                    cdParams->cdStatus = PM_BISSC_CD_READ_WRITE_INVALID;
                }
                //
                // Start bit == 0 occurs when the encoder needs longer for a
                // read or write access. This is currently not supported by
                // the state machine.
                //
                if(rxdS == PM_BISSC_CD_START_CLEAR)
                {
                    cdParams->cdStatus = PM_BISSC_CD_READ_WRITE_UNSUPPORTED;
                }
            }
            cdParams->cdState = PM_BISSC_CD_PARSED;
            break;

        case PM_BISSC_CD_PARSED:
            cdParams->cdState = PM_BISSC_CD_WAIT;
            waitCount = 1;
            break;

        case PM_BISSC_CD_WAIT:
            //
            // The start bit of a control frame must be preceded by at least
            // 14 cycles with CDM = "0".
            //
            waitCount++;
            if(waitCount == 14U) cdParams->cdState = PM_BISSC_CD_NONE;
            break;

        case PM_BISSC_CD_NONE:
            break;

        default:
            break;
    }
    return;
}

//
// Private functions
//

static uint64_t bissc_getBits(uint16_t numBitsToFetch,
                              uint16_t numBitsParsed, uint16_t bufferWidth)
{
    //
    // Refer to the header file documentation
    //
    uint64_t returnBits;   // The bits extracted from the array
    uint16_t numElements;  // Number of elements with bits to be extracted
    uint16_t baseElement;  // Lowest index element with bits to extract
    uint16_t bitIndex;     // Bit position in the baseElement
    uint32_t shift;
    uint32_t mask;
    uint16_t element;

    uint32_t *data = (uint32_t *)&scdRxData[0];

    baseElement = numBitsParsed / bufferWidth;
    bitIndex = bufferWidth - (numBitsParsed % bufferWidth);
    numElements = ( (2U * bufferWidth) + numBitsToFetch - bitIndex - 1U)
                    / bufferWidth;

    if(numBitsToFetch > bitIndex)
    {

        //
        // Case 1a Highest index element:
        //
        //  x = invalid data
        //
        //  A  A   A  ...  A  x  x  x  x   RxData[baseElement+numElements-1]
        //
        // - Calculate the shift required to right justify the data
        //   and remove the invalid data
        // - Place the data in the return variable right justified
        // - Track the most significant open bit within returnBits
        //
        element = baseElement + numElements - 1U;
        shift = (numElements - 1U) * bufferWidth - numBitsToFetch + bitIndex;
        returnBits = data[element] >> shift;
        uint32_t tempIndex = bufferWidth - shift;

        //
        // Case 1b:
        //
        // If there are more than 2 elements, this loop will execute for the
        // remaining elements with the exception of the baseElement.
        //
        for(element = baseElement + numElements - 2U;
                      element > baseElement;
                      element--)
        {
            returnBits |= data[element] << tempIndex;
            tempIndex += bufferWidth;
        }

        //
        // RxData[baseElement]
        //
        //  P = previously parsed bit
        //
        //  P  P*  A  ...  A  A  A  A  A   RxData[baseElement]
        //     ^--bitIndex
        //
        // - Calculate the mask required to remove previously parsed
        //   bits. The desired bits are already right justified.
        // - The shift required depends on previous shift values
        // - Shift and load the desired bits into the return variable
        //
        mask = (1U << bitIndex) - 1U;
        returnBits |= (uint64_t)(data[baseElement] & mask)
                << (uint64_t)tempIndex;
    }

    else
    {
        //
        // Case 2:
        //
        // - Calculate the mask required to remove previously parsed
        //   bits. The bits to extract are already right justified.
        // - Shift and load the desired bits into the return variable
        //
        shift = bitIndex - numBitsToFetch;
        mask = (1 << numBitsToFetch) - 1;
        returnBits = (data[baseElement] >> shift) & mask;
    }
    return(returnBits);
}


static inline void bissc_setCDMBit(enum cdmValue value)
{
    EALLOW;
    HWREGBITHW(PM_BISSC_FSM_BASE
               + CLB_LOGICCTL
               + CLB_O_GP_REG, 7, (value & 0x1) );
    EDIS;
}

//*****************************************************************************
//
// End of file
//
//*****************************************************************************
