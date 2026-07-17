# YJH 작업 로그 (2)

## 2026-07-17

### 작업 번호
- 1번째 작업

### 진행 범위
- 향후 YJH 구현 작업의 운영 규칙 확정 및 기록 체계 시작

### 수행 작업
1. 참고 문서 기준 확정
- `yjh_chat_1.md`, `yjh_plan.md`를 기준 문서로 사용

2. 파일 네이밍 규칙 확정
- 신규 생성 파일은 `yjh_` 접두어 사용

3. 변경 이력 기록 규칙 확정
- 새로운 코드 작성 또는 의미 있는 변경 발생 시 `yjh_chat_2.md`에 기록

### 현재 상태
- 다음 구현 작업부터 본 문서에 변경 이력을 연속 기록

## 2026-07-17

### 작업 번호
- 2번째 작업

### 진행 범위
- 바이브 코딩 기준에 맞춰 기존 단일 계획서를 실행형 초안으로 재정리

### 수행 작업
1. 계획서 분리 전략 반영
- 게임 흐름 계획서 / 베이스 캐릭터 계획서 / 통합 접점 시트 3분리 원칙 명시

2. 아키텍처 방향 명확화
- 다중 상속 중심이 아닌 컴포지션 중심 구조로 고정
- 컴포넌트 책임(Health/Combat/State) 분배안 반영

3. 실행 단위 재구성
- Step A~E로 작업 순서를 재정의
- 각 Step별 완료 기준/검증 기준/롤백 기준 추가

### 현재 상태
- yjh_plan.md가 구현 전 의사결정과 진행 체크에 바로 사용할 수 있는 초안 상태로 갱신됨

## 2026-07-17

### 작업 번호
- 3번째 작업

### 진행 범위
- 계획서 분리 실행 및 실제 개발 준비물 문서화

### 수행 작업
1. 게임모드 전용 계획서 생성
- yjh_plan_gamemode.md 추가

2. 베이스 캐릭터 전용 계획서 생성
- yjh_plan_base_character.md 추가

3. 통합 접점 시트 생성
- yjh_plan_integration_contract.md 추가

4. 실제 개발 준비물 체크리스트 생성
- yjh_dev_execution_checklist.md 추가

### 현재 상태
- 총괄 계획서(yjh_plan.md) + 분리 계획서 2종 + 통합 계약 시트 + 실행 체크리스트 체계가 준비됨

## 2026-07-17

### 작업 번호
- 4번째 작업

### 진행 범위
- 분리 문서 3종의 용어 통일 + 확장성 기준 명시 + 코드 스켈레톤 단계 문서 추가

### 수행 작업
1. 통합 접점 계약 고정
- MatchId/PlayerId 용어를 CombatSessionId/CombatantId로 통일
- 게임모드<->캐릭터 계약 함수명 정렬

2. 게임모드/베이스캐릭터 계획서 완료 기준 동기화
- FinalizeOnce 기준과 종료 사유(EndReason) 용어 동기화

3. 확장성 최우선 원칙 명시
- 파생 캐릭터 제작자가 C++ 수정 없이 작업 가능한 구조를 핵심 요구사항으로 추가

4. 코드 스켈레톤 단계 문서 생성
- yjh_plan_code_skeleton.md 추가

### 현재 상태
- 요청한 1/2/3 항목(계약 확정, 용어 통일, 스켈레톤 단계 진입)이 문서 기준으로 완료됨

## 2026-07-17

### 작업 번호
- 5번째 작업

### 진행 범위
- "하나씩 진행" 기준의 1단계(변경 규칙 고정) 수행

### 수행 작업
1. 변경 정책 문서 생성
- yjh_change_policy.md 추가
- 변경 등급(A/B/C)과 파괴적 변경 절차 정의

2. 스켈레톤 계획서에 순차 진행 섹션 추가
- 1단계 완료, 2~6단계 대기 상태로 명시

### 현재 상태
- 큰 틀 고정 이후 첫 실행 항목(변경 규칙 고정)이 완료되어, 다음 단계(책임 경계 테스트 고정)로 진행 가능

