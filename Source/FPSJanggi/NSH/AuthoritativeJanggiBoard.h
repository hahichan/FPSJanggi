// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AuthoritativeJanggiBoard.generated.h"

class APlayerController;
class ABoardPlayerController;
class AArenaPlaceholderCharacter;
class ACameraActor;
class APawn;

UENUM(BlueprintType)
enum class EJanggiTeam : uint8
{
	Unassigned,
	Blue,
	Red
};

UENUM(BlueprintType)
enum class EJanggiPieceType : uint8
{
	None,
	Soldier,
	Cannon,
	Chariot,
	Elephant,
	General,
	Guard,
	Horse
};

UENUM(BlueprintType)
enum class EJanggiFormation : uint8
{
	HorseElephantElephantHorse UMETA(DisplayName = "마-상-상-마"),
	HorseElephantHorseElephant UMETA(DisplayName = "마-상-마-상"),
	ElephantHorseElephantHorse UMETA(DisplayName = "상-마-상-마"),
	ElephantHorseHorseElephant UMETA(DisplayName = "상-마-마-상")
};

UENUM(BlueprintType)
enum class EBoardMatchPhase : uint8
{
	BoardTurn,
	ArenaTransition,
	ArenaBattle,
	BattleResolution,
	MatchFinished
};

USTRUCT(BlueprintType)
struct FPSJANGGI_API FBoardBattleContext
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	bool bActive = false;

	UPROPERTY(BlueprintReadOnly)
	int32 OriginIndex = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly)
	int32 DestinationIndex = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly)
	EJanggiTeam AttackerTeam = EJanggiTeam::Unassigned;

	UPROPERTY(BlueprintReadOnly)
	EJanggiPieceType AttackerType = EJanggiPieceType::None;

	UPROPERTY(BlueprintReadOnly)
	EJanggiPieceType DefenderType = EJanggiPieceType::None;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AActor> Attacker = nullptr;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AActor> Defender = nullptr;
};

