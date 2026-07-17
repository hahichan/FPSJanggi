// Copyright Epic Games, Inc. All Rights Reserved.

#include "BoardPlayerController.h"

#include "ArenaDebugWidget.h"
#include "BoardStatusWidget.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "FormationSelectionWidget.h"
#include "Engine/World.h"
#include "InputCoreTypes.h"
#include "LobbyWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "HAL/IConsoleManager.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "SessionSubsystem.h"

namespace
{
#if !UE_BUILD_SHIPPING
FAutoConsoleCommandWithWorld GYJHStartArenaRealCommand(
	TEXT("yjh.startarenareal"),
	TEXT("Request YJH ArenaBattleReal test transition while ArenaBattle is active."),
	FConsoleCommandWithWorldDelegate::CreateLambda([](UWorld* World)
	{
		if (!World)
		{
			return;
		}
		for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
		{
			if (ABoardPlayerController* Controller = Cast<ABoardPlayerController>(It->Get()))
			{
				if (Controller->IsLocalController())
				{
					Controller->RequestYJHArenaStart();
					break;
				}
			}
		}
	}));
#endif
}

ABoardPlayerController::ABoardPlayerController()
{
	bAutoManageActiveCameraTarget = false;
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
}

void ABoardPlayerController::BeginPlay()
{
	Super::BeginPlay();
	if (IsLocalController())
	{
		bFrontEndLobby = ShouldShowFrontEndLobby();
		if (bFrontEndLobby)
		{
			GetWorldTimerManager().ClearTimer(BoardCameraSetupTimerHandle);
			// MainMap's legacy Blueprint creates InterFace_MS during its own BeginPlay.
			// Remove it now and once more after Blueprint startup so its formation
			// buttons never remain visible behind the front-end lobby.
			ReplaceLegacyFormationWidget();
			FTimerHandle LegacyLobbyWidgetTimer;
			GetWorldTimerManager().SetTimer(
				LegacyLobbyWidgetTimer, this, &ABoardPlayerController::ReplaceLegacyFormationWidget, 0.15f, false);
			CreateLobbyWidget();
			SetupLobbyCamera();
			return;
		}
		// A remote client can receive its team before MainMap's placed camera actors
		// are ready. Retry locally instead of falling back to the pawn camera.
		bAutoManageActiveCameraTarget = false;
		ScheduleBoardCameraSetup();
		FTimerHandle FormationWidgetTimer;
		GetWorldTimerManager().SetTimer(
			FormationWidgetTimer, this, &ABoardPlayerController::ReplaceLegacyFormationWidget, 0.15f, false);
		CreateBoardStatusWidget();
		GetWorldTimerManager().SetTimer(
			BoardStatusRefreshTimerHandle,
			this,
			&ABoardPlayerController::RefreshBoardStatus,
			0.25f,
			true,
			0.1f);
	}
}

void ABoardPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(BoardCameraSetupTimerHandle);
	GetWorldTimerManager().ClearTimer(BoardStatusRefreshTimerHandle);
	GetWorldTimerManager().ClearTimer(YJHArenaAutoReturnTimerHandle);
	YJHArenaSubStateMachine.Reset();
	HideLegalMoveMarkers();
	for (UStaticMeshComponent* Marker : LegalMoveOuterMarkerPool)
	{
		if (IsValid(Marker)) Marker->DestroyComponent();
	}
	for (UStaticMeshComponent* Marker : LegalMoveInnerMarkerPool)
	{
		if (IsValid(Marker)) Marker->DestroyComponent();
	}
	LegalMoveOuterMarkerPool.Reset();
	LegalMoveInnerMarkerPool.Reset();
	LegalMoveOuterMaterial = nullptr;
	LegalMoveInnerMaterial = nullptr;
	RemoveArenaDebugWidget();
	RemoveLobbyWidget();
	if (LobbyCamera.IsValid() && bOwnsRuntimeLobbyCamera)
	{
		LobbyCamera->Destroy();
	}
	LobbyCamera.Reset();
	bOwnsRuntimeLobbyCamera = false;
	if (RuntimeBoardCamera.IsValid())
	{
		RuntimeBoardCamera->Destroy();
		RuntimeBoardCamera.Reset();
	}
	Super::EndPlay(EndPlayReason);
}

void ABoardPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ABoardPlayerController, AssignedBoardTeam);
}

bool ABoardPlayerController::InputKey(const FInputKeyEventArgs& Params)
{
#if !UE_BUILD_SHIPPING
	if (Params.Key == EKeys::F8 && Params.Event == IE_Pressed && IsLocalController())
	{
		RequestYJHArenaStart();
		return true;
	}
#endif
	if (Params.Key == EKeys::LeftMouseButton && Params.Event == IE_Pressed && IsLocalController())
	{
		if (bFrontEndLobby) return Super::InputKey(Params);
		if (AAuthoritativeJanggiBoard* Board = FindAuthoritativeBoard())
		{
			HandleBoardPointerClick(Board);
			return true;
		}
	}
	return Super::InputKey(Params);
}

void ABoardPlayerController::RequestBoardClick(const FVector& LocalPosition)
{
	if (!LocalPosition.ContainsNaN()) ServerRequestBoardClick(FVector_NetQuantize10(LocalPosition));
}

void ABoardPlayerController::SetAssignedBoardTeam(EJanggiTeam Team)
{
	if (HasAuthority())
	{
		AssignedBoardTeam = Team;
		ForceNetUpdate();
		if (IsLocalController())
		{
			if (!ShouldShowFrontEndLobby())
			{
				ScheduleBoardCameraSetup();
				ReplaceLegacyFormationWidget();
			}
		}
	}
}

