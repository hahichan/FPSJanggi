// Fill out your copyright notice in the Description page of Project Settings.


#include "ArenaGameMode.h"
#include "BaseJanggiPiece.h"
#include "JanggiGameInstance.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/PlayerStart.h"
#include "Engine/TargetPoint.h"
#include "GameFramework/Pawn.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Engine/World.h"

AArenaGameMode::AArenaGameMode()
{
	CountdownDuration = 3.0f;
	PostMatchDelay = 4.0f;
	BoardLevelMapPath = TEXT("/Game/Maps/BoardLevel");
	AttackerPawnClass = nullptr;
	DefenderPawnClass = nullptr;
	TestPieceClassA = nullptr;
	TestPieceClassB = nullptr;
	ArenaSpawnTag = FName(TEXT("ArenaSpawn"));
	CurrentMatchPhase = EArenaMatchPhase::WaitingForPlayers;
	ManualSpawnRetryInterval = 0.25f;

	// 테스트 단계에서는 직접 SpawnActor + Possess를 사용하므로 기본 자동 폰 스폰은 비활성화
	DefaultPawnClass = nullptr;

	bHasSpawnedPlayerPawns = false;
	AttackerController = nullptr;
	DefenderController = nullptr;
	AttackerPiece = nullptr;
	DefenderPiece = nullptr;
}

void AArenaGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	// 레벨 로드 초기 단계에서 GameInstance의 전투 요청 데이터 읽기
	LoadDuelRequestFromInstance();
}

void AArenaGameMode::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Log, TEXT("[AArenaGameMode] BeginPlay - DefaultPawnClass=%s (manual spawn mode)"), *GetNameSafe(DefaultPawnClass));
	if (IsValid(DefaultPawnClass))
	{
		UE_LOG(LogTemp, Warning, TEXT("[AArenaGameMode] DefaultPawnClass is set (%s). This can spawn engine default pawns before manual player spawn."), *GetNameSafe(DefaultPawnClass));
	}

	StartManualSpawnRetry();

	// 서버에서만 플레이어 폰 수동 스폰 초기화를 수행
	TrySpawnPlayerPawns();
}

void AArenaGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	StartManualSpawnRetry();

	// 플레이어가 순차적으로 들어오는 경우를 대비해 다시 시도
	TrySpawnPlayerPawns();
}

APawn* AArenaGameMode::SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot)
{
	// 본 모드는 SpawnActor + Possess 수동 스폰을 사용하므로 엔진 기본 폰 자동 스폰을 차단한다.
	UE_LOG(LogTemp, Verbose, TEXT("[AArenaGameMode] SpawnDefaultPawnFor blocked for controller=%s"), *GetNameSafe(NewPlayer));
	return nullptr;
}

void AArenaGameMode::StartManualSpawnRetry()
{
	if (!HasAuthority())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}

	if (!GetWorldTimerManager().IsTimerActive(TimerHandle_ManualSpawnRetry))
	{
		GetWorldTimerManager().SetTimer(TimerHandle_ManualSpawnRetry, this, &AArenaGameMode::TrySpawnPlayerPawns, ManualSpawnRetryInterval, true);
	}
}

void AArenaGameMode::StopManualSpawnRetry()
{
	if (GetWorldTimerManager().IsTimerActive(TimerHandle_ManualSpawnRetry))
	{
		GetWorldTimerManager().ClearTimer(TimerHandle_ManualSpawnRetry);
	}
}

