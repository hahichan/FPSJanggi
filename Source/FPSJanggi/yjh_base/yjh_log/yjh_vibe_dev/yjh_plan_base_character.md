# YJH 베이스 캐릭터 계획서 (초안)

## 0. 문서 목적
- 1대1 전투용 베이스 캐릭터의 책임과 컴포지션 구조를 고정한다.
- 게임모드 오케스트레이션은 본 문서 범위에서 제외한다.
- 핵심 목표는 "작성자 외 다른 개발자가 쉽게 파생 캐릭터를 만들 수 있는 확장성"을 보장하는 것이다.

## 1. 소유 책임
- 이동/시점/입력 뼈대
- 체력/피해/사망 이벤트 전달
- 전투 컴포넌트 호출
- 상태 표시(로컬 UI 훅)

## 2. 비책임
- 매치 전체 승패 확정
- 보드 결과 반영
- 라운드 관리

## 3. 목표 클래스 구조
- yjh_ArenaCombatantBase
- yjh_ArenaHealthComponent
- yjh_ArenaCombatComponent
- (선택) yjh_ArenaInputBufferComponent

## 4. 공통 용어(통합 시트와 동일)
- CombatSessionId
- CombatantId
- EndReason
- FinalizeOnce

## 4-1. 기물 식별 체계(중요)
- 장기 진영 구분: TeamInfo (Han / Cho)
- 기물 종류 구분: PieceType (King, Chariot, Cannon, Horse, Elephant, Guard, Pawn)
- 실제 전투 식별은 TeamInfo + PieceType + PieceInstanceId 조합으로 관리
- 총 14개 기물(진영별 7종) 구분이 가능해야 하며, ArenaBattleReal 진입 시 GameMode에서 전달받아야 함

## 5. 컴포지션 규칙
- 기능은 컴포넌트로 분리하고 Base는 조립/중계만 담당
- Base에 수치 로직 직접 구현 금지
- 다중 상속은 인터페이스 구현 용도로만 허용

## 6. 확장성 우선 설계 규칙(중요)
- 파생 캐릭터 제작자가 C++ 수정 없이도 캐릭터를 만들 수 있게 DataAsset/Blueprint 노출 지점을 제공
- 특정 캐릭터 전용 하드코딩(메시, 애니메이션, 입력 키, 수치)을 Base에 두지 않음
- 컴포넌트 교체 가능 구조를 유지(예: 전투 컴포넌트만 교체)
- 파생 캐릭터 제작 가이드를 문서화하고 최소 예제 1개를 제공
- 전투 제한 시간은 캐릭터/말 정보 기반으로 오버라이드 가능하게 노출
- 필수 입력/금지항목/검증 기준은 yjh_content_guardrail_spec.md를 따른다.
- 입력 계층 역할 분리는 yjh_input_role_matrix.md를 따른다.

## 6-1. 베이스 캐릭터 필수 구성 항목(초안)
- Identity: CombatantId, TeamInfo, PieceType, PieceInstanceId
- Stats: HP/이동/공격/방어/저항/타임아웃 오버라이드
- Runtime: 현재 HP, 상태, 최근 피격 시간, 사망 보고 플래그
- Input: IMC 슬롯 + IA 참조(스킬 N개 확장 가능)
- Camera/Visual: 상대 확인용 전투 카메라 슬롯 + 1인칭 팔 메쉬 슬롯
- Components: Health/Combat/InputBuffer(optional)

## 6-2. 스탯/런타임 확장 가이드
- 기본 스탯은 최소 필드만 고정하고, 실제 밸런스용 세부 스탯은 DataAsset로 확장
- Runtime 상태는 복제 대상과 로컬 전용 상태를 분리
- 예시 Runtime 필드: CurrentHP, CharacterState, bDeathReported, LastDamageServerTime

기본 스탯 v0 (확정)
- MaxHP
- CurrentHP
- MoveSpeed
- RegenPerSecond
- Defense
- StatusResistance (Stun 등)
- JumpPower
- Acceleration
- HitboxRadius
- HitboxHalfHeight
- CharacterScale

추가 권장 스탯(확장)
- Stamina(or ActionResource)
- DamageMultiplierTaken
- CooldownReduction
- HealingMultiplier