void ABoardPlayerController::RequestFormation(EJanggiFormation Formation)
{
	if (AssignedBoardTeam == EJanggiTeam::Blue || AssignedBoardTeam == EJanggiTeam::Red)
	{
		ServerRequestFormation(Formation);
	}
}

void ABoardPlayerController::RequestDebugArenaWinner(EJanggiTeam WinnerTeam)
{
#if !UE_BUILD_SHIPPING
	if (WinnerTeam == EJanggiTeam::Blue || WinnerTeam == EJanggiTeam::Red)
	{
		ServerResolveDebugArenaWinner(WinnerTeam);
	}
#endif
}

void ABoardPlayerController::RequestReturnToLobby()
{
	if (IsLocalController())
	{
		ServerRequestReturnToLobby();
	}
}

void ABoardPlayerController::RequestYJHArenaStart()
{
#if !UE_BUILD_SHIPPING
	if (!IsLocalController())
	{
		return;
	}
	ServerRequestYJHArenaStart();
#endif
}

void ABoardPlayerController::ExitLobbyForLocalPreview()
{
#if !UE_BUILD_SHIPPING
	if (!IsLocalController()) return;
	bFrontEndLobby = false;
	RemoveLobbyWidget();
	if (LobbyCamera.IsValid() && bOwnsRuntimeLobbyCamera)
	{
		LobbyCamera->Destroy();
	}
	LobbyCamera.Reset();
	bOwnsRuntimeLobbyCamera = false;
	FInputModeGameAndUI InputMode;
	InputMode.SetHideCursorDuringCapture(false);
	SetInputMode(InputMode);
	CreateBoardStatusWidget();
	ScheduleBoardCameraSetup();
	ReplaceLegacyFormationWidget();
	UE_LOG(LogTemp, Display, TEXT("LOBBY_LOCAL_PREVIEW_STARTED team=%s"), *UEnum::GetValueAsString(AssignedBoardTeam));
#endif
}

void ABoardPlayerController::OnRep_AssignedBoardTeam()
{
	if (IsLocalController())
	{
		if (!ShouldShowFrontEndLobby())
		{
			ScheduleBoardCameraSetup();
			ReplaceLegacyFormationWidget();
		}
	}
}

void ABoardPlayerController::ClientBeginArenaTransition_Implementation(
	FVector ArenaFocusLocation,
	FVector BlueFighterLocation,
	FVector RedFighterLocation,
	FVector BlueCameraLocation,
	FVector RedCameraLocation,
	float BlendSeconds)
{
	if (!IsLocalController() || !GetWorld()) return;
	GetWorldTimerManager().ClearTimer(YJHArenaAutoReturnTimerHandle);
	YJHArenaSubStateMachine.EnterArenaBattle();
	// The canonical board view is the team-oriented runtime camera. Legacy
	// Blueprint formation logic may have replaced GetViewTarget(), so do not
	// capture that stale camera for the return path.
	PreviousViewTarget = BoardCamera.IsValid() ? BoardCamera.Get() : GetViewTarget();
	if (ArenaCamera.IsValid()) ArenaCamera->Destroy();
	const FVector CameraLocation = AssignedBoardTeam == EJanggiTeam::Red
		? RedCameraLocation : BlueCameraLocation;
	const FRotator CameraRotation = (ArenaFocusLocation - CameraLocation).Rotation();
	ArenaCamera = GetWorld()->SpawnActor<ACameraActor>(CameraLocation, CameraRotation);
	if (ArenaCamera.IsValid())
	{
		ArenaCamera->GetCameraComponent()->SetFieldOfView(72.0f);
		SetViewTargetWithBlend(ArenaCamera.Get(), FMath::Max(0.1f, BlendSeconds), VTBlend_Cubic, 2.0f, false);
		CreateArenaDebugWidget();
		OnArenaPresentationStarted(AssignedBoardTeam);
		UE_LOG(LogTemp, Display, TEXT("BOARD_ARENA_CAMERA_READY team=%s location=%s focus=%s"),
			*UEnum::GetValueAsString(AssignedBoardTeam), *CameraLocation.ToCompactString(), *ArenaFocusLocation.ToCompactString());
	}
}

void ABoardPlayerController::ClientEndArenaTransition_Implementation(float BlendSeconds)
{
	if (!IsLocalController()) return;
	GetWorldTimerManager().ClearTimer(YJHArenaAutoReturnTimerHandle);
	YJHArenaSubStateMachine.Reset();
	RemoveArenaDebugWidget();
	AActor* ReturnTarget = BoardCamera.IsValid() ? BoardCamera.Get() : PreviousViewTarget.Get();
	if (IsValid(ReturnTarget))
		SetViewTargetWithBlend(ReturnTarget, FMath::Max(0.1f, BlendSeconds), VTBlend_Cubic, 2.0f, false);
	else
		ScheduleBoardCameraSetup();
	if (ArenaCamera.IsValid())
	{
		ArenaCamera->SetLifeSpan(FMath::Max(0.1f, BlendSeconds) + 0.1f);
		ArenaCamera.Reset();
	}
	OnArenaPresentationEnded(AssignedBoardTeam);
	UE_LOG(LogTemp, Display, TEXT("BOARD_ARENA_CAMERA_RETURN team=%s target=%s"),
		*UEnum::GetValueAsString(AssignedBoardTeam), IsValid(ReturnTarget) ? *ReturnTarget->GetName() : TEXT("None"));
	PreviousViewTarget.Reset();
}