void AArenaGameMode::TrySpawnPlayerPawns()
{
	if (!HasAuthority() || bHasSpawnedPlayerPawns)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}

	const TSubclassOf<ABaseJanggiPiece> ResolvedAttackerClass = IsValid(AttackerPawnClass) ? AttackerPawnClass : TestPieceClassA;
	const TSubclassOf<ABaseJanggiPiece> ResolvedDefenderClass = IsValid(DefenderPawnClass) ? DefenderPawnClass : TestPieceClassB;

	UE_LOG(LogTemp, Log, TEXT("[AArenaGameMode] Spawn class resolve: Attacker=%s (Primary=%s, Fallback=%s), Defender=%s (Primary=%s, Fallback=%s)"),
		*GetNameSafe(*ResolvedAttackerClass),
		*GetNameSafe(*AttackerPawnClass),
		*GetNameSafe(*TestPieceClassA),
		*GetNameSafe(*ResolvedDefenderClass),
		*GetNameSafe(*DefenderPawnClass),
		*GetNameSafe(*TestPieceClassB));

	if (!IsValid(ResolvedAttackerClass))
	{
		UE_LOG(LogTemp, Warning, TEXT("[AArenaGameMode] Attacker class is not set. Set AttackerPawnClass or TestPieceClassA."));
		return;
	}

	if (!IsValid(ResolvedDefenderClass))
	{
		UE_LOG(LogTemp, Warning, TEXT("[AArenaGameMode] Defender class is not set. Set DefenderPawnClass or TestPieceClassB."));
		return;
	}

	TArray<APlayerController*> ValidControllers;
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PlayerController = It->Get();
		if (IsValid(PlayerController))
		{
			ValidControllers.Add(PlayerController);
		}
	}

	if (ValidControllers.Num() < 2)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AArenaGameMode] Need at least 2 player controllers for manual spawn. Current: %d"), ValidControllers.Num());
		return;
	}

	ValidControllers.Sort([](const APlayerController& Lhs, const APlayerController& Rhs)
	{
		const APlayerState* LState = Lhs.PlayerState;
		const APlayerState* RState = Rhs.PlayerState;
		const int32 LId = IsValid(LState) ? LState->GetPlayerId() : TNumericLimits<int32>::Max();
		const int32 RId = IsValid(RState) ? RState->GetPlayerId() : TNumericLimits<int32>::Max();
		return LId < RId;
	});

	APlayerController* PlayerZeroController = ValidControllers[0];
	APlayerController* PlayerOneController = ValidControllers[1];

	if (!IsValid(PlayerZeroController) || !IsValid(PlayerOneController) || PlayerZeroController == PlayerOneController)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AArenaGameMode] Failed to resolve distinct Player 0 / Player 1 controllers."));
		return;
	}

	if (IsValid(AttackerPiece) || IsValid(DefenderPiece))
	{
		bHasSpawnedPlayerPawns = true;
		StopManualSpawnRetry();
		return;
	}

	TArray<AActor*> ArenaSpawnPoints;
	GatherArenaSpawnPoints(ArenaSpawnPoints);

	AActor* StartPoint0 = nullptr;
	AActor* StartPoint1 = nullptr;
	if (!SelectTwoRandomSpawnPoints(ArenaSpawnPoints, StartPoint0, StartPoint1))
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[AArenaGameMode] Player_0 class=%s"), *GetNameSafe(*ResolvedAttackerClass));
	UE_LOG(LogTemp, Log, TEXT("[AArenaGameMode] Player_1 class=%s"), *GetNameSafe(*ResolvedDefenderClass));

	ABaseJanggiPiece* SpawnedAttacker = SpawnPlayerPawnForController(PlayerZeroController, ResolvedAttackerClass, StartPoint0, TEXT("Start_Point_0 (Player_0)"), EJanggiFaction::Red, TEXT("Arena.Attacker"));
	ABaseJanggiPiece* SpawnedDefender = SpawnPlayerPawnForController(PlayerOneController, ResolvedDefenderClass, StartPoint1, TEXT("Start_Point_1 (Player_1)"), EJanggiFaction::Blue, TEXT("Arena.Defender"));

	if (IsValid(SpawnedAttacker) && IsValid(SpawnedDefender))
	{
		AttackerController = PlayerZeroController;
		DefenderController = PlayerOneController;
		AttackerPiece = SpawnedAttacker;
		DefenderPiece = SpawnedDefender;
		bHasSpawnedPlayerPawns = true;
		StopManualSpawnRetry();
	}
}