Base/Runtime 스탯 필드표 (초안)
| 구분 | 필드 | 설명 |
|---|---|---|
| BaseStats | MaxHP | 최대 체력 원본값 |
| BaseStats | MoveSpeed | 이동 속도 원본값 |
| BaseStats | RegenPerSecond | 초당 재생 원본값 |
| BaseStats | Defense | 방어력 원본값 |
| BaseStats | StatusResistance | 상태이상 저항 원본값 |
| BaseStats | JumpPower | 점프력 원본값 |
| BaseStats | Acceleration | 가속도 원본값 |
| BaseStats | HitboxRadius | 피격 판정 반지름 원본값 |
| BaseStats | HitboxHalfHeight | 피격 판정 반높이 원본값 |
| BaseStats | CharacterScale | 캐릭터 크기 배율 원본값 |
| RuntimeStats | CurrentHP | 현재 체력 |
| RuntimeStats | RuntimeMoveSpeed | 버프/디버프 반영 이동속도 |
| RuntimeStats | RuntimeDefense | 버프/디버프 반영 방어력 |
| RuntimeStats | RuntimeJumpPower | 버프/디버프 반영 점프력 |
| RuntimeStats | RuntimeAcceleration | 버프/디버프 반영 가속도 |
| RuntimeStats | RuntimeHitboxRadius | 반영된 피격 반지름 |
| RuntimeStats | RuntimeHitboxHalfHeight | 반영된 피격 반높이 |
| RuntimeStats | RuntimeCharacterScale | 반영된 캐릭터 크기 |
| RuntimeStats | CharacterState | Preparing/Fighting/Stunned/Dead |

쿨타임 관리 위치
- SkillData에는 CooldownSec만 정의
- 실제 쿨타임 상태(남은 시간/사용 시각)는 CombatComponent 런타임 맵(TMap<SkillId, LastUseTime>)에서 관리
- 서버 권한 기준으로만 쿨타임 확정

## 6-3. 입력 슬롯 설계 가이드
- 스킬 슬롯 수는 2개 고정이 아니라 N개 확장 구조로 설계
- IA_Skill1..IA_SkillN -> Slot1..SlotN -> SkillData 매핑 규칙 사용
- 슬롯/매핑 정보는 Blueprint에서 변경 가능하도록 노출(EditDefaultsOnly/BlueprintReadWrite)

입력 기본값 v0 (확정)
- Move: WASD
- Jump: Space
- Skill Slots: 10개 (Slot1..Slot10)

초기 구현 범위(v2)
- 시작 시점부터 Slot1..Slot10 전부를 베이스 캐릭터 표준 슬롯으로 노출한다.
- 슬롯은 비어 있을 수 있으나, 슬롯 컨테이너/입력 경로/쿨타임 조회 경로는 10개 기준으로 항상 유지한다.

## 6-4. 카메라/비주얼 설계 가이드
- 기본 전투 카메라 슬롯(상대 확인 가능 시야) 제공
- 1인칭 팔 메쉬 슬롯 제공(필요 캐릭터만 사용)
- 카메라/비주얼 자산은 베이스 하드코딩 금지, Blueprint/DataAsset 참조 기반으로 교체 가능해야 함

BP 친화 커스터마이징 필수
- 캐릭터 메쉬 교체 가능
- VFX/SFX 키 기반 교체 가능
- 사운드 재생 훅(BlueprintImplementableEvent) 제공

패시브 스킬 정책 v0
- 기본 재생 패시브 적용 허용
- 벽타기 패시브 적용 허용
- 패시브는 SkillData의 PassiveType 또는 CharacterPassiveSet으로 관리하고, 실행 트리거는 CombatComponent/Movement 연동에서 처리

## 7. 핵심 인터페이스(예시)
- IYJH_Damageable: ApplyDamage, IsDead
- IYJH_CombatantState: OnCombatStart, OnCombatEnd

## 8. 복제/동기화 규칙
- CurrentHP/MaxHP는 복제 대상
- HP 변경은 서버에서만 수행
- OnRep_CurrentHP에서 UI 갱신 이벤트 발생
- DeathState 전환은 1회만 허용

## 9. 이벤트 계약
- OnDamaged(CombatSessionId, InstigatorCombatantId, Amount)
- OnDead(CombatSessionId, DeadCombatantId, KillerCombatantId, EndReason)
- OnAttackRequested(Target)
- OnAttackResolved(Result)

## 10. 완료 기준(공통 용어 기준)
- 양측 캐릭터 스폰 및 기본 입력 동작
- 서버 피해 반영 시 양 클라 체력 동기화
- 동일 CombatSessionId에서 OnDead 이벤트 1회만 발생
- 파생 캐릭터 1종을 C++ 추가 수정 없이 생성 가능

## 11. 테스트 시나리오
1. 단일 타격 HP 감소
2. 연속 타격 중 동기화 확인
3. HP 0 도달 시 중복 사망 방지
4. 사망 직후 입력 차단
5. 파생 캐릭터 샘플 생성 후 전투 진입 검증

## 12. 구현 우선순위
1. Base + Health 최소 루프
2. Combat 요청/해결 최소 루프
3. 파생 캐릭터 제작 지점(Blueprint/DataAsset) 노출
4. 예외 케이스 보강

## 13. 연계 설계 문서
- 베이스캐릭터 설계 진행 계획: yjh_plan_base_character_phase2.md
- 스킬데이터 포맷 명세: yjh_skill_data_format_spec.md
- 스킬 실행기 명세: yjh_skill_executor_spec.md
- 전투 HUD 명세: yjh_ui_hud_spec.md
- UI 데이터 바인딩 계약: yjh_ui_data_binding_contract.md
- UI 신규 데이터 Delta 템플릿: yjh_ui_binding_delta_template.md
