// Copyright Epic Games, Inc. All Rights Reserved.

#include "KKWProjectScriptsActor.h"

AKKWProjectScriptsActor::AKKWProjectScriptsActor()
{
	PrimaryActorTick.bCanEverTick = false;
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	Tags.AddUnique(TEXT("KKW_Scripts_Folder"));

	kkw_script_paths = {
		TEXT("Scripts/KKW/inspect_character_animations.py"),
		TEXT("Scripts/KKW/inspect_mainmap_actors.py"),
		TEXT("Scripts/KKW/inspect_viewmodel_assets.py"),
		TEXT("Scripts/KKW/setup_mainmap_piece_actors.py"),
		TEXT("Scripts/KKW/validate_ability_setup.py"),
	};
}
