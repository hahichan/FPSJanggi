// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AuthoritativeJanggiBoard.h"
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "yjh_base/yjh_ArenaSubStateMachine.h"
#include "BoardPlayerController.generated.h"

class ACameraActor;
class UArenaCombatStatusWidget;
class UArenaDebugWidget;
class UBoardStatusWidget;
class UFormationSelectionWidget;
class ULobbyWidget;
class UMaterialInstanceDynamic;
class UStaticMeshComponent;

UCLASS(Blueprintable)
class FPSJANGGI_API ABoardPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ABoardPlayerController();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual bool InputKey(const FInputKeyEventArgs& Params) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "FPS Janggi|Board")
	void RequestBoardClick(const FVector& LocalPosition);

	UFUNCTION(BlueprintPure, Category = "FPS Janggi|Board")
	EJanggiTeam GetAssignedBoardTeam() const { return AssignedBoardTeam; }

	void SetAssignedBoardTeam(EJanggiTeam Team);
	void RequestFormation(EJanggiFormation Formation);
	void RequestDebugArenaWinner(EJanggiTeam WinnerTeam);
	void RequestYJHArenaStart();
	void RequestYJHArenaStartWithClassPaths(const FString& BlueClassPath, const FString& RedClassPath);
	void RequestReturnToLobby();
	void ExitLobbyForLocalPreview();

	UFUNCTION(Client, Reliable)
	void ClientBeginArenaTransition(
		FVector ArenaFocusLocation,
		FVector BlueFighterLocation,
		FVector RedFighterLocation,
		FVector BlueCameraLocation,
		FVector RedCameraLocation,
		float BlendSeconds);

	UFUNCTION(Client, Reliable)
	void ClientEndArenaTransition(float BlendSeconds);

	/** Local presentation hook for HUD, cracked-glass effects, BGM, and SFX. */
	UFUNCTION(BlueprintImplementableEvent, Category = "FPS Janggi|Arena|Presentation")
	void OnArenaPresentationStarted(EJanggiTeam LocalTeam);

	/** Local presentation hook fired after the board camera starts returning. */
	UFUNCTION(BlueprintImplementableEvent, Category = "FPS Janggi|Arena|Presentation")
	void OnArenaPresentationEnded(EJanggiTeam LocalTeam);

	UFUNCTION(Client, Reliable)
	void ClientRunLateJoinSmokeCheck();

	UFUNCTION(Client, Reliable)
	void ClientShowLegalMoves(const TArray<int32>& BoardIndices);

	UFUNCTION(Client, Reliable)
	void ClientClearLegalMoves();

	UFUNCTION(Client, Reliable)
	void ClientShowBoardNotice(const FString& Message, bool bError);

	UFUNCTION(Client, Reliable)
	void ClientReturnToLobby();

	UFUNCTION(Exec, Category = "FPS Janggi|Diagnostics")
	void TestBoardClick(int32 BoardX, int32 BoardY);

	UFUNCTION(Exec, Category = "FPS Janggi|Diagnostics")
	void DumpBoardNetworkState();

	UFUNCTION(Exec, Category = "FPS Janggi|Diagnostics")
	void ResolveTestBattle(bool bAttackerWins = true);

	/** Development-only deterministic move used by headless multiplayer smoke tests. */
	UFUNCTION(Exec, Category = "FPS Janggi|Diagnostics")
	void RunBoardSmokeTest();

	UFUNCTION(Exec, Category = "FPS Janggi|Diagnostics")
	void RunArenaSmokeTest();

	UFUNCTION(Exec, Category = "FPS Janggi|Diagnostics")
	void RunGeneralDefeatSmokeTest();

	UFUNCTION(Exec, Category = "FPS Janggi|Diagnostics")
	void YJHStartArenaReal();