## 2026-07-17

### 작업 번호
- 6번째 작업

### 진행 범위
- "하나씩 진행" 기준의 2단계(책임 경계 테스트 고정) 수행

### 수행 작업
1. 책임 경계 테스트 명세서 생성
- yjh_boundary_test_spec.md 추가
- TC-BND-01~05로 게임모드/캐릭터 책임 경계 검증 항목 확정

2. 스켈레톤 계획서 단계 상태 갱신
- 2단계를 완료로 변경하고 기준 문서 연결

3. 실행 체크리스트 연동
- 테스트 작업물에 yjh_boundary_test_spec.md 추가

### 현재 상태
- 1단계(변경 규칙) + 2단계(책임 경계 테스트)가 완료되어, 다음 단계(확장성 인수인계 기준 고정) 진행 가능

## 2026-07-17

### 작업 번호
- 7번째 작업

### 진행 범위
- "하나씩 진행" 기준의 3단계(확장성 인수인계 기준 고정) 수행

### 수행 작업
1. 확장성 인수인계 기준서 생성
- yjh_extensibility_handoff_spec.md 추가
- 파생 캐릭터 제작자 중심의 BP/DataAsset 확장 지점과 DoD 확정

2. 스켈레톤 계획서 단계 상태 갱신
- 3단계를 완료로 변경하고 기준 문서 연결

3. 실행 체크리스트 연동
- 확장성/인수인계 항목에 기준 문서 연결 추가

### 현재 상태
- 1~3단계(변경 규칙, 책임 경계 테스트, 확장성 인수인계 기준)가 완료되어, 다음 단계(디버그 관측 포인트 고정) 진행 가능

## 2026-07-17

### 작업 번호
- 8번째 작업

### 진행 범위
- "하나씩 진행" 기준의 4단계(디버그 관측 포인트 고정) 수행
- 계획 단계 문서는 가변 가능하다는 운영 원칙 명시

### 수행 작업
1. 디버그 관측 포인트 기준서 생성
- yjh_debug_observability_spec.md 추가
- 로그 키/필드/콘솔 명령/오버레이/진단 순서 초안 고정

2. 스켈레톤 계획서 단계 상태 갱신
- 4단계를 완료(Draft)로 변경

3. 실행 체크리스트 연동
- 디버그 작업물에 기준 문서 연결 추가

4. 변경 정책 보강
- 계획 단계 문서 가변성 규칙 추가

### 현재 상태
- 1~4단계가 완료되었고, 다음 단계(최소 수직 슬라이스 확정)로 진행 가능

## 2026-07-17

### 작업 번호
- 9번째 작업

### 진행 범위
- "하나씩 진행" 기준의 5단계(최소 수직 슬라이스 확정) 수행

### 수행 작업
1. 최소 수직 슬라이스 명세서 생성
- yjh_min_vertical_slice_spec.md 추가
- 전투 진입 -> 피해 -> 사망 -> FinalizeOnce -> 보드 복귀 최소 루프를 Draft로 고정

2. 스켈레톤 계획서 단계 상태 갱신
- 5단계를 완료(Draft)로 변경

3. 실행 체크리스트 연동
- 테스트 작업물에 최소 수직 슬라이스 명세서 연결 추가

### 현재 상태
- 1~5단계가 완료되어, 다음 단계(보류 항목 결정 순서 확정) 진행 가능

## 2026-07-17

### 작업 번호
- 10번째 작업

### 진행 범위
- "하나씩 진행" 기준의 6단계(보류 항목 결정 순서 확정) 수행

### 수행 작업
1. 보류 항목 결정 순서표 생성
- yjh_deferred_decision_order.md 추가
- 결정 순서: 데미지 -> 히트 판정 -> 노티파이 -> 입력 버퍼/콤보

2. 스켈레톤 계획서 단계 상태 갱신
- 6단계를 완료(Draft)로 변경

3. 실행 체크리스트 연동
- 보류 항목 관리 섹션 추가 및 기준 문서 연결

