// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "BoardGameMode.generated.h"

UCLASS(Blueprintable)
class FPSJANGGI_API ABoardGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	ABoardGameMode();
	virtual void PostLogin(APlayerController* NewPlayer) override;
};
