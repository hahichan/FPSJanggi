# FPS 장기 팀 작업 연결 가이드

이 문서는 각자 만든 FPS 시스템과 캐릭터를 현재 `nsh` 핵심 시스템에 안전하게 연결하기 위한 팀 공용 안내문입니다.

## 먼저 보는 연결 지도

각 담당자는 `Source/FPSJanggi/NSH`의 핵심 파일 안에 자기 기능을 직접 합쳐 넣는 방식이 아닙니다. 자기 전투 Character/Component/Blueprint를 새로 만들고, 이미 열어 둔 아래 슬롯·함수·이벤트에 연결하면 됩니다.

| 담당 작업 | 새 작업물을 만들 위치 | 현재 핵심 코드와 연결할 위치 |
| --- | --- | --- |
| 공통 FPS 이동·조준·체력·데미지·자가 재생 | `Source/FPSJanggi/Combat`에 새 C++ 클래스/컴포넌트를 만들거나 `/Game/User/Blueprints/Combat`에 공통 Character Blueprint 생성 | 사망 시 캐릭터의 `Get Owner`를 `AuthoritativeJanggiBoard`로 Cast하고 `Report Arena Combatant Defeated(Self)` 호출 |
| 마·상·졸 캐릭터 | `/Game/User/Blueprints/Combat/Pieces` 아래에 공통 Character를 상속한 청·한 Pawn Blueprint 생성 | `MainMap > JanggiBoard1 > FPS Janggi > Arena > Integration`의 Blue/Red Map에서 `Horse`, `Elephant`, `Soldier` 슬롯에 Class 지정 |
| 사·차·포 캐릭터 | `/Game/User/Blueprints/Combat/Pieces` 아래에 공통 Character를 상속한 청·한 Pawn Blueprint 생성 | 같은 Map의 `Guard`, `Chariot`, `Cannon` 슬롯에 Class 지정 |
| 궁 캐릭터 | 같은 위치에 청·한 궁 Pawn Blueprint 생성 | 같은 Map의 `General` 슬롯에 Class 지정 |
| 전투 HUD·유리 균열·효과음·BGM | 기존 `/Game/User/Blueprints/NewPlayerController`의 Event Graph | `Event On Arena Presentation Started`와 `Event On Arena Presentation Ended` 구현 |

`AuthoritativeJanggiBoard`는 연결된 Class를 서버에서 자동 스폰·복제하고 올바른 팀의 `BoardPlayerController`가 자동 Possess합니다. 따라서 캐릭터 담당자가 스폰 코드, 팀 배정 코드, 전투 카메라 코드를 다시 만들 필요가 없습니다.

### 실제 연결 흐름

```text
장기 기물 충돌
→ AuthoritativeJanggiBoard가 보드 정지
→ JanggiBoard1의 Blue/Red Combatant Class Map에서 기물 종류에 맞는 Pawn Class 선택
→ 서버가 청·한 Pawn을 전장 위치에 스폰하고 복제
→ 각 팀 BoardPlayerController가 자기 Pawn을 Possess
→ On Arena Presentation Started로 HUD/BGM 시작
→ 캐릭터의 이동·조준·공격·체력 시스템 동작
→ 패자 Pawn이 Report Arena Combatant Defeated(Self) 호출
→ 핵심 코드가 패자 장기 기물 제거, 승자 이동, 다음 턴 또는 궁 패배 종료 처리
→ On Arena Presentation Ended로 HUD/BGM 정리
→ 기존 팀별 보드 카메라로 복귀
```

### 기존 코드에서 찾아볼 정확한 연결 지점