void ABoardPlayerController::ClientRunLateJoinSmokeCheck_Implementation()
{
#if !UE_BUILD_SHIPPING
	FTimerHandle DumpTimer;
	GetWorldTimerManager().SetTimer(DumpTimer, FTimerDelegate::CreateWeakLambda(this, [this]()
	{
		DumpBoardNetworkState();
	}), 4.0f, false);
#endif
}

void ABoardPlayerController::ClientShowLegalMoves_Implementation(const TArray<int32>& BoardIndices)
{
	HideLegalMoveMarkers();
	AAuthoritativeJanggiBoard* Board = FindAuthoritativeBoard();
	if (!IsLocalController() || !Board || !EnsureLegalMoveMarkerPool(Board, BoardIndices.Num()))
	{
		return;
	}
	int32 CreatedDestinations = 0;
	for (const int32 BoardIndex : BoardIndices)
	{
		if (BoardIndex < 0 || BoardIndex >= 90)
		{
			continue;
		}
		const FVector CellWorldPosition = Board->GetCellWorldPosition(BoardIndex);
		const FVector CellLocalPosition = Board->GetActorTransform().InverseTransformPosition(CellWorldPosition);
		UStaticMeshComponent* OuterMarker = LegalMoveOuterMarkerPool[CreatedDestinations];
		UStaticMeshComponent* InnerMarker = LegalMoveInnerMarkerPool[CreatedDestinations];
		OuterMarker->SetRelativeLocation(CellLocalPosition + FVector(0.0f, 0.0f, 1.20f));
		OuterMarker->SetRelativeRotation(FRotator::ZeroRotator);
		OuterMarker->SetRelativeScale3D(FVector(0.048f, 0.048f, 0.006f));
		OuterMarker->SetHiddenInGame(false, true);
		OuterMarker->SetVisibility(true, true);
		InnerMarker->SetRelativeLocation(CellLocalPosition + FVector(0.0f, 0.0f, 1.58f));
		InnerMarker->SetRelativeRotation(FRotator::ZeroRotator);
		InnerMarker->SetRelativeScale3D(FVector(0.032f, 0.032f, 0.008f));
		InnerMarker->SetHiddenInGame(false, true);
		InnerMarker->SetVisibility(true, true);
		++CreatedDestinations;

		UE_LOG(LogTemp, VeryVerbose, TEXT("BOARD_LEGAL_MOVE_WORLD_MARKER index=%d local=%s world=%s"),
			BoardIndex, *CellLocalPosition.ToCompactString(), *CellWorldPosition.ToCompactString());
	}

	UE_LOG(LogTemp, Display, TEXT("BOARD_LEGAL_MOVES_SHOWN team=%s count=%d pooled_components=%d mode=jade_disc_pool"),
		*UEnum::GetValueAsString(AssignedBoardTeam), CreatedDestinations,
		LegalMoveOuterMarkerPool.Num() + LegalMoveInnerMarkerPool.Num());
}

void ABoardPlayerController::ClientClearLegalMoves_Implementation()
{
	HideLegalMoveMarkers();
}

void ABoardPlayerController::ClientShowBoardNotice_Implementation(const FString& Message, bool bError)
{
	if (!IsLocalController())
	{
		return;
	}
	if (!BoardStatusWidget.IsValid())
	{
		CreateBoardStatusWidget();
	}
	if (UBoardStatusWidget* Widget = BoardStatusWidget.Get())
	{
		Widget->ShowNotice(FText::FromString(Message), bError);
		BoardNoticeExpiresAtSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() + 1.5 : -1.0;
	}
}

void ABoardPlayerController::ClientReturnToLobby_Implementation()
{
	if (!IsLocalController()) return;
	HideLegalMoveMarkers();
	RemoveArenaDebugWidget();
	if (UFormationSelectionWidget* Widget = FormationSelectionWidget.Get()) Widget->RemoveFromParent();
	FormationSelectionWidget.Reset();
	if (UBoardStatusWidget* Widget = BoardStatusWidget.Get()) Widget->RemoveFromParent();
	BoardStatusWidget.Reset();
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (USessionSubsystem* Sessions = GameInstance->GetSubsystem<USessionSubsystem>())
		{
			Sessions->ReturnToLobby();
			return;
		}
	}
	UGameplayStatics::OpenLevel(this, FName(TEXT("/Game/User/Map/MainMap")), true);
}

void ABoardPlayerController::TestBoardClick(int32 BoardX, int32 BoardY)
{
	// Exact index-to-position spacing from origin/master's JanggiBoard1 Blueprint.
	RequestBoardClick(FVector(BoardX * 8.0, BoardY * 8.5, 0.0));
}

void ABoardPlayerController::DumpBoardNetworkState()
{
	if (const AAuthoritativeJanggiBoard* Board = FindAuthoritativeBoard())
	{
		UE_LOG(LogTemp, Display,
			TEXT("BOARD_STATE controller_role=%s board_role=%s team=%s turn=%s phase=%s commands=%d paused=%s time=%.1f"),
			*UEnum::GetValueAsString(GetLocalRole()), *UEnum::GetValueAsString(Board->GetLocalRole()),
			*UEnum::GetValueAsString(AssignedBoardTeam), *UEnum::GetValueAsString(Board->GetCurrentTurnTeam()),
			*UEnum::GetValueAsString(Board->GetMatchPhase()), Board->GetAuthoritativeClickCount(),
			Board->IsBoardInputPaused() ? TEXT("true") : TEXT("false"), Board->GetRemainingTurnSeconds());
	}
}

