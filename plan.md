# Fix Ethernet CM Core Freeze / Communication Drop Issue

The CM core hangs (ALIVE LED 145 and ETH LED 146 freeze) primarily due to **Ethernet DMA Descriptor Ring Corruption** and **Unsafe Interrupt Nesting**. The `-O1` optimization changes memory layout and execution timing just enough to trigger these critical bugs, but they are ticking time bombs even at `-O2`.

## Proposed Changes

### 1. Fix RX Descriptor Circular Queue Corruption (CRITICAL)
- **Problem**: `driverlib` requests `numBuildMsgs` (default 8) buffers during initialization. However, `ETH_RX_NUM_PKT_DESC` is only 4. Our `App_ethGetPacketBuffer` blindly wraps around `idx % 4`, handing the driver the same 4 descriptors twice! This creates a circular linked list inside the hardware DMA ring, causing infinite loops or Hard Faults.
- **Fix**: Explicitly set `pInitCfg->numBuildMsgs = ETH_RX_NUM_PKT_DESC;` during initialization. Also, safely track available RX buffers to return `NULL` instead of corrupting the queue if exhausted.

### 2. Fix TX Descriptor Wrap-Around Bug
- **Problem**: When the PC blasts TX packets (or if the link is down), `sendEthernetFrame` blindly queues descriptors using `s_ucTxPktDescIdx = (idx + 1) % 4`. If 5 packets are queued before the DMA finishes sending them, the 5th packet reuses Descriptor 0, truncating the driverlib's internal `waitQueue` and `descQueue`.
- **Fix**: Introduce `s_ucTxDescAvailCount`. `sendEthernetFrame` will only send if `s_ucTxDescAvailCount > 0`. `App_ethTxCallback` will increment it upon completion.

### 3. Fix TX Buffer Overwrite (Data Corruption)
- **Problem**: `xEthDriver.txBuf` is a single shared array. If we queue a Boot Done packet and immediately queue an ARP reply, the second packet overwrites the payload of the first packet *while the DMA is still transmitting the first one*.
- **Fix**: Change `txBuf` to a 2D array `txBuf[ETH_TX_NUM_PKT_DESC][ETH_TX_BUF_SIZE]`. Each TX descriptor will have its own dedicated payload buffer.

### 4. Fix Nested Interrupt Safety
- **Problem**: `driverlib` internally disables and enables interrupts inside `Ethernet_removePacketsFromRxQueue` (called by `isr_EmacRx0`). Our `Platform_enableCoreInterrupt` blindly calls `__enable_irq()`, which globally re-enables interrupts *while we are still inside the RX ISR*. This allows new interrupts to nest unsafely, blowing up the stack.
- **Fix**: Implement an interrupt nesting counter (`s_uiInterruptNestCount`) so `__enable_irq()` is only called when the outer-most disable is released.

## Files to Modify

### `ATTLA_T_CM/HAL/hal_Ethernet_cm.h`
- Change `stEthDriverState.txBuf` to `uint8_t txBuf[ETH_TX_NUM_PKT_DESC][ETH_TX_BUF_SIZE];`
- Add `bool Ethernet_isTxAvailable(void);`
- Add `uint8_t* Ethernet_getTxBuffer(void);`

### `ATTLA_T_CM/HAL/hal_Ethernet_cm.c`
- Update `Platform_disableCoreInterrupt` and `Platform_enableCoreInterrupt` to use a nesting counter.
- Set `pInitCfg->numBuildMsgs = ETH_RX_NUM_PKT_DESC;` in `Initial_Ethernet`.
- Add `s_ucTxDescAvailCount` logic and `Ethernet_isTxAvailable` getter.
- Fix `App_ethGetPacketBuffer` to track `s_ucRxDescAvailCount` and return `NULL` if depleted.
- Increment `s_ucTxDescAvailCount` inside `App_ethTxCallback`.

### `ATTLA_T_CM/CSU/csu_Ethernet_cm.c`
- Change `sendEthernetFrame` to check `Ethernet_isTxAvailable()` and fetch the correct `txBuf` index.
- Update `buildAndSendUdpPacket` and `sendAckResponse` to use the dynamically allocated `txBuf` index instead of sharing the static buffer.
