# CSU / HAL 계층 함수명 및 변수명 명명 규칙 조사 보고서

## 1. 개요
사용자의 지시에 따라 CSU, HAL, main 계층의 모든 소스 코드(.c) 및 헤더 파일(.h)을 대상으로, 함수명 및 변수명에 csu_ 또는 hal_ 접두어가 사용되었는지 전수 조사를 진행하였습니다.

**규칙 기준:** 모듈 및 파일명에만 소문자 csu_, hal_ 접두어를 사용해야 하며, 함수나 변수명에는 해당 접두어 사용을 엄격히 금지합니다. (단, 헤더 가드 등의 매크로는 대문자 HAL_, CSU_ 사용 허용)

## 2. 변수명 위반 사항 조사 결과
- 전체 코드베이스 검색 결과, csu_ 또는 hal_ 접두어가 포함된 **변수명은 발견되지 않았습니다.**

## 3. 함수명 위반 사항 조사 결과 (총 7건)
아래의 7개 함수들이 명명 규칙을 위반하여 소문자 접두어를 포함하고 있으므로, 접두어를 제거하는 수정이 필요합니다.

### [CSU 계층 위반 함수 - 4건]
1. csu_Bit_RunPBIT()
   - 위치: csu_Bit.c, csu_Bit.h, csu_Control.c
2. csu_Bit_RunCBIT()
   - 위치: csu_Bit.c, csu_Bit.h, csu_Control.c
3. csu_Control_CalibrateCurrentOffset()
   - 위치: csu_Control.c, csu_Control.h
4. csu_Control_SystemOperation()
   - 위치: csu_Control.c, csu_Control.h, hal_Adc.c

### [HAL 계층 위반 함수 - 3건]
1. hal_Led_InitGpio()
   - 위치: hal_Led.c, hal_Led.h, hal_DspInit.c
2. hal_Led_WritePin()
   - 위치: hal_Led.c, hal_Led.h, csu_Led.c
3. hal_Led_TogglePin()
   - 위치: hal_Led.c, hal_Led.h, csu_Led.c

## 4. 매크로 위반 여부 (위반 없음)
- HAL_MOTORDRIVER_SPI_CLK_PIN 등 대문자로 시작하는 매크로나, CSU_ADC_H 등의 헤더 가드는 명명 규칙(대문자 허용)에 부합하여 위반 사항이 아님을 확인하였습니다.

## 5. 개선 및 리팩토링 방안
위 7개의 함수를 선언, 정의, 호출하는 모든 소스 코드 영역에서 접두어를 제거하여 다음과 같이 수정합니다.

- csu_Bit_RunPBIT -> Bit_RunPBIT
- csu_Bit_RunCBIT -> Bit_RunCBIT
- csu_Control_CalibrateCurrentOffset -> Control_CalibrateCurrentOffset
- csu_Control_SystemOperation -> Control_SystemOperation
- hal_Led_InitGpio -> Led_InitGpio
- hal_Led_WritePin -> Led_WritePin
- hal_Led_TogglePin -> Led_TogglePin

> 사용자 승인 시, 이 내용을 바탕으로 plan.md를 작성하고 즉시 전체 코드 수정을 진행하겠습니다.