void ABoardPlayerController::ResolveTestBattle(bool bAttackerWins)
{
	if (AAuthoritativeJanggiBoard* Board = FindAuthoritativeBoard())
	{
		const FBoardBattleContext Context = Board->GetBattleContext();
		if (Context.bActive)
		{
			const EJanggiTeam Defender = Context.AttackerTeam == EJanggiTeam::Blue
				? EJanggiTeam::Red : EJanggiTeam::Blue;
			Board->ResolveArenaBattle(bAttackerWins ? Context.AttackerTeam : Defender);
		}
	}
}

void ABoardPlayerController::RunBoardSmokeTest()
{
#if !UE_BUILD_SHIPPING
	if (!GetWorld()) return;
	FTimerHandle SelectTimer;
	FTimerHandle DumpTimer;
	GetWorldTimerManager().SetTimer(SelectTimer, FTimerDelegate::CreateWeakLambda(this, [this]()
	{
		if (AAuthoritativeJanggiBoard* Board = FindAuthoritativeBoard())
		{
			Board->RunFirstLegalMoveForTesting(this);
		}
	}), 1.0f, false);
	GetWorldTimerManager().SetTimer(DumpTimer, FTimerDelegate::CreateWeakLambda(this, [this]()
	{
		DumpBoardNetworkState();
	}), 2.0f, false);
#endif
}

void ABoardPlayerController::RunArenaSmokeTest()
{
#if !UE_BUILD_SHIPPING
	UE_LOG(LogTemp, Display, TEXT("BOARD_ARENA_SMOKE_TEST controller_start"));
	if (!GetWorld()) return;
	if (AAuthoritativeJanggiBoard* Board = FindAuthoritativeBoard())
	{
		if (!Board->RunFirstArenaBattleForTesting(this))
		{
			UE_LOG(LogTemp, Error, TEXT("BOARD_ARENA_SMOKE_TEST setup_failed"));
			return;
		}
	}
	FTimerHandle ResolveTimer;
	FTimerHandle DumpTimer;
	GetWorldTimerManager().SetTimer(ResolveTimer, FTimerDelegate::CreateWeakLambda(this, [this]()
	{
		if (const AAuthoritativeJanggiBoard* Board = FindAuthoritativeBoard())
		{
			const FBoardBattleContext Context = Board->GetBattleContext();
			if (Context.bActive)
			{
				const EJanggiTeam DefenderTeam = Context.AttackerTeam == EJanggiTeam::Blue
					? EJanggiTeam::Red : EJanggiTeam::Blue;
				RequestDebugArenaWinner(DefenderTeam);
			}
		}
	}), 3.0f, false);
	GetWorldTimerManager().SetTimer(DumpTimer, FTimerDelegate::CreateWeakLambda(this, [this]()
	{
		DumpBoardNetworkState();
	}), 4.0f, false);
#endif
}

void ABoardPlayerController::RunGeneralDefeatSmokeTest()
{
#if !UE_BUILD_SHIPPING
	if (AAuthoritativeJanggiBoard* Board = FindAuthoritativeBoard())
	{
		Board->RunGeneralDefeatForTesting(this);
	}
#endif
}

void ABoardPlayerController::YJHStartArenaReal()
{
#if !UE_BUILD_SHIPPING
	RequestYJHArenaStart();
#endif
}

void ABoardPlayerController::ServerRequestBoardClick_Implementation(FVector_NetQuantize10 LocalPosition)
{
	const UWorld* World = GetWorld();
	if (!World) return;
	const double Now = World->GetTimeSeconds();
	if (LastServerBoardClickTimeSeconds >= 0.0 && Now - LastServerBoardClickTimeSeconds < 0.08) return;
	LastServerBoardClickTimeSeconds = Now;
	if (AAuthoritativeJanggiBoard* Board = FindAuthoritativeBoard())
		Board->SubmitAuthoritativeClick(this, FVector(LocalPosition));
}

void ABoardPlayerController::ServerRequestFormation_Implementation(EJanggiFormation Formation)
{
	if (AAuthoritativeJanggiBoard* Board = FindAuthoritativeBoard())
	{
		Board->SelectFormation(this, Formation);
	}
}

void ABoardPlayerController::ServerResolveDebugArenaWinner_Implementation(EJanggiTeam WinnerTeam)
{
#if !UE_BUILD_SHIPPING
	if (WinnerTeam == EJanggiTeam::Blue || WinnerTeam == EJanggiTeam::Red)
	{
		if (AAuthoritativeJanggiBoard* Board = FindAuthoritativeBoard())
		{
			UE_LOG(LogTemp, Display, TEXT("BOARD_ARENA_DEBUG_WINNER_REQUEST team=%s requester=%s"),
				*UEnum::GetValueAsString(WinnerTeam), *UEnum::GetValueAsString(AssignedBoardTeam));
			Board->ResolveArenaBattle(WinnerTeam);
		}
	}
#endif
}

void ABoardPlayerController::ServerRequestYJHArenaStart_Implementation()
{
#if !UE_BUILD_SHIPPING
	AAuthoritativeJanggiBoard* Board = FindAuthoritativeBoard();
	if (!Board || Board->GetMatchPhase() != EBoardMatchPhase::ArenaBattle)
	{
		ClientShowBoardNotice(TEXT("YJH Arena Start is available during ArenaBattle only"), true);
		return;
	}

	UE_LOG(LogTemp, Display, TEXT("YJH_ARENA_REAL_STARTED requester=%s"), *UEnum::GetValueAsString(AssignedBoardTeam));
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (ABoardPlayerController* Controller = Cast<ABoardPlayerController>(It->Get()))
		{
			Controller->ClientRunYJHArenaStartStub(1.0f);
		}
	}
