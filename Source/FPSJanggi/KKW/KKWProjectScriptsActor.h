// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KKWProjectScriptsActor.generated.h"

UCLASS()
class FPSJANGGI_API AKKWProjectScriptsActor : public AActor
{
	GENERATED_BODY()

public:
	AKKWProjectScriptsActor();

protected:
	UPROPERTY(EditAnywhere, Category = "kkw_scripts")
	TArray<FString> kkw_script_paths;
};
