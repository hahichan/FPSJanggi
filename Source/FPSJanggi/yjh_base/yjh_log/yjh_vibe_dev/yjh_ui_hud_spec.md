# YJH 전투 HUD UI 명세서 (Draft)

## 0. 목적
- 전투 중 필수 정보(Hp/Speed/Cooldown)를 사용자에게 일관되게 노출한다.
- 스킬 구동기 세부가 확정되기 전에도 UI 구조를 선행 고정한다.

## 1. 기본 레이아웃
- 위치: 화면 우측 하단(Right Bottom)
- 정렬: 위에서 아래로 Status Panel -> Skill Cooldown Panel

## 2. Status Panel (우측 하단 상단 블록)
필수 표시 항목
- CurrentHP / MaxHP
- CurrentSpeed

표시 규칙
- HP: "현재/최대" 형식 텍스트 + 선택적으로 바(progress)
- Speed: RuntimeMoveSpeed 값을 실시간 텍스트로 표시
- 데이터 출처: RuntimeStats (OnRep 및 UI 갱신 이벤트 연동)

## 3. Skill Cooldown Panel (우측 하단 하단 블록)
- 슬롯 수: v0 기준 10개(Slot1..Slot10)
- 각 슬롯 표시 항목
  - SlotIndex
  - SkillIcon(optional)
  - Cooldown Remaining Sec

쿨타임 표시 규칙
- SkillData만 있고 런타임 쿨타임 소스가 없을 때: Placeholder 상태("Ready" 또는 "-") 표시
- CombatComponent 런타임 쿨타임 맵 연동 후: 남은 시간 초 단위 표시
- 패시브 스킬 슬롯도 항상 표시한다.
- 패시브 슬롯은 Passive 배지를 노출하고 쿨타임 숫자는 숨김 처리한다.

## 4. BP 친화 정책
- HUD 위젯은 Blueprint에서 교체 가능해야 함
- 색상/폰트/아이콘/정렬을 BP에서 변경 가능해야 함
- 추후 레이아웃 변경을 고려해 데이터 바인딩 인터페이스를 분리

## 5. 이벤트/바인딩 계약(초안)
- OnRuntimeStatChanged(CurrentHP, MaxHP, RuntimeMoveSpeed)
- OnCooldownChanged(SlotIndex, RemainingSec)
- OnSkillSlotMapped(SlotIndex, SkillId, bIsPassive)
- 상세 계약, BP 호출 방식, 변경 절차는 yjh_ui_data_binding_contract.md를 기준으로 한다.

## 6. 검증 기준
1. 우측 하단에 HP/MaxHP/Speed가 정상 표시된다.
2. Slot1..Slot10 UI가 생성된다.
3. 쿨타임 소스 미연동 상태에서도 UI가 깨지지 않는다.
4. 쿨타임 연동 후 남은 시간이 감소 표시된다.

## 7. 추후 변경 메모
- 미니맵/버프 아이콘/상태이상 아이콘은 후속 단계에서 추가
- 현재 문서는 구조 고정용 Draft
