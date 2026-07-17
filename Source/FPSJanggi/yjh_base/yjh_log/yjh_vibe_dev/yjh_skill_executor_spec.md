# YJH 스킬 실행기 명세서 (v2 Draft)

## 0. 목적
- SkillData를 실제 동작으로 해석/실행하는 실행기 계층을 정의한다.

## 1. 실행 위치
- yjh_ArenaCombatComponent 내부 또는 Executor 모듈
- 서버 권한으로 최종 판정/적용 수행

컴포넌트 명명 정책
- 문서 기본 명칭은 yjh_ArenaCombatComponent를 사용한다.
- 별도 SkillExecutorComponent를 둘 경우에도 책임은 동일하며, Combat 계층 하위 실행기로 취급한다.

스킬관리자/스킬실행기 분리 정책
- SkillManager 책임: 슬롯 매핑, 스킬 보유 목록, 쿨타임/차지 상태 관리
- SkillExecutor 책임: 요청 검증, 실행기 디스패치, 최종 적용 판정
- 단일 컴포넌트로 합치는 것은 허용한다. 단, 내부 함수/영역에서 Manager 책임과 Executor 책임을 분리해야 한다.
- 분리하지 않고 혼합 구현할 경우 유지보수 난이도 상승, 테스트 분리 어려움, 회귀 위험 증가.

현재 프로젝트 선택(확정)
- 물리 구조는 단일 컴포넌트(yjh_ArenaCombatComponent)로 간다.
- 내부를 SkillManager 영역과 SkillExecutor 영역으로 분리해 구현한다.

## 2. 기본 실행기 목록(v2)
1. HitScanExecutor
- 히트스캔 기반 공격(사거리/피해량/피격 범위/쿨타임)

2. ProjectileExecutor
- 투사체 기반 공격(비행 에셋, 중력/무게/속도, 충돌 판정, 피해량)

3. DashExecutor
- 돌진(돌진 파워/방향 입력 기반)

4. CooldownControlExecutor
- 쿨타임 조작 스킬(스킬별 쿨타임 감소/증가)
- 내부적으로 음수 쿨타임 금지(최소 0.0 클램프)
- 쿨타임 상한은 999.0초로 제한한다.

5. TeleportExecutor
- 순간이동(목표 위치 검증 + 끼임 방지 안전 보정)
- 목표 지점 및 주변 대체 지점 모두 불가하면 취소하고 실패 코드를 반환한다.

6. PassiveExecutor
- 입력 없이 조건 기반으로 주기 실행(재생/오라/자동보호)

## 3. 서버 검증 순서
1. CombatSessionId 유효성 확인
2. 캐릭터 상태 사용 가능 여부 확인
3. 슬롯->SkillData 매핑 확인
4. 쿨다운/자원 검사
5. 범위/타깃 조건 검사
6. 실행 및 결과 로그 기록

## 3-1. 런타임 실행 파이프라인(v2)
1. ReceiveInput(SlotIndex)
2. ResolveSkillData(SlotIndex -> SkillId)
3. SnapshotSkillDataReadOnly(SkillId)
4. ValidateRequest(Session/State/Cooldown/Range/Target)
5. ExecuteByType(Executor Dispatch)
6. ApplyResult(HitScan/Projectile/Dash/Cooldown/Teleport/Passive)
7. StartCooldown(SkillId)
8. EmitEvents(OnSkillSlotMapped/OnCooldownChanged/CombatLog)

## 3-2. 동시 입력 처리 규칙
- 입력 우선순위는 yjh_input_role_matrix.md를 따름
- 동일 틱에 여러 Skill 입력이 들어오면 Slot 큐(선입력 우선) 처리
- Move/Look 입력은 Skill 검증과 병행 처리 가능