#endif
}

void ABoardPlayerController::ClientRunYJHArenaStartStub_Implementation(float ReturnDelaySeconds)
{
#if !UE_BUILD_SHIPPING
	if (!IsLocalController())
	{
		return;
	}

	FString FailureReason;
	if (!YJHArenaSubStateMachine.TryEnterArenaBattleReal(FailureReason))
	{
		ClientShowBoardNotice(FailureReason, true);
		return;
	}

	ClientShowBoardNotice(TEXT("YJH Arena Mode Entered (Stub)"), false);
	GetWorldTimerManager().ClearTimer(YJHArenaAutoReturnTimerHandle);
	GetWorldTimerManager().SetTimer(
		YJHArenaAutoReturnTimerHandle,
		this,
		&ABoardPlayerController::HandleYJHArenaAutoReturn,
		FMath::Max(0.1f, ReturnDelaySeconds),
		false);
#endif
}

void ABoardPlayerController::HandleYJHArenaAutoReturn()
{
#if !UE_BUILD_SHIPPING
	YJHArenaSubStateMachine.ReturnToArenaBattle();
	ClientShowBoardNotice(TEXT("Returned to ArenaBattle UI"), false);
#endif
}

void ABoardPlayerController::ServerRequestReturnToLobby_Implementation()
{
	AAuthoritativeJanggiBoard* Board = FindAuthoritativeBoard();
	if (!Board || Board->GetMatchPhase() != EBoardMatchPhase::MatchFinished)
	{
		ClientShowBoardNotice(TEXT("대국이 끝난 뒤 로비로 돌아갈 수 있습니다"), true);
		return;
	}
	UE_LOG(LogTemp, Display, TEXT("BOARD_RETURN_TO_LOBBY_REQUEST requester=%s"), *UEnum::GetValueAsString(AssignedBoardTeam));
	TArray<ABoardPlayerController*> RemoteControllers;
	ABoardPlayerController* LocalHostController = nullptr;
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (ABoardPlayerController* Controller = Cast<ABoardPlayerController>(It->Get()))
		{
			if (Controller->IsLocalController())
			{
				LocalHostController = Controller;
			}
			else
			{
				RemoteControllers.Add(Controller);
			}
		}
	}
	// Send remote clients first. Opening the listen host's lobby map can tear down the
	// net driver immediately, so the host must be the final controller to travel.
	for (ABoardPlayerController* Controller : RemoteControllers)
	{
		Controller->ClientReturnToLobby();
	}
	if (LocalHostController)
	{
		TWeakObjectPtr<ABoardPlayerController> WeakHostController = LocalHostController;
		FTimerHandle HostReturnTimer;
		GetWorldTimerManager().SetTimer(HostReturnTimer, FTimerDelegate::CreateWeakLambda(this, [WeakHostController]()
		{
			if (ABoardPlayerController* HostController = WeakHostController.Get())
			{
				HostController->ClientReturnToLobby();
			}
		}), 0.5f, false);
	}
}

AAuthoritativeJanggiBoard* ABoardPlayerController::FindAuthoritativeBoard() const
{
	if (CachedBoard.IsValid())
	{
		return CachedBoard.Get();
	}
	TArray<AActor*> Boards;
	UGameplayStatics::GetAllActorsOfClass(this, AAuthoritativeJanggiBoard::StaticClass(), Boards);
	CachedBoard = Boards.Num() > 0 ? Cast<AAuthoritativeJanggiBoard>(Boards[0]) : nullptr;
	return CachedBoard.Get();
}

void ABoardPlayerController::HandleBoardPointerClick(AAuthoritativeJanggiBoard* Board)
{
	if (!IsValid(Board) || Board->IsBoardInputPaused()) return;
	FHitResult HitResult;
	if (GetHitResultUnderCursorByChannel(UEngineTypes::ConvertToTraceType(ECC_Visibility), true, HitResult))
		RequestBoardClick(Board->GetActorTransform().InverseTransformPosition(HitResult.Location));
}

void ABoardPlayerController::ReplaceLegacyFormationWidget()
{
	if (!IsLocalController()) return;
	if (UClass* LegacyClass = LoadClass<UUserWidget>(
		nullptr, TEXT("/Game/User/Blueprints/InterFace_MS.InterFace_MS_C")))
	{
		TArray<UUserWidget*> LegacyWidgets;
		UWidgetBlueprintLibrary::GetAllWidgetsOfClass(this, LegacyWidgets, LegacyClass, false);
		for (UUserWidget* Widget : LegacyWidgets)
		{
			if (Widget) Widget->RemoveFromParent();
		}
	}

	// Lobby owns the entire front-end viewport. Only remove legacy widgets here;
	// the canonical formation UI is created after a room has been entered.
	if (bFrontEndLobby || ShouldShowFrontEndLobby()) return;

	if ((AssignedBoardTeam == EJanggiTeam::Blue || AssignedBoardTeam == EJanggiTeam::Red) &&
		!FormationSelectionWidget.IsValid())
	{
		UFormationSelectionWidget* Widget = CreateWidget<UFormationSelectionWidget>(
			this, UFormationSelectionWidget::StaticClass());
		if (Widget)
		{
			FormationSelectionWidget = Widget;
			Widget->AddToViewport(100);
			UE_LOG(LogTemp, Display, TEXT("FORMATION_UI_SHOWN team=%s"),
				*UEnum::GetValueAsString(AssignedBoardTeam));
		}
	}
}