void AArenaGameMode::GatherArenaSpawnPoints(TArray<AActor*>& OutSpawnPoints) const
{
	OutSpawnPoints.Reset();

	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		UE_LOG(LogTemp, Warning, TEXT("[AArenaGameMode] GatherArenaSpawnPoints failed: World is invalid."));
		return;
	}

	const bool bUseAllStarts = ArenaSpawnTag.IsNone();
	if (bUseAllStarts)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AArenaGameMode] ArenaSpawnTag is None. Using all PlayerStart/TargetPoint actors as spawn candidates."));
	}

	TArray<AActor*> FoundPlayerStarts;
	TArray<AActor*> CandidatePlayerStarts;
	UGameplayStatics::GetAllActorsOfClass(World, APlayerStart::StaticClass(), FoundPlayerStarts);

	for (AActor* Actor : FoundPlayerStarts)
	{
		APlayerStart* PlayerStart = Cast<APlayerStart>(Actor);
		if (!IsValid(PlayerStart))
		{
			continue;
		}

		const bool bMatchedActorTag = PlayerStart->ActorHasTag(ArenaSpawnTag);
		const bool bMatchedPlayerStartTag = (PlayerStart->PlayerStartTag == ArenaSpawnTag);
		if (bUseAllStarts || bMatchedActorTag || bMatchedPlayerStartTag)
		{
			CandidatePlayerStarts.Add(PlayerStart);
		}
	}

	if (CandidatePlayerStarts.Num() >= 2)
	{
		OutSpawnPoints = CandidatePlayerStarts;
		UE_LOG(LogTemp, Log, TEXT("[AArenaGameMode] GatherArenaSpawnPoints: using %d PlayerStart points (TargetPoint ignored)."), OutSpawnPoints.Num());
		return;
	}

	OutSpawnPoints = CandidatePlayerStarts;

	TArray<AActor*> FoundTargetPoints;
	UGameplayStatics::GetAllActorsOfClass(World, ATargetPoint::StaticClass(), FoundTargetPoints);

	for (AActor* Actor : FoundTargetPoints)
	{
		if (!IsValid(Actor))
		{
			continue;
		}

		if (bUseAllStarts || Actor->ActorHasTag(ArenaSpawnTag))
		{
			OutSpawnPoints.Add(Actor);
		}
	}

	if (bUseAllStarts)
	{
		UE_LOG(LogTemp, Log, TEXT("[AArenaGameMode] GatherArenaSpawnPoints: %d total points found (PlayerStart=%d, TargetPoint fallback used=%s)."), OutSpawnPoints.Num(), CandidatePlayerStarts.Num(), FoundTargetPoints.Num() > 0 ? TEXT("true") : TEXT("false"));
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("[AArenaGameMode] GatherArenaSpawnPoints: %d points tagged '%s' found (PlayerStart=%d)."), OutSpawnPoints.Num(), *ArenaSpawnTag.ToString(), CandidatePlayerStarts.Num());
	}
}

bool AArenaGameMode::SelectTwoRandomSpawnPoints(const TArray<AActor*>& SpawnPoints, AActor*& OutStartPoint0, AActor*& OutStartPoint1) const
{
	OutStartPoint0 = nullptr;
	OutStartPoint1 = nullptr;

	if (SpawnPoints.Num() < 2)
	{
		UE_LOG(LogTemp, Error, TEXT("[AArenaGameMode] Need at least 2 ArenaSpawn points. Current: %d"), SpawnPoints.Num());
		return false;
	}

	const int32 Index0 = FMath::RandRange(0, SpawnPoints.Num() - 1);
	int32 Index1 = FMath::RandRange(0, SpawnPoints.Num() - 1);

	while (Index1 == Index0)
	{
		Index1 = FMath::RandRange(0, SpawnPoints.Num() - 1);
	}

	OutStartPoint0 = SpawnPoints.IsValidIndex(Index0) ? SpawnPoints[Index0] : nullptr;
	OutStartPoint1 = SpawnPoints.IsValidIndex(Index1) ? SpawnPoints[Index1] : nullptr;

	if (!IsValid(OutStartPoint0) || !IsValid(OutStartPoint1))
	{
		UE_LOG(LogTemp, Error, TEXT("[AArenaGameMode] Random spawn selection produced invalid point(s)."));
		return false;
	}

	return true;
}

