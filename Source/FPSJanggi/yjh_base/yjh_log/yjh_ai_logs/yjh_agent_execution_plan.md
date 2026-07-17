# YJH 에이전트 단일 실행 계획서 (v1)

## 0. 목적
- 문서가 많아도 에이전트는 본 문서 1개를 시작점으로 사용한다.
- 구현 중 의사결정 충돌이 발생하면 본 문서의 우선순위를 따른다.

## 1. 적용 범위
- ArenaBattleReal 1v1 전투
- 베이스 캐릭터 + 스킬 실행기 + 게임모드 종료 판정
- 서버 권한 멀티플레이 기준

## 2. 단일 규칙 요약 (먼저 고정)
1. FinalizeOnce
- 세션별 원자 커밋은 1회만 성공한다.
- 늦게 도착한 종료 후보는 결과를 덮어쓰지 않는다.

2. Timeout vs Grace
- Grace 진행 중에도 TimeoutDeadline 비교는 계속된다.
- 같은 서버 틱에서 ForcedAbort와 Timeout이 동시에 성립하면 ForcedAbort를 우선한다.

3. SkillData vs Runtime Context
- SkillData는 읽기 전용 고정값이다.
- 동적값(방향/위치/히트)은 RuntimeSkillRequestContext에서만 사용한다.
- 클라이언트 컨텍스트는 서버 재검증 후에만 적용한다.

## 3. 에이전트 실행 순서 (고정)
1. 세션/종료 안정화
- FinalizeOnce 원자 경로를 먼저 구현한다.
- 종료 우선순위 적용 로그를 1회만 남기도록 고정한다.

2. 스킬 요청 파이프라인 구현
- Slot 입력 -> SkillData 조회 -> 읽기 전용 스냅샷 -> 서버 검증 -> 실행 -> 쿨타임 반영 순서로 구현한다.

3. HUD 최소 연동
- HP/Speed/Slot Cooldown 표시만 먼저 연결한다.
- Placeholder 정책은 Ready 또는 - 중 확정값 하나로 통일한다.

4. 예외 케이스 검증
- 동시사망, Timeout, Disconnect Grace 만료, 중복 종료 요청을 우선 테스트한다.

## 4. 구현 중 금지사항
- Base 캐릭터에 스킬별 하드코딩 금지
- SkillData 원본에 런타임 값 쓰기 금지
- 클라이언트 시간으로 승패/종료 판정 금지
- 종료 확정 이후 재브로드캐스트 금지

## 5. 최소 완료 조건 (Gate)
- 세션당 Finalize 1회만 기록
- 동일 틱 동시 이벤트에서 우선순위가 일관 동작
- 슬롯 입력 후 쿨타임/실패코드/HUD 반영이 서버 기준으로 재현
- Draw/Timeout/ForcedAbort 결과가 테스트 5회 연속 동일

## 6. 빠른 참조 문서 (필요할 때만)
1. yjh_plan_gamemode.md
2. yjh_termination_priority_spec.md
3. yjh_disconnect_grace_policy.md
4. yjh_session_lifecycle_spec.md
5. yjh_skill_executor_spec.md
6. yjh_runtime_skill_request_context_spec.md
7. yjh_ui_data_binding_contract.md

## 7. 로그 기록 규칙
- 의미 있는 코드/문서 변경은 yjh_chat_3.md에 작업 번호로 기록한다.
- 신규 문서는 yjh_ 접두어를 사용한다.