### 현재 상태
- 1~6단계(변경 규칙, 책임 경계, 확장성 인수인계, 디버그 관측, 최소 수직 슬라이스, 보류 항목 순서)가 계획 단계 Draft 기준으로 완료됨

## 2026-07-17

### 작업 번호
- 11번째 작업

### 진행 범위
- 사용자 확정 규칙(비김/타임아웃/시작 데이터)을 게임모드/통합계약/수직슬라이스 문서에 반영

### 수행 작업
1. 승패/비김 규칙 반영
- Draw 규칙 추가: 동시 HP 0, 타임아웃
- Draw 처리 정책: 양측 제거(BothEliminated)

2. 시작 데이터 규칙 반영
- ArenaBattle -> ArenaBattleReal 전이 시 TeamInfo + PieceInfo 전달 규칙 추가
- 전이 실패 사유별 Abort 규칙 추가

3. 테스트/완료 기준 보강
- 최소 수직 슬라이스에 Draw/Timeout 검증 항목 추가

### 현재 상태
- 종료 결과 타입과 전이 입력 데이터가 문서 계약으로 고정되어, 이후 코드 단계에서 충돌 가능성이 크게 감소함

## 2026-07-17

### 작업 번호
- 12번째 작업

### 진행 범위
- 동시 판정 규칙을 "같은 서버 틱 데미지 배치"로 명확화하고 타임아웃 설정 가변 정책 반영

### 수행 작업
1. 게임모드 규칙 명확화
- Draw 판정을 "같은 서버 틱의 동일 데미지 배치 처리 후 양측 HP 0"으로 명시
- TimeoutSeconds를 설정값으로 조절 가능하도록 정책 추가

2. 통합 계약 보강
- DamageBatch, TimeoutSeconds 공통 용어 추가
- Draw 판정 기준을 계약 레벨로 명시

3. 수직 슬라이스 검증 보강
- 동시 사망 기준을 같은 서버 틱 배치 처리로 고정
- TimeoutSeconds 변경 반영 검증 항목 추가

### 현재 상태
- 동시사망/비김 판정 기준이 네트워크 환경에서도 일관되게 적용되도록 문서 기준이 강화됨

## 2026-07-17

### 작업 번호
- 13번째 작업

### 진행 범위
- 타임아웃 기본값 60초 확정 및 캐릭터/말별 오버라이드 가능 정책 반영

### 수행 작업
1. 게임모드 타임아웃 정책 갱신
- 기본값 60초 확정
- PieceTimeoutOverrideSeconds 기반 오버라이드 허용 정책 추가

2. 통합 계약 보강
- TimeoutDefaultSeconds(60) 및 PieceTimeoutOverrideSeconds(optional) 필드 추가
- SpawnContext 최소 필드에 타임아웃 관련 필드 추가

3. 수직 슬라이스/확장성 문서 동기화
- 기본 60초 테스트 + 오버라이드 테스트 항목 추가
- 베이스 캐릭터 확장 규칙에 타임아웃 오버라이드 노출 요구 추가

### 현재 상태
- 타임아웃이 기본 60초로 동작하면서도 향후 캐릭터/말별로 쉽게 조절 가능한 문서 계약이 고정됨

## 2026-07-17

### 작업 번호
- 14번째 작업

### 진행 범위
- 게임모드 보강 요청(종료 우선순위/세션 생명주기/이탈 유예/관측지표/매치버전/테스트매트릭스/가드레일) 초안 문서화

### 수행 작업
1. 신규 초안 문서 생성
- yjh_termination_priority_spec.md
- yjh_session_lifecycle_spec.md
- yjh_disconnect_grace_policy.md
- yjh_observability_metrics_draft.md
- yjh_test_matrix.md
- yjh_content_guardrail_spec.md
- yjh_match_rule_version_spec.md

2. 핵심 계획서 연동
- 게임모드 계획서에 종료 우선순위, 서버시계 기준, 이탈 유예, 매치 버전, 지표/매트릭스 연결
- 통합 계약 시트에 GraceSeconds/MatchRuleVersion 및 ForcedAbort 우선 규칙 반영
- 베이스 캐릭터 계획서에 콘텐츠 가드레일 기준 연결