| 기존 파일/에셋 | 팀원이 사용하는 부분 | 담당자가 할 일 |
| --- | --- | --- |
| `Source/FPSJanggi/NSH/AuthoritativeJanggiBoard.h` | `BlueArenaCombatantClasses`, `RedArenaCombatantClasses` | 구조를 수정하지 않고, 아래 `JanggiBoard1` Details에서 완성된 Pawn Class를 각 Map 슬롯에 지정 |
| `Source/FPSJanggi/NSH/AuthoritativeJanggiBoard.h` | `ReportArenaCombatantDefeated(AActor*)` | Health Component가 서버에서 사망을 확정한 순간 패자 Pawn 자신을 인자로 한 번 호출 |
| `Source/FPSJanggi/NSH/BoardPlayerController.h` | `OnArenaPresentationStarted`, `OnArenaPresentationEnded` | `NewPlayerController` Blueprint에서 전투 HUD·화면 효과·BGM 시작과 정리를 구현 |
| `Content/User/Blueprints/NewPlayerController.uasset` | 위 두 Blueprint Event | UI·음향 담당자만 수정하고 부모를 `BoardPlayerController`로 유지 |
| `Content/User/Map/MainMap.umap`의 `JanggiBoard1` | `FPS Janggi > Arena > Integration` | 최종 통합 담당자 한 명이 Blue/Red Map에 캐릭터 Class를 등록 |
| 각자 새로 만드는 Combat Pawn | Owner와 Possess 상태 | Owner인 `AuthoritativeJanggiBoard`에 패배를 보고하고, 핵심 코드가 지정한 Controller의 Possess를 그대로 사용 |

`AuthoritativeJanggiBoard.cpp`, `BoardPlayerController.cpp`, `BoardGameMode.cpp` 안에 캐릭터별 조건문을 추가하는 방식으로 연결하지 않습니다. 위 표의 Class Map, 패배 보고 함수, Blueprint Event만 연결하면 캐릭터가 핵심 흐름에 들어오도록 구성되어 있습니다.

### 담당자별로 실제로 수정하는 곳

#### 이정환

새로 만드는 것이 기본입니다.

- 공통 Character/Pawn 또는 C++ 기반 클래스
- Health Component
- 이동·조준·공격·자가 재생 컴포넌트 또는 로직
- 공통 Input Action과 Mapping Context

기존 파일 중에서는 `NewPlayerController` Blueprint의 아래 두 이벤트만 UI·음향 연결을 위해 수정합니다.

```text
Event On Arena Presentation Started
Event On Arena Presentation Ended
```

`BoardPlayerController.cpp`, `BoardGameMode.cpp`, `AuthoritativeJanggiBoard.cpp`에 FPS 기능을 직접 넣지 않습니다.

#### 천지성

이정환의 공통 전투 Character를 부모로 두고 다음 6개 Class를 만듭니다.

```text
BP_Horse_Blue_Combatant
BP_Horse_Red_Combatant
BP_Elephant_Blue_Combatant
BP_Elephant_Red_Combatant
BP_Soldier_Blue_Combatant
BP_Soldier_Red_Combatant
```

완성된 Class는 `JanggiBoard1`의 `Horse`, `Elephant`, `Soldier` 슬롯으로 연결합니다. 캐릭터 Blueprint 안에 별도 스폰·Possess·카메라 코드를 만들지 않습니다.

#### 김건우

이정환의 공통 전투 Character를 부모로 두고 다음 6개 Class를 만듭니다.

```text
BP_Guard_Blue_Combatant
BP_Guard_Red_Combatant
BP_Chariot_Blue_Combatant
BP_Chariot_Red_Combatant
BP_Cannon_Blue_Combatant
BP_Cannon_Red_Combatant
```

완성된 Class는 `JanggiBoard1`의 `Guard`, `Chariot`, `Cannon` 슬롯으로 연결합니다. 캐릭터 Blueprint 안에 별도 스폰·Possess·카메라 코드를 만들지 않습니다.

#### 궁 담당자

공통 전투 Character를 부모로 `BP_General_Blue_Combatant`, `BP_General_Red_Combatant`를 만들고 `General` 슬롯에 연결합니다. 궁의 체력이 0이 되어 같은 패배 보고 함수를 호출하면 핵심 코드가 자동으로 경기를 종료합니다.

---

현재 `nsh`에는 다음 기능이 들어 있습니다.