ABaseJanggiPiece* AArenaGameMode::SpawnPlayerPawnForController(APlayerController* PlayerController, TSubclassOf<ABaseJanggiPiece> PieceClass, AActor* SpawnPoint, const TCHAR* SpawnPointLabel, EJanggiFaction AssignedFaction, FName RoleTag)
{
	if (!HasAuthority() || !IsValid(PlayerController) || !IsValid(*PieceClass))
	{
		UE_LOG(LogTemp, Warning, TEXT("[AArenaGameMode] SpawnPlayerPawnForController failed. Invalid controller or class."));
		return nullptr;
	}

	const int32 PlayerId = IsValid(PlayerController->PlayerState) ? PlayerController->PlayerState->GetPlayerId() : INDEX_NONE;
	UE_LOG(LogTemp, Log, TEXT("[AArenaGameMode] %s - PlayerId=%d Controller=%s SpawnClass=%s"),
		SpawnPointLabel,
		PlayerId,
		*GetNameSafe(PlayerController),
		*GetNameSafe(*PieceClass));

	if (!IsValid(SpawnPoint))
	{
		UE_LOG(LogTemp, Warning, TEXT("[AArenaGameMode] SpawnPlayerPawnForController failed. %s is invalid."), SpawnPointLabel);
		return nullptr;
	}

	APawn* ExistingPawn = PlayerController->GetPawn();
	if (IsValid(ExistingPawn))
	{
		UE_LOG(LogTemp, Log, TEXT("[AArenaGameMode] %s - Replacing existing pawn: %s"), SpawnPointLabel, *GetNameSafe(ExistingPawn->GetClass()));
		PlayerController->UnPossess();
		ExistingPawn->Destroy();
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	SpawnParams.Owner = PlayerController;

	ABaseJanggiPiece* SpawnedPiece = GetWorld()->SpawnActor<ABaseJanggiPiece>(*PieceClass, SpawnPoint->GetActorLocation(), SpawnPoint->GetActorRotation(), SpawnParams);
	if (!IsValid(SpawnedPiece))
	{
		UE_LOG(LogTemp, Warning, TEXT("[AArenaGameMode] Failed to spawn test piece at %s."), SpawnPointLabel);
		return nullptr;
	}

	if (USceneComponent* RootComp = SpawnedPiece->GetRootComponent())
	{
		RootComp->SetMobility(EComponentMobility::Movable);
	}

	if (UCapsuleComponent* CapsuleComp = SpawnedPiece->GetCapsuleComponent())
	{
		CapsuleComp->SetMobility(EComponentMobility::Movable);
	}

	if (USkeletalMeshComponent* BodyMesh = SpawnedPiece->GetMesh())
	{
		BodyMesh->SetMobility(EComponentMobility::Movable);
	}

	PlayerController->Possess(SpawnedPiece);
	SpawnedPiece->SetFaction(AssignedFaction);
	SpawnedPiece->Tags.AddUnique(RoleTag);
	SpawnedPiece->Tags.AddUnique(TEXT("Arena.Piece"));
	SpawnedPiece->SetCharacterState(EJanggiPieceState::Fighting);

	const bool bPossessSucceeded = (PlayerController->GetPawn() == SpawnedPiece);
	UE_LOG(LogTemp, Log, TEXT("[AArenaGameMode] %s - Possess %s. PawnClass=%s"),
		SpawnPointLabel,
		bPossessSucceeded ? TEXT("SUCCEEDED") : TEXT("FAILED"),
		*GetNameSafe(SpawnedPiece->GetClass()));
	UE_LOG(LogTemp, Log, TEXT("[AArenaGameMode] %s - Classified as Faction=%d RoleTag=%s Pawn=%s"),
		SpawnPointLabel,
		static_cast<int32>(AssignedFaction),
		*RoleTag.ToString(),
		*GetNameSafe(SpawnedPiece));

	return SpawnedPiece;
}

void AArenaGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	if (CurrentMatchPhase != EArenaMatchPhase::MatchEnded)
	{
		APlayerController* ExitingPC = Cast<APlayerController>(Exiting);
		if (IsValid(ExitingPC))
		{
			// 전투 중 한쪽이 연결을 끊거나 나가면 남은 플레이어 승리 처리
			if (ExitingPC == AttackerController && IsValid(DefenderController))
			{
				EndMatch(EDuelResult::DefenderWin, ActiveRequestData.DefenderPlayerId, ActiveRequestData.AttackerPlayerId);
			}
			else if (ExitingPC == DefenderController && IsValid(AttackerController))
			{
				EndMatch(EDuelResult::AttackerWin, ActiveRequestData.AttackerPlayerId, ActiveRequestData.DefenderPlayerId);
			}
		}
	}
}

