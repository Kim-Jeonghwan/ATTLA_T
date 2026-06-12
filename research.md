# 리서치 보고서: CM/IPC 및 비트필드 제어 관련 코드 조사

## 1. 개요
사용자 요청 및 `GEMINI.md` (글로벌 코딩 규칙)에 따라, 펌웨어 내의 CSU, HAL, main 파일들을 전수 조사하여 다음 사항을 확인했습니다.
1. CM 코어 및 IPC와 관련된 내용이나 주석 색출.
2. Driverlib 방식으로 작성되지 않고 직접 비트필드나 레지스터(`Regs.`, `.bit`, `HWREG`)를 제어하는 레거시 코드 색출 (단, `csu_`나 `hal_` 접두어가 없는 파일 제외).

## 2. 조사 결과

### 2.1 CM 코어 및 IPC 관련 내용 발견 (삭제 대상)
다음 파일들에서 CM 코어 및 IPC 관련 주석이나 매크로가 발견되었습니다. 해당 내용들은 향후 구현 단계에서 모두 삭제되어야 합니다.

*   **`ATTLA_T_CPU1/main.c`**
    *   **Line 53:** `	- 시스템 초기화 및 CM 코어와의 동기화를 수행한 후, 주기에 맞춰 태스크를 실행합니다.` (주석 내 CM 언급)
*   **`ATTLA_T_CPU1/main.h`**
    *   **Line 33~34:** 
        ```c
        #ifndef MEMCFG_GSRAMMASTER_CM
        #define MEMCFG_GSRAMMASTER_CM    2
        #endif
        ```
*   **`ATTLA_T_CPU1/HAL/hal_DspInit.c`**
    *   **Line 14:** ` * 2026. 06. 02. - CM 코어 기동 시점(Initial_CmCore)을 동기화(IPC_sync) 직전 최고의 타이밍으로 대이동 교정`
    *   **Line 18:** ` * 2026. 06. 02. - IPC_sync 대기 전에 이더넷 PHY 기동(initEmacGpioPins)이 되도록 기동 시퀀스 순서 최우선 개편`
    *   **Line 21:** ` * 2026. 06. 04. - IPC 동기화(Initial_IPC) 호출을 DSP_Initialization 내부에서 main.c로 상향 이동`
    *   **Line 22:** ` * 2026. 06. 04. - CM 하드폴트 원천 박멸을 위해 미존재 인터럽트 권한 양도 API 삭제 및 초기화 안정화`
    *   **Line 61:** `    - CM 코어와 IPC 하드웨어를 동기화시키고 각종 페리페럴을 초기화합니다.` (주석 내 CM 및 IPC 언급)

### 2.2 Driverlib 미적용 비트필드 구문 조사 결과
`Regs.`, `.bit`, `HWREG` 등을 포함하는 비트필드/레지스터 직접 제어 코드를 대상으로 검색 및 분석을 진행한 결과:
*   대상 파일(`csu_*.c/h`, `hal_*.c/h`, `main.c`, `main.h`) 내에는 **비트필드를 사용하는 코드가 존재하지 않습니다.**
*   모든 대상 주변장치(ADC, EPWM, SPI, GPIO 등)는 현재 `ADC_setPrescaler()`, `SPI_setConfig()`, `EPWM_setActionQualifierAction()` 등 **TI C2000 Driverlib API를 올바르게 적용하여 작성**되어 있음을 확인했습니다.
*   *참고:* `HAL/easy28x_driverlib_v12.2.c` 와 같은 파일에서는 비트필드 및 `HWREG`가 다수 사용되고 있으나, 이 파일은 사용자의 룰("`csu_`나 `hal_`이 붙지 않는 파일은 제외")에 따라 조사 대상 및 수정 범위에서 안전하게 제외되었습니다.

## 3. 조치 계획 (Action Plan)
리서치가 완료됨에 따라, 사용자의 구현 승인이 떨어지면 다음과 같이 변경을 진행할 예정입니다:
1. `main.c`, `main.h`, `hal_DspInit.c` 에 남아있는 CM 및 IPC 관련 주석과 매크로를 전부 삭제합니다.
2. 그 외 파일들 중 Driverlib을 적용해야 할 비트필드 레거시 코드는 발견되지 않았으므로, 별도의 수정 없이 기존 Driverlib 스타일을 유지합니다.
