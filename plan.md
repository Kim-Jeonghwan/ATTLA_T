# 구현 계획 (Implementation Plan)

## 1. 개요 (Goal Description)
`Pmn24V` (브레이크 전압 감지) 신호가 Active(Low, `0U`)일 때를 정상으로 인식하고, 비활성화(High, `1U`) 상태일 때 결함으로 판단하도록 코드를 수정합니다.
(Modify the code so that the `Pmn24V` (brake voltage detection) signal is recognized as normal when it is Active(Low, `0U`), and triggers a fault when it is inactive(High, `1U`).)

## 2. 제안하는 변경 사항 (Proposed Changes)

### CSU 
---
#### [MODIFY] [csu_Bit.c](file:///d:/Nexcom/Firmware/01_Project/04_ATTLA/ATTLA_T/ATTLA_T/ATTLA_T_CPU1/CSU/csu_Bit.c)
- `Bit_OvVoltage_Check` 함수 내의 조건문을 변경합니다.
  (Change the condition statement in the `Bit_OvVoltage_Check` function.)
- `if (xDio.Pmn24V == 0U)` ➡️ `if (xDio.Pmn24V == 1U)`
- 파일 최상단 헤더 주석의 `Version` 및 `Last Updated`를 업데이트합니다. (Update `Version` and `Last Updated` in the header comment.)
- 헤더 주석 직후의 `Modification History` 주석 블록에 신규 수정 이력을 추가합니다. (Add a new modification history entry immediately after the header block.)

## 3. 검증 계획 (Verification Plan)
- 코드 컴파일을 통해 문법적 오류가 발생하지 않는지 확인합니다. (Verify no syntax errors occur through code compilation.)
- 수정된 부분을 사용자가 리뷰하여 의도와 일치하는지 점검합니다. (Have the user review the modifications to ensure they align with the intent.)
- 사용자가 직접 CCS를 통해 빌드 및 장비 테스트를 수행합니다. (The user will manually build and run equipment tests via CCS.)

## 4. 사용자 검토 대기 (User Review Required)
위 계획을 검토하시고, 진행 여부 및 추가 요청 사항을 남겨주시면 감사하겠습니다. 수정해도 좋다면 **"진행"** 또는 **"구현 시작"**이라고 말씀해 주십시오. 
(Please review the plan above and provide any feedback or additional requests. If it is good to go, please tell me to "Proceed" or "Start implementation".)
