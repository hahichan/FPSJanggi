# YJH 작업 로그 (4)

## 2026-07-17

### 작업 번호
- 33번째 작업

### 진행 범위
- 전투 시스템 + 베이스 캐릭터 + 스킬 시스템 C++ 최소 구현 및 타깃 빌드 검증

### 수행 작업
1. 공통 전투/스킬 타입 구현
- yjh_ArenaCombatSkillTypes.h 추가
- Team/Piece/ExecType/PassiveType/FailCode enum 추가
- SkillExecutionResult, RuntimeSkillRequestContext, SkillDefinition 구조체 추가

2. 스킬 데이터자산 구현
- yjh_SkillDataAsset.h/.cpp 추가
- SkillData 검증 및 슬롯/스킬 맵 생성(BuildSkillMaps) 구현
- MaxSkillSlots 설정값 기반 슬롯 운용, Cooldown 범위(0~999) 검증 반영

3. 베이스 체력 시스템 구현
- yjh_ArenaHealthComponent.h/.cpp 추가
- 서버 권한 HP 처리(피해/회복), 복제, OnDamaged/OnDead 이벤트 구현

4. 전투/스킬 컴포넌트 구현
- yjh_ArenaCombatComponent.h/.cpp 추가
- 단일 컴포넌트 내부에서 SkillManager(보유/매핑/쿨타임) + SkillExecutor(검증/실행) 책임 분리
- 10슬롯 초기화/매핑, RequestSkillBySlot, 쿨타임 조회 구현
- 6개 실행기(HitScan/Projectile/Dash/CooldownControl/Teleport/Passive) 처리 경로 구현
- Teleport 실패 시 SKX_TeleportNoValidDestination 반환, 쿨타임 [0,999] 클램프 구현

5. 베이스 캐릭터 구현
- yjh_ArenaCombatantBase.h/.cpp 추가
- Health/Combat 컴포넌트 조립, Begin/EndCombatSession, TriggerSkillBySlot 구현
- BP 연출 훅(OnDamagedVisualOnly/OnDeadVisualOnly) 연결

6. 빌드 검증(정책 준수)
- FPSJanggiEditor Win64 Development 빌드 성공
- FPSJanggi Win64 Development 빌드 성공

### 현재 상태
- 문서에서 고정한 파이프라인 기준(서버 권한/하드코딩 금지/BP 수정 용이성)에 맞는 최소 구현 뼈대가 준비되었고, 두 타깃 빌드까지 통과한 상태

## 2026-07-17

### 작업 번호
- 34번째 작업

### 진행 범위
- 베이스 캐릭터 기본 이동 기능 구현(전후/좌우/시점/이동속도) + 빌드 검증

### 수행 작업
1. 기본 이동 API 추가
- yjh_ArenaCombatantBase.h에 이동 함수 추가
	- MoveForward(float), MoveRight(float)
	- TurnYaw(float), LookPitch(float)
	- SetWalkSpeed(float), SetMovementEnabled(bool)

2. 이동 파라미터/상태값 추가
- BaseWalkSpeed, TurnRateScale, LookRateScale 편집 가능 값 추가
- bMovementEnabled 상태값 추가

3. 이동 로직 구현
- yjh_ArenaCombatantBase.cpp에 컨트롤러 기준 이동벡터 계산 및 AddMovementInput 적용
- 사망/이동비활성 상태에서 입력 차단
- BeginPlay에서 MaxWalkSpeed 적용
- 전투 종료/사망 시 이동 비활성화 및 즉시 정지 처리

4. 검증
- FPSJanggiEditor Win64 Development 빌드 성공
- FPSJanggi Win64 Development 빌드 성공

### 현재 상태
- 기존 전투/스킬/체력 시스템 위에 베이스 이동까지 포함된 최소 플레이 가능 캐릭터 베이스가 준비된 상태

## 2026-07-17

### 작업 번호
- 35번째 작업

### 진행 범위
- ArenaBattleReal 상태에서 실제 1대1 전투 루프가 닫히도록 전투 캐릭터-보드 통합 확장

### 수행 작업
1. 베이스 캐릭터 전투 입력/공격 루프 확장
- yjh_ArenaCombatantBase에 SetupPlayerInputComponent 추가
- 기본 입력 바인딩 추가: MoveForward/MoveRight/Turn/LookUp/PrimaryAction
- PrimaryAction 서버 RPC 추가(ServerPrimaryAction)
- 서버 권한 근접 공격 판정(Overlap) + 데미지 적용 구현
- 공격 쿨타임/사망 상태/전투 활성 상태 검증 추가

2. 죽음 -> 보드 승패 확정 경로 연결
- 캐릭터 사망 시 Owner Board(AAuthoritativeJanggiBoard)에 ReportArenaCombatantDefeated(this) 호출

