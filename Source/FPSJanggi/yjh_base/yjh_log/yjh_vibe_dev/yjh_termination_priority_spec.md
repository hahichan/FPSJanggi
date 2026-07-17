# YJH 종료 우선순위 명세서 (Draft)

## 0. 목적
- 종료 사유가 동시에 발생할 때 판정 충돌을 방지한다.

## 1. 우선순위(고정)
1. ForcedAbort(강제중단)
2. DrawBySameTickBatch(동시사망 비김)
3. DrawByTimeout(타임아웃 비김)

## 2. 적용 규칙
- 서버는 같은 판정 구간에서 종료 후보를 수집한 뒤 우선순위 1회 적용
- FinalizeOnce 규칙에 따라 최종 종료는 1회만 확정

판정 구간 고정 규칙
- 판정 구간은 서버 틱 단위이며, 해당 틱에서 수집된 후보만 비교한다.
- 틱 경계 이후 도착한 후보는 다음 구간으로 이월하지 않고 LateCandidate 로그로만 남긴다.
- 우선순위 적용 후에는 즉시 FinalizeOnce 원자 커밋을 시도한다.
- 원자 커밋 실패 시 종료 결과를 덮어쓰지 않는다.

## 3. 결과 매핑
- ForcedAbort -> EndReason=ForcedAbort, ResultType=Draw, RemovePolicy=BothEliminated
- DrawBySameTickBatch -> EndReason=SimultaneousDeath, ResultType=Draw, RemovePolicy=BothEliminated
- DrawByTimeout -> EndReason=Timeout, ResultType=Draw, RemovePolicy=BothEliminated

## 4. 로그 키
- YJH_TERM_PRIORITY_CANDIDATES
- YJH_TERM_PRIORITY_APPLIED
- YJH_TERM_FINALIZED