추가 정책
- 동일 틱 동일 Slot 재입력은 마지막 입력만 유효 처리(중복 방지)
- CastTimeSec > 0 인 스킬은 시전 중 인터럽트 규칙을 별도 표로 관리
- Teleport는 목적지 충돌 검사 실패 시 "최근 유효 지점"으로 폴백한다.
- Teleport 폴백 탐색 후 유효 지점이 없으면 즉시 취소한다.
- SkillData는 읽기 전용 규칙값이며, 실행 중 동적값은 RuntimeSkillRequestContext에서만 소비한다.
- 서버는 컨텍스트의 방향/위치/타깃을 재검증하고, 실패 시 FailCode를 기록한 뒤 즉시 중단한다.
- 클라 예측 확정은 사용하지 않고 서버 확정 결과만 반영한다.

## 4. 실패 처리
- 검증 실패 시 실행 중단
- 사유 코드 로그 기록
- 클라에는 실패 이벤트만 통지

실패 사유 코드(v2)
- SKX_InvalidSession
- SKX_InvalidState
- SKX_NoSkillMapped
- SKX_CooldownActive
- SKX_NotEnoughResource
- SKX_OutOfRange
- SKX_InvalidTarget
- SKX_ExecutionBlocked
- SKX_TeleportBlocked
- SKX_CooldownUnderflowGuard
- SKX_CooldownOverflowGuard
- SKX_TeleportNoValidDestination

로그 최소 필드
- CombatSessionId
- CombatantId
- SkillId
- SlotIndex
- FailCode
- ServerTime

## 5. 확장 규칙
- 새 스킬은 우선 기존 실행기 파라미터 조합으로 구성
- 신규 메커닉이 필요할 때만 Executor 1개 추가
- BaseCharacter에 스킬별 하드코딩 금지
- BP는 입력/연출/지표 표시를 담당하고, 최종 판정은 서버 실행기가 확정한다.

쿨다운 조작 경계
- 조작 후 쿨다운 값은 [0.0, 999.0] 범위로 클램프한다.

## 5-1. 1차 구현 범위(확정)
1. HitScanExecutor
2. ProjectileExecutor
3. DashExecutor
4. CooldownControlExecutor
5. TeleportExecutor
6. PassiveExecutor

구현 방식
- 위 6개를 한 번에 구현한다.

구현 이유
- 1~3은 전투 체감/판정 확인에 직결
- 4~5는 전략성과 이동 메타 형성에 필요
- 6은 캐릭터 개성 확장용 기본 계층

## 5-2. Blueprint 제작자용 호출 노드(계획)
| 블루프린트 노드명 | 입력 타입 | 출력 타입 | 한국어 설명 |
|---|---|---|---|
| RequestSkillBySlot | SlotIndex: Name, OptionalTarget: Actor | SkillExecutionResult(struct) | 슬롯 기반 스킬 실행 요청 |
| GetRemainingCooldownBySlot | SlotIndex: Name | float | 슬롯 남은 쿨타임 조회 |
| GetRemainingCooldownBySkillId | SkillId: Name | float | 스킬 ID 기준 남은 쿨타임 조회 |
| SetCombatContext | CombatSessionId: Name, CombatantId: Name | void | 전투 컨텍스트 설정 |
| InitializeSkillMap | none | bool | 데이터자산 매핑/검증 초기화 |

이벤트 노드(디스패처)
- OnSkillSlotMapped(SlotIndex: Name, SkillId: Name, bIsPassive: bool)
- OnCooldownChanged(SlotIndex: Name, RemainingSec: float)
- OnSkillRequestFailed(FailCode: Enum)

반환 규약
- SkillExecutionResult는 최소 bSuccess/FailCode를 포함한다.
- Teleport 실패 시 FailCode=SKX_TeleportNoValidDestination을 반환한다.

## 6. 연동 지점
- 스킬데이터 포맷: yjh_skill_data_format_spec.md
- 입력 역할표: yjh_input_role_matrix.md
- 초기 스킬 카탈로그: yjh_skill_initial_catalog.md
- BP 제작 가이드: yjh_bp_skill_authoring_guide.md