- Steam 대국실 생성·검색·합류 흐름
- 서버 권한 장기 규칙, 턴, 기물 생성·이동·제거
- 진형 선택과 청·한 팀 배정
- 기물 충돌 시 보드 정지, 전장 전환, 결과 반영, 보드 복귀
- 궁 패배 시 경기 종료와 로비 복귀
- 팀별 보드·전투·로비 카메라
- 실제 캐릭터가 아직 연결되지 않은 기물의 임시 캐릭터 fallback

각 담당자는 위 기능을 새로 만들거나 복사하지 말고, 아래 연결 지점에 자기 작업만 추가해 주세요.

---

## 1. 담당 범위

| 담당 | 맡은 작업 | 핵심 시스템에 전달할 결과물 |
| --- | --- | --- |
| 이정환 | FPS 이동·조준·데미지·체력·자가 재생·공용 스킬·전투 HUD·유리 균열·효과음·BGM | 공통 전투 Character/Pawn 기반 클래스, 공통 컴포넌트, 입력과 로컬 UI 연결 |
| 천지성 | 마, 상, 졸 | 청·한 전투 Pawn Blueprint 각 1개씩과 사용 에셋 경로 |
| 김건우 | 사, 차, 포 | 청·한 전투 Pawn Blueprint 각 1개씩과 사용 에셋 경로 |
| 미정 | 궁 | 청·한 궁 전투 Pawn Blueprint와 사용 에셋 경로 |
| 남상현 | 핵심 통합 | 최종 `MainMap` 슬롯 지정, 장기·세션·카메라 회귀 테스트, 충돌 해결 |

캐릭터 담당자는 체력이나 이동을 기물마다 복사하지 않습니다. 이정환 담당의 공통 기반 클래스를 상속하고, 기물별 메시·애니메이션·무기·고유 능력만 구현합니다.

---

## 2. 작업을 시작하는 방법

항상 최신 `origin/nsh`에서 자기 브랜치를 새로 만듭니다.

```powershell
git fetch origin
git switch -c <자기이름>/<작업이름> origin/nsh
```

예시:

```powershell
git switch -c ljh/fps-combat-base origin/nsh
git switch -c cjs/horse-elephant-soldier origin/nsh
git switch -c kkw/guard-chariot-cannon origin/nsh
```

- 다른 친구의 개발 브랜치에서 새 브랜치를 따지 않습니다.
- `nsh`의 장기 핵심 코드를 자기 브랜치 코드로 되돌리지 않습니다.
- `Saved`, `Intermediate`, `DerivedDataCache`, 로컬 빌드 결과는 커밋하지 않습니다.
- 작업 중 `nsh`가 갱신되면 자기 브랜치에 최신 `origin/nsh`를 반영한 뒤 충돌을 해결합니다.

---

## 3. 절대로 교체하면 안 되는 핵심 축

다음 항목은 장기·멀티플레이·카메라가 연결된 핵심 축입니다.

- GameMode의 Player Controller Class: `BoardPlayerController`
- `NewPlayerController`의 부모 클래스: `BoardPlayerController`
- 장기 규칙과 전투 결과 처리: `AuthoritativeJanggiBoard`
- 방 생성·합류·로비 복귀: `SessionSubsystem`
- `MainMap`의 `JanggiBoard1`, 팀별 보드 카메라, 로비 카메라

새 PlayerController나 새 GameMode로 통째로 교체하면 팀 배정, 클릭 RPC, 카메라, 로비 복귀가 끊깁니다. FPS 이동·조준·체력은 PlayerController를 교체하지 말고 실제 전투 `Character/Pawn`과 Actor Component에 구현합니다.

핵심 코드에 새 기능이 필요하면 같은 기능을 별도로 만들지 말고 남상현에게 필요한 이벤트나 함수 하나를 요청해 주세요.

---

## 4. 공통 FPS 시스템 담당자 안내

### 4.1 권장 구조

