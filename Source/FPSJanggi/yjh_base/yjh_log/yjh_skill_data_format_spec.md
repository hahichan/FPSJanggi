# YJH 스킬데이터 포맷 명세서 (v2 Draft)

## 0. 목적
- 스킬 실행에 필요한 데이터 필드 표준을 정의한다.

## 1. 핵심 원칙
- IA는 입력 의도만 정의한다.
- SkillData는 스킬의 규칙/수치/연출 키를 정의한다.
- 실제 실행은 CombatComponent/Executor가 수행한다.
- 재생/벽타기 같은 기능도 "패시브 스킬"로 SkillData에서 정의한다.
- 실시간 값(순간이동 목표 위치, 돌진 방향 등)은 SkillData에 쓰지 않고 Runtime Request Context 구조체로 전달한다.

## 2. 필수 필드
- SkillId (FName)
- InputSlot (enum or tag)
- ExecType (HitScan, Projectile, Dash, CooldownControl, Teleport, Passive)
- CooldownSec (float)
- MaxRange (float)
- BasePower (float)
- bServerAuthorityOnly (bool)

필수 필드 v2 표준
| 필드 | 타입 | 예시 | 한국어 설명 |
|---|---|---|---|
| SkillId | FName | SK_HitScan_Burst | 스킬 고유 식별자 |
| InputSlot | FName/Tag | Slot1 | 입력 슬롯 매핑 키 |
| ExecType | Enum | HitScan | 실행기 타입 |
| CooldownSec | float | 8.0 | 기준 쿨타임(초) |
| MaxRange | float | 350.0 | 최대 유효 거리 |
| BasePower | float | 20.0 | 기본 위력(피해/효과량 기반값) |
| bServerAuthorityOnly | bool | true | 서버 권한으로만 확정할지 여부 |

## 2-1. 실행기별 필수 파라미터(v2)
| ExecType | 필드 | 타입 | 한국어 설명 |
|---|---|---|---|
| HitScan | HitScanRange | float | 히트스캔 사거리 |
| HitScan | HitScanRadius | float | 피격 범위 반지름 |
| HitScan | HitScanDamage | float | 피해량 |
| Projectile | ProjectileClassKey | FName | 투사체 에셋 키 |
| Projectile | ProjectileSpeed | float | 투사체 속도 |
| Projectile | ProjectileGravityScale | float | 중력 계수 |
| Projectile | ProjectileMassScale | float | 무게 계수 |
| Projectile | ProjectileDamage | float | 투사체 피해량 |
| Dash | DashPower | float | 돌진 파워 |
| Dash | DashDirectionMode | Enum | 전방/입력기반 방향 모드 |
| CooldownControl | CooldownTargetSkillId | FName | 쿨타임 조작 대상 스킬 |
| CooldownControl | CooldownDeltaSec | float | 쿨타임 변화량(감소는 음수) |
| Teleport | TeleportMaxDistance | float | 최대 순간이동 거리 |
| Teleport | TeleportSafetyRadius | float | 목적지 충돌 안전 반경 |
| Teleport | TeleportFallbackTries | int32 | 대체 지점 탐색 횟수 |

## 3. 선택 필드
- CastTimeSec
- Charges
- CostValue
- MontageKey
- VFXKey
- SFXKey
- TimeoutOverrideSeconds(optional)
- bIsPassive
- PassiveType (Regen, WallRun 등)
- SkillIconKey
- CooldownDisplayName

v2 추가 선택 필드
- HitScanRange
- HitScanRadius
- HitScanDamage
- ProjectileClassKey
- ProjectileSpeed
- ProjectileGravityScale
- ProjectileMassScale
- ProjectileDamage
- DashPower
- DashDirectionMode
- CooldownTargetSkillId
- CooldownDeltaSec
- TeleportMaxDistance
- TeleportSafetyRadius
- TeleportFallbackTries

선택 필드 v2 표준
| 필드 | 타입 | 기본값 | 한국어 설명 |
|---|---|---|---|
| CastTimeSec | float | 0.0 | 시전 시간(초) |
| Charges | int32 | 0 | 충전 횟수(0은 미사용) |
| CostValue | float | 0.0 | 자원 소모량 |
| MontageKey | FName | None | 애니메이션 몽타주 키 |
| VFXKey | FName | None | 시각효과 키 |
| SFXKey | FName | None | 사운드 키 |
| TimeoutOverrideSeconds | float? | null | 기물별 시간 오버라이드 |
| bIsPassive | bool | false | 패시브 여부 |
| PassiveType | Enum | None | 패시브 종류 |
| SkillIconKey | FName | None | HUD 아이콘 키 |
| CooldownDisplayName | FText | Empty | HUD 표시 이름 |

