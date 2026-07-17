# YJH 블루프린트 스킬 제작 가이드 (v1)

## 0. 목적
- 타 개발자가 블루프린트에서 스킬을 쉽게 제작할 수 있도록 노드명/타입/의미를 고정한다.

주의
- 본 문서의 노드명은 "계획 인터페이스 명세"다.
- 실제 코드 구현 전에는 노드가 존재하지 않을 수 있으며, 구현 시 동일 명칭/시그니처를 맞추는 것을 원칙으로 한다.

인터페이스 고정 정책
- 본 문서의 BP 노드명/시그니처는 v1 고정 인터페이스로 사용한다.
- 변경이 필요하면 파괴적 변경 절차(C등급)를 따른다.

## 1. BP에서 자주 쓰는 호출 노드
| 노드명 | 입력 데이터 타입 | 출력 데이터 타입 | 한국어 설명 |
|---|---|---|---|
| InitializeSkillMap | 없음 | bool | SkillDataAsset 검증 및 슬롯 매핑 초기화 |
| SetCombatContext | Name(CombatSessionId), Name(CombatantId) | 없음 | 전투 세션 정보 주입 |
| RequestSkillBySlot | Name(SlotIndex), Actor(OptionalTarget) | SkillExecutionResult(struct) | 슬롯 스킬 실행 요청 |
| GetRemainingCooldownBySlot | Name(SlotIndex) | float | 슬롯 기준 남은 쿨타임 조회 |
| GetRemainingCooldownBySkillId | Name(SkillId) | float | 스킬 ID 기준 남은 쿨타임 조회 |

## 2. BP에서 바인딩할 이벤트 노드
| 이벤트명 | 파라미터 타입 | 한국어 설명 |
|---|---|---|
| OnSkillSlotMapped | Name SlotIndex, Name SkillId, bool bIsPassive | 슬롯-스킬 매핑 UI 갱신 |
| OnCooldownChanged | Name SlotIndex, float RemainingSec | 쿨타임 숫자/마스크 갱신 |
| OnSkillRequestFailed | Enum FailCode | 실패 사유 표시/디버그 로그 |

## 3. 스킬별 BP 입력 데이터 체크리스트

런타임 요청 컨텍스트 규칙
- 실시간 값(목표 위치/방향/히트결과)은 SkillDataAsset 원본을 수정하지 않고 Runtime Request Context 구조체로 전달한다.
- 구조체 상세는 yjh_runtime_skill_request_context_spec.md를 따른다.

### 3.1 히트스캔 공격 (HitScan)
- SkillId: Name
- HitScanRange: float
- HitScanRadius: float
- HitScanDamage: float
- CooldownSec: float

### 3.2 투사체 공격 (Projectile)
- SkillId: Name
- ProjectileClassKey: Name
- ProjectileSpeed: float
- ProjectileGravityScale: float
- ProjectileMassScale: float
- ProjectileDamage: float
- CooldownSec: float

### 3.3 돌진 (Dash)
- SkillId: Name
- DashPower: float
- DashDirectionMode: Enum(Forward/InputVector)
- CooldownSec: float

### 3.4 쿨타임 조작 (CooldownControl)
- SkillId: Name
- CooldownTargetSkillId: Name
- CooldownDeltaSec: float (감소는 음수 허용)
- 내부 처리 규칙: 결과 쿨타임은 반드시 0 이상으로 클램프

### 3.5 순간이동 (Teleport)
- SkillId: Name
- TeleportMaxDistance: float
- TeleportSafetyRadius: float
- TeleportFallbackTries: int32
- 내부 처리 규칙: 목적지 충돌 시 최근 유효 지점으로 폴백

## 4. 권장 블루프린트 흐름
1. Character BeginPlay에서 InitializeSkillMap 호출
2. Widget은 Construct(OnInitialized)에서 이벤트 바인딩 수행
3. SetCombatContext 호출
4. IA 이벤트에서 RequestSkillBySlot 호출
5. OnCooldownChanged로 UI 갱신
6. OnSkillRequestFailed로 실패 코드 노출

## 5. 금지 사항
- BP에서 서버 권한 데이터(최종 판정/최종 쿨타임)를 직접 확정하지 않는다.
- IA에 스킬 수치를 직접 하드코딩하지 않는다.
- 슬롯 매핑이 실패한 상태에서 실행 노드를 호출하지 않는다.

권한 정책
- 클라이언트는 요청만 수행하고 최종 확정은 서버 결과를 따른다.
