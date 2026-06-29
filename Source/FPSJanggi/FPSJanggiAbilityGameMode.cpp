// Copyright Epic Games, Inc. All Rights Reserved.

#include "FPSJanggiAbilityGameMode.h"

#include "FPSJanggiAbilityPlayerController.h"

AFPSJanggiAbilityGameMode::AFPSJanggiAbilityGameMode()
{
	DefaultPawnClass = nullptr;
	PlayerControllerClass = AFPSJanggiAbilityPlayerController::StaticClass();
}