3. 질문 항목 반영
- 강제중단 우선순위를 최상위로 고정
- 세션 시간 기준은 서버 시계 기준으로 고정
- 이탈 정책은 유예 후 ForcedAbort 후보 처리로 반영
- 스폰 실패는 1회 자동 재시도 후 Abort로 반영

### 현재 상태
- 게임모드 실무 운영에 필요한 정책/지표/검증 초안이 모두 문서화되어 코드 단계 전 점검이 가능한 상태가 됨

## 2026-07-17

### 작업 번호
- 15번째 작업

### 진행 범위
- 베이스 캐릭터 입력 계층(IA/IMC/SkillData) 역할표 초안 작성

### 수행 작업
1. 입력 계층 역할표 문서 생성
- yjh_input_role_matrix.md 추가
- IA/IMC/SkillData/Base/Combat/Health 책임 경계 및 금지 항목 정의

2. 베이스 캐릭터 계획서 연동
- 확장성 규칙 섹션에 입력 계층 역할표 기준 문서 연결

### 현재 상태
- 타 개발자가 IMC/IA/SkillData 중심으로 확장할 수 있는 입력 구조 초안이 확정됨

## 2026-07-17

### 작업 번호
- 16번째 작업

### 진행 범위
- 요청한 순서(베이스캐릭터 대화 -> 스킬데이터 포맷 -> 스킬 실행기)에 맞춘 문서 초안 세트 생성

### 수행 작업
1. 베이스캐릭터 Phase 2 계획서 생성
- yjh_plan_base_character_phase2.md 추가

2. 스킬데이터 포맷 명세서 생성
- yjh_skill_data_format_spec.md 추가

3. 스킬 실행기 명세서 생성
- yjh_skill_executor_spec.md 추가

4. 베이스캐릭터 계획서 연계 섹션 추가
- 연계 설계 문서(Phase2/SkillData/Executor) 연결

### 현재 상태
- 베이스캐릭터 상세 대화 후 바로 문서 확정으로 넘어갈 수 있는 뼈대가 준비됨

## 2026-07-17

### 작업 번호
- 17번째 작업

### 진행 범위
- 사용자 요구(14개 기물 식별, N개 스킬 슬롯, 카메라/팔메쉬, 확장 스탯/런타임) 기반 베이스캐릭터/스킬데이터 문서 보강

### 수행 작업
1. 베이스캐릭터 계획서 보강
- TeamInfo/PieceType/PieceInstanceId 기반 기물 식별 체계 추가
- 필수 구성 항목(Identity/Stats/Runtime/Input/Camera/Components) 섹션 추가
- 입력 슬롯 N개 확장 가이드 및 BP 변경 가능 정책 추가
- 전투 카메라/1인칭 팔메쉬 슬롯 가이드 추가

2. 스킬데이터 포맷 보강
- IA_SkillN -> SlotN 매핑 규칙 추가
- 슬롯 중복 매핑 금지 검증 규칙 추가

### 현재 상태
- 베이스캐릭터 요구사항 초안이 사용자 의도(장기 14기물 구분, N스킬 확장, BP 주도 확장)에 맞게 강화됨

## 2026-07-17

### 작업 번호
- 18번째 작업

### 진행 범위
- 사용자 확정 사항(스탯 구성, 쿨타임 위치, 10슬롯, 기본입력, 패시브, BP 확장)을 베이스/스킬데이터 문서에 반영

### 수행 작업
1. 스탯/런타임 항목 보강
- MaxHP/CurrentHP/MoveSpeed/RegenPerSecond/Defense/StatusResistance 기본 스탯 반영
- 추가 권장 스탯 목록 제안 추가

2. 쿨타임 관리 위치 고정
- SkillData는 CooldownSec 기준값만 보관
- 실제 쿨타임 상태는 CombatComponent 런타임에서 서버 권한으로 관리

