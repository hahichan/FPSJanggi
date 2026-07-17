# YJH 최소 수직 슬라이스 명세서 (Step 5, Draft)

## 0. 목적
- 기능 완성 전에 "한 판이 끝까지 도는 최소 루프"를 먼저 확정한다.
- 본 문서는 계획 단계 Draft이며 실제 코드 작성 전까지 변경 가능하다.

## 1. 슬라이스 범위(MVP-Lite)
- 2인 접속(Host + Client)
- ArenaBattleReal 진입
- 단일 공격 루프(테스트 데미지 허용)
- 한 명 HP 0 도달
- FinalizeOnce 승패 확정
- NSH 보드 복귀
- 동시 사망 시 Draw + 양측 제거
- 타임아웃 종료 시 Draw + 양측 제거
- 동시 사망 판정은 같은 서버 틱의 동일 데미지 배치 처리 기준 사용
- 타임아웃 기본값 60초

## 2. 비범위
- 콤보/입력 버퍼
- 정교한 히트 판정
- 애니메이션 노티파이 연출 완성
- 밸런스 튜닝

## 3. 성공 기준(DoD)
1. CombatSessionId 기준으로 시작-종료 로그가 1개 세션으로 닫힌다.
2. FinalizeCombatSession이 세션당 1회만 실행된다.
3. Dead 이벤트는 패배자 기준 1회만 보고된다.
4. 보드 복귀 이벤트가 누락 없이 발생한다.
5. 같은 절차 3회 반복 시 결과 일관성이 유지된다.
6. Draw 케이스에서 RemovePolicy가 BothEliminated로 기록된다.
7. TimeoutSeconds 설정값 변경 시 타임아웃 종료 시점이 함께 변경된다.
8. PieceTimeoutOverrideSeconds 적용 시 해당 전투에 오버라이드 값이 우선 적용된다.

## 4. 실행 절차
1. Host/Client 준비
2. yjh.start(또는 UI 버튼)로 전투 진입
3. 테스트 공격 1~N회로 HP 감소 확인
4. HP 0 도달 시 종료 확정 확인
5. 보드 복귀 확인
6. 동일 테스트 3회 반복
7. 동시 사망 유도 테스트 1회 수행
8. 타임아웃 종료 테스트 1회 수행
9. 기본 60초 기준 타임아웃 테스트 1회 수행
10. 오버라이드 값(예: 30초) 적용 테스트 1회 수행

## 5. 필수 관측 포인트
- YJH_DBG_SESSION_BEGIN
- YJH_DBG_DAMAGE_APPLY
- YJH_DBG_DEATH_REPORT
- YJH_DBG_FINALIZE_ONCE
- YJH_DBG_RETURN_TO_BOARD

## 6. 실패 판정
- Finalize 2회 이상 발생
- HP 0인데 종료 미발생
- 종료 후 복귀 누락
- 세션 로그가 중간에 끊김
- Draw인데 한쪽만 제거됨
- 타임아웃 종료가 Draw로 기록되지 않음

## 7. 롤백 규칙
- 슬라이스 실패 시 세부 기능(콤보/연출) 추가 금지
- 원인 해소 전 다음 단계 확장 중단

## 8. 다음 단계 연결
- Step 6 보류 항목 결정 순서 확정으로 이동
