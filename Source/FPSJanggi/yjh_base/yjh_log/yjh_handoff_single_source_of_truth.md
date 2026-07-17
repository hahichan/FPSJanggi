# YJH 핸드오프 단일 기준서 (SSOT)

## 0. 목적
- 신규/외부 개발자가 첫 진입 시 본 문서 1개만 읽고 구현 기준을 이해하도록 한다.
- 초안 문서가 여러 개여도 최종 기준선은 본 문서가 우선한다.

## 1. 최종 고정 기준 (현재)
1. 슬롯 정책
- 베이스 캐릭터는 시작 시점부터 Slot1..Slot10을 표준 슬롯으로 노출한다.
- 미사용 슬롯은 Reserved/Empty 상태로 유지한다.

2. 명칭 정책
- 기본 컴포넌트 명칭은 yjh_ArenaCombatComponent를 사용한다.
- 내부적으로 SkillManager/SkillExecutor 책임을 분리해 설계한다.

3. 데이터 정책
- SkillDataAsset: 고정값(규칙/기준값)
- Runtime Request Context: 실시간값(위치/방향/히트)
- 실시간값을 SkillDataAsset 원본에 쓰는 행위 금지

4. 권한 정책
- 최종 판정/쿨타임 확정/승패 확정은 서버 권한으로만 수행

5. 하드코딩 정책
- Base 클래스에 캐릭터 전용 로직 하드코딩 금지

## 2. 타 개발자 최소 작업 순서
1. 베이스 캐릭터 자식 BP 생성
2. Slot1..Slot10 IA 매핑
3. SkillData 작성(고정값)
4. Runtime Request Context 전달(BP 계산값)
5. 실행 실패코드 로그 확인
6. HUD 이벤트 연동 확인

## 3. 필수 문서 읽기 순서
1. yjh_agent_execution_plan.md
2. yjh_plan_base_character.md
3. yjh_skill_data_format_spec.md
4. yjh_skill_executor_spec.md
5. yjh_runtime_skill_request_context_spec.md
6. yjh_bp_skill_authoring_guide.md
7. yjh_ui_data_binding_contract.md

## 4. 오해 방지
- BP 노드 목록은 계획 인터페이스 기준이며, 구현 시 동일 이름/시그니처를 맞춘다.
- 노드가 아직 없는 상태는 문서 오류가 아니라 "구현 전 단계" 상태다.

## 5. 변경 관리
- 본 기준서 변경 시 yjh_chat_3.md에 작업 번호로 기록
- 파괴적 변경은 yjh_change_policy.md의 C등급 절차를 따른다.