3. 입력/스킬 슬롯 정책 반영
- 기본 이동 WASD, 점프 Space 반영
- 스킬 슬롯 v0를 10개(Slot1..Slot10)로 반영

4. 패시브/BP 확장 반영
- 기본 재생 패시브, 벽타기 패시브 허용 정책 추가
- Mesh/VFX/SFX/사운드 훅의 BP 교체 가능 정책 추가

### 현재 상태
- 베이스캐릭터와 스킬데이터 포맷이 실제 제작 흐름(타개발자 BP 중심 확장)에 맞는 수준으로 구체화됨

## 2026-07-17

### 작업 번호
- 19번째 작업

### 진행 범위
- Base/Runtime 스탯 필드표 + PassiveType 초안 + 동시 입력 우선순위 규칙 추가

### 수행 작업
1. 베이스캐릭터 스탯 문서 보강
- JumpPower/Acceleration/HitboxRadius/HitboxHalfHeight/CharacterScale 추가
- BaseStats/RuntimeStats 필드표 추가

2. 스킬데이터 포맷 보강
- Regen/WallRun을 패시브 스킬로 명시
- PassiveType v0 초안(None/Regen/WallRun/AutoShield/MoveSpeedAura) 추가

3. 입력 우선순위 정책 보강
- 입력 역할표에 우선순위(P0~P3) 및 동시 입력 처리 원칙 추가
- 스킬 실행기 명세에 슬롯 큐(선입력 우선) 규칙 추가

### 현재 상태
- 스탯 구조, 패시브 타입, 동시 입력 처리 기준이 문서 계약으로 고정되어 다음 단계(구체 값/검증 로직 확정)로 진행 가능

## 2026-07-17

### 작업 번호
- 20번째 작업

### 진행 범위
- 전투 HUD 요구사항(우측하단 HP/MaxHP/Speed + 스킬 쿨타임)을 문서 계약으로 고정

### 수행 작업
1. HUD 명세서 생성
- yjh_ui_hud_spec.md 추가
- 우측하단 Status Panel(HP/MaxHP/Speed) + Skill Cooldown Panel(Slot1..Slot10) 정의

2. 쿨타임 미연동 단계 대응 정책 추가
- 스킬 구동기 미완성 상태에서도 Placeholder("Ready"/"-") 표시 규칙 정의

3. 연동 문서 업데이트
- 베이스캐릭터 계획서 연계 문서에 HUD 명세 추가
- 스킬데이터 포맷에 UI 키(SkillIconKey/CooldownDisplayName) 및 HUD 쿨타임 규칙 추가
- 개발 체크리스트 디버그 작업물에 HUD 명세 연결

### 현재 상태
- UI 구현 전 단계에서도 표시 구조와 데이터 바인딩 기준이 고정되어 추후 구현/변경이 쉬운 상태가 됨

## 2026-07-17

### 작업 번호
- 21번째 작업

### 진행 범위
- 데이터 바인딩 계약 전체 명시 + 신규 데이터 발생 시 별도 파일 관리 규칙 고정

### 수행 작업
1. 마스터 계약서 생성
- yjh_ui_data_binding_contract.md 추가
- 데이터 키/한국어 설명/원천/이벤트/Blueprint 사용 방식/변경 방식 명시

2. Blueprint 호출/변경 방식 문서화
- 위젯 바인딩 순서(Construct->Bind->Snapshot->Update->Destruct Unbind) 명시
- 이벤트별 권장 노드 패턴 및 호출 주체 분리 원칙 명시

3. 신규 데이터 Delta 관리 체계 추가
- yjh_ui_binding_delta_template.md 추가
- 파일명 규칙(yjh_ui_binding_delta_YYYYMMDD_NN)과 호환성 등급(A/B/C) 고정

4. 연동 문서 갱신
- HUD 명세서, 베이스캐릭터 계획서, 실행 체크리스트에 신규 계약 문서 연결

### 현재 상태
- UI 데이터 추가/변경 시 "별도 Delta 파일 생성 -> Master 반영" 흐름이 고정되어 추후 협업 충돌을 줄일 수 있는 상태가 됨