공통 전투 캐릭터 하나와 기능별 컴포넌트를 만든 뒤 모든 기물 캐릭터가 이를 상속하도록 합니다.

예시 구조:

```text
BP_CombatCharacterBase
├─ HealthComponent
├─ Damage/Hit 처리
├─ 기본 이동과 조준
├─ 공용 자가 재생
└─ 공용 입력과 피격 알림

BP_Horse_Blue_Combatant      -> BP_CombatCharacterBase 상속
BP_Horse_Red_Combatant       -> BP_CombatCharacterBase 상속
BP_Cannon_Blue_Combatant     -> BP_CombatCharacterBase 상속
...
```

C++로 만들 경우에도 같은 역할 분리를 사용합니다.

- Character/Pawn: 이동, 조준, 입력, 소유 관계
- Health Component: 현재/최대 체력, 데미지, 사망, 재생
- 기물 자식 Blueprint: 메시, 애니메이션, 무기, 기물별 능력
- PlayerController Blueprint: 로컬 HUD, 화면 효과, BGM/SFX 전환

### 4.2 서버 권한 규칙

- 체력, 데미지, 재생, 스킬 성공 판정은 서버에서 변경합니다.
- 클라이언트는 공격이나 스킬 사용을 서버 RPC로 요청합니다.
- 서버가 사거리, 쿨다운, 현재 전투 상태를 확인한 뒤 적용합니다.
- 체력은 Replicated/RepNotify로 양쪽 화면에 동기화합니다.
- 피격 이펙트·사운드·나이아가라만 Multicast 또는 RepNotify 결과로 재생합니다.
- 자가 재생은 매 프레임 Tick보다 서버 Timer를 사용합니다.

### 4.3 체력이 0이 되었을 때

전투 캐릭터의 Owner는 `JanggiBoard1`입니다. 서버에서 다음 순서로 한 번만 호출합니다.

```text
Get Owner
→ Cast To AuthoritativeJanggiBoard
→ Report Arena Combatant Defeated
   Defeated Combatant = Self
```

`Report Arena Combatant Defeated`가 장기판의 승자 이동, 패자 제거, 다음 턴, 궁 패배 종료를 처리합니다.

캐릭터 코드에서 다음 작업을 직접 하면 안 됩니다.

- 장기판 위 기물 Destroy
- 장기 기물 좌표 변경
- 강제로 다음 턴 시작
- 임의로 보드 카메라 복귀
- `ResolveArenaBattle` 중복 호출

Actor를 서버에서 Destroy해도 패배를 자동 감지하지만, 명시적으로 `Report Arena Combatant Defeated`를 호출하는 방식을 기본으로 사용합니다. 중복 사망 처리를 막기 위해 `bIsDead` 같은 서버 상태를 두세요.

### 4.4 입력과 Possess

전투가 시작되면 핵심 시스템이 청·한 캐릭터를 생성하고 해당 팀의 `BoardPlayerController`가 자동 Possess합니다.

- 캐릭터가 스스로 PlayerController를 생성하거나 교체하지 않습니다.
- Pawn의 BeginPlay/OnPossessed에서 보드 또는 전투 카메라를 강제로 지정하지 않습니다.
- Enhanced Input Mapping Context는 로컬 소유 Pawn에만 추가합니다.
- UnPossessed/EndPlay에서 추가했던 입력 Context와 임시 상태를 정리합니다.

---

## 5. 캐릭터 담당자 안내

### 5.1 필요한 결과물

기물 하나당 청·한용 Pawn/Character Blueprint를 준비합니다.

예시:

```text
BP_Horse_Blue_Combatant
BP_Horse_Red_Combatant
BP_Elephant_Blue_Combatant
BP_Elephant_Red_Combatant
BP_Soldier_Blue_Combatant
BP_Soldier_Red_Combatant
```

청·한이 메시나 재질만 다르면 공통 부모 Blueprint를 하나 두고 팀별 자식을 만드는 것을 권장합니다.

각 Blueprint는 다음을 확인해야 합니다.

