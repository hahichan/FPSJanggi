// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NSH/BoardGameMode.h"
#include "YJHArenaDebugGameMode.generated.h"

UENUM(BlueprintType)
enum class EYJHArenaDebugCombatantMode : uint8
{
	KeepBoardSetting UMETA(DisplayName = "Keep Board Setting"),
	ForceTemporaryTest UMETA(DisplayName = "Force Temporary Test (test_a/test_b)"),
	ForcePieceMapped UMETA(DisplayName = "Force Piece-Mapped Classes")
};

/**
 * Debug-only game mode that fast-forwards the match to ArenaBattle and optionally
 * triggers YJH Arena Start automatically.
 */
UCLASS(Blueprintable)
class FPSJANGGI_API AYJHArenaDebugGameMode : public ABoardGameMode
{
	GENERATED_BODY()

public:
	AYJHArenaDebugGameMode();

	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPS Janggi|Debug|Arena")
	bool bAutoStartArenaBattle = true;

	/** Select which combatant source policy should be applied before ArenaBattleReal starts. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPS Janggi|Debug|Arena")
	EYJHArenaDebugCombatantMode ArenaCombatantMode = EYJHArenaDebugCombatantMode::KeepBoardSetting;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPS Janggi|Debug|Arena")
	bool bAutoStartArenaReal = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FPS Janggi|Debug|Arena", meta = (ClampMin = "0.1"))
	float AutoStartRetryIntervalSeconds = 0.5f;

private:
	void TryAutoStartArenaFlow();
	class AAuthoritativeJanggiBoard* FindAuthoritativeBoard() const;

	FTimerHandle ArenaAutoStartTimerHandle;
	bool bArenaCombatantModeApplied = false;
	bool bArenaBattleTriggered = false;
	bool bArenaRealTriggered = false;
};
