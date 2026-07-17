// Copyright Epic Games, Inc. All Rights Reserved.

#include "NSH/YJHArenaDebugGameMode.h"

#include "GameFramework/GameModeBase.h"
#include "GameFramework/HUD.h"
#include "EngineUtils.h"
#include "NSH/AuthoritativeJanggiBoard.h"
#include "NSH/BoardPlayerController.h"
#include "TimerManager.h"

namespace
{
	constexpr TCHAR IntegratedGameModeClassPath[] = TEXT("/Game/User/Blueprints/NewGameMode.NewGameMode_C");
}

AYJHArenaDebugGameMode::AYJHArenaDebugGameMode()
{
	if (UClass* IntegratedGameModeClass = LoadClass<AGameModeBase>(nullptr, IntegratedGameModeClassPath))
	{
		if (const AGameModeBase* IntegratedDefaults = Cast<AGameModeBase>(IntegratedGameModeClass->GetDefaultObject()))
		{
			if (IntegratedDefaults->PlayerControllerClass)
			{
				PlayerControllerClass = IntegratedDefaults->PlayerControllerClass;
			}
			if (IntegratedDefaults->DefaultPawnClass)
			{
				DefaultPawnClass = IntegratedDefaults->DefaultPawnClass;
			}
			if (IntegratedDefaults->HUDClass)
			{
				HUDClass = IntegratedDefaults->HUDClass;
			}
		}
	}
}

void AYJHArenaDebugGameMode::BeginPlay()
{
	Super::BeginPlay();

	GetWorldTimerManager().SetTimer(
		ArenaAutoStartTimerHandle,
		this,
		&AYJHArenaDebugGameMode::TryAutoStartArenaFlow,
		FMath::Max(0.1f, AutoStartRetryIntervalSeconds),
		true,
		0.2f);
}

void AYJHArenaDebugGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	TryAutoStartArenaFlow();
}

void AYJHArenaDebugGameMode::TryAutoStartArenaFlow()
{
#if UE_BUILD_SHIPPING
	GetWorldTimerManager().ClearTimer(ArenaAutoStartTimerHandle);
	return;
#else
	AAuthoritativeJanggiBoard* Board = FindAuthoritativeBoard();
	if (!Board)
	{
		return;
	}

	if (!bArenaCombatantModeApplied)
	{
		switch (ArenaCombatantMode)
		{
		case EYJHArenaDebugCombatantMode::ForceTemporaryTest:
			Board->SetUseTemporaryTestArenaCombatants(true);
			UE_LOG(LogTemp, Display, TEXT("YJH_DEBUG_ARENA_MODE mode=ForceTemporaryTest use_temp_test_combatants=true"));
			break;
		case EYJHArenaDebugCombatantMode::ForcePieceMapped:
			Board->SetUseTemporaryTestArenaCombatants(false);
			UE_LOG(LogTemp, Display, TEXT("YJH_DEBUG_ARENA_MODE mode=ForcePieceMapped use_temp_test_combatants=false"));
			Board->LogArenaCombatantClassMeshState();
			break;
		case EYJHArenaDebugCombatantMode::KeepBoardSetting:
		default:
			UE_LOG(LogTemp, Display, TEXT("YJH_DEBUG_ARENA_MODE mode=KeepBoardSetting use_temp_test_combatants=%s"),
				Board->IsUsingTemporaryTestArenaCombatants() ? TEXT("true") : TEXT("false"));
			break;
		}

		bArenaCombatantModeApplied = true;
	}

	if (bAutoStartArenaBattle && !bArenaBattleTriggered)
	{
		if (APlayerController* FirstController = GetWorld()->GetFirstPlayerController())
		{
			if (Board->RunFirstArenaBattleForTesting(FirstController))
			{
				bArenaBattleTriggered = true;
			}
		}
	}

	if (bAutoStartArenaReal && !bArenaRealTriggered && Board->GetMatchPhase() == EBoardMatchPhase::ArenaBattle)
	{
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			ABoardPlayerController* Controller = Cast<ABoardPlayerController>(It->Get());
			if (!Controller || !Controller->IsLocalController())
			{
				continue;
			}

			Controller->RequestYJHArenaStart();
			bArenaRealTriggered = true;
			break;
		}
	}

	const bool bDoneArenaBattle = !bAutoStartArenaBattle || bArenaBattleTriggered;
	const bool bDoneArenaReal = !bAutoStartArenaReal || bArenaRealTriggered;
	if (bDoneArenaBattle && bDoneArenaReal)
	{
		GetWorldTimerManager().ClearTimer(ArenaAutoStartTimerHandle);
	}
#endif
}

AAuthoritativeJanggiBoard* AYJHArenaDebugGameMode::FindAuthoritativeBoard() const
{
	if (!GetWorld())
	{
		return nullptr;
	}

	for (TActorIterator<AAuthoritativeJanggiBoard> It(GetWorld()); It; ++It)
	{
		return *It;
	}
	return nullptr;
}