void ABoardPlayerController::CreateArenaDebugWidget()
{
#if !UE_BUILD_SHIPPING
	if (!IsLocalController() || ArenaDebugWidget.IsValid()) return;
	if (UArenaDebugWidget* Widget = CreateWidget<UArenaDebugWidget>(this, UArenaDebugWidget::StaticClass()))
	{
		ArenaDebugWidget = Widget;
		Widget->AddToViewport(1000);
		UE_LOG(LogTemp, Display, TEXT("BOARD_ARENA_DEBUG_UI_READY team=%s"), *UEnum::GetValueAsString(AssignedBoardTeam));
	}
#endif
}

void ABoardPlayerController::RemoveArenaDebugWidget()
{
	if (UArenaDebugWidget* Widget = ArenaDebugWidget.Get()) Widget->RemoveFromParent();
	ArenaDebugWidget.Reset();
}

void ABoardPlayerController::CreateBoardStatusWidget()
{
	if (!IsLocalController() || bFrontEndLobby || BoardStatusWidget.IsValid())
	{
		return;
	}
	if (UBoardStatusWidget* Widget = CreateWidget<UBoardStatusWidget>(this, UBoardStatusWidget::StaticClass()))
	{
		BoardStatusWidget = Widget;
		Widget->AddToViewport(90);
		RefreshBoardStatus();
		UE_LOG(LogTemp, Display, TEXT("BOARD_STATUS_UI_READY team=%s"),
			*UEnum::GetValueAsString(AssignedBoardTeam));
	}
}

void ABoardPlayerController::RefreshBoardStatus()
{
	if (!IsLocalController() || bFrontEndLobby)
	{
		return;
	}
	if (const AAuthoritativeJanggiBoard* Board = FindAuthoritativeBoard())
	{
		const EBoardMatchPhase Phase = Board->GetMatchPhase();
		if ((Phase == EBoardMatchPhase::ArenaTransition || Phase == EBoardMatchPhase::ArenaBattle ||
			Phase == EBoardMatchPhase::BattleResolution) && !ArenaDebugWidget.IsValid())
		{
			CreateArenaDebugWidget();
		}
		const bool bBoardViewPhase = Phase == EBoardMatchPhase::BoardTurn;
		if (bBoardViewPhase && BoardCamera.IsValid() && !ArenaCamera.IsValid() &&
			GetViewTarget() != BoardCamera.Get())
		{
			SetViewTarget(BoardCamera.Get());
			UE_LOG(LogTemp, Display, TEXT("BOARD_CAMERA_REASSERTED team=%s target=%s reason=legacy_override"),
				*UEnum::GetValueAsString(AssignedBoardTeam), *BoardCamera->GetName());
		}
	}
	if (!BoardStatusWidget.IsValid())
	{
		CreateBoardStatusWidget();
	}
	if (GetWorld() && GetWorld()->GetTimeSeconds() < BoardNoticeExpiresAtSeconds)
	{
		return;
	}
	if (UBoardStatusWidget* Widget = BoardStatusWidget.Get())
	{
		if (const AAuthoritativeJanggiBoard* Board = FindAuthoritativeBoard())
		{
			Widget->UpdateBoardStatus(
				AssignedBoardTeam,
				Board->GetCurrentTurnTeam(),
				Board->GetWinningTeam(),
				Board->GetMatchPhase(),
				Board->IsBoardInputPaused(),
				Board->GetRemainingTurnSeconds());
		}
		else
		{
			Widget->UpdateBoardStatus(
				AssignedBoardTeam,
				EJanggiTeam::Unassigned,
				EJanggiTeam::Unassigned,
				EBoardMatchPhase::BoardTurn,
				true,
				0.0f);
		}
	}
}

bool ABoardPlayerController::ShouldShowFrontEndLobby() const
{
	if (!GetWorld() || GetNetMode() != NM_Standalone) return false;
	if (FParse::Param(FCommandLine::Get(), TEXT("SkipLobby")) ||
		FParse::Param(FCommandLine::Get(), TEXT("BoardSmokeTest")) ||
		FParse::Param(FCommandLine::Get(), TEXT("ArenaSmokeTest")) ||
		FParse::Param(FCommandLine::Get(), TEXT("GeneralSmokeTest")))
	{
		return false;
	}
	return true;
}

void ABoardPlayerController::CreateLobbyWidget()
{
	if (!IsLocalController() || LobbyWidget.IsValid()) return;
	if (ULobbyWidget* Widget = CreateWidget<ULobbyWidget>(this, ULobbyWidget::StaticClass()))
	{
		LobbyWidget = Widget;
		Widget->AddToViewport(200);
		FInputModeUIOnly InputMode;
		SetInputMode(InputMode);
		bShowMouseCursor = true;
		UE_LOG(LogTemp, Display, TEXT("LOBBY_UI_READY mode=standalone"));
	}
}

void ABoardPlayerController::RemoveLobbyWidget()
{
	if (ULobbyWidget* Widget = LobbyWidget.Get()) Widget->RemoveFromParent();
	LobbyWidget.Reset();
}

