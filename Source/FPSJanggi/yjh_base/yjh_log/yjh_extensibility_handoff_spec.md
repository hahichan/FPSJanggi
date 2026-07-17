# YJH 확장성 인수인계 기준서 (Step 3)

## 0. 목적
- 베이스 캐릭터를 작성한 사람과 파생 캐릭터를 제작하는 사람을 분리해도 개발 속도와 품질이 유지되도록 기준을 고정한다.

## 1. 핵심 원칙
- 파생 캐릭터 제작자는 가능한 한 C++ 수정 없이 Blueprint/DataAsset 중심으로 작업한다.
- 베이스 클래스는 조립/중계 책임만 갖고, 캐릭터 전용 로직 하드코딩을 금지한다.
- 확장 포인트는 문서화된 공개 지점만 사용한다.

## 2. 필수 확장 포인트 목록
### Blueprint 확장 포인트
- OnCombatSessionStarted(CombatSessionId)
- OnCombatSessionEnded(CombatSessionId, EndReason)
- OnDamagedVisualOnly(InstigatorCombatantId, Amount)
- OnDeadVisualOnly(KillerCombatantId, EndReason)

### DataAsset 키(최소)
- CharacterId
- MaxHP
- MoveSpeed
- BaseDamage
- AttackRange
- AttackCooldown
- MontageSet(Attack/Hit/Death)

### 컴포넌트 교체 지점
- HealthComponentClass
- CombatComponentClass
- InputBufferComponentClass(optional)

## 3. 인수인계 패키지(필수 산출물)
1. 파생 캐릭터 제작 가이드 1부
2. 샘플 파생 캐릭터 1종(yjh_ 접두어)
3. 데이터 에셋 샘플 1종
4. 실패/디버그 가이드 1부(주요 로그 키 포함)

## 4. 완료 기준(DoD)
- 파생 캐릭터 제작자가 가이드만 보고 1종 추가 후 전투 진입 성공
- 신규 캐릭터 추가 시 필수 C++ 수정 파일 3개 이하
- 기존 캐릭터/매치 흐름 회귀 없음
- 주요 이벤트 로그로 문제 지점 추적 가능

## 5. 금지 규칙
- Base 클래스에 특정 캐릭터 애니메이션/수치/입력 하드코딩
- 파생 캐릭터 추가 때 게임모드 수정을 기본 전제로 삼는 설계
- 문서 없는 비공개 확장 포인트 사용

## 6. 검증 체크리스트
1. 샘플 캐릭터 복제 후 DataAsset 값만 변경하여 전투 진입 가능한가
2. OnDamaged/OnDead 연출을 BP에서 교체해도 서버 판정 로직이 유지되는가
3. 신규 캐릭터 추가 시 변경 파일 수가 기준 이내인가
4. 에러 발생 시 로그 키로 원인 경로를 3분 내 특정 가능한가

## 7. 로그 키 표준
- YJH_EXT_HANDOFF_START
- YJH_EXT_HANDOFF_READY
- YJH_EXT_HANDOFF_VALIDATE
- YJH_EXT_HANDOFF_FAIL
- YJH_EXT_HANDOFF_DONE
