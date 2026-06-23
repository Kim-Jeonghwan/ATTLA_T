1. 먼저, 현재 프로젝트(ATTLA_T)의 CSU, HAL, main 을 모두 읽을 것
2. D:\Nexcom\Firmware\01_Project\02_Tester\TMDSCNCD28388D_T\TMDSCNCD28388D_T 경로의 CPU1과 CM 의 CSU, HAL, main 을 모두 읽을 것

3. 현재 W6100에 할당되어 있는 체계와의 통신 이더넷을 PHY칩을 사용한 동일 DSP(F28388D) CM코어 이더넷으로 변경하려고함

4. W6100은 SPIA 유지한채로 모니터링용 이더넷으로 사용 예정

5. PHY 칩은 DP83822HFRHBT
6. PHY 입력 클럭 25 MHz
7. MII 모드 사용
8. DSP 할당 핀은 현재 미정(TBD)
9. HX188NL 아이솔레이터로 PHY와 외부 연결

10. CM 코어 소스 및 구조, 관련 CPU1 소스는 D:\Nexcom\Firmware\01_Project\02_Tester\TMDSCNCD28388D_T\TMDSCNCD28388D_T
경로의 CPU1과 CM 자료를 참고 이식하려고 함 (연결 핀은 TBD로, 주석으로 강조 표기)

11. 체계와의 이더넷 통신 규격 및 구조는 이 ATTLA_T 이더넷 스펙.md 와 연동통제안과 아키텍쳐.md 를 따를 것.

모두 구현하기 위한 조사 시작