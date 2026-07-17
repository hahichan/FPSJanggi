# YJH 베이스캐릭터 설계 진행 계획서 (Phase 2, Draft)

## 0. 목적
- 베이스캐릭터 상세 요구사항을 대화로 확정한 뒤, 스킬데이터 포맷과 스킬 실행기 문서까지 연속 완료한다.

## 1. 진행 순서(고정)
1. 베이스캐릭터 필수 구성요소 확정
2. 스킬데이터 포맷 명세 확정
3. 스킬 실행기(Executor) 명세 확정
4. 세 문서 간 용어/필드 동기화

## 2. 단계별 산출물
### Step 1: 베이스캐릭터 구성 대화
- 결과 문서: yjh_plan_base_character.md 업데이트
- 핵심 항목: Stats/Input/Camera/Visual/Runtime 분류표

### Step 2: 스킬데이터 포맷 문서화
- 결과 문서: yjh_skill_data_format_spec.md
- 핵심 항목: 필수 필드, 선택 필드, 검증 규칙, 슬롯 매핑 규칙

### Step 3: 스킬 실행기 문서화
- 결과 문서: yjh_skill_executor_spec.md
- 핵심 항목: 기본 실행기 목록(Damage/Dash/Buff/Projectile), 서버 검증 순서, 확장 규칙

## 3. 완료 기준
- 타 개발자가 C++ 수정 없이 IMC/IA/SkillData 조합으로 파생 캐릭터 1종 제작 가능
- 신규 스킬 추가 시 BaseCharacter 수정 없이 SkillData + Executor 조합으로 처리 가능
- 문서 3종 간 용어(CombatSessionId/Slot/SkillId)가 충돌 없이 일치

## 4. 운영 규칙
- 계획 단계 Draft 유지(변경 가능)
- 의미 있는 변경은 yjh_chat_2.md 기록
- 신규 파일은 yjh_ 접두어 사용