bool AArenaGameMode::LoadDuelRequestFromInstance()
{
	UJanggiGameInstance* GI = Cast<UJanggiGameInstance>(GetGameInstance());
	if (IsValid(GI) && GI->HasValidDuelRequest())
	{
		ActiveRequestData = GI->GetDuelRequestData();
		return true;
	}

	UE_LOG(LogTemp, Warning, TEXT("[AArenaGameMode] Valid DuelRequestData not found in GameInstance!"));
	return false;
}

void AArenaGameMode::TryMatchmakingAndSpawn()
{
	if (CurrentMatchPhase != EArenaMatchPhase::WaitingForPlayers || !ActiveRequestData.IsValid())
	{
		return;
	}

	// 1. 현재 접속한 모든 플레이어 컨트롤러를 순회하여 공격자와 방어자를 ID 기준으로 식별
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (!IsValid(PC) || !IsValid(PC->PlayerState))
		{
			continue;
		}

		// 플레이어 식별 문자열 가져오기 (UniqueId String 또는 인덱스 기반 ID)
		FString PlayerIdStr = PC->PlayerState->GetUniqueId().IsValid() ? PC->PlayerState->GetUniqueId()->ToString() : FString::FromInt(PC->PlayerState->GetPlayerId());

		if (AttackerController == nullptr && PlayerIdStr == ActiveRequestData.AttackerPlayerId)
		{
			AttackerController = PC;
		}
		else if (DefenderController == nullptr && PlayerIdStr == ActiveRequestData.DefenderPlayerId)
		{
			DefenderController = PC;
		}
	}

	// 2. 공격자와 방어자 모두 컨트롤러 연결 및 식별이 완료되었으면 캐릭터 스폰 진행
	if (IsValid(AttackerController) && IsValid(DefenderController))
	{
		// 제안: "Attacker"와 "Defender" 태그가 붙은 PlayerStart에 각각 스폰
		AttackerPiece = SpawnAndPossessPiece(AttackerController, ActiveRequestData.AttackerPieceClass, FName("Attacker"), EJanggiFaction::Red);
		DefenderPiece = SpawnAndPossessPiece(DefenderController, ActiveRequestData.DefenderPieceClass, FName("Defender"), EJanggiFaction::Blue);

		if (IsValid(AttackerPiece) && IsValid(DefenderPiece))
		{
			// 스폰 성공 시 카운트다운 페이즈로 전환
			StartCountdown();
		}
	}
}

