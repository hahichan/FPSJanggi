# YJH 게임모드 계획서 (초안)

## 0. 문서 목적
- ArenaBattleReal 1대1 전투의 매치 흐름을 게임모드 관점에서 고정한다.
- 캐릭터 내부 구현은 다루지 않고, 오케스트레이션과 판정 책임만 다룬다.

## 1. 소유 책임
- 전투 진입 조건 확인
- 전투 시작/종료 타이밍 제어
- 승패 확정(서버 권한)
- 보드 시스템(NSH) 결과 반영 호출
- 예외 상황(중복 종료, 타임아웃, 이탈) 처리

## 2. 비책임
- 이동/시점/입력 세부
- 공격 판정 알고리즘 상세
- 캐릭터 애니메이션/FX

## 3. 상태 흐름(서버 기준)
1. Idle
2. PrepareArena
3. ArenaBattleReal
4. Resolve
5. ReturnToBoard

## 4. 공통 용어(통합 시트와 동일)
- CombatSessionId
- CombatantId
- EndReason
- FinalizeOnce

## 5. 핵심 계약
- BeginCombatSession(CombatSessionId, RequestContext)
- NotifyCombatantReady(CombatSessionId, CombatantId)
- ReportCombatResult(CombatSessionId, LoserCombatantId, EndReason)
- FinalizeCombatSession(CombatSessionId, Result)
- AbortCombatSession(CombatSessionId, EndReason)

## 5-1. 시작 입력 데이터(고정)
- ArenaBattle -> ArenaBattleReal 전이 시점에 아래 정보를 함께 전달
	- TeamInfo(진영)
	- PieceInfo(말 종류/식별)
- BeginCombatSession 실패 시 예외 처리
	- 캐릭터 정보 누락: 즉시 AbortCombatSession(InvalidSpawnData)
	- 스폰 실패: 1회 자동 재시도 후 실패 시 AbortCombatSession(SpawnFailed)
	- 세션 불일치: 즉시 AbortCombatSession(InvalidSession)

## 5-2. 세션 생명주기/시간 기준
- 서버 시계만 판정 기준으로 사용
- 클라이언트 표시는 서버 기준 시간 동기화 값 사용
- 상세 기준: yjh_session_lifecycle_spec.md

## 5-3. 종료 우선순위
- ForcedAbort > DrawBySameTickBatch > DrawByTimeout
- 상세 기준: yjh_termination_priority_spec.md

## 6. 중복/경합 방지 규칙
- CombatSessionId 단위로 진행 중 세션 1개만 허용
- FinalizeOnce 규칙: Finalize는 원자적으로 1회만 성공
- 동일 LoserCombatantId 보고 중복 무시
- 타이머 기반 자동 종료는 Resolve 상태에서만 유효

FinalizeOnce 원자성 고정 규칙
- 세션별 원자 플래그 bFinalizeCommitted(초기 false)를 둔다.
- Finalize/Abort 진입 시 CompareExchange(false -> true) 성공한 1회만 Commit한다.
- CompareExchange 실패 경로는 LateCandidate로 로그만 남기고 상태 변경/브로드캐스트를 금지한다.
- Commit 성공 경로만 Result 반영, EndReason 확정, 브로드캐스트 1회를 수행한다.

## 6-1. 타임아웃 정책(가변)
- TimeoutSeconds는 설정값(DataAsset/Config)으로 조절 가능
- 기본값은 60초
- 캐릭터/말 정보 기반 오버라이드 허용(예: PieceTimeoutOverrideSeconds)
- 최종 적용값은 서버에서 결정
- 코드 하드코딩 대신 설정값 기반으로 관리
- Timeout 도달 시 결과는 Draw(EndReason=Timeout, RemovePolicy=BothEliminated)

## 7. 네트워크 규칙
- 승패 확정과 결과 반영은 서버에서만 수행
- 클라 요청은 Request 수준, 판정은 서버 단독
- 종료 브로드캐스트는 확정 후 1회 전송
- 이탈/재접속 유예 정책 적용(기본 8초)
- 상세 기준: yjh_disconnect_grace_policy.md

## 7-1. 매치 버전 필드
- MatchRuleVersion을 세션 시작 시 고정
- Finalize/지표/테스트 리포트에 동일 버전 기록
- 상세 기준: yjh_match_rule_version_spec.md

## 8. 완료 기준(공통 용어 기준)
- 2인 접속에서 전투 시작/종료가 5회 연속 안정 동작
- FinalizeCombatSession 이벤트가 CombatSessionId당 1회만 기록
- NSH 보드 복귀가 중복 없이 동작
- 비김(Draw) 발생 시 양측 제거 정책이 일관되게 적용

## 9. 테스트 시나리오
1. 정상 승패 종료
2. 동시 사망 유사 상황(레이스) -> Draw
3. 종료 직전 재요청(연타)
4. 한 명 이탈
5. 서버 지연 상황에서 재전송
6. 타임아웃 종료 -> Draw

## 9-1. 결과 판정 규칙(고정)
- RedWin: Blue HP가 먼저 0
- BlueWin: Red HP가 먼저 0
- Draw: 같은 서버 틱의 동일 데미지 배치 처리 후 양측 HP가 모두 0
- Draw(Timeout): 제한 시간 도달
- Draw 처리 정책: 양측 제거(BothEliminated)

## 10. 로그 표준
- YJH_GM_MATCH_START
- YJH_GM_MATCH_FINALIZE
- YJH_GM_MATCH_ABORT
- YJH_GM_MATCH_RETURN

## 10-1. 관측 지표 연결
- TimeoutRate, DrawRate, ForcedAbortRate, AvgCombatDurationSec를 기본 지표로 집계
- 상세 기준: yjh_observability_metrics_draft.md

## 10-2. 테스트 매트릭스 연결
- 상세 기준: yjh_test_matrix.md

## 11. 구현 우선순위
1. 상태/계약 함수 고정
2. FinalizeOnce 보장
3. NSH 반영 연결
4. 예외 처리 추가