/** Server-authoritative rules and battle transition layer for JanggiBoard1. */
UCLASS(Blueprintable)
class FPSJANGGI_API AAuthoritativeJanggiBoard : public AActor
{
	GENERATED_BODY()

public:
	AAuthoritativeJanggiBoard();

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	bool SubmitAuthoritativeClick(APlayerController* RequestingController, const FVector& LocalPosition);
	bool RunFirstLegalMoveForTesting(APlayerController* RequestingController);
	bool RunFirstArenaBattleForTesting(APlayerController* RequestingController);
	bool RunGeneralDefeatForTesting(APlayerController* RequestingController);
	void EnsureBoardInitialized();
	bool SelectFormation(APlayerController* RequestingController, EJanggiFormation Formation);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "FPS Janggi|Board")
	void SetBoardInputPaused(bool bPaused);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "FPS Janggi|Arena")
	void ResolveArenaBattle(EJanggiTeam WinnerTeam);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "FPS Janggi|Arena")
	void AbortArenaBattleForced(const FString& Reason);

	/**
	 * Server-side integration point for a combat character's health/death system.
	 * Passing either active arena combatant resolves the battle for the other team.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "FPS Janggi|Arena|Integration")
	void ReportArenaCombatantDefeated(AActor* DefeatedCombatant);

	/** Returns the currently spawned arena actor for UI, health, and ability integration. */
	UFUNCTION(BlueprintPure, Category = "FPS Janggi|Arena|Integration")
	AActor* GetArenaCombatant(EJanggiTeam Team) const;

	UFUNCTION(BlueprintPure, Category = "FPS Janggi|Board")
	bool IsBoardInputPaused() const { return bBoardInputPaused; }

	UFUNCTION(BlueprintPure, Category = "FPS Janggi|Board")
	int32 GetAuthoritativeClickCount() const { return ClickHistory.Num(); }

	UFUNCTION(BlueprintPure, Category = "FPS Janggi|Board")
	EJanggiTeam GetCurrentTurnTeam() const { return CurrentTurnTeam; }

	UFUNCTION(BlueprintPure, Category = "FPS Janggi|Board")
	float GetRemainingTurnSeconds() const;

	UFUNCTION(BlueprintPure, Category = "FPS Janggi|Arena")
	EBoardMatchPhase GetMatchPhase() const { return MatchPhase; }

	UFUNCTION(BlueprintPure, Category = "FPS Janggi|Board")
	EJanggiTeam GetWinningTeam() const { return WinningTeam; }

	UFUNCTION(BlueprintPure, Category = "FPS Janggi|Arena")
	FBoardBattleContext GetBattleContext() const { return BattleContext; }

	UFUNCTION(BlueprintPure, Category = "FPS Janggi|Arena")
	FName GetActiveCombatSessionId() const { return ActiveCombatSessionId; }

	UFUNCTION(BlueprintPure, Category = "FPS Janggi|Board")
	FVector GetCellWorldPosition(int32 BoardIndex) const;

	UFUNCTION(BlueprintPure, Category = "FPS Janggi|Lobby")
	FVector GetLobbyCameraWorldLocation() const;

	UFUNCTION(BlueprintPure, Category = "FPS Janggi|Lobby")
	FVector GetLobbyCameraFocusWorldLocation() const;

	UFUNCTION(BlueprintPure, Category = "FPS Janggi|Lobby")
	float GetLobbyCameraFieldOfView() const { return LobbyCameraFieldOfView; }

	UFUNCTION(BlueprintPure, Category = "FPS Janggi|Lobby")
	ACameraActor* GetLobbyCameraActor() const { return LobbyCameraActor; }

	/** Converts the two legacy placement handles into a real, previewable level camera. */
	UFUNCTION(CallInEditor, Category = "FPS Janggi|Lobby|Placement",
		meta = (DisplayName = "Create Lobby Camera From Offset Handles"))
	void CreateLobbyCameraFromOffsetHandles();

	/** Copies the active level editor viewport transform to the saved lobby camera. */
	UFUNCTION(CallInEditor, Category = "FPS Janggi|Lobby|Placement",
		meta = (DisplayName = "Snap Lobby Camera To Current Editor View"))
	void SnapLobbyCameraToCurrentEditorView();

	UFUNCTION(BlueprintImplementableEvent, Category = "FPS Janggi|Arena")
	void OnArenaBattleStarted(const FBoardBattleContext& Context);

	UFUNCTION(BlueprintImplementableEvent, Category = "FPS Janggi|Arena")
	void OnArenaBattleResolved(const FBoardBattleContext& Context, EJanggiTeam WinnerTeam);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPS Janggi|Rules", meta = (ClampMin = "5.0"))
	float TurnTimeLimitSeconds = 60.0f;

	/** Zero disables the restriction. Default prevents one piece being spammed indefinitely. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPS Janggi|Rules", meta = (ClampMin = "0"))
	int32 MaxConsecutiveMovesPerPiece = 2;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPS Janggi|Arena", meta = (ClampMin = "0.1"))
	float ArenaTransitionSeconds = 1.25f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPS Janggi|Arena", meta = (ClampMin = "5.0"))
	float ArenaBattleTimeLimitSeconds = 60.0f;

	/** Optional per-piece override for arena timeout. Value <= 0 keeps default. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPS Janggi|Arena")
	TMap<EJanggiPieceType, float> PieceArenaTimeoutOverrideSeconds;

	/** Exact fixed actor scale used by origin/master's JanggiBoard1 spawn transforms. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPS Janggi|Board", meta = (ClampMin = "0.01"))
	float PieceSpawnScale = 1.8f;

	/** Optional placed camera. Its saved transform and FOV are used exactly when assigned. */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "FPS Janggi|Lobby|Placement")
	TObjectPtr<ACameraActor> LobbyCameraActor = nullptr;

	/** Fallback lobby camera controls used only when LobbyCameraActor is not assigned. */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "FPS Janggi|Lobby|Placement",
		meta = (MakeEditWidget = true, EditCondition = "LobbyCameraActor == nullptr", EditConditionHides))
	FVector LobbyCameraOffset = FVector(36.0, 34.0, 82.0);

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "FPS Janggi|Lobby|Placement",
		meta = (MakeEditWidget = true, EditCondition = "LobbyCameraActor == nullptr", EditConditionHides))
	FVector LobbyCameraFocusOffset = FVector(36.0, 34.0, 0.0);

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "FPS Janggi|Lobby|Placement", meta = (ClampMin = "20.0", ClampMax = "120.0"))
	float LobbyCameraFieldOfView = 58.0f;

	/** Select the placed JanggiBoard1 actor to move these five points with viewport gizmos. */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "FPS Janggi|Arena|Placement", meta = (MakeEditWidget = true))
	FVector ArenaBlueSpawnOffset = FVector(-650.0, 1800.0, 200.0);

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "FPS Janggi|Arena|Placement", meta = (MakeEditWidget = true))
	FVector ArenaRedSpawnOffset = FVector(650.0, 1800.0, 200.0);

	/** Enable random spawn selection from map actors tagged with ArenaSpawnPointTag. */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "FPS Janggi|Arena|Placement")
	bool bUseTaggedArenaSpawnPoints = true;

	/** Tag to place on arena spawn-point actors in the map. */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "FPS Janggi|Arena|Placement")
	FName ArenaSpawnPointTag = FName(TEXT("YJH_ArenaSpawn"));

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "FPS Janggi|Arena|Placement", meta = (MakeEditWidget = true))
	FVector ArenaBlueCameraOffset = FVector(-1070.0, 1800.0, 390.0);

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "FPS Janggi|Arena|Placement", meta = (MakeEditWidget = true))
	FVector ArenaRedCameraOffset = FVector(1070.0, 1800.0, 390.0);

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "FPS Janggi|Arena|Placement", meta = (MakeEditWidget = true))
	FVector ArenaCameraFocusOffset = FVector(0.0, 1800.0, 305.0);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPS Janggi|Arena", meta = (ClampMin = "0.01"))
	float ArenaPlaceholderScale = 1.0f;

	/**
	 * Optional real combat Pawn classes, keyed by board-piece type. Leave an entry
	 * empty to retain the existing visual-only placeholder for that piece.
	 */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "FPS Janggi|Arena|Integration")
	TMap<EJanggiPieceType, TSubclassOf<APawn>> BlueArenaCombatantClasses;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "FPS Janggi|Arena|Integration")
	TMap<EJanggiPieceType, TSubclassOf<APawn>> RedArenaCombatantClasses;

	/** Temporary hook: force Blue=test_a, Red=test_b for quick arena iteration. */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "FPS Janggi|Arena|Integration|Temporary")
	bool bUseTemporaryTestArenaCombatants = true;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "FPS Janggi|Arena|Integration|Temporary")
	TSubclassOf<APawn> TemporaryBlueCombatantClass;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "FPS Janggi|Arena|Integration|Temporary")
	TSubclassOf<APawn> TemporaryRedCombatantClass;

	UPROPERTY(ReplicatedUsing = OnRep_ClickHistory)
	TArray<FVector_NetQuantize10> ClickHistory;

	UPROPERTY(Replicated)
	bool bBoardInputPaused = true;

	UPROPERTY(Replicated)
	EJanggiTeam CurrentTurnTeam = EJanggiTeam::Blue;

	UPROPERTY(Replicated)
	EBoardMatchPhase MatchPhase = EBoardMatchPhase::BoardTurn;

	UPROPERTY(Replicated)
	EJanggiTeam WinningTeam = EJanggiTeam::Unassigned;

	UPROPERTY(ReplicatedUsing = OnRep_BattleContext)
	FBoardBattleContext BattleContext;

	UPROPERTY(Replicated)
	double TurnDeadlineServerTime = 0.0;

	UFUNCTION()
	void OnRep_ClickHistory();

	UFUNCTION()
	void OnRep_BattleContext();