ABaseJanggiPiece* AArenaGameMode::SpawnAndPossessPiece(APlayerController* PlayerController, TSoftClassPtr<ABaseJanggiPiece> PieceClass, FName StartTag, EJanggiFaction AssignedFaction)
{
	if (!IsValid(PlayerController) || PieceClass.IsNull())
	{
		return nullptr;
	}

	// 1. SoftClassPtr 동기식 로드 (또는 사전에 비동기 로드된 클래스 가져오기)
	UClass* LoadedClass = PieceClass.LoadSynchronous();
	if (!IsValid(LoadedClass))
	{
		UE_LOG(LogTemp, Error, TEXT("[AArenaGameMode] Failed to load piece class for %s"), *StartTag.ToString());
		return nullptr;
	}

	// 2. PlayerStart 액터 검색
	AActor* SpawnPoint = FindPlayerStartByTag(StartTag);
	FVector SpawnLocation = IsValid(SpawnPoint) ? SpawnPoint->GetActorLocation() : FVector::ZeroVector;
	FRotator SpawnRotation = IsValid(SpawnPoint) ? SpawnPoint->GetActorRotation() : FRotator::ZeroRotator;

	// 3. 기존 Pawn이 있다면 빙의 해제 및 파괴
	APawn* OldPawn = PlayerController->GetPawn();
	if (IsValid(OldPawn))
	{
		PlayerController->UnPossess();
		OldPawn->Destroy();
	}

	// 4. 새 전투 캐릭터 스폰
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	SpawnParams.Owner = PlayerController;

	ABaseJanggiPiece* NewPiece = GetWorld()->SpawnActor<ABaseJanggiPiece>(LoadedClass, SpawnLocation, SpawnRotation, SpawnParams);
	if (IsValid(NewPiece))
	{
		// 스폰된 기물 초기화 (준비 상태 유지)
		NewPiece->SetFaction(AssignedFaction);
		if (AssignedFaction == EJanggiFaction::Red)
		{
			NewPiece->Tags.AddUnique(TEXT("Arena.Attacker"));
		}
		else if (AssignedFaction == EJanggiFaction::Blue)
		{
			NewPiece->Tags.AddUnique(TEXT("Arena.Defender"));
		}
		NewPiece->Tags.AddUnique(TEXT("Arena.Piece"));
		NewPiece->SetCharacterState(EJanggiPieceState::Preparing);
		PlayerController->Possess(NewPiece);
	}

	return NewPiece;
}

AActor* AArenaGameMode::FindPlayerStartByTag(FName StartTag)
{
	TArray<AActor*> PlayerStarts;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), PlayerStarts);

	for (AActor* Actor : PlayerStarts)
	{
		APlayerStart* Start = Cast<APlayerStart>(Actor);
		if (IsValid(Start) && Start->PlayerStartTag == StartTag)
		{
			return Start;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("[AArenaGameMode] PlayerStart tag '%s' not found in level."), *StartTag.ToString());
	return nullptr;
}

void AArenaGameMode::StartCountdown()
{
	CurrentMatchPhase = EArenaMatchPhase::Countdown;

	// 카운트다운 이벤트 연출을 위해 블루프린트 통지
	OnCountdownStartedEvent(CountdownDuration);

	// 타이머를 통해 카운트다운 종료 후 전투 시작
	GetWorldTimerManager().SetTimer(TimerHandle_Countdown, this, &AArenaGameMode::OnCountdownFinished, CountdownDuration, false);
}

void AArenaGameMode::OnCountdownFinished()
{
	CurrentMatchPhase = EArenaMatchPhase::Fighting;

	// 양쪽 플레이어 기물의 상태를 Fighting(전투 가능)으로 변경
	if (IsValid(AttackerPiece))
	{
		AttackerPiece->SetCharacterState(EJanggiPieceState::Fighting);
	}
	if (IsValid(DefenderPiece))
	{
		DefenderPiece->SetCharacterState(EJanggiPieceState::Fighting);
	}

	// 블루프린트에 전투 시작 알림 (UI "FIGHT!" 출력, BGM 재생 등)
	OnFightStartedEvent();
}

