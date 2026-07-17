# YJH 테스트 매트릭스 (Draft)

## 0. 목적
- 테스트 케이스를 표 형태로 고정해 누락을 줄이고 사용자에게도 상태를 공유 가능하게 한다.

## 1. 매트릭스
| ID | 영역 | 조건 | 기대 결과 | 우선순위 |
|---|---|---|---|---|
| TM-01 | 정상 종료 | Red HP 0 | BlueWin 1회 Finalize | High |
| TM-02 | 동시사망 | 같은 틱 DamageBatch로 양측 HP 0 | Draw + BothEliminated | High |
| TM-03 | 타임아웃 | EffectiveTimeoutSeconds 도달 | Draw + BothEliminated | High |
| TM-04 | 강제중단 | ForcedAbort 트리거 | Draw + BothEliminated, 우선 적용 | High |
| TM-05 | 단일 이탈 | Grace 8초 내 복귀 | 세션 지속 | Medium |
| TM-06 | 단일 이탈 | Grace 8초 초과 | ForcedAbort 후보 반영 | High |
| TM-07 | 양측 이탈 | 동시 이탈 후 Grace 만료 | ForcedAbort 종료 | High |
| TM-08 | 스폰 실패 | 1회 재시도 후 실패 | Abort + 명시 로그 | High |
| TM-09 | 오버라이드 타임아웃 | PieceTimeoutOverrideSeconds=30 | 30초 기준 종료 | Medium |
| TM-10 | 중복 종료 요청 | 종료 이벤트 다중 유입 | Finalize 1회만 성공 | High |

## 2. 공개 운영
- 내부 QA용 상세 로그와 사용자 공유 요약 표를 분리 운영
- 사용자 공유 표에는 ID/상태/수정 버전만 표시