3. 보드 ArenaBattle 규칙 확장(문서 반영)
- ActiveCombatSessionId 생성/관리 추가
- FinalizeOnce 보호(bArenaFinalizeRequested) 추가
- 동일 틱 사망 집계(PendingDefeatedTeams) 후 다음 틱 일괄 확정 추가
- 양측 사망 시 Draw 처리 및 양측 말 제거(BothEliminated) 구현
- 타임아웃 만료 시 Draw 처리로 변경
- Arena 기본 제한시간 60초로 조정, 말별 오버라이드(PieceArenaTimeoutOverrideSeconds) 추가

4. 자식 캐릭터 클래스 선언 추가
- yjh_ArenaCombatantTemplate_Blue/Red 클래스 추가
- 팀/이동/기본공격 초기값 프리셋 제공

5. 보드 기본 클래스 매핑 자동화
- AuthoritativeJanggiBoard 생성자에서 Blue/Red 각 말 타입에 대해 템플릿 클래스 기본 매핑
- 별도 맵 설정이 비어 있어도 ArenaBattle 시 기본 전투 캐릭터로 스폰 가능

6. 입력 설정 보강
- DefaultInput.ini에 기본 매핑 추가
	- Axis: MoveForward/MoveRight/Turn/LookUp
	- Action: PrimaryAction(LeftMouseButton)

7. 빌드 검증
- FPSJanggiEditor Win64 Development 빌드 성공
- FPSJanggi Win64 Development 빌드 성공

### 현재 상태
- 보드 전투 진입 후 전투 캐릭터가 스폰/빙의되고 이동+기본공격으로 HP를 깎아 사망 보고를 올리면, 보드 승패 확정/드로우/복귀까지 서버 권한 루프가 동작 가능한 상태

## 2026-07-17

### 작업 번호
- 36번째 작업

### 진행 범위
- 임시 요구사항 반영: ArenaBattleReal에서 Blue=test_a, Red=test_b 강제 소환

### 수행 작업
1. 임시 테스트 전투 캐릭터 클래스 추가
- yjh_ArenaCombatantTestA.h/.cpp 추가 (CombatantId/PieceInstanceId = test_a)
- yjh_ArenaCombatantTestB.h/.cpp 추가 (CombatantId/PieceInstanceId = test_b)

2. 보드 임시 스폰 오버라이드 추가
- AuthoritativeJanggiBoard에 임시 스폰 설정 추가
	- bUseTemporaryTestArenaCombatants (기본 true)
	- TemporaryBlueCombatantClass
	- TemporaryRedCombatantClass
- 기본값으로 TestA/TestB 클래스 자동 설정
- ArenaBattle 진입 시 임시 클래스 우선 스폰 후 빙의
- 임시 스폰 실패 시 기존 기물 기반 스폰 경로로 자동 폴백

3. 빌드 검증
- FPSJanggiEditor Win64 Development 빌드 성공
- FPSJanggi Win64 Development 빌드 성공

### 현재 상태
- 요청한 임시 동작 기준으로 ArenaBattleReal에서 양 진영이 test_a/test_b로 스폰되며, 이후 이전 상태머신의 장기 정보 연동 로직으로 교체하기 쉬운 구조로 준비됨

## 2026-07-17

### 작업 번호
- 37번째 작업

### 진행 범위
- 미구현 보강: 스킬 슬롯 1~10 입력 라우팅 + 전투 세션 시작 시 HP 초기화

### 수행 작업
1. 캐릭터 스킬 슬롯 입력 라우팅 구현
- yjh_ArenaCombatantBase에 TriggerSkillSlotByIndex(int32) 추가
- SetupPlayerInputComponent에서 SkillSlot1~SkillSlot10 액션 바인딩 추가
- 내부 SlotName 생성(BuildSlotName) + 슬롯별 래퍼 함수(1~10) 추가

2. 전투 세션 시작 시 체력 초기화 구현
- BeginCombatSession에서 서버 권한 기준 HealthComponent.InitializeHealth(MaxHP) 호출
- MaxHP 편집 파라미터 추가

3. 입력 설정 확장
- DefaultInput.ini에 SkillSlot1~SkillSlot10 액션 매핑 추가
	- 1~0 키로 슬롯 입력 가능

4. 빌드 검증
- FPSJanggiEditor Win64 Development 빌드 성공
- FPSJanggi Win64 Development 빌드 성공

### 현재 상태
- 이동/기본공격 + 스킬 슬롯(1~10) 입력 + 세션 HP 초기화까지 포함된 전투 플레이 루프가 C++ 기준으로 동작 가능한 상태

## 2026-07-17

### 작업 번호
- 38번째 작업

### 진행 범위
- 임시 test_a/test_b 경로에도 이전 상태머신 기물 문맥(piece-binding context) 전달

