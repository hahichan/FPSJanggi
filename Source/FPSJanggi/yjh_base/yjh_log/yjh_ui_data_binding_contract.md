# YJH UI 데이터 바인딩 계약서 (Master)

## 0. 목적
- UI와 전투 로직 사이 데이터 전달 약속을 단일 문서로 고정한다.
- 새 데이터 추가 시 변경 충돌을 줄이고, Blueprint 작업자가 즉시 반영 가능하도록 한다.

## 1. 범위
- 전투 HUD(Status/Cooldown) 바인딩
- 런타임 이벤트 계약
- Blueprint 호출 방식
- 변경 절차(신규 데이터 추가/기존 데이터 변경)

## 2. 데이터 필드 계약 (v0)
| 계약 키 | 영문 의미 | 한국어 설명 | 소유/원천 | 동기화 기준 | UI 갱신 트리거 | BP 사용 방식 | 변경 방식 |
|---|---|---|---|---|---|---|---|
| CurrentHP | Current Hit Point | 현재 체력 | HealthComponent RuntimeStats | 서버 권한, 복제 | OnRuntimeStatChanged | ProgressBar/Text 바인딩 | 타입 변경 금지, 의미 확장 시 신규 키 추가 |
| MaxHP | Maximum Hit Point | 최대 체력 | HealthComponent BaseStats | 초기값+복제 | OnRuntimeStatChanged | 분모값 표시/바 최대값 | 의미 변경 금지 |
| RuntimeMoveSpeed | Runtime Move Speed | 버프/디버프 반영 이동속도 | Combat/Movement RuntimeStats | 서버 계산, 복제/이벤트 | OnRuntimeStatChanged | Speed 텍스트 표시 | 단위 변경 시 Delta 문서 필수 |
| SlotIndex | Skill Slot Index | 스킬 슬롯 번호 | Input/Combat 슬롯 맵 | 로컬 표시 + 서버 검증 | OnSkillSlotMapped, OnCooldownChanged | 슬롯 위젯 선택 갱신 | 범위 확장 시 문서 버전 증가 |
| SkillId | Skill Identifier | 스킬 식별자 | SkillData | 매핑 시 확정 | OnSkillSlotMapped | 아이콘/라벨 조회 키 | 포맷 변경 시 호환 표기 필수 |
| bIsPassive | Passive Flag | 패시브 여부 | SkillData | 매핑 시 확정 | OnSkillSlotMapped | 패시브 배지/숨김 처리 | bool 의미 변경 금지 |
| RemainingSec | Cooldown Remaining Seconds | 남은 쿨타임(초) | CombatComponent RuntimeCooldownMap | 서버 권한 기준 | OnCooldownChanged | 쿨타임 숫자/마스크 표시 | 반올림 규칙 변경 시 Delta 문서 필수 |
| CooldownSourceState | Cooldown Source State | 쿨타임 소스 연결 상태 | CombatComponent/HUD Adapter | 상태 이벤트 | OnCooldownSourceStateChanged | Placeholder/Live 모드 전환 | enum 항목 추가 시 하위 호환 규칙 명시 |

CooldownSourceState v0
- Placeholder: 런타임 쿨타임 미연동 상태
- Live: 런타임 쿨타임 연동 완료 상태

## 3. 이벤트 계약 (v0)

### 3.1 OnRuntimeStatChanged
- 시그니처: OnRuntimeStatChanged(CurrentHP, MaxHP, RuntimeMoveSpeed)
- 용도: 우측 하단 Status Panel 즉시 갱신
- 호출 주체: 서버 반영 후 로컬 UI 브릿지(클라)
- Blueprint 처리: 위젯에서 바인딩 후 HP 텍스트/바, Speed 텍스트 갱신

### 3.2 OnSkillSlotMapped
- 시그니처: OnSkillSlotMapped(SlotIndex, SkillId, bIsPassive)
- 용도: 슬롯별 아이콘/패시브 표기 구성
- 호출 주체: 전투 시작 시 슬롯 매핑 완료 시점
- Blueprint 처리: SlotIndex로 슬롯 위젯 선택 후 아이콘, 배지, 라벨 반영

