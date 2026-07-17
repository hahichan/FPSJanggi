# YJH 작업 로그

## 핵심 요약 (1번째 작업 ~ 4번째 작업)

1. 1번째 작업: YJH 테스트 진입 뼈대 추가
- yjh_ 서브 상태 머신 추가: None -> ArenaBattle -> ArenaBattleReal
- ArenaDebugWidget에 YJH Arena Start 버튼 추가
- 테스트 진입 경로 3개 확보: UI 버튼, F8, 콘솔(yjh.startarenareal)
- 현재는 스텁 동작만 구현: 성공 문구 출력 후 1초 뒤 ArenaBattle로 복귀
- Shipping에서는 테스트 UI/경로 비활성

2. 2번째 작업: 컴파일 에러 수정
- 원인: 콘솔 커맨드 델리게이트 시그니처 불일치(C2672 Invoke)
- 조치: 람다 시그니처를 (UWorld*) 형태로 수정
- 결과: 코드 컴파일 오류 해결

3. 3번째 작업: 버튼 미노출 보강
- 원인 후보: 아레나 전환 과정의 UI 재구성으로 버튼 위젯이 사라질 수 있음
- 조치 1: ArenaTransition/ArenaBattle/BattleResolution에서 위젯 없으면 재생성
- 조치 2: ArenaDebugWidget z-order를 120 -> 1000으로 상향
- 결과: 아레나 페이즈 중 버튼 복구/노출 안정성 개선

4. 4번째 작업: 빌드 검증 완료
- FPSJanggi Win64 Development: 성공
- FPSJanggiEditor Win64 Development: 성공

### 추후 수정 시 빠른 체크 포인트
- "기물 충돌 · 전투 진행 중" 문구는 ArenaTransition/ArenaBattle/BattleResolution 공통 문구임
- 버튼이 안 보이면 Output Log에서 BOARD_ARENA_DEBUG_UI_READY 로그 유무 먼저 확인
- Editor 빌드 실패 시 UnrealEditor-FPSJanggi.dll 파일 점유(LNK1104) 여부 확인
- 현재 YJH Arena Start는 전투 로직이 아닌 테스트 스텁 경로임

## 2026-07-17

### 작업 번호
- 1번째 작업

### 진행 범위
- NSH ArenaBattle 내부에서 동작하는 YJH 서브 상태 머신(테스트 스텁) 뼈대 추가
- ArenaDebugWidget(전투 결과 판정 UI)에 YJH Arena Start 버튼 추가
- 테스트 진입 경로 A+B 추가
  - A: UI 버튼
  - B: F8 키 + 콘솔 명령(yjh.startarenareal)

### 수행 작업
1. yjh_ 접두어 상태 머신 코드 추가
- Source/FPSJanggi/yjh_base/yjh_ArenaSubStateMachine.h
- Source/FPSJanggi/yjh_base/yjh_ArenaSubStateMachine.cpp
- 상태: None, ArenaBattle, ArenaBattleReal
- 기능: ArenaBattle 진입/Real 진입 시도/1초 후 ArenaBattle 복귀 지원

2. BoardPlayerController 확장
- RequestYJHArenaStart 요청 경로 추가
- ServerRequestYJHArenaStart RPC 추가
- ClientRunYJHArenaStartStub RPC 추가
- HandleYJHArenaAutoReturn 타이머 복귀 처리 추가
- F8 입력으로 YJH 시작 요청 추가(Shipping 제외)
- Exec 함수 YJHStartArenaReal 추가(Shipping 제외)
- 콘솔 명령 yjh.startarenareal 등록(Shipping 제외)

3. ArenaDebugWidget 확장
- 버튼 라벨: YJH Arena Start
- 클릭 시 BoardPlayerController::RequestYJHArenaStart 호출
- 기존 청/한 승리 버튼은 유지

### 현재 상태
- 내부 전투 로직은 아직 미구현(요청대로 스텁만 구성)
- 전환 성공 텍스트 표시 후 1초 뒤 ArenaBattle UI 상태로 복귀하도록 구성
- 테스트 버튼/경로는 Shipping 빌드에서 제외

## 2026-07-17

### 작업 번호
- 2번째 작업

### 진행 범위
- 컴파일 에러 원인 분석 및 수정
- 게임 타깃 빌드 재검증

### 수행 작업
1. BoardPlayerController 콘솔 커맨드 델리게이트 시그니처 수정
- 파일: Source/FPSJanggi/NSH/BoardPlayerController.cpp
- 수정 내용: FConsoleCommandWithWorldDelegate 람다를 (const TArray<FString>&, UWorld*)에서 (UWorld*)로 변경

2. 빌드 확인
- FPSJanggiEditor: 컴파일 단계 통과, 링크 단계에서 UnrealEditor-FPSJanggi.dll 파일 잠금으로 실패
- FPSJanggi: Win64 Development 빌드 성공

### 현재 상태
- 사용자 보고 컴파일 에러(C2672 Invoke) 해결
- 남은 이슈는 Editor 실행 중 DLL 점유로 인한 링크 실패(LNK1104)

## 2026-07-17

### 작업 번호
- 3번째 작업

### 진행 범위
- ArenaBattle 중 디버그 버튼 미노출 문제 보강

### 수행 작업
1. 아레나 페이즈에서 버튼 위젯 자동 재보장
- 파일: Source/FPSJanggi/NSH/BoardPlayerController.cpp
- 내용: RefreshBoardStatus에서 ArenaTransition/ArenaBattle/BattleResolution 상태일 때 ArenaDebugWidget이 없으면 CreateArenaDebugWidget 재호출

2. 버튼 가시성 우선순위 상향
- 파일: Source/FPSJanggi/NSH/BoardPlayerController.cpp
- 내용: ArenaDebugWidget AddToViewport 우선순위를 120에서 1000으로 상향

### 현재 상태
- 아레나 페이즈 진입 후 다른 UI에 가려지거나 제거되어도 주기적 갱신에서 버튼 재생성

## 2026-07-17

### 작업 번호
- 4번째 작업

### 진행 범위
- 변경사항 빌드 검증

### 수행 작업
1. FPSJanggi Win64 Development 빌드 실행
- 결과: 성공

2. FPSJanggiEditor Win64 Development 빌드 실행
- 결과: 성공

### 현재 상태
- 게임/에디터 타깃 모두 빌드 성공 확인