### 수행 작업
1. 임시 스폰 캐릭터 문맥 주입 강화
- SpawnTemporaryArenaCombatant에서 AYJHArenaCombatantBase 캐스팅 후 문맥 적용
	- TeamInfo
	- PieceType(보드 기물 -> 전투 기물 타입 매핑)
	- CombatantId(없을 때 팀/기물 기반 자동 생성)
	- PieceInstanceId(Origin/Destination 기반 생성)

2. 빌드 검증
- FPSJanggiEditor Win64 Development 빌드 성공
- FPSJanggi Win64 Development 빌드 성공

### 현재 상태
- 현재 임시 test_a/test_b 스폰을 유지하면서도, 이전 상태머신에서 넘어온 기물 정보가 전투 캐릭터 데이터에 전달되는 경로까지 확보됨

## 2026-07-17

### 작업 번호
- 39번째 작업

### 진행 범위
- 맵 태그 기반 아레나 랜덤 스폰(양측 중복 위치 금지) 구현

### 수행 작업
1. 태그 스폰 설정 추가
- AuthoritativeJanggiBoard에 배치 옵션 추가
	- bUseTaggedArenaSpawnPoints (기본 true)
	- ArenaSpawnPointTag (기본 YJH_ArenaSpawn)

2. 스폰 위치 선택 로직 추가
- PrepareArenaSpawnPair() 구현
- 태그 액터 목록에서 Blue/Red 스폰 지점 2개를 랜덤 선발
- 동일 액터/동일 위치 선발 방지(중복 금지)
- 포인트가 부족하면 기존 오프셋 스폰으로 안전 폴백

3. 스폰/카메라 연동
- GetArenaFighterWorldLocation()가 선택된 스폰 쌍을 우선 사용
- SpawnArenaCombatants()에서 전투 시작 시 스폰 쌍을 확정
- DestroyArenaCombatants()에서 스폰 쌍 캐시 초기화

4. 빌드 검증
- FPSJanggiEditor Win64 Development 빌드 성공
- FPSJanggi Win64 Development 빌드 성공

### 현재 상태
- 맵에 같은 태그를 가진 스폰 포인트를 2개 이상 두면, 매 전투마다 양측이 서로 다른 지점에서 랜덤 소환됨

## 2026-07-17

### 작업 번호
- 40번째 작업

### 진행 범위
- ArenaBattleReal GameMode 오케스트레이션 보강 + YJHStart 버튼 스텁 정리

### 수행 작업
1. GameMode 세션 모니터 추가
- BoardGameMode에 ArenaBattleReal 모니터 루프 추가(0.25s)
- PostLogin/Logout 기준으로 팀 이탈 상태 관리
- ArenaDisconnectGraceSeconds(기본 8초) 추가

2. 강제 중단(ForcedAbort) 구현
- Grace 만료 시 GameMode에서 Board->AbortArenaBattleForced("DisconnectGraceExpired") 호출
- Board에 AbortArenaBattleForced API 추가
- ForcedAbort 로그 기록 후 Draw(BothEliminated 경로)로 종료

3. YJHStart 버튼 동작 정리
- 기존 1초 자동복귀 스텁 제거
- YJHStart 시 ArenaBattleReal 상태 유지(자동 Return 타이머 미설정)

4. 빌드 검증
- FPSJanggiEditor Win64 Development 빌드 성공
- FPSJanggi Win64 Development 빌드 성공

### 현재 상태
- ArenaBattleReal에서 서버 권한 전투 루프 + Draw/Timeout/ForcedAbort(DisconnectGraceExpired) 종료 경로까지 동작하는 상태

## 2026-07-17

### 작업 번호
- 32번째 작업

### 진행 범위
- 구현 직전 최종 의사결정(슬롯/쿨타임/텔레포트/UI/권한/BP 인터페이스) 문서 고정

### 수행 작업
1. 실행기 정책 확정
- 단일 컴포넌트 구조 + 내부 SkillManager/SkillExecutor 책임 분리 고정
- 6개 실행기 한 번에 구현 방침 고정
- 쿨타임 상한 999.0 및 오버플로 가드 실패코드 추가
- 텔레포트 유효 지점 없음 시 취소 + SKX_TeleportNoValidDestination 반환 규칙 추가

2. 데이터/컨텍스트 규칙 확정
- SkillData 슬롯/쿨타임 정책에 MaxSkillSlots 설정값 및 0~999 범위 고정
- Runtime Request Context 구조체 강제 사용 규칙 보강

3. UI/BP 규칙 확정
- 패시브 슬롯도 항상 표시, Passive 배지 표시, 쿨타임 숫자 숨김으로 고정
- BP 노드/시그니처 v1 고정 인터페이스 정책 명시
- 서버 권한 최종 확정(클라 요청/서버 확정) 명시

4. 확장성 규칙 보강
- 스탯 수치가 DataAsset/BP에서 언제든 수정 가능해야 한다는 원칙 명시

### 현재 상태
- 구현 전 결정사항이 문서에 고정되어, 다음 구현 단계에서 해석 차이 없이 바로 작업 가능한 상태가 됨
