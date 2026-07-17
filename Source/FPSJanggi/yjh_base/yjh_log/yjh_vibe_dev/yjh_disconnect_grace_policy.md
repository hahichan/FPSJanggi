# YJH 이탈/재접속 유예 정책서 (Draft)

## 0. 목적
- 일시 네트워크 불안정에서 즉시 종료를 피하고 공정한 판정을 유지한다.

## 1. 기본 정책
- 단일 이탈 발생 시 GraceSeconds 동안 유예
- GraceSeconds 기본값: 8초(설정값으로 변경 가능)
- 유예 중 해당 캐릭터는 입력 정지 상태로 유지

## 2. 유예 종료 처리
- 유예 내 복귀 성공: 세션 지속
- 유예 초과 미복귀: ForcedAbort 후보 추가
- 양측 동시 이탈: Grace 만료 후 ForcedAbort로 종료

## 3. 타임아웃과 관계
- 유예 중에도 전투 타임아웃 카운트는 서버 정책에 따라 지속(초안 기본: 지속)
- 종료 우선순위는 yjh_termination_priority_spec.md 적용

겹침 상황 고정 규칙
- GraceSeconds 진행 중에도 TimeoutDeadlineAtServer 비교는 계속 수행한다.
- 같은 서버 틱에서 Grace 만료(ForcedAbort 후보)와 Timeout 후보가 동시에 성립하면 ForcedAbort를 우선 적용한다.
- 같은 서버 틱 동시 성립 여부는 서버 시계와 틱 경계만으로 판단한다.

## 4. 로그 키
- YJH_NET_DISCONNECT_DETECTED
- YJH_NET_GRACE_STARTED
- YJH_NET_RECONNECTED
- YJH_NET_GRACE_EXPIRED
