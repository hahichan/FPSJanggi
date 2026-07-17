# YJH 관측성 지표 초안 (Draft)

## 0. 관측성 지표란?
- 로그와 별개로 운영 상태를 수치로 집계해 품질을 추적하는 지표.
- 예: 타임아웃 비율, 비김 비율, 평균 전투 시간.

## 1. 핵심 KPI
- MatchCountTotal
- MatchFinalizeSuccessRate
- DrawRate
- TimeoutRate
- ForcedAbortRate
- AvgCombatDurationSec
- AvgReconnectRecoverySec

## 2. 품질 경보 기준(초안)
- TimeoutRate > 15%: 경보
- ForcedAbortRate > 5%: 경보
- FinalizeSuccessRate < 99%: 경보

## 3. 집계 단위
- 세션 단위: CombatSessionId
- 시간 단위: 1h / 24h / 7d
- 버전 단위: MatchRuleVersion

## 4. 최소 저장 필드
- CombatSessionId
- MatchRuleVersion
- ResultType
- EndReason
- DurationSec
- DisconnectCount

## 5. 운영 활용
- 패치 전/후 지표 비교
- 말/캐릭터별 timeout 편향 탐지
- QA 재현 우선순위 결정