void ABoardPlayerController::SetupLobbyCamera()
{
	if (!IsLocalController() || !GetWorld() || !bFrontEndLobby) return;
	AAuthoritativeJanggiBoard* Board = FindAuthoritativeBoard();
	if (!Board)
	{
		FTimerHandle RetryHandle;
		GetWorldTimerManager().SetTimer(RetryHandle, this, &ABoardPlayerController::SetupLobbyCamera, 0.2f, false);
		return;
	}
	if (LobbyCamera.IsValid() && bOwnsRuntimeLobbyCamera) LobbyCamera->Destroy();
	LobbyCamera.Reset();
	bOwnsRuntimeLobbyCamera = false;
	if (ACameraActor* PlacedLobbyCamera = Board->GetLobbyCameraActor())
	{
		LobbyCamera = PlacedLobbyCamera;
		SetViewTarget(PlacedLobbyCamera);
		UE_LOG(LogTemp, Display, TEXT("LOBBY_CAMERA_READY source=placed actor=%s location=%s rotation=%s fov=%.1f"),
			*PlacedLobbyCamera->GetName(),
			*PlacedLobbyCamera->GetActorLocation().ToCompactString(),
			*PlacedLobbyCamera->GetActorRotation().ToCompactString(),
			PlacedLobbyCamera->GetCameraComponent()->FieldOfView);
		return;
	}
	// The editor conversion button also tags the saved actor. This secondary
	// lookup keeps the lobby deterministic if a Blueprint instance temporarily
	// loses the direct actor reference after recompilation.
	TArray<AActor*> CameraActors;
	UGameplayStatics::GetAllActorsOfClass(this, ACameraActor::StaticClass(), CameraActors);
	for (AActor* Actor : CameraActors)
	{
		ACameraActor* TaggedLobbyCamera = Cast<ACameraActor>(Actor);
		if (!TaggedLobbyCamera || !TaggedLobbyCamera->ActorHasTag(TEXT("Lobby"))) continue;
		LobbyCamera = TaggedLobbyCamera;
		SetViewTarget(TaggedLobbyCamera);
		UE_LOG(LogTemp, Display, TEXT("LOBBY_CAMERA_READY source=tagged actor=%s location=%s rotation=%s fov=%.1f"),
			*TaggedLobbyCamera->GetName(),
			*TaggedLobbyCamera->GetActorLocation().ToCompactString(),
			*TaggedLobbyCamera->GetActorRotation().ToCompactString(),
			TaggedLobbyCamera->GetCameraComponent()->FieldOfView);
		return;
	}
	const FVector CameraLocation = Board->GetLobbyCameraWorldLocation();
	const FVector FocusLocation = Board->GetLobbyCameraFocusWorldLocation();
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParameters.ObjectFlags |= RF_Transient;
	LobbyCamera = GetWorld()->SpawnActor<ACameraActor>(
		CameraLocation,
		(FocusLocation - CameraLocation).Rotation(),
		SpawnParameters);
	if (LobbyCamera.IsValid())
	{
		bOwnsRuntimeLobbyCamera = true;
		LobbyCamera->GetCameraComponent()->SetFieldOfView(Board->GetLobbyCameraFieldOfView());
		SetViewTarget(LobbyCamera.Get());
		UE_LOG(LogTemp, Display, TEXT("LOBBY_CAMERA_READY source=fallback location=%s focus=%s fov=%.1f"),
			*CameraLocation.ToCompactString(), *FocusLocation.ToCompactString(), Board->GetLobbyCameraFieldOfView());
	}
}

bool ABoardPlayerController::EnsureLegalMoveMarkerPool(
	AAuthoritativeJanggiBoard* Board,
	int32 RequiredDestinations)
{
	if (!Board || !Board->GetRootComponent()) return false;

	UStaticMesh* MarkerMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	UMaterialInterface* MarkerMaterialBase = LoadObject<UMaterialInterface>(
		nullptr, TEXT("/Engine/EngineDebugMaterials/DebugMeshMaterial.DebugMeshMaterial"));
	if (!LegalMoveOuterMaterial && MarkerMaterialBase)
	{
		LegalMoveOuterMaterial = UMaterialInstanceDynamic::Create(MarkerMaterialBase, this);
		if (LegalMoveOuterMaterial)
		{
			LegalMoveOuterMaterial->SetVectorParameterValue(
				TEXT("Color"), FLinearColor(0.015f, 0.22f, 0.11f, 1.0f));
		}
	}
	if (!LegalMoveInnerMaterial && MarkerMaterialBase)
	{
		LegalMoveInnerMaterial = UMaterialInstanceDynamic::Create(MarkerMaterialBase, this);
		if (LegalMoveInnerMaterial)
		{
			LegalMoveInnerMaterial->SetVectorParameterValue(
				TEXT("Color"), FLinearColor(0.18f, 0.82f, 0.48f, 1.0f));
		}
	}
	if (!MarkerMesh || !LegalMoveOuterMaterial || !LegalMoveInnerMaterial)
	{
		UE_LOG(LogTemp, Error, TEXT("BOARD_LEGAL_MOVE_MARKER_POOL_FAILED mesh=%s material=%s"),
			MarkerMesh ? TEXT("true") : TEXT("false"),
			LegalMoveOuterMaterial && LegalMoveInnerMaterial ? TEXT("true") : TEXT("false"));
		return false;
	}

	auto CreateMarker = [Board, MarkerMesh](UMaterialInterface* Material, int32 SortPriority)
		-> UStaticMeshComponent*
	{
		UStaticMeshComponent* Marker = NewObject<UStaticMeshComponent>(Board, NAME_None, RF_Transient);
		if (!Marker) return nullptr;
		Board->AddInstanceComponent(Marker);
		Marker->SetStaticMesh(MarkerMesh);
		Marker->SetMaterial(0, Material);
		Marker->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Marker->SetGenerateOverlapEvents(false);
		Marker->SetCanEverAffectNavigation(false);
		Marker->SetCastShadow(false);
		Marker->SetTranslucentSortPriority(SortPriority);
		Marker->SetHiddenInGame(true, true);
		Marker->SetVisibility(false, true);
		Marker->SetupAttachment(Board->GetRootComponent());
		Marker->RegisterComponent();
		return Marker;
	};

	while (LegalMoveOuterMarkerPool.Num() < RequiredDestinations)
	{
		UStaticMeshComponent* OuterMarker = CreateMarker(LegalMoveOuterMaterial, 100);
		UStaticMeshComponent* InnerMarker = CreateMarker(LegalMoveInnerMaterial, 101);
		if (!OuterMarker || !InnerMarker)
		{
			if (IsValid(OuterMarker)) OuterMarker->DestroyComponent();
			if (IsValid(InnerMarker)) InnerMarker->DestroyComponent();
			return false;
		}
		LegalMoveOuterMarkerPool.Add(OuterMarker);
		LegalMoveInnerMarkerPool.Add(InnerMarker);
	}
	return true;
}

