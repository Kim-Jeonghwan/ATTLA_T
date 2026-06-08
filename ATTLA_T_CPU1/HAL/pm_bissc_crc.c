//###########################################################################
//
// FILE: pm_bissc_crc.c
//
// TITLE: BiSS-C encoder interface CRC functions
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

#include "pm_bissc_include.h"

void bissc_generateCRCTable(uint16_t nBits,
                            uint16_t polynomial,
                            uint16_t *pTable)
{
    int16_t i, j;
    uint16_t accum;

    polynomial <<= (8U - nBits);
    for(i = 0; i < 256U ; i++){
        accum  = i;
        for( j = 0; j < 8U; j++){
            if(accum & 0x80)
            {
                //
                // If leading bit is 1:
                // - shift accum to left
                // - mask off unwanted MSbs and xor the rest with polynomial
                //
                accum = ((accum << 1) & 0xFFU) ^ polynomial;
            }
            else
            {
                //
                // If leading bit is 0:
                // - shift accum to left
                // - mask off unwanted MSbs
                //
                accum = ((accum << 1) & 0xFFU);
            }
        }
        pTable[i] = accum;
    }
}


uint16_t bissc_getCRC(uint16_t crcStart, uint16_t numBitsPoly,
        uint16_t *data, uint16_t *crcTable, uint16_t numBytes)
{
    uint16_t tableIndex;
    uint16_t j;
    uint16_t byteIndex;
    uint16_t crcResult = crcStart;

    byteIndex = numBytes - 1U;
    int16_t * const msg_ptr = (int16_t *)data;

    for (j = 0; j < numBytes ; j++, byteIndex--)
    {
        int16_t myByte = __byte((int *)msg_ptr, byteIndex);
        tableIndex = crcResult   ^ myByte;
        crcResult  = crcTable[tableIndex];
    }
    return(uint16_t)(crcResult  >> (8 - numBitsPoly));
}

//*****************************************************************************
//
// End of file
//
//*****************************************************************************