- 부모가 `Pawn`, `Character` 또는 팀 공통 전투 기반 클래스인지
- `Replicates` 활성화
- 이동을 사용하면 `Replicate Movement` 활성화
- 무기와 나이아가라가 서버 전용 객체를 직접 참조하지 않는지
- BeginPlay에 테스트용 Auto Possess, 고정 카메라, 임의 위치 이동이 남아 있지 않은지
- 캐릭터 단독 테스트용 스폰 Actor가 `MainMap`에 남아 있지 않은지

### 5.2 자동으로 받는 정보

스폰된 캐릭터에는 다음 Actor Tag가 붙습니다.

- 공통: `ArenaCombatant`
- 팀: `ArenaBlue` 또는 `ArenaRed`
- 종류: `ArenaPiece_Horse`, `ArenaPiece_Elephant`, `ArenaPiece_Soldier`, `ArenaPiece_Guard`, `ArenaPiece_Chariot`, `ArenaPiece_Cannon`, `ArenaPiece_General`

팀별 재질·UI·능력 초기화에는 이 태그 또는 소유 중인 `BoardPlayerController`의 `Get Assigned Board Team`을 사용합니다. 액터 이름이나 스폰 순서로 팀을 추측하지 않습니다.

태그는 핵심 시스템이 스폰을 마친 뒤 지정하므로 Pawn의 최초 `BeginPlay`보다 늦을 수 있습니다. 태그가 필요한 초기화는 `OnPossessed` 이후나 별도의 초기화 이벤트에서 수행하고, 최초 `BeginPlay` 한 번만 보고 팀을 확정하지 않습니다.

### 5.3 최종 슬롯 연결

최종 통합 담당자가 PIE를 종료한 뒤 다음 위치에서 연결합니다.

```text
MainMap
→ World Outliner
→ JanggiBoard1
→ Details
→ FPS Janggi / Arena / Integration
```

여기에는 두 Map이 있습니다.

- `Blue Arena Combatant Classes`
- `Red Arena Combatant Classes`

각 Map에 아래 키와 완성된 Pawn Blueprint Class를 지정합니다.

| 장기 기물 | 슬롯 키 |
| --- | --- |
| 졸/병 | `Soldier` |
| 포 | `Cannon` |
| 차 | `Chariot` |
| 상 | `Elephant` |
| 궁/왕 | `General` |
| 사 | `Guard` |
| 마 | `Horse` |

슬롯이 비어 있으면 해당 기물은 기존 임시 캐릭터로 동작합니다. 모든 캐릭터가 완성되기 전에도 다른 기물 테스트가 가능합니다.

### 5.4 MainMap 충돌 방지

`MainMap.umap`은 텍스트 병합이 불가능한 바이너리 파일입니다. 캐릭터 담당자 여러 명이 각자 슬롯을 지정해 커밋하지 마세요.

캐릭터 담당자는 다음 정보만 통합 담당자에게 전달합니다.

```text
기물: Horse
Blue Class: /Game/.../BP_Horse_Blue_Combatant
Red Class: /Game/.../BP_Horse_Red_Combatant
부모 클래스: BP_CombatCharacterBase
필요 입력/컴포넌트: ...
```

최종 `MainMap` 슬롯 지정과 저장은 한 사람이 한 번에 수행합니다.

---

## 6. HUD·유리 균열·음향 담당자 안내

`NewPlayerController`는 `BoardPlayerController`의 자식으로 유지하고 다음 Blueprint Event를 구현합니다.

### On Arena Presentation Started

입력: `Local Team`

- 전투 HUD 생성 또는 표시
- 내 Pawn의 Health Component 연결
- 전투 BGM 시작
- 전장 진입 SFX 재생
- 필요한 경우 전투용 입력 모드 적용

### On Arena Presentation Ended

입력: `Local Team`

- 전투 HUD 제거
- 피격 유리 균열 초기화
- 전투 BGM과 반복 SFX 정지
- 보드 BGM 복귀
- 임시 입력 Context와 위젯 참조 정리