void AArenaGameMode::ReportPieceDeath(ABaseJanggiPiece* DeadPiece, AActor* Killer)
{
	if (!HasAuthority())
	{
		return;
	}

	if (CurrentMatchPhase != EArenaMatchPhase::Fighting || !IsValid(DeadPiece))
	{
		return;
	}

	// 사망한 기물의 플레이어 판별
	bool bAttackerDead = (DeadPiece == AttackerPiece);
	bool bDefenderDead = (DeadPiece == DefenderPiece);

	if (!bAttackerDead && !bDefenderDead)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AArenaGameMode] ReportPieceDeath ignored. DeadPiece is not part of this match. Piece=%s"), *GetNameSafe(DeadPiece));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[AArenaGameMode] ReportPieceDeath received. DeadPiece=%s Killer=%s"), *GetNameSafe(DeadPiece), *GetNameSafe(Killer));

	// 동시 사망(무승부) 판정을 위해 반대편 기물의 상태도 확인
	if (bAttackerDead && bDefenderDead)
	{
		EndMatch(EDuelResult::Draw, TEXT(""), TEXT(""));
	}
	else if (bAttackerDead)
	{
		EndMatch(EDuelResult::DefenderWin, ActiveRequestData.DefenderPlayerId, ActiveRequestData.AttackerPlayerId);
	}
	else if (bDefenderDead)
	{
		EndMatch(EDuelResult::AttackerWin, ActiveRequestData.AttackerPlayerId, ActiveRequestData.DefenderPlayerId);
	}
}

void AArenaGameMode::EndMatch(EDuelResult Result, const FString& WinnerId, const FString& LoserId)
{
	if (CurrentMatchPhase == EArenaMatchPhase::MatchEnded)
	{
		return;
	}

	CurrentMatchPhase = EArenaMatchPhase::MatchEnded;

	// 남은 생존 캐릭터들의 상태를 Stunned 또는 Preparing으로 변경하여 추가 공격/이동 방지
	if (IsValid(AttackerPiece) && AttackerPiece != DefenderPiece)
	{
		AttackerPiece->SetCharacterState(EJanggiPieceState::Preparing);
	}
	if (IsValid(DefenderPiece) && DefenderPiece != AttackerPiece)
	{
		DefenderPiece->SetCharacterState(EJanggiPieceState::Preparing);
	}

	// 1. GameInstance에 결과 데이터 저장
	UJanggiGameInstance* GI = Cast<UJanggiGameInstance>(GetGameInstance());
	if (IsValid(GI))
	{
		FDuelResultData ResultData;
		ResultData.Result = Result;
		ResultData.BoardCellIndex = ActiveRequestData.BoardCellIndex;
		ResultData.WinnerPlayerId = WinnerId;
		ResultData.LoserPlayerId = LoserId;

		GI->SetDuelResultData(ResultData);
		// 아레나에서 요청 데이터 사용이 끝났으므로 비움
		GI->ClearDuelRequestData();
	}

	// 2. 승리한 플레이어 컨트롤러 판별 (UI 통지용)
	APlayerController* WinnerPC = nullptr;
	if (Result == EDuelResult::AttackerWin)
	{
		WinnerPC = AttackerController;
	}
	else if (Result == EDuelResult::DefenderWin)
	{
		WinnerPC = DefenderController;
	}

	// 3. 블루프린트 이벤트 호출 (승리 카메라 연출, 결과 UI 스폰 등)
	OnMatchEndedEvent(Result, WinnerPC);

	// 4. 일정 시간(PostMatchDelay) 대기 후 보드 레벨로 ServerTravel
	GetWorldTimerManager().SetTimer(TimerHandle_ReturnToBoard, this, &AArenaGameMode::ReturnToBoardLevel, PostMatchDelay, false);
}

void AArenaGameMode::ReturnToBoardLevel()
{
	UWorld* World = GetWorld();
	if (IsValid(World) && !BoardLevelMapPath.IsEmpty())
	{
		// 멀티플레이어 Server Travel 수행 (리스닝 서버 옵션 유지)
		FString TravelURL = FString::Printf(TEXT("%s?listen"), *BoardLevelMapPath);
		World->ServerTravel(TravelURL);
	}
}
