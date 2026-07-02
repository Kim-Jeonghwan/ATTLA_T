# PMN24V 결함 판정 로직 조사 결과 (Research Report for PMN24V Fault Logic)

## 1. 개요 (Overview)
사용자 요청(101)에 따라 전체 파일 구조를 파악하였으며, `Pmn24V` (브레이크 전압 감지) 관련 로직을 조사했습니다. 
현재 코드는 `Pmn24V`가 `0U` (Active Low)일 때 결함(Fault)으로 판정하고 있어, 사용자의 의도(`Active(Low) == 정상`)와 반대로 동작하고 있습니다.

## 2. 현황 분석 (Current Status Analysis)
**대상 파일 (Target File):** `ATTLA_T_CPU1/CSU/csu_Bit.c`
**해당 함수 (Target Function):** `Bit_OvVoltage_Check(void)`

현재 구현된 코드는 다음과 같습니다. (Current Implementation:)
```c
  // 신규 PM_n24V 브레이크 전압 감시 (Active Low, 디바운싱 필터 적용)
  if (xDio.Pmn24V == 0U) {
    if (BitCnt_Brk24V > xBitLimit.ovvBrkTimeCnt) {
      xBit.faultOvVoltBrk = 1U;
      xBit.faultFlagSet = 1U;
      xBit.informAll |= 0x00002000U;
      BitCnt_Brk24V = 0U;
    } else
      BitCnt_Brk24V++;
  } else {
    if (BitCnt_Brk24V > 0U)
      BitCnt_Brk24V--;
  }
```
위 로직에 따르면 `xDio.Pmn24V == 0U` 일 때 에러 카운터(`BitCnt_Brk24V`)가 증가하고, 임계값을 초과하면 결함(`faultOvVoltBrk`)을 띄우고 있습니다.

## 3. 원인 파악 및 수정 방향 (Root Cause & Solution)
`Pmn24V`는 하드웨어적으로 Active Low로 설계되어 있어, `0U`일 때가 정상(브레이크 전압 인가됨)입니다. 따라서 전압이 인가되지 않은 비정상 상태(High, `1U`)에서 에러로 판정해야 합니다.

**수정 제안 (Proposed Fix):**
`csu_Bit.c` 의 `if (xDio.Pmn24V == 0U)` 구문을 `if (xDio.Pmn24V == 1U)` 로 변경해야 합니다.

```diff
-  if (xDio.Pmn24V == 0U) {
+  if (xDio.Pmn24V == 1U) {
```

## 4. 진행 여부 확인 (Request to Proceed)
본 조사 결과를 확인해 주시기 바랍니다. 수정 내용에 동의하시면 **구현(계획 반영 및 코드 수정)을 시작**하라고 지시해 주십시오. 
(Please review this research result. If you agree with the proposed fix, please instruct me to start the implementation.)