주의 사항:

- 위젯·화면 균열·2D 사운드는 로컬 컨트롤러에서만 생성합니다.
- 체력 수치는 서버가 결정하고 UI는 복제된 결과만 표시합니다.
- Event BeginPlay에서 전투 HUD를 무조건 띄우지 않습니다. 로비와 보드 단계에서도 같은 PlayerController를 사용합니다.
- 캐릭터가 바뀔 때 기존 Health Component Delegate 바인딩을 해제합니다.
- `NewPlayerController.uasset`도 바이너리이므로 UI 담당자 한 명만 수정하는 것을 권장합니다.

---

## 7. 제출 전 테스트

### 캐릭터 하나당 필수 확인

1. 플레이 인원을 `2`, Net Mode를 `Play As Listen Server`로 실행합니다.
2. 양쪽에서 진형을 선택해 기물 32개가 모두 보이는지 확인합니다.
3. 해당 기물이 충돌하도록 이동해 전장으로 들어갑니다.
4. 청·한 캐릭터가 각각 한 번만 생성되는지 확인합니다.
5. 두 플레이어가 서로 다른 자기 팀 캐릭터를 조작하는지 확인합니다.
6. 이동, 조준, 공격, 데미지, 체력이 양쪽에서 동일하게 보이는지 확인합니다.
7. 체력이 0이 되면 패자 캐릭터와 해당 장기 기물만 제거되는지 확인합니다.
8. 승자 장기 기물이 목적지로 이동하고 다음 턴이 시작되는지 확인합니다.
9. 양쪽 카메라가 자기 진영의 원래 보드 카메라로 돌아오는지 확인합니다.
10. 같은 과정을 두 번 반복해 이전 Pawn, HUD, BGM, 입력이 남지 않는지 확인합니다.
11. 궁은 패배 후 다음 턴이 아니라 경기 종료 화면과 로비 복귀로 이어지는지 확인합니다.
12. 슬롯을 잠시 비웠을 때 기존 임시 캐릭터 fallback도 정상인지 확인합니다.

### C++를 수정한 경우

에디터를 닫고 두 타깃을 모두 빌드합니다.

```powershell
& 'C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat' FPSJanggiEditor Win64 Development 'C:\gege\FPSJanggi.uproject' -WaitMutex -NoHotReload -NoUBA
& 'C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat' FPSJanggi Win64 Development 'C:\gege\FPSJanggi.uproject' -WaitMutex -NoHotReload -NoUBA
```

핵심 시스템을 건드린 경우 다음 자동 검사도 실행합니다.

```powershell
& '.\.agents\skills\fpsjanggi-multiplayer-regression\scripts\run_standalone_lobby_smoke.ps1'
& '.\.agents\skills\fpsjanggi-multiplayer-regression\scripts\run_two_player_smoke.ps1'
& '.\.agents\skills\fpsjanggi-multiplayer-regression\scripts\run_two_player_smoke.ps1' -Arena
```

---

## 8. 통합 담당자에게 전달할 내용

작업을 넘길 때 아래 양식을 채워 주세요.

```text
[담당 작업]
- 담당자:
- 기물 또는 시스템:

[에셋 경로]
- Blue Pawn Class:
- Red Pawn Class:
- 공통 부모/컴포넌트:

[네트워크]
- 서버 RPC:
- Replicated/RepNotify 변수:
- Multicast 또는 로컬 이펙트:

[입력과 UI]
- 사용 Input Action / Mapping Context:
- 생성하는 Widget:
- 정리 시점:

[테스트]
- 2인 Listen Server 확인:
- 승패 후 보드 복귀 확인:
- 반복 전투 확인:
- 남은 문제:
```

완성 기준은 “내 화면에서 캐릭터가 움직인다”가 아니라, 서버와 클라이언트 양쪽에서 같은 체력·위치·승패를 보고 전투 종료 후 장기판으로 정상 복귀하는 것입니다.
