// Copyright Epic Games, Inc. All Rights Reserved.

#include "BoardStatusWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"

namespace
{
FText TeamDisplayName(EJanggiTeam Team)
{
	switch (Team)
	{
	case EJanggiTeam::Blue: return FText::FromString(TEXT("청"));
	case EJanggiTeam::Red: return FText::FromString(TEXT("한"));
	default: return FText::FromString(TEXT("미정"));
	}
}
}

TSharedRef<SWidget> UBoardStatusWidget::RebuildWidget()
{
	if (!WidgetTree)
	{
		WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"));
	}
	if (WidgetTree->RootWidget)
	{
		return Super::RebuildWidget();
	}

	StatusCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("TurnStatusCanvas"));
	WidgetTree->RootWidget = StatusCanvas;
	StatusBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("TurnStatusBorder"));
	StatusBorder->SetPadding(FMargin(18.0f, 9.0f));
	StatusBorder->SetHorizontalAlignment(HAlign_Center);
	StatusBorder->SetVerticalAlignment(VAlign_Center);
	StatusBorder->SetVisibility(ESlateVisibility::HitTestInvisible);

	StatusText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TurnStatusText"));
	StatusText->SetJustification(ETextJustify::Center);
	StatusText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	FSlateFontInfo Font = StatusText->GetFont();
	Font.Size = 26;
	Font.OutlineSettings.OutlineSize = 1;
	Font.OutlineSettings.OutlineColor = FLinearColor::Black;
	StatusText->SetFont(Font);
	StatusBorder->SetContent(StatusText);

	UCanvasPanelSlot* StatusSlot = StatusCanvas->AddChildToCanvas(StatusBorder);
	StatusSlot->SetAnchors(FAnchors(0.5f, 0.0f));
	StatusSlot->SetAlignment(FVector2D(0.5f, 0.0f));
	StatusSlot->SetPosition(FVector2D(0.0f, 24.0f));
	StatusSlot->SetSize(FVector2D(440.0f, 62.0f));
	SetVisibility(ESlateVisibility::HitTestInvisible);
	UE_LOG(LogTemp, Display, TEXT("BOARD_STATUS_UI_BUILT root=%s text=%s"),
		WidgetTree->RootWidget ? TEXT("true") : TEXT("false"),
		StatusText ? TEXT("true") : TEXT("false"));
	return Super::RebuildWidget();
}

void UBoardStatusWidget::UpdateBoardStatus(
	EJanggiTeam LocalTeam,
	EJanggiTeam TurnTeam,
	EJanggiTeam WinnerTeam,
	EBoardMatchPhase Phase,
	bool bInputPaused,
	float RemainingSeconds)
{
	if (!StatusBorder || !StatusText)
	{
		return;
	}

	FText NewText;
	FLinearColor NewColor(0.05f, 0.05f, 0.05f, 0.88f);
	if (LocalTeam == EJanggiTeam::Unassigned)
	{
		NewText = FText::FromString(TEXT("플레이어 배정 대기 중"));
	}
	else if (Phase == EBoardMatchPhase::MatchFinished)
	{
		NewText = FText::Format(
			FText::FromString(TEXT("게임 종료 · {0} 승리")),
			TeamDisplayName(WinnerTeam));
		NewColor = WinnerTeam == EJanggiTeam::Blue
			? FLinearColor(0.02f, 0.16f, 0.55f, 0.96f)
			: FLinearColor(0.55f, 0.04f, 0.03f, 0.96f);
	}
	else if (Phase == EBoardMatchPhase::ArenaTransition || Phase == EBoardMatchPhase::ArenaBattle ||
		Phase == EBoardMatchPhase::BattleResolution)
	{
		NewText = FText::FromString(TEXT("기물 충돌 전투 진행 중"));
		NewColor = FLinearColor(0.35f, 0.12f, 0.02f, 0.92f);
	}
	else if (bInputPaused)
	{
		NewText = FText::Format(
			FText::FromString(TEXT("내 팀: {0} · 진형 선택/상대 대기 중")),
			TeamDisplayName(LocalTeam));
	}
	else
	{
		const bool bMyTurn = TurnTeam == LocalTeam;
		const int32 Seconds = FMath::Max(0, FMath::CeilToInt(RemainingSeconds));
		NewText = FText::Format(
			FText::FromString(bMyTurn
				? TEXT("내 차례 · {0} · {1}초")
				: TEXT("상대 차례 · {0} · {1}초")),
			TeamDisplayName(TurnTeam),
			FText::AsNumber(Seconds));
		NewColor = TurnTeam == EJanggiTeam::Blue
			? FLinearColor(0.02f, 0.16f, 0.55f, 0.92f)
			: FLinearColor(0.55f, 0.04f, 0.03f, 0.92f);
	}

	if (!NewText.EqualTo(LastStatusText))
	{
		StatusText->SetText(NewText);
		LastStatusText = NewText;
	}
	if (!NewColor.Equals(LastStatusColor))
	{
		StatusBorder->SetBrushColor(NewColor);
		LastStatusColor = NewColor;
	}
}

void UBoardStatusWidget::ShowNotice(const FText& Message, bool bError)
{
	if (!StatusBorder || !StatusText)
	{
		return;
	}
	StatusText->SetText(Message);
	StatusBorder->SetBrushColor(bError
		? FLinearColor(0.62f, 0.05f, 0.02f, 0.95f)
		: FLinearColor(0.04f, 0.42f, 0.12f, 0.95f));
	LastStatusText = Message;
	LastStatusColor = StatusBorder->GetBrushColor();
}