## 2026-07-17

### 작업 번호
- 22번째 작업

### 진행 범위
- 스킬 구동기/스킬데이터를 v1 기준으로 보강하고, 초기 스킬 세트(10종)를 확정

### 수행 작업
1. 스킬데이터 포맷 v1 보강
- yjh_skill_data_format_spec.md를 v1 Draft로 갱신
- 필수/선택 필드 표준표, 추가 검증 규칙, Validation 실패코드, 샘플 세트 추가

2. 스킬 실행기 v1 보강
- yjh_skill_executor_spec.md를 v1 Draft로 갱신
- 런타임 파이프라인(입력->검증->실행->쿨타임->이벤트), 실패코드, 로그 최소 필드 추가
- 초기 구현 우선순위(Damage->Dash->Projectile->Buff->Passive) 명시

3. 초기 스킬 카탈로그 신규 생성
- yjh_skill_initial_catalog.md 추가
- Slot1..Slot10 기준 초기 액티브 10종 + 패시브 2종 후보 확정

### 현재 상태
- 이제 구현 단계에서 "무엇을 먼저 만들지"가 명확해졌고, 스킬데이터와 구동기 계약이 같은 버전 기준(v1)으로 정렬된 상태가 됨

## 2026-07-17

### 작업 번호
- 23번째 작업

### 진행 범위
- 스킬 구동기 최소 코드 스켈레톤 작성(데이터자산/실행기 컴포넌트/공통 타입)

### 수행 작업
1. 공통 타입 파일 생성
- yjh_ArenaSkillTypes.h 추가
- ExecType/PassiveType/FailCode enum 및 SkillData/ExecutionResult 구조체 작성

2. 스킬 데이터자산 생성
- yjh_ArenaSkillDataAsset.h/.cpp 추가
- Skill 배열 기반 BuildSkillMaps 검증 함수 구현
- 중복 SkillId, 슬롯 중복, 음수 쿨타임/사거리 등 기본 Validation 반영

3. 스킬 구동기 컴포넌트 생성
- yjh_ArenaSkillExecutorComponent.h/.cpp 추가
- InitializeSkillMap / RequestSkillBySlot / GetRemainingCooldown / SetCombatContext 구현
- ExecuteByType(Damage/Dash/Buff/Projectile/Passive) 로그 기반 스텁 구현
- OnSkillSlotMapped / OnCooldownChanged / OnSkillRequestFailed 델리게이트 노출

### 현재 상태
- Blueprint에서 DataAsset을 연결해 슬롯 매핑/요청/쿨타임 조회를 호출할 수 있는 최소 실행 뼈대가 준비됨

## 2026-07-17

### 작업 번호
- 24번째 작업

### 진행 범위
- 구현 중단 후 바이브코딩 사전 계획 단계로 전환

### 수행 작업
1. 바이브코딩 사전 계획서 생성
- yjh_vibe_coding_preflight_plan.md 추가
- 막힘 정의, 복구 프로토콜, 게이트(A/B/C), 시작 승인 조건 문서화

2. 총괄 계획서 연동
- yjh_plan.md에 "사전 계획 우선 규칙" 섹션 추가
- 게이트 통과 전 코드 작성 금지 원칙 명시

3. 실행 체크리스트 연동
- yjh_dev_execution_checklist.md에 사전 게이트 확인 항목 추가

### 현재 상태
- 지금 작업의 성격이 "바이브코딩 구현 전 계획서 단계"로 고정되었고, 막힘 발생 시 복구 경로까지 문서화된 상태가 됨

## 2026-07-17

### 작업 번호
- 25번째 작업

### 진행 범위
- BP 제작자 친화형 스킬 문서 재정의 + 기본 스킬 구동기 대상 교체

### 수행 작업
1. 스킬 실행기 명세 v2로 갱신
- 기본 실행기 목록을 HitScan/Projectile/Dash/CooldownControl/Teleport/Passive로 재구성
- 쿨다운 음수 방지 클램프, 텔레포트 끼임 방지 폴백 정책 추가
- BP 호출 노드명/이벤트명/타입 표 추가

