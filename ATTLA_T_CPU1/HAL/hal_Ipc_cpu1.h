/**********************************************************************
   Nexcom Co., Ltd.
   Filename         : hal_Ipc_cpu1.h
   Version          : 00.02
   Description      : CM Core IPC 디바이스 드라이버 헤더
   Programmer       : Kim Jeonghwan
   Last Updated     : 2026. 07. 01. (헤더 버전 동기화 및 템플릿 유지)
**********************************************************************/

/*
 * Modification History
 * --------------------
 * 2026. 07. 01. - 헤더 버전 동기화 및 템플릿 유지 (코딩 규칙 적용)
 * 2026. 06. 23. - main.h -> main_cpu1.h 인클루드 명칭 리팩토링
 * 2026. 06. 23. - CM 기동 검출을 위한 IPC 제어 선언
 */

#ifndef HAL_IPC_CPU1_H
#define HAL_IPC_CPU1_H

#include "main_cpu1.h"

/* CM 기동 상태 구조체 */
typedef struct {
    bool isCmReady; // CM 코어의 부팅 및 통신 준비 완료 상태를 나타내는 플래그 (true: 준비됨, false: 미준비)
} stIpcState;

extern volatile stIpcState xIpcState;

/* 함수 프로토타입 */
void Initial_IPC_Clear(void);
void Initial_IPC(void);

#endif // HAL_IPC_CPU1_H
