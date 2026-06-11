BIT 항목 - 없는 것 구현
과전류 - 모터(ADC_ISEN_MOT), 브레이크(ADC_ISEN_BRK)
과전압 - 입력 전압(ADC_VSEN_28V), 브레이크전압(PM_n24V)
과열 - 보드 온도(ADC_TSEN_BD)
게이트 오류 - 모터드라이버(DRV_nFAULT)
모터 Stall 보호 - 위치 기준치 이하/이상
모터 과속 보호 - 속도 기준치 이하/이상
엔코더 - 에러,워닝
모든 폴트 리셋 함수 - 모든 폴트 초기화(0)

각 리미트 아래와 같이 선언
#define BIT_LIMIT_OVC_MOT_MAX 10.0f // 최대 연속 전류 9.34A 초과 시 Fault
#define BIT_LIMIT_OVC_BRK_MAX 1.5f  // 브레이크 최대 동작 전류 1.0A 초과 시 Fault
#define BIT_LIMIT_OVT_BD_MAX  80.0f // 보드 내부 온도
#define BIT_LIMIT_OVV_28V_MAX 32.0f // 28V 과전압


속도 측정 방법 ? 그냥 10번 받아서 1ms 만드는지 가중치가 있는지