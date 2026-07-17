# YJH 초기 스킬 카탈로그 (v1, 교체본)

## 0. 목적
- 스킬 구동기 초기 개발 단계에서 바로 사용할 스킬 세트를 고정한다.
- Slot1..Slot10 기준으로 최소 1회전 플레이가 가능하도록 구성한다.

## 1. 구성 원칙
- 공격/진입/원거리/생존/패시브를 균형 있게 배치
- 각 스킬은 기존 Executor 조합으로 구현 가능해야 함
- 신규 메커닉이 필요하면 카탈로그에서 제외하고 후속 단계로 이관

## 2. 기본 스킬 구동기 구현 대상(요청 반영)
기존 카탈로그 스킬은 모두 폐기하고, 시작 시점부터 10슬롯 구조로 운영한다.

| Slot | SkillId | ExecType | 한국어 설명 | 기본 쿨타임 | 핵심 데이터 |
|---|---|---|---|---|---|
| Slot1 | SK_HitScan_Burst | HitScan | 히트스캔 공격 | 6s | 사거리/피해량/피격범위/쿨타임 |
| Slot2 | SK_Projectile_Lance | Projectile | 투사체 기반 공격 | 10s | 투사체 에셋/중력/무게/충돌/피해량 |
| Slot3 | SK_Dash_Impact | Dash | 돌진 후 충돌 타격 | 8s | 돌진 파워/방향 모드/충돌 처리 |
| Slot4 | SK_Cooldown_Sync | CooldownControl | 지정 스킬 쿨타임 감소 | 20s | 대상 SkillId/감소량/음수 방지 클램프 |
| Slot5 | SK_Teleport_Shift | Teleport | 순간이동 | 14s | 이동거리/안전반경/끼임 방지 폴백 |
| Slot6 | SK_PassiveRegen | Passive | 패시브 재생(슬롯 표시용) | 0s | PassiveType=Regen |
| Slot7 | SK_PassiveWallRun | Passive | 패시브 벽타기(슬롯 표시용) | 0s | PassiveType=WallRun |
| Slot8 | Unassigned | Reserved | 예약 슬롯 A | 0s | 추후 확장 |
| Slot9 | Unassigned | Reserved | 예약 슬롯 B | 0s | 추후 확장 |
| Slot10 | Unassigned | Reserved | 예약 슬롯 C | 0s | 추후 확장 |

## 3. 패시브 후보(기본 2종)
| SkillId | PassiveType | 한국어 설명 | 적용 방식 |
|---|---|---|---|
| SK_PassiveRegen | Regen | 전투 중 초당 체력 재생 | PassiveExecutor 주기 실행 |
| SK_PassiveWallRun | WallRun | 벽타기 가능 상태 부여 | Movement 연동 토글 |

## 4. 구현 순서(초기)
1. SK_HitScan_Burst
2. SK_Projectile_Lance
3. SK_Dash_Impact
4. SK_Cooldown_Sync
5. SK_Teleport_Shift
6. SK_PassiveRegen
7. SK_PassiveWallRun
8. Slot8..Slot10 예약 슬롯 정책 확정

## 5. 제외/보류 규칙
- 군중제어(CC) 누적 저항/해제 스킬은 후속 단계로 보류
- 소환형/설치형 오브젝트는 Projectile 안정화 이후 추가
- 글로벌 오라(전장 전체 영향)는 성능 측정 후 도입

## 6. 검증 체크
1. Slot1..Slot10 슬롯 컨테이너가 누락 없이 로딩된다.
2. 각 스킬 쿨타임이 HUD에 반영된다.
3. 패시브 2종이 입력 없이 동작한다.
4. 실패 코드가 로그에 남는다.
5. CooldownControl로 감소된 쿨타임은 0 미만으로 내려가지 않는다.
6. Teleport 실패 시 안전 지점 폴백으로 끼임을 방지한다.
7. Slot8..Slot10은 예약 슬롯으로 비정상 실행 없이 Idle 상태를 유지한다.
