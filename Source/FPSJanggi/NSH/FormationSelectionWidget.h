// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Blueprint/UserWidget.h"
#include "FormationSelectionWidget.generated.h"

UCLASS()
class FPSJANGGI_API UFormationSelectionWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	UFUNCTION()
	void SelectHorseElephantElephantHorse();

	UFUNCTION()
	void SelectHorseElephantHorseElephant();

	UFUNCTION()
	void SelectElephantHorseElephantHorse();

	UFUNCTION()
	void SelectElephantHorseHorseElephant();

	void SubmitFormation(uint8 FormationValue);
};