### 3.3 OnCooldownChanged
- 시그니처: OnCooldownChanged(SlotIndex, RemainingSec)
- 용도: 슬롯 쿨타임 숫자/오버레이 갱신
- 호출 주체: 쿨타임 시작/갱신/종료 시점
- Blueprint 처리: RemainingSec > 0 이면 숫자 표시, 0 이하면 Ready 처리

### 3.4 OnCooldownSourceStateChanged
- 시그니처: OnCooldownSourceStateChanged(CooldownSourceState)
- 용도: Placeholder 모드와 Live 모드 전환
- 호출 주체: HUD Adapter 또는 CombatComponent 상태 전환 시
- Blueprint 처리: Placeholder일 때 "-" 또는 "Ready" 정책 적용

## 4. Blueprint에서 부르는 방식 (호출 방식)

## 4.1 권장 바인딩 흐름
1. Widget Construct에서 Combatant 참조를 획득한다.
2. Event Dispatcher 또는 Interface 이벤트에 Bind Event를 수행한다.
3. 바인딩 직후 Snapshot 요청 함수(RequestHUDSnapshot)로 초기값을 1회 동기화한다.
4. 각 이벤트 수신 시 해당 UI 블록만 최소 갱신한다.
5. Widget Destruct에서 Unbind 처리한다.

## 4.2 권장 노드 패턴
- Bind Event to OnRuntimeStatChanged
- Bind Event to OnSkillSlotMapped
- Bind Event to OnCooldownChanged
- Bind Event to OnCooldownSourceStateChanged
- SetText / SetPercent / SetVisibility

## 4.3 BP 호출 주체 분리 원칙
- 전투 로직 호출: CombatComponent/Adapter
- UI 반영 호출: HUD Widget
- 금지: Widget에서 서버 권한 데이터를 직접 수정

## 5. 변경 방식 (신규 데이터 추가/기존 데이터 변경)

## 5.1 신규 데이터 추가 절차 (필수)
1. 새 데이터가 생기면 "반드시" 별도 파일을 생성한다.
2. 파일명 규칙: yjh_ui_binding_delta_YYYYMMDD_NN.md
3. Delta 파일에 아래 6개를 기록한다.
- 추가/변경/삭제 구분
- 데이터 키
- 한국어 설명
- 이벤트 영향
- Blueprint 마이그레이션 방법
- 호환성 등급(A/B/C)
4. 이후 Master 문서(yjh_ui_data_binding_contract.md)에 반영하고 버전 라인을 올린다.

## 5.2 기존 데이터 변경 절차 (필수)
- 의미 변경(semantic change): 기존 키 유지 금지, 신규 키 추가 후 단계적 이전
- 타입 변경(type change): Delta 문서 + 마이그레이션 가이드 필수
- 이벤트 제거/이름 변경: 최소 1개 버전 동안 호환 래퍼 제공 권장

## 5.3 호환성 등급
- A: UI 영향 없음(문서 보강 수준)
- B: BP 그래프 일부 수정 필요
- C: 기존 BP 깨짐 가능, 마이그레이션 필수

## 6. Blueprint 변경 방식 (마이그레이션 가이드 규격)
Delta 문서에는 반드시 아래를 포함한다.
- Before 노드 체인
- After 노드 체인
- 교체 대상 이벤트/핀 이름
- 기본값/예외값 처리 규칙
- 테스트 체크 3종(초기값/실시간 갱신/종료 상태)

## 7. 검증 기준 (DoD)
1. Status Panel이 CurrentHP/MaxHP/RuntimeMoveSpeed를 정상 갱신한다.
2. Slot1..Slot10 중 매핑된 슬롯이 정확히 표시된다.
3. 쿨타임 소스 미연동(Placeholder) 상태에서 UI 깨짐이 없다.
4. Live 상태 전환 후 RemainingSec이 감소 반영된다.
5. 신규 데이터 추가 시 Delta 파일이 생성되어 있다.

## 8. 연계 문서
- yjh_ui_hud_spec.md
- yjh_skill_data_format_spec.md
- yjh_skill_executor_spec.md
- yjh_dev_execution_checklist.md
