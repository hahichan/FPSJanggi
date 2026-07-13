// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AuthoritativeJanggiBoard.h"
#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"
#include "BoardStatusWidget.generated.h"

class UBorder;
class UCanvasPanel;
class UTextBlock;

/** Compact turn indicator constructed in C++ so it works without a new Blueprint asset. */
UCLASS()
class FPSJANGGI_API UBoardStatusWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void UpdateBoardStatus(
		EJanggiTeam LocalTeam,
		EJanggiTeam TurnTeam,
		EJanggiTeam WinnerTeam,
		EBoardMatchPhase Phase,
		bool bInputPaused,
		float RemainingSeconds);
	void ShowNotice(const FText& Message, bool bError);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	UPROPERTY(Transient)
	TObjectPtr<UBorder> StatusBorder;

	UPROPERTY(Transient)
	TObjectPtr<UCanvasPanel> StatusCanvas;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> StatusText;

	FText LastStatusText;
	FLinearColor LastStatusColor = FLinearColor::Transparent;
};
