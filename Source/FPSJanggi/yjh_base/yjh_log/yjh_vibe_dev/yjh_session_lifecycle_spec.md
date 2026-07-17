# YJH 세션 생명주기 명세서 (Draft)

## 0. 의미
- 세션 생명주기는 전투 세션이 생성되어 종료/정리될 때까지의 상태, 시간 기준, 이벤트 흐름을 뜻한다.

## 1. 시간 기준(고정)
- 판정 시간은 서버 시계만 사용
- 클라이언트 표시는 서버 기준 시간을 동기화해 보여줌
- 클라이언트 로컬 시간은 UI 참고용, 판정 근거로 사용 금지

## 2. 상태 흐름
1. SessionCreated
2. WaitingCombatantsReady
3. CombatRunning
4. ResolvePending
5. Finalized
6. ReturnedToBoard

## 3. 핵심 타임스탬프
- T0CreatedAtServer
- T1CombatStartAtServer
- T2FinalizeAtServer
- T3ReturnToBoardAtServer

## 4. 타임아웃 계산
- TimeoutDeadlineAtServer = T1CombatStartAtServer + EffectiveTimeoutSeconds
- EffectiveTimeoutSeconds = PieceTimeoutOverrideSeconds or TimeoutDefaultSeconds(60)

타임아웃/유예 겹침 규칙
- Disconnect Grace는 TimeoutDeadlineAtServer를 연장하지 않는다.
- 판정은 항상 TimeoutDeadlineAtServer >= NowServer 조건으로만 수행한다.
- 같은 서버 틱에서 Timeout과 ForcedAbort 후보가 동시에 발생하면 종료 우선순위 명세를 따른다.

## 5. 검증 항목
- 모든 Finalize 이벤트에 T2가 기록되는지
- Timeout 판정 시 Deadline 비교가 서버 시계 기준인지
- Grace 중 Timeout 발생 케이스에서 우선순위 적용 로그가 1회만 기록되는지
