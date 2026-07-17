# YJH 작업 로그 (4)

- 2026-07-17: `AAuthoritativeJanggiBoard::SpawnTemporaryArenaCombatant`에 팀/기물 기반 메시 주입을 추가하고, 보드 기물 타입 해석 실패(`None`) 시 Soldier 메시로 폴백하도록 보강. ArenaBattleReal에서 상대가 보이지 않던 임시 test 소환 경로를 교정.
- 2026-07-17: 임시 test 소환 경로에서 클래스 기본 메시(사용자 설정 에셋)를 우선 유지하도록 수정. 메시가 비어 있는 경우에만 팀/기물 기반 폴백 메시를 적용해 사용자 커스텀 에셋이 덮어써지지 않게 조정.
- 2026-07-17: Arena 전투 캐릭터 스폰에서 하드코딩 메시 경로(`GetArenaCharacterMeshPath`)를 제거. 이제 임시/일반 소환 모두 클래스에 설정된 메시만 사용하며, 메시 미설정 상태는 경고 로그로만 노출.
- 2026-07-17: `YJHArenaDebugGameMode`에서 기본값으로 `bDisableTemporaryTestCombatants=true`를 적용해 디버그 실행 시 `bUseTemporaryTestArenaCombatants`를 자동 비활성화. 기물별 매핑 클래스(CDO) 메시 설정 상태를 로그(`YJH_DEBUG_ARENA_CLASS`)로 출력하도록 추가.
- 2026-07-17: `YJHArenaDebugGameMode`의 보드 protected 멤버 직접 접근을 제거하고 `AAuthoritativeJanggiBoard` 공개 API(`IsUsingTemporaryTestArenaCombatants`, `SetUseTemporaryTestArenaCombatants`, `LogArenaCombatantClassMeshState`)를 통해 제어하도록 정리. 컴파일 오류(C2248) 해소.
- 2026-07-17: 런타임 로그 확인 결과 `YJHArenaCombatantTemplate_Blue/Red` CDO 메시가 `None`이라 디버그 강제 전환 시 보이지 않는 문제가 재현됨. `YJHArenaDebugGameMode` 기본값을 `bDisableTemporaryTestCombatants=false`로 조정해 사용자 설정 test 클래스 경로를 기본 유지.
- 2026-07-17: `YJHArenaDebugGameMode`에 `ArenaCombatantMode` 선택(enum) 추가. 디버그 실행 시 `KeepBoardSetting / ForceTemporaryTest / ForcePieceMapped` 중 선택 가능하게 하여 ArenaBattleReal 진입 전에 전투원 소스를 명시적으로 결정.
- 2026-07-17: 디버그 전용 `ArenaDebugWidget`에 `/Game/User/piece` Pawn Blueprint 목록(Blue/Red 콤보 선택) 추가. Start 시 선택 클래스 경로를 서버 RPC로 전달해 `AuthoritativeJanggiBoard::SetTemporaryTestArenaCombatantClasses`를 적용한 뒤 ArenaBattleReal 시작. `YJHArenaDebugGameMode` 기본 `bAutoStartArenaReal=false`로 수동 선택 시작 대기.
- 2026-07-17: 근본 원인 보강 - 선택 클래스 적용 타이밍이 늦어 이미 스폰된 전투원에 반영되지 않던 문제 수정. `ApplyTemporaryClassesToActiveArenaSession`를 추가해 ArenaBattle 활성 세션에서도 선택 직후 전투원을 재스폰/재빙의/세션초기화 후 ArenaReal로 진입하도록 변경.
- 2026-07-17: 스킬 슬롯 매핑 안정화 - `FYJHSkillDefinition`에 고정 슬롯 enum(`InputSlotEnum`: Slot1~Slot10) 추가. 데이터 에셋은 enum 우선 매핑, 기존 문자열 `InputSlot`은 하위호환(Deprecated)으로 유지. 유효 슬롯 범위를 벗어난 문자열은 `SKV_InvalidSlotMapping`으로 거절하도록 강화.
- 2026-07-17: 입력/표시 보강 - `BoardPlayerController::InputKey`가 Arena 중 좌클릭을 보드 클릭으로 선점하던 경로를 BoardTurn 전용으로 제한해 전투 Pawn Enhanced Input이 수신되도록 수정. ArenaBattleReal 시작 시 `ArenaDebugWidget` 제거 + `ArenaCombatStatusWidget`(HP/Slot1~10 쿨타임 텍스트) 생성/주기 갱신 추가.
- 2026-07-17: SkillData 에셋 편집 UX 보강 - `InputSlotEnum` 표시명을 `Input Slot`으로 통일하고 legacy 문자열 슬롯은 `AdvancedDisplay`로 축소. 또한 슬롯 번호가 `MaxSkillSlots`를 넘으면 `SKV_SlotOutOfRange` 오류를 반환하도록 검증 추가.
