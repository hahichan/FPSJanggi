# YJH 실제 개발 준비물 체크리스트

## 0. 목적
- 계획서 외에 실제 구현/검증에 필요한 작업물을 체크한다.

사전 게이트
- 구현 시작 전 yjh_vibe_coding_preflight_plan.md의 Gate A/B/C 통과 여부를 먼저 체크한다.

## 1. 코드 작업물
- yjh_ 네이밍 규칙 기반 클래스/컴포넌트 파일
- 공통 로그 매크로 또는 태그 상수
- 서버 권한 검증 유틸(중복 종료 방지 포함)

## 2. 데이터/설정 작업물
- 전투 기본 수치(DataAsset 또는 설정값)
- 기본 입력 매핑(공격/회피/상호작용)
- 테스트용 스폰 포인트/전투 시작 트리거
- 파생 캐릭터 템플릿용 설정 프리셋(애니메이션/능력치/스킬 슬롯)
- 런타임 스킬 요청 컨텍스트 명세(yjh_runtime_skill_request_context_spec.md)
- 핸드오프 단일 기준서(yjh_handoff_single_source_of_truth.md)

## 3. 디버그 작업물
- 콘솔 명령 세트(yjh.start, yjh.abort, yjh.dumpstate)
- 상태 오버레이(UI 텍스트)로 MatchId/State 표시
- 서버/클라 로그 필터 프리셋
- 디버그 관측 포인트 기준서(yjh_debug_observability_spec.md)
- 전투 HUD 명세서(yjh_ui_hud_spec.md)
- UI 데이터 바인딩 계약서(yjh_ui_data_binding_contract.md)
- UI 바인딩 Delta 템플릿(yjh_ui_binding_delta_template.md)

## 4. 테스트 작업물
- 2인 PIE 반복 테스트 절차 문서
- 재현 템플릿(증상, 재현 순서, 기대/실제)
- 회귀 체크리스트(시작, 전투, 종료, 복귀)
- 파생 캐릭터 온보딩 테스트(새 캐릭터 1종 추가 후 즉시 전투 가능 여부)
- 책임 경계 테스트 명세서(yjh_boundary_test_spec.md)
- 최소 수직 슬라이스 명세서(yjh_min_vertical_slice_spec.md)

## 5. 빌드/운영 작업물
- Win64 Development 빌드 확인 루틴
- Editor/Game 타깃 빌드 순서 고정
- 실패 시 즉시 확인할 로그 경로 메모

## 6. 필수 산출물 최소 세트(MVP)
1. 게임모드 코드 뼈대
2. 베이스 캐릭터 + Health 컴포넌트
3. 통합 계약 함수 연결
4. 2인 테스트 절차 문서
5. 회귀 체크리스트
6. 파생 캐릭터 제작 가이드(비개발자 포함)

## 7. 확장성/인수인계 필수 항목(중요)
- 베이스 캐릭터 확장 포인트 목록: 오버라이드 함수, BlueprintImplementableEvent, DataAsset 키
- 금지 항목 목록: Base 클래스 하드코딩(특정 캐릭터 전용 로직)
- 샘플 캐릭터 1종(yjh_ 접두어)과 생성 절차 문서
- 새 캐릭터 추가시 수정해야 할 파일을 3개 이하로 제한
- 기준 문서: yjh_extensibility_handoff_spec.md

## 8. 바로 다음 액션
1. 계획서 3종 기준으로 Step A 계약 표 확정
2. 코드 파일 스켈레톤 생성(yjh_ 접두어)
3. 디버그 명령 1차 세트 적용

## 9. 보류 항목 관리
- 보류 항목 결정 순서표: yjh_deferred_decision_order.md