2. 스킬데이터 포맷 v2로 갱신
- 실행기별 필수 파라미터 표 추가(히트스캔/투사체/돌진/쿨감/순간이동)
- v2 검증 규칙 추가(투사체 필수키, 쿨감 하한 0, 텔레포트 안전 반경 등)

3. 초기 스킬 카탈로그 교체
- 기존 10종 액티브 스킬 목록 폐기
- 기본 구현 대상 5종(SK_HitScan_Burst, SK_Projectile_Lance, SK_Dash_Impact, SK_Cooldown_Sync, SK_Teleport_Shift)으로 교체

4. BP 제작 가이드 신규 생성
- yjh_bp_skill_authoring_guide.md 추가
- 노드명/데이터 타입/한국어 설명/스킬별 입력 체크리스트 명시

### 현재 상태
- 타 개발자가 블루프린트에서 어떤 노드를 어떤 타입으로 연결해야 하는지 바로 참고 가능한 문서 체계가 준비됨

## 2026-07-17

### 작업 번호
- 26번째 작업

### 진행 범위
- 문서 정합성 맞춤 + 계획 단계 유지를 위해 AI 추가 코드 제거

### 수행 작업
1. 문서 정합성 보정
- yjh_plan_base_character.md의 쿨타임 필드명을 CooldownSec으로 통일
- 슬롯 정책을 "구조 10슬롯 유지 + 초기 구현 Slot1..Slot5"로 명시
- yjh_skill_data_format_spec.md의 v2 기준 표기 정리 및 Buff 잔여 규칙 제거
- yjh_skill_executor_spec.md의 파이프라인/실패코드 버전 표기 v2 정리 및 컴포넌트 명명 정책 명시
- yjh_bp_skill_authoring_guide.md의 위젯 초기화 시점을 Construct(OnInitialized)로 보정

2. 코드 제거(요청 반영)
- 아래 AI 추가 신규 코드 파일 5개 삭제
	- yjh_ArenaSkillTypes.h
	- yjh_ArenaSkillDataAsset.h
	- yjh_ArenaSkillDataAsset.cpp
	- yjh_ArenaSkillExecutorComponent.h
	- yjh_ArenaSkillExecutorComponent.cpp

### 현재 상태
- 현재 저장소 상태가 "계획서 중심"으로 재정렬되었고, 코드 구현은 보류된 상태로 유지됨

## 2026-07-17

### 작업 번호
- 27번째 작업

### 진행 범위
- 슬롯 10개 시작 기준 고정 + 스킬관리자/실행기 정책 명시 + 구조체 강제 명세/핸드오프 문서 추가

### 수행 작업
1. 슬롯 정책 고정
- 베이스캐릭터/카탈로그/입력역할표를 시작 시점 Slot1..Slot10 기준으로 통일
- 미사용 슬롯은 Reserved/Empty 유지 정책 추가

2. 스킬관리자/실행기 정책 명시
- yjh_skill_executor_spec.md에 SkillManager 책임(보유/매핑/쿨타임)과 SkillExecutor 책임(검증/실행/판정) 분리 규칙 추가
- 단일 컴포넌트 통합은 허용하되 내부 책임 분리 원칙 명시

3. 런타임 요청 구조체 강제
- yjh_runtime_skill_request_context_spec.md 신규 생성
- 실시간 값은 Runtime Request Context 구조체로 전달하고 SkillDataAsset 원본 수정 금지 규칙 명시

4. 오해 방지 및 인수인계 강화
- yjh_bp_skill_authoring_guide.md에 "계획 인터페이스" 주의 문구 추가
- yjh_handoff_single_source_of_truth.md 신규 생성(단일 기준서)
- 실행 체크리스트에 신규 문서 2종 연결

### 현재 상태
- 외부 개발자가 문서만 보고도 10슬롯 구조/데이터 전달 방식/책임 분리 기준을 동일하게 적용할 수 있는 상태가 됨
