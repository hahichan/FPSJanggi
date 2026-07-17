// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Blueprint/UserWidget.h"
#include "Components/ComboBoxString.h"
#include "ArenaDebugWidget.generated.h"

class UTextBlock;

/** Development-only winner controls used until the character combat system is integrated. */
UCLASS()
class FPSJANGGI_API UArenaDebugWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	void RebuildPieceClassCandidates();
	void RefreshSelectionSummary();

	UFUNCTION()
	void ResolveBlueWinner();

	UFUNCTION()
	void ResolveRedWinner();

	UFUNCTION()
	void RequestYJHArenaStart();

	void ResolveWinner(uint8 TeamValue);

	UPROPERTY(Transient)
	TObjectPtr<UComboBoxString> BluePieceCombo = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UComboBoxString> RedPieceCombo = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> SelectionSummaryText = nullptr;

	TArray<FString> CandidateLabels;
	TMap<FString, FString> LabelToClassPath;
};