void ABoardPlayerController::HideLegalMoveMarkers()
{
	for (UStaticMeshComponent* Marker : LegalMoveOuterMarkerPool)
	{
		if (!IsValid(Marker)) continue;
		Marker->SetHiddenInGame(true, true);
		Marker->SetVisibility(false, true);
	}
	for (UStaticMeshComponent* Marker : LegalMoveInnerMarkerPool)
	{
		if (!IsValid(Marker)) continue;
		Marker->SetHiddenInGame(true, true);
		Marker->SetVisibility(false, true);
	}
}

void ABoardPlayerController::ScheduleBoardCameraSetup()
{
	if (!IsLocalController() || !GetWorld() || bFrontEndLobby) return;
	BoardCameraSetupAttempts = 0;
	GetWorldTimerManager().ClearTimer(BoardCameraSetupTimerHandle);
	GetWorldTimerManager().SetTimer(
		BoardCameraSetupTimerHandle,
		this,
		&ABoardPlayerController::TrySetupBoardCamera,
		0.25f,
		true,
		0.05f);
}

void ABoardPlayerController::TrySetupBoardCamera()
{
	if (!IsLocalController() || !GetWorld())
	{
		GetWorldTimerManager().ClearTimer(BoardCameraSetupTimerHandle);
		return;
	}

	const FName RequiredCameraName = AssignedBoardTeam == EJanggiTeam::Blue
		? FName(TEXT("CameraActor_1"))
		: AssignedBoardTeam == EJanggiTeam::Red ? FName(TEXT("CameraActor_2")) : NAME_None;
	if (!RequiredCameraName.IsNone())
	{
		TArray<AActor*> CameraActors;
		UGameplayStatics::GetAllActorsOfClass(this, ACameraActor::StaticClass(), CameraActors);
		for (AActor* Actor : CameraActors)
		{
			ACameraActor* Camera = Cast<ACameraActor>(Actor);
			if (!Camera || Camera->GetFName() != RequiredCameraName) continue;

			FRotator TeamViewRotation = Camera->GetActorRotation();
			if (AAuthoritativeJanggiBoard* Board = FindAuthoritativeBoard())
			{
				// Formation selection only swaps the Horse/Elephant ordering. It never
				// changes which end belongs to each team. Keep the placed camera's
				// location and forward direction, but roll its screen-up axis so the
				// local player's own home row is always at the bottom of the screen.
				const FVector BlueHome = Board->GetCellWorldPosition(4);
				const FVector RedHome = Board->GetCellWorldPosition(85);
				const FVector OwnHome = AssignedBoardTeam == EJanggiTeam::Blue ? BlueHome : RedHome;
				const FVector OpponentHome = AssignedBoardTeam == EJanggiTeam::Blue ? RedHome : BlueHome;
				const FVector DesiredScreenUp = (OpponentHome - OwnHome).GetSafeNormal();
				if (!DesiredScreenUp.IsNearlyZero())
				{
					TeamViewRotation = FRotationMatrix::MakeFromXZ(
						Camera->GetActorForwardVector(), DesiredScreenUp).Rotator();
				}
			}

			if (RuntimeBoardCamera.IsValid()) RuntimeBoardCamera->Destroy();
			FActorSpawnParameters SpawnParameters;
			SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			SpawnParameters.ObjectFlags |= RF_Transient;
			ACameraActor* TeamCamera = GetWorld()->SpawnActor<ACameraActor>(
				Camera->GetActorLocation(), TeamViewRotation, SpawnParameters);
			if (TeamCamera)
			{
				TeamCamera->GetCameraComponent()->SetFieldOfView(Camera->GetCameraComponent()->FieldOfView);
				RuntimeBoardCamera = TeamCamera;
				BoardCamera = TeamCamera;
			}
			else
			{
				BoardCamera = Camera;
			}
			SetViewTarget(BoardCamera.Get());
			GetWorldTimerManager().ClearTimer(BoardCameraSetupTimerHandle);
			UE_LOG(LogTemp, Display, TEXT("BOARD_CAMERA_READY team=%s source=%s view=%s location=%s rotation=%s own_side=bottom"),
				*UEnum::GetValueAsString(AssignedBoardTeam), *Camera->GetName(),
				BoardCamera.IsValid() ? *BoardCamera->GetName() : TEXT("None"),
				*Camera->GetActorLocation().ToCompactString(), *TeamViewRotation.ToCompactString());
			return;
		}
	}

	if (++BoardCameraSetupAttempts >= 40)
	{
		GetWorldTimerManager().ClearTimer(BoardCameraSetupTimerHandle);
		UE_LOG(LogTemp, Error, TEXT("BOARD_CAMERA_FAILED team=%s required_camera=%s"),
			*UEnum::GetValueAsString(AssignedBoardTeam), *RequiredCameraName.ToString());
	}
}
