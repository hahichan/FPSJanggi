# YJH 입력 계층 역할표 (IA/IMC/SkillData) Draft

## 0. 목적
- IMC/IA 중심 확장 구조를 고정하고, 타 개발자가 Blueprint/DataAsset 위주로 캐릭터를 제작할 수 있게 역할 경계를 정의한다.

## 1. 역할표
| 계층 | 소유 책임 | 비책임 | 확장 주체 |
|---|---|---|---|
| IA (Input Action) | 입력 의도 정의(이동, 시점, 공격, 스킬 슬롯 호출) | 실제 데미지/판정/승패 로직 | 디자이너/캐릭터 제작자 |
| IMC (Input Mapping Context) | 키/패드 입력을 IA에 매핑, 상황별 컨텍스트 전환 | 스킬 효과/수치 결정 | 디자이너/캐릭터 제작자 |
| SkillData (DataAsset) | 스킬 슬롯별 파라미터, 쿨다운, 수치, 연출 참조 키 | 입력 디바이스 매핑 | 기획/디자이너 |
| yjh_ArenaCombatantBase | 입력 수신, 컴포넌트 중계, 공통 이동/피격 훅 | 캐릭터 개별 스킬 하드코딩 | 프로그래머(기초) |
| yjh_ArenaCombatComponent | IA 요청을 스킬 실행 요청으로 변환, 서버 검증 요청 | 키 바인딩 관리 | 프로그래머 |
| yjh_ArenaHealthComponent | HP/피해/사망 처리 | 입력 처리 | 프로그래머 |

## 2. 핵심 원칙
- IA는 스킬 그 자체가 아니라 "입력 의도"로 사용한다.
- IMC는 입력 장치/키맵만 담당한다.
- 실제 스킬 효과는 SkillData + CombatComponent에서 결정한다.
- 파생 캐릭터는 IMC/IA/SkillData 조합 변경만으로 제작 가능해야 한다.

## 3. 권장 IA 최소 세트
- IA_Move (Axis2D)
- IA_Look (Axis2D)
- IA_BasicAttack (Bool)
- IA_Skill1..IA_Skill10 (Bool)
- IA_Dodge (Bool)
- IA_Interact(optional)

프로젝트 고정 규칙
- 본 프로젝트는 시작 시점부터 10개 스킬 슬롯을 표준으로 사용한다.
- 사용하지 않는 슬롯은 Reserved/Empty 상태로 유지하고 입력 경로는 제거하지 않는다.

## 4. 권장 IMC 분리
- IMC_Common: 이동/시점/상호작용
- IMC_Combat: 공격/스킬/회피
- IMC_Debug: 디버그 입력(개발 전용)

## 5. 파생 캐릭터 제작 절차(요약)
1. SkillData 복제 후 수치/연출 키 설정
2. 캐릭터 전용 IMC 생성 또는 기존 IMC 변형
3. IA 매핑 점검
4. Blueprint에서 OnDamagedVisualOnly/OnDeadVisualOnly 연출 연결
5. 테스트 매트릭스 TM-01/TM-03/TM-10 실행

## 6. 금지 항목
- IA 이름에 캐릭터 고유 구현을 직접 하드코딩(예: IA_RedGeneralFinalSkillHardcoded)
- IMC에서 수치 로직을 직접 처리
- Base 클래스에서 캐릭터별 키맵 분기 하드코딩

## 7. 검증 포인트
- 입력 변경이 있어도 서버 판정 로직은 동일하게 유지되는가
- IMC 교체만으로 캐릭터 조작 체계 변경이 가능한가
- SkillData 교체만으로 스킬 수치/쿨다운 튜닝이 가능한가

## 8. 입력 우선순위 규칙(동시 입력 처리)
- Priority 0: Move/Look (항상 연속 처리)
- Priority 1: Defensive 입력(회피/방어/점프)
- Priority 2: Skill Slot 입력(Slot1..Slot10)
- Priority 3: UI/Debug 입력

동시 입력 처리 원칙
- 같은 틱에서 다중 입력이 들어오면 우선순위 순서로 검증/실행
- Move/Look은 다른 입력과 병행 허용
- Skill 입력 간 충돌 시 큐 정책(선입력 우선) 적용
