# YJH 디버그 관측 포인트 기준서 (Step 4, Draft)

## 0. 목적
- 전투 세션 문제를 빠르게 재현/진단하기 위한 공통 관측 포인트를 고정한다.
- 본 문서는 계획 단계 Draft이며, 실제 코드 작성 전까지 변경 가능하다.

## 1. 운영 원칙
- 로그 키는 고정하되, 필드 추가/조정은 허용한다.
- 디버그 도구는 "문제 추적 시간 단축"을 최우선으로 설계한다.
- 서버 판정 경로와 클라 체감 경로를 분리해서 관측한다.

## 2. 필수 로그 키
- YJH_DBG_SESSION_BEGIN
- YJH_DBG_SESSION_STATE
- YJH_DBG_DAMAGE_APPLY
- YJH_DBG_DEATH_REPORT
- YJH_DBG_FINALIZE_ONCE
- YJH_DBG_ABORT
- YJH_DBG_RETURN_TO_BOARD

## 3. 로그 필드 표준
- CombatSessionId
- CombatantId
- Role(Server/Client)
- StateName
- EventSeq
- EndReason
- TimestampMs
- MatchRuleVersion

## 4. 콘솔 디버그 명령(초안)
- yjh.dbg.dump_session
- yjh.dbg.dump_combatants
- yjh.dbg.force_abort
- yjh.dbg.trace_on
- yjh.dbg.trace_off

## 5. 화면 관측(오버레이)
- Session: CombatSessionId, 현재 State
- Combatant: HP, DeadFlag, LastDamage
- Finalize: FinalizeOnce 상태값
- Net: Server/Client Role, Ping

## 6. 이슈 진단 순서
1. YJH_DBG_SESSION_BEGIN 존재 확인
2. YJH_DBG_SESSION_STATE 전이 순서 확인
3. 피해 적용/사망 보고 이벤트 확인
4. YJH_DBG_FINALIZE_ONCE 중복 여부 확인
5. 보드 복귀 이벤트 확인

## 7. 완료 기준(DoD)
- 주요 이슈 3종(중복 종료, HP 비동기, 복귀 누락)을 로그만으로 원인 분류 가능
- 재현자와 수정자가 동일 로그 키로 대화 가능
- 3분 내 진단 시작점(SessionId/State/EventSeq) 식별 가능

## 7-1. 관측 지표(초안)
- MatchCountTotal
- DrawRate
- TimeoutRate
- ForcedAbortRate
- AvgCombatDurationSec
- 지표 정의 문서: yjh_observability_metrics_draft.md

## 8. 변경 정책
- 계획 단계에서는 자유 수정 가능(Draft)
- 코드 반영 시작 시점에 v1.0으로 승격
- 승격 후 파괴적 변경은 yjh_change_policy.md의 C등급 절차 적용
