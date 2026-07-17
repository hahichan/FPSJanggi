# YJH 콘텐츠 제작 가드레일 명세서 (Draft)

## 0. 목적
- 파생 캐릭터 제작자가 실수 없이 확장할 수 있도록 필수 입력/금지 항목/검증 절차를 고정한다.

## 1. 필수 입력(DataAsset)
- CharacterId
- TeamInfo
- PieceInfo
- MaxHP
- BaseDamage
- MoveSpeed
- TimeoutOverrideSeconds(optional)
- MontageSet(Attack/Hit/Death)

## 2. 필수 이벤트 구현(Blueprint)
- OnCombatSessionStarted
- OnDamagedVisualOnly
- OnDeadVisualOnly

## 3. 금지 항목
- Base 클래스에 캐릭터 전용 수치/애니메이션 하드코딩
- 게임모드 코드 수정을 파생 캐릭터 추가 필수 조건으로 요구
- 문서 없는 임의 확장 포인트 사용

## 4. 자동 검증(초안)
- 세션 시작 전 DataAsset 필수 필드 null 검사
- 누락 시 전투 시작 금지 + 에러 코드 반환
- 로그 키: YJH_GUARDRAIL_VALIDATE_FAIL

## 5. 인수인계 체크
- 신규 캐릭터 추가 시 수정 파일 3개 이하 여부
- 테스트 매트릭스 TM-01, TM-03, TM-10 통과