protected:
	UPROPERTY(ReplicatedUsing = OnRep_AssignedBoardTeam, BlueprintReadOnly, Category = "FPS Janggi|Board")
	EJanggiTeam AssignedBoardTeam = EJanggiTeam::Unassigned;

	UFUNCTION()
	void OnRep_AssignedBoardTeam();

	UFUNCTION(Server, Reliable)
	void ServerRequestBoardClick(FVector_NetQuantize10 LocalPosition);

	UFUNCTION(Server, Reliable)
	void ServerRequestFormation(EJanggiFormation Formation);

	UFUNCTION(Server, Reliable)
	void ServerResolveDebugArenaWinner(EJanggiTeam WinnerTeam);

	UFUNCTION(Server, Reliable)
	void ServerRequestYJHArenaStart();

	UFUNCTION(Server, Reliable)
	void ServerRequestYJHArenaStartWithClassPaths(const FString& BlueClassPath, const FString& RedClassPath);

	UFUNCTION(Server, Reliable)
	void ServerRequestReturnToLobby();

	UFUNCTION(Client, Reliable)
	void ClientRunYJHArenaStartStub(float ReturnDelaySeconds);

private:
	double LastServerBoardClickTimeSeconds = -1.0;
	TWeakObjectPtr<AActor> PreviousViewTarget;
	mutable TWeakObjectPtr<AAuthoritativeJanggiBoard> CachedBoard;
	TWeakObjectPtr<ACameraActor> BoardCamera;
	TWeakObjectPtr<ACameraActor> RuntimeBoardCamera;
	TWeakObjectPtr<ACameraActor> ArenaCamera;
	TWeakObjectPtr<ACameraActor> LobbyCamera;
	TWeakObjectPtr<UArenaDebugWidget> ArenaDebugWidget;
	TWeakObjectPtr<UArenaCombatStatusWidget> ArenaCombatStatusWidget;
	TWeakObjectPtr<UFormationSelectionWidget> FormationSelectionWidget;
	TWeakObjectPtr<UBoardStatusWidget> BoardStatusWidget;
	TWeakObjectPtr<ULobbyWidget> LobbyWidget;
	UPROPERTY(Transient)
	TArray<TObjectPtr<UStaticMeshComponent>> LegalMoveOuterMarkerPool;
	UPROPERTY(Transient)
	TArray<TObjectPtr<UStaticMeshComponent>> LegalMoveInnerMarkerPool;
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> LegalMoveOuterMaterial = nullptr;
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> LegalMoveInnerMaterial = nullptr;
	FTimerHandle BoardCameraSetupTimerHandle;
	FTimerHandle BoardStatusRefreshTimerHandle;
	int32 BoardCameraSetupAttempts = 0;
	double BoardNoticeExpiresAtSeconds = -1.0;
	bool bFrontEndLobby = false;
	bool bOwnsRuntimeLobbyCamera = false;
	FYJHArenaSubStateMachine YJHArenaSubStateMachine;
	FTimerHandle YJHArenaAutoReturnTimerHandle;

	AAuthoritativeJanggiBoard* FindAuthoritativeBoard() const;
	void HandleYJHArenaAutoReturn();
	void HideBoardPhaseWidgetsForArenaReal();
	void TryActivateArenaBattleRealControl();
	void HandleBoardPointerClick(AAuthoritativeJanggiBoard* Board);
	void ReplaceLegacyFormationWidget();
	void CreateArenaDebugWidget();
	void RemoveArenaDebugWidget();
	void CreateArenaCombatStatusWidget();
	void RemoveArenaCombatStatusWidget();
	void RefreshArenaCombatStatusWidget();
	void CreateBoardStatusWidget();
	void RefreshBoardStatus();
	bool ShouldShowFrontEndLobby() const;
	void CreateLobbyWidget();
	void RemoveLobbyWidget();
	void SetupLobbyCamera();
	bool EnsureLegalMoveMarkerPool(AAuthoritativeJanggiBoard* Board, int32 RequiredDestinations);
	void HideLegalMoveMarkers();
	void ScheduleBoardCameraSetup();
	void TrySetupBoardCamera();

	FTimerHandle YJHArenaControlActivateRetryTimerHandle;
	int32 YJHArenaControlActivateRetryCount = 0;
};
