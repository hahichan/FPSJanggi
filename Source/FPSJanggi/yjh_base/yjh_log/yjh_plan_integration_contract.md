# YJH 통합 접점 시트 (초안)

## 0. 목적
- 게임모드와 베이스 캐릭터 간 호출 계약만 관리한다.
- 구현 세부는 각 계획서에서 관리한다.

## 1. 공통 용어(고정)
- CombatSessionId: 전투 세션 식별자(기존 MatchId 용어 통일)
- CombatantId: 전투 참가자 식별자(PlayerId 매핑)
- EndReason: 전투 종료 사유(승패/중단/시간초과)
- FinalizeOnce: 승패 확정 1회 보장 규칙
- TeamInfo: 진영 정보(Blue/Red)
- PieceInfo: 말 정보(말 종류/식별)
- ResultType: RedWin/BlueWin/Draw
- DamageBatch: 같은 서버 틱에 수집된 유효 데미지 집합
- TimeoutSeconds: 전투 제한 시간 설정값(가변)
- TimeoutDefaultSeconds: 기본 전투 제한 시간(60)
- PieceTimeoutOverrideSeconds: 캐릭터/말별 제한 시간 오버라이드(optional)
- GraceSeconds: 이탈 유예 시간 설정값(기본 8)
- MatchRuleVersion: 전투 규칙 세트 버전

## 2. 게임모드 -> 캐릭터
- BeginCombatSession(CombatSessionId, SpawnContext)
- SetCombatEnabled(CombatSessionId, bEnabled)
- ForceTerminateCombatSession(CombatSessionId, EndReason)

SpawnContext 최소 필드
- TeamInfo
- PieceInfo
- SpawnTransform
- TimeoutDefaultSeconds(기본 60)
- PieceTimeoutOverrideSeconds(optional)
- MatchRuleVersion

## 3. 캐릭터 -> 게임모드
- ReportCombatantReady(CombatSessionId, CombatantId)
- ReportCombatantDead(CombatSessionId, DeadCombatantId, KillerCombatantId, EndReason)
- ReportCombatFault(CombatSessionId, CombatantId, ErrorCode)

## 4. 서버 권한 경계
- Begin/Finalize/Abort는 서버 전용
- 클라는 Request/Report만 가능
- 승패 확정은 서버 단일 지점에서 처리

## 5. 중복 방지 키
- CombatSessionId
- CombatantId
- FinalizeToken(내부)

## 6. 실패 처리 계약
- Timeout: AbortCombatSession(Timeout)
- DuplicateReport: Ignore + WarningLog
- InvalidState: Ignore + ErrorLog
- DisconnectGraceExpired: AbortCombatSession(ForcedAbort)

## 6-1. 결과 전달 계약
- FinalizeCombatSession(Result)에서 전달할 최소 필드
	- ResultType(RedWin/BlueWin/Draw)
	- WinnerTeam(optional, Draw 시 null)
	- LoserTeam(optional, Draw 시 null)
	- EndReason
	- RemovePolicy(BothEliminated when Draw)

Draw 판정 기준
- 같은 서버 틱의 동일 DamageBatch 적용 완료 후 양측 HP <= 0 이면 Draw

종료 우선순위 기준
- ForcedAbort > DrawBySameTickBatch > DrawByTimeout

## 7. 로그 이벤트 이름
- YJH_INT_BEGIN
- YJH_INT_READY
- YJH_INT_DEAD
- YJH_INT_FINALIZE
- YJH_INT_ABORT

## 8. 버전 관리
- ContractVersion: v0.1
- breaking change 발생 시 버전 업데이트 후 양측 문서 동시 수정
