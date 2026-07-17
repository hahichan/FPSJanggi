# YJH 코드 스켈레톤 생성 계획서 (Step 3)

## 0. 목적
- 실제 구현 전에 최소 컴파일 가능한 코드 골격을 만든다.
- 기능 완성보다 계약 고정과 확장 지점 확보를 우선한다.

## 1. 스켈레톤 대상 파일
1. yjh_ArenaCombatantBase.h/.cpp
2. yjh_ArenaHealthComponent.h/.cpp
3. yjh_ArenaCombatComponent.h/.cpp
4. (선택) yjh_ArenaCombatTypes.h

## 2. 공통 타입(우선 고정)
- FYJHCombatSessionId
- EYJHCombatEndReason
- FYJHCombatResult
- FYJHCombatSpawnContext

## 3. 필수 공개 함수(초기 시그니처)
### yjh_ArenaCombatantBase
- BeginCombatSession(CombatSessionId, SpawnContext)
- EndCombatSession(CombatSessionId, EndReason)
- SetCombatEnabled(CombatSessionId, bEnabled)
- GetCombatantId()

### yjh_ArenaHealthComponent
- InitializeHealth(MaxHP)
- ApplyServerDamage(CombatSessionId, InstigatorCombatantId, Amount)
- IsDead()
- GetCurrentHP()

### yjh_ArenaCombatComponent
- RequestAttack(TargetCombatantId)
- SetCombatContext(CombatSessionId, OwnerCombatantId)
- CancelPendingAction(EndReason)

## 4. 확장성 가드레일(중요)
- Base 내부에 특정 캐릭터 전용 하드코딩 금지
- 수치/애니메이션/입력은 DataAsset 또는 Blueprint로 위임
- 파생 캐릭터는 신규 C++ 작성 없이 생성 가능한 경로 유지

## 5. 완료 기준
- 대상 파일 모두 컴파일 성공
- 로그로 함수 호출 경로 확인 가능
- 빈 구현이어도 계약 함수가 연결되어 호출 가능
- 파생 캐릭터 샘플 1종을 Blueprint로 생성 가능

## 6. 작업 순서
1. 타입/열거형 선언
2. 클래스 선언 및 UPROPERTY/UFUNCTION 뼈대
3. 컴포넌트 부착 및 기본 로그
4. 컴파일 확인
5. 샘플 파생 캐릭터 생성 경로 검증

## 7. 보류 항목(다음 대화에서 결정)
- 데미지 공식
- 히트 판정 방식
- 입력 버퍼/콤보 규칙
- 애니메이션 노티파이 연동

## 8. 하나씩 진행 순서(현재 진행중)
1. 변경 규칙 고정 (완료)
- 기준 문서: yjh_change_policy.md
- 변경 등급(A/B/C)과 파괴적 변경 절차를 먼저 적용

2. 책임 경계 테스트 고정 (완료)
- 기준 문서: yjh_boundary_test_spec.md
- 게임모드/캐릭터 경계 위반 검출 테스트(TC-BND-01~05) 고정

3. 확장성 인수인계 기준 고정 (완료)
- 기준 문서: yjh_extensibility_handoff_spec.md
- 파생 캐릭터 제작자 중심 확장 포인트/BP-DataAsset 인수인계 기준 고정

4. 디버그 관측 포인트 고정 (완료, Draft)
- 기준 문서: yjh_debug_observability_spec.md
- 로그 키/필드/오버레이/진단 순서를 계획 단계 가변 초안으로 고정

5. 최소 수직 슬라이스 확정 (완료, Draft)
- 기준 문서: yjh_min_vertical_slice_spec.md
- "전투 진입 -> 피해 -> 사망 -> FinalizeOnce -> 보드 복귀" 최소 루프 기준 고정

6. 보류 항목 결정 순서 확정 (완료, Draft)
- 기준 문서: yjh_deferred_decision_order.md
- 데미지 -> 판정 -> 노티파이 -> 콤보 순서와 진입 조건 고정