PassiveType v0 (초안)
- None
- Regen
- WallRun
- AutoShield
- MoveSpeedAura

## 4. 슬롯 매핑 규칙
- IA_Skill1 -> Slot1 -> SkillData
- IA_Skill2 -> Slot2 -> SkillData
- ...
- IA_SkillN -> SlotN -> SkillData
- 매핑 누락 시 실행 금지 + Warning 로그

추가 규칙
- 스킬 개수는 N개 확장 가능 구조로 설계(고정 2개 금지)
- 슬롯 인덱스는 캐릭터 DataAsset에서 정의 가능
- v0 기본 슬롯 수는 10개(Slot1..Slot10)

쿨타임 규칙
- SkillData의 CooldownSec는 기준값
- 실제 남은 쿨타임은 CombatComponent 런타임에서 관리
- HUD는 Runtime 쿨타임 소스가 없을 때 Placeholder("Ready" or "-")를 표시

패시브 규칙
- bIsPassive=true 인 경우 입력 없이도 조건 충족 시 실행 가능
- Regen/WallRun은 패시브 스킬로 등록 권장

## 5. 검증 규칙
- SkillId 중복 금지
- CooldownSec >= 0
- MaxRange >= 0
- ExecType 미지정 금지
- 필수 필드 누락 시 세션 시작 전 Validation 실패 처리
- Slot 중복 매핑 금지(동일 Slot에 다중 SkillId 금지)
- bIsPassive=true 인 스킬은 InputSlot 없이도 등록 가능

v1 호환 검증 규칙
- bIsPassive=false 인 스킬은 InputSlot 필수
- Charges > 0 인 경우 CooldownSec > 0 권장
- ExecType=Projectile 인 경우 MaxRange > 0 필수
- ExecType=Passive 인 경우 BasePower는 효과 계수/량으로 해석(0 이상 권장)
- CooldownDisplayName이 비어 있으면 SkillId 문자열 fallback 사용

v2 추가 검증 규칙
- HitScan 계열은 HitScanRange > 0, HitScanDamage >= 0 필수
- Projectile 계열은 ProjectileClassKey/ProjectileSpeed 필수
- CooldownControl은 CooldownTargetSkillId 필수
- CooldownControl 실행 결과는 최소 0.0으로 클램프(음수 금지)
- Teleport는 TeleportMaxDistance > 0, TeleportSafetyRadius > 0 필수

## 5-1. 추천 실패 사유 코드(Validation)
- SKV_MissingRequiredField
- SKV_DuplicateSkillId
- SKV_InvalidExecType
- SKV_InvalidRange
- SKV_InvalidCooldown
- SKV_DuplicateSlotMapping
- SKV_InvalidPassiveConfig

## 5-2. SkillData v2 샘플(요약)
| SkillId | Slot | ExecType | CooldownSec | MaxRange | BasePower | 비고 |
|---|---|---|---|---|---|---|
| SK_HitScan_Burst | Slot1 | HitScan | 6.0 | 1200 | 22 | 히트스캔 연사 |
| SK_Projectile_Lance | Slot2 | Projectile | 10.0 | 2000 | 35 | 투사체 창 공격 |
| SK_Dash_Impact | Slot3 | Dash | 8.0 | 400 | 18 | 돌진 충돌 타격 |
| SK_Cooldown_Sync | Slot4 | CooldownControl | 20.0 | 0 | 0 | 지정 스킬 쿨감 |
| SK_Teleport_Shift | Slot5 | Teleport | 14.0 | 900 | 0 | 안전 지점 순간이동 |

## 6. 연동 지점
- 입력 구조: yjh_input_role_matrix.md
- 콘텐츠 가드레일: yjh_content_guardrail_spec.md
- 스킬 실행기: yjh_skill_executor_spec.md
- 초기 스킬 카탈로그: yjh_skill_initial_catalog.md
- BP 제작 가이드: yjh_bp_skill_authoring_guide.md
- 런타임 요청 컨텍스트 명세: yjh_runtime_skill_request_context_spec.md