private:
	int32 SelectedIndex = INDEX_NONE;
	TWeakObjectPtr<AActor> LastMovedPiece;
	mutable TArray<TWeakObjectPtr<AActor>> PieceCache;
	int32 ConsecutiveMovesForPiece = 0;
	double LastServerClickTime = -1.0;
	bool bBoardInitializationRequested = false;
	bool bBlueFormationSelected = false;
	bool bRedFormationSelected = false;
	EJanggiFormation BlueFormation = EJanggiFormation::ElephantHorseHorseElephant;
	EJanggiFormation RedFormation = EJanggiFormation::ElephantHorseHorseElephant;
	FTimerHandle TurnTimerHandle;
	FTimerHandle ArenaTransitionTimerHandle;
	FTimerHandle ArenaBattleTimerHandle;
	FTimerHandle ArenaPendingDefeatFinalizeTimerHandle;
	FTimerHandle PieceSynchronizationTimerHandle;
	FTimerHandle PieceVisualSynchronizationTimerHandle;
	UPROPERTY(Replicated)
	TObjectPtr<AActor> ArenaBlueCombatant = nullptr;
	UPROPERTY(Replicated)
	TObjectPtr<AActor> ArenaRedCombatant = nullptr;
	int32 PieceVisualSynchronizationAttempts = 0;
	bool bDestroyingArenaCombatants = false;
	bool bArenaFinalizeRequested = false;
	FName ActiveCombatSessionId = NAME_None;
	TSet<EJanggiTeam> PendingDefeatedTeams;
	bool bArenaSpawnPairResolved = false;
	FVector ArenaBlueSpawnWorldLocation = FVector::ZeroVector;
	FVector ArenaRedSpawnWorldLocation = FVector::ZeroVector;

	bool IsValidBoardLocalPosition(const FVector& LocalPosition) const;
	int32 GetBoardIndex(const FVector& SanitizedLocalPosition) const;
	FVector GetCellLocalPosition(int32 BoardIndex) const;
	TArray<int32> GetLegalDestinations(int32 FromIndex, EJanggiTeam Team) const;
	void SendLegalDestinations(ABoardPlayerController* Controller, int32 FromIndex) const;
	void ClearLegalMoveMarkersForAllPlayers() const;
	AActor* GetPieceAt(int32 BoardIndex) const;
	void RebuildPieceCache() const;
	int32 SynchronizeBlueprintPieceArrayFromWorld();
	void BeginPieceVisualSynchronization();
	void SynchronizePieceVisualsFromWorld();
	void ApplyPieceVisualScale(AActor* Piece) const;
	bool ApplyFormationToStartArray(EJanggiTeam Team, EJanggiFormation Formation);
	AActor* SpawnAuthoritativePiece(int32 PieceValue, int32 BoardIndex);
	EJanggiTeam GetPieceTeam(const AActor* Piece) const;
	EJanggiPieceType GetPieceType(const AActor* Piece) const;
	bool IsLegalMove(int32 FromIndex, int32 ToIndex, EJanggiPieceType PieceType, EJanggiTeam Team) const;
	bool IsPathClearOrthogonal(int32 FromRow, int32 FromColumn, int32 ToRow, int32 ToColumn) const;
	bool IsPalaceCell(int32 Row, int32 Column, EJanggiTeam Team) const;
	bool IsConnectedPalaceDiagonal(int32 FromRow, int32 FromColumn, int32 ToRow, int32 ToColumn) const;
	void ClearExistingBlueprintSelection();
	bool RemoveBoardPieceAt(int32 BoardIndex);
	void CompleteBoardMove(int32 DestinationIndex);
	void StartTurn(EJanggiTeam Team);
	void HandleTurnExpired();
	void FinishMatch(EJanggiTeam WinnerTeam);
	void StartArenaBattle(int32 DestinationIndex);
	void EnterArenaBattlePhase();
	void HandleArenaBattleExpired();
	void FinalizeArenaPendingDefeats();
	void NotifyArenaCamera(bool bEnteringArena);
	void PrepareArenaSpawnPair();
	FVector GetArenaCenterWorldLocation() const;
	FVector GetArenaFighterWorldLocation(EJanggiTeam Team) const;
	FVector GetArenaCameraWorldLocation(EJanggiTeam Team) const;
	FVector GetArenaCameraFocusWorldLocation() const;
	void SpawnArenaCombatants();
	AActor* SpawnTemporaryArenaCombatant(EJanggiTeam Team, TSubclassOf<APawn> CombatantClass);
	AActor* SpawnArenaCombatant(EJanggiTeam Team, EJanggiPieceType Type);
	void PossessArenaCombatant(EJanggiTeam Team, AActor* Combatant);
	void DestroyArenaCombatants();
	float GetActiveArenaTimeoutSeconds() const;
	FName BuildCombatSessionId() const;
	UFUNCTION()
	void HandleArenaCombatantDestroyed(AActor* DestroyedActor);
	bool FindFirstLegalNonCaptureMove(EJanggiTeam Team, int32& OutFromIndex, int32& OutToIndex) const;
};
