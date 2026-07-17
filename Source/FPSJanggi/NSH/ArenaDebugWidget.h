// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Blueprint/UserWidget.h"
#include "ArenaDebugWidget.generated.h"

/** Development-only winner controls used until the character combat system is integrated. */
UCLASS()
class FPSJANGGI_API UArenaDebugWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	UFUNCTION()
	void ResolveBlueWinner();

	UFUNCTION()
	void ResolveRedWinner();

	UFUNCTION()
	void RequestYJHArenaStart();

	void ResolveWinner(uint8 TeamValue);
};
