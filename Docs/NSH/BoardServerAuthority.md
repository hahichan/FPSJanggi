# 장기판 서버 권한화

## 현재 구조

`NewPlayerController`의 좌클릭은 기존 Blueprint `Click` 함수를 직접 호출하지 않는다.

1. 로컬 플레이어가 보드 좌표를 계산한다.
2. 소유 중인 `ABoardPlayerController`가 `ServerRequestBoardClick` RPC를 보낸다.
3. 서버의 `AAuthoritativeJanggiBoard`가 요청을 검증한다.
4. 서버가 C++ 행마법으로 선택과 이동을 판정하고 승인된 좌표만 명령 기록에 추가한다.
5. 서버가 기물 Actor를 직접 이동·제거한다. 클라이언트와 늦게 참가한 플레이어는 복제된 Actor 위치와 보드 상태를 수신한다.

main의 기존 `JanggiBoard1` 표현용 그래프는 유지하지만, 멀티플레이 규칙 판정과 기물 상태 변경에는 사용하지 않는다.

## 서버 검증

- 요청자가 현재 월드의 유효한 `ABoardPlayerController`인지 확인
- 첫 번째 접속자를 Blue, 두 번째 접속자를 Red로 배정하고 그 이후 접속자는 Unassigned 처리
- 현재 턴과 요청자의 팀이 같은지 확인
- 기물 선택 단계에서 선택한 `PieceMesh`가 요청자 팀의 기물인지 확인
- 이동 단계에서 C++ 행마법과 길막을 검증한 뒤에만 이동 또는 아레나 전환
- 입력 일시 정지 상태 확인
- 9×10 장기판 범위 확인 및 100 단위 셀 중심으로 좌표 정규화
- 컨트롤러별 0.08초 RPC 간격 제한
- 한 경기의 복제 명령 기록을 1,024개로 제한

## 기존 Blueprint에서 발견한 문제

`CheckChariatBlue`, `CheckChariatRed`, `CheckCannonBlue`, `CheckCannonRed`의 왼쪽 탐색은
`index % 9 != 8`만 확인하고 `index >= 0`을 확인하지 않는다. 0열을 지나면 음수 인덱스로 계속
진행해 Blueprint runaway loop가 발생할 수 있다.

네 함수에 `point >= 0 AND point % 9 != 8` 조건을 삽입해 Blueprint를 다시 컴파일하고 저장했다.
수정에 사용한 일회성 편집기 도구는 소스에서 제거했다.

## 검증 상태

- 완료: 기존 Blueprint 부모 클래스 연결 및 에디터 저장
- 완료: MainMap 검사 오류 0, 경고 0 및 단일 복제 보드 확인
- 완료: 차 `(0,0)` 및 포 `(2,1)` 클릭 후 runaway loop 없음
- 완료: UE 5.8 Editor 및 Win64 Development Game 타깃 빌드
- 완료: listen server + client에서 Blue/Red 팀 배정 확인
- 완료: Red가 Blue 턴에 보낸 요청 거절 및 명령 수 0 유지 확인
- 완료: Blue 병 `(3,0) -> (4,0)` 이동 후 명령 수 2, Red 턴 전환 확인
- 완료: Blue 호스트의 이동 결과가 Red 클라이언트의 `ROLE_SimulatedProxy` 기물에 복제됨
- 완료: 이동 후 참가한 클라이언트가 기물 위치, 명령 2개, Red 턴으로 동기화됨

## 별도 확인 사항

- 로컬 네트워크 권한 테스트는 `-nosteam` IP NetDriver로 수행했다. Steam 세션 생성/검색과 실제 Steam 전송 검사는 별도 항목이다.
- `/Game/User/Asset/FloorCircle`이 존재하지 않는 `/Game/Material/M_Floot`을 참조한다는 기존 로드 경고가 있다. 장기판 권한 테스트에는 영향을 주지 않았지만 패키징 전 참조 수정이 필요하다.
