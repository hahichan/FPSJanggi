# YJH 런타임 스킬 요청 컨텍스트 명세서 (v1)

## 0. 목적
- 스킬 실행 시점의 실시간 값 전달 형식을 강제해 팀 구현 분기를 방지한다.
- SkillDataAsset 원본은 고정값만 보관하고, 동적값은 본 컨텍스트 구조체로 전달한다.

## 1. 강제 구조체 (계약)
권장 C++ 구조체명
- FYJHRuntimeSkillRequestContext

필수 필드
| 필드명 | 타입 | 한국어 설명 |
|---|---|---|
| CombatSessionId | Name | 전투 세션 식별자 |
| CombatantId | Name | 요청 주체 식별자 |
| SkillId | Name | 요청 스킬 식별자 |
| SlotIndex | Name | 입력 슬롯 인덱스 |
| SourceLocation | Vector | 요청 시점 캐릭터 위치 |
| SourceForward | Vector | 요청 시점 전방 벡터 |
| RequestedDirection | Vector | BP가 계산한 요청 방향 |
| RequestedLocation | Vector | BP가 계산한 목표 위치 |
| bHasRequestedLocation | bool | 목표 위치 유효 여부 |
| ClientRequestTimeSec | float | 로컬 요청 시각(디버그 용도) |

선택 필드
| 필드명 | 타입 | 한국어 설명 |
|---|---|---|
| AimHitLocation | Vector | 조준 트레이스 히트 위치 |
| AimHitNormal | Vector | 히트 노멀 |
| AimHitActor | SoftObjectPath/String | 히트 대상 식별 정보 |
| OptionalTargetId | Name | 타깃 식별자(선택) |

## 2. 스킬 타입별 필수 사용 필드
| ExecType | 필수 필드 | 비고 |
|---|---|---|
| HitScan | RequestedDirection or AimHitLocation | 거리/각도 검증 필요 |
| Projectile | RequestedDirection | 발사 초기 방향 계산 |
| Dash | RequestedDirection | 입력 기반 돌진 방향 |
| CooldownControl | OptionalTargetId(optional) | 대상 스킬은 SkillData에서 확인 |
| Teleport | RequestedLocation, bHasRequestedLocation | 목적지 검증/폴백 필수 |
| Passive | 없음(시스템 트리거) | 요청 컨텍스트 최소값만 유지 |

## 3. 검증 규칙
1. SkillDataAsset 원본을 런타임 값으로 수정하지 않는다.
2. Teleport는 bHasRequestedLocation=false이면 즉시 실패 또는 폴백 처리한다.
3. RequestedLocation/RequestedDirection은 서버에서 재검증한다.
4. SlotIndex와 SkillId 조합이 매핑 테이블과 일치해야 한다.

실행 데이터 소스 고정 규칙
5. 실행 시작 시 SkillData는 읽기 전용 스냅샷으로 1회 로드한다.
6. 실행 중 동적값은 RuntimeSkillRequestContext만 사용하며 SkillData 재기록/재계산을 금지한다.
7. 클라이언트 전달 컨텍스트는 서버에서 신뢰하지 않고 서버 재검증 결과를 최종값으로 사용한다.

## 4. 실패 코드 연동
- 위치 미유효: SKX_InvalidTarget or SKX_TeleportBlocked
- 사거리 초과: SKX_OutOfRange
- 세션 불일치: SKX_InvalidSession

## 5. 연계 문서
- yjh_skill_data_format_spec.md
- yjh_skill_executor_spec.md
- yjh_bp_skill_authoring_guide.md
