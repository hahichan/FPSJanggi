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
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

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
	HideLegalMoveMarkers();
	RemoveArenaDebugWidget();
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
	if (Params.Key == EKeys::LeftMouseButton && Params.Event == IE_Pressed && IsLocalController())
	{
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
			ScheduleBoardCameraSetup();
			ReplaceLegacyFormationWidget();
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

void ABoardPlayerController::OnRep_AssignedBoardTeam()
{
	if (IsLocalController())
	{
		ScheduleBoardCameraSetup();
		ReplaceLegacyFormationWidget();
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
		UE_LOG(LogTemp, Display, TEXT("BOARD_ARENA_CAMERA_READY team=%s location=%s focus=%s"),
			*UEnum::GetValueAsString(AssignedBoardTeam), *CameraLocation.ToCompactString(), *ArenaFocusLocation.ToCompactString());
	}
}

void ABoardPlayerController::ClientEndArenaTransition_Implementation(float BlendSeconds)
{
	if (!IsLocalController()) return;
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
	if (!IsLocalController() || !Board)
	{
		return;
	}

	UStaticMesh* MarkerMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	UMaterialInterface* MarkerMaterialBase = LoadObject<UMaterialInterface>(
		nullptr, TEXT("/Engine/EngineDebugMaterials/DebugMeshMaterial.DebugMeshMaterial"));
	UMaterialInstanceDynamic* MarkerMaterial = MarkerMaterialBase
		? UMaterialInstanceDynamic::Create(MarkerMaterialBase, this) : nullptr;
	if (MarkerMaterial)
	{
		MarkerMaterial->SetVectorParameterValue(
			TEXT("Color"), FLinearColor(0.01f, 1.0f, 0.04f, 1.0f));
	}
	if (!MarkerMesh || !MarkerMaterial || !Board->GetRootComponent())
	{
		UE_LOG(LogTemp, Error, TEXT("BOARD_LEGAL_MOVE_WORLD_MARKER_FAILED mesh=%s material=%s root=%s"),
			MarkerMesh ? TEXT("true") : TEXT("false"),
			MarkerMaterial ? TEXT("true") : TEXT("false"),
			Board->GetRootComponent() ? TEXT("true") : TEXT("false"));
		return;
	}

	LegalMoveWorldMarkers.Reserve(BoardIndices.Num());
	for (const int32 BoardIndex : BoardIndices)
	{
		if (BoardIndex < 0 || BoardIndex >= 90)
		{
			continue;
		}
		const FVector CellWorldPosition = Board->GetCellWorldPosition(BoardIndex);
		FVector CellLocalPosition = Board->GetActorTransform().InverseTransformPosition(CellWorldPosition);
		// The engine sphere is 100 units in diameter. At 0.045 relative scale its
		// local radius is 2.25, so 2.5 places it clearly above the board surface.
		CellLocalPosition.Z += 2.5f;

		UStaticMeshComponent* Marker = NewObject<UStaticMeshComponent>(Board, NAME_None, RF_Transient);
		if (!Marker) continue;
		Board->AddInstanceComponent(Marker);
		Marker->SetStaticMesh(MarkerMesh);
		Marker->SetMaterial(0, MarkerMaterial);
		Marker->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Marker->SetGenerateOverlapEvents(false);
		Marker->SetCanEverAffectNavigation(false);
		Marker->SetCastShadow(false);
		Marker->SetRenderInMainPass(true);
		Marker->SetHiddenInGame(false, true);
		Marker->SetVisibility(true, true);
		Marker->SetTranslucentSortPriority(100);
		Marker->SetupAttachment(Board->GetRootComponent());
		Marker->SetRelativeLocation(CellLocalPosition);
		Marker->SetRelativeRotation(FRotator::ZeroRotator);
		Marker->SetRelativeScale3D(FVector(0.045f));
		Marker->RegisterComponent();
		LegalMoveWorldMarkers.Add(Marker);

		UE_LOG(LogTemp, VeryVerbose, TEXT("BOARD_LEGAL_MOVE_WORLD_MARKER index=%d local=%s world=%s"),
			BoardIndex, *CellLocalPosition.ToCompactString(), *CellWorldPosition.ToCompactString());
	}

	UE_LOG(LogTemp, Display, TEXT("BOARD_LEGAL_MOVES_SHOWN team=%s count=%d mode=board_world_sphere material=debug_green"),
		*UEnum::GetValueAsString(AssignedBoardTeam), LegalMoveWorldMarkers.Num());
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
		Widget->AddToViewport(120);
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
	if (!IsLocalController() || BoardStatusWidget.IsValid())
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
	if (!IsLocalController())
	{
		return;
	}
	if (const AAuthoritativeJanggiBoard* Board = FindAuthoritativeBoard())
	{
		const EBoardMatchPhase Phase = Board->GetMatchPhase();
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

void ABoardPlayerController::HideLegalMoveMarkers()
{
	for (UStaticMeshComponent* Marker : LegalMoveWorldMarkers)
	{
		if (IsValid(Marker)) Marker->DestroyComponent();
	}
	LegalMoveWorldMarkers.Reset();
}

void ABoardPlayerController::ScheduleBoardCameraSetup()
{
	if (!IsLocalController() || !GetWorld()) return;
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
