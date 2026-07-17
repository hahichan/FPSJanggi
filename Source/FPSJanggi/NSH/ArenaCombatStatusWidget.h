// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"
#include "ArenaCombatStatusWidget.generated.h"

class AYJHArenaCombatantBase;
class UBorder;
class UTextBlock;

/** Runtime combat HUD fallback showing HP and cooldown state in ArenaBattleReal. */
UCLASS()
class FPSJANGGI_API UArenaCombatStatusWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void UpdateFromCombatant(const AYJHArenaCombatantBase* Combatant);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	UPROPERTY(Transient)
	TObjectPtr<UBorder> RootBorder;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> HeaderText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> HpText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> CooldownText;
};
