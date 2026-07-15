// Copyright Epic Games, Inc. All Rights Reserved.

#include "ArenaDebugWidget.h"

#include "AuthoritativeJanggiBoard.h"
#include "BoardPlayerController.h"
#include "JanggiUIStyle.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"

namespace
{
UButton* AddWinnerButton(UWidgetTree* Tree, UHorizontalBox* Row, const FString& Label, const FLinearColor& Color)
{
	UButton* Button = Tree->ConstructWidget<UButton>();
	FPSJanggiUI::StyleButton(
		Button,
		Color,
		FMath::Lerp(Color, FLinearColor::White, 0.13f),
		FMath::Lerp(Color, FLinearColor::Black, 0.22f),
		10.0f,
		FLinearColor(0.65f, 0.50f, 0.24f, 0.85f));
	UTextBlock* Text = Tree->ConstructWidget<UTextBlock>();
	Text->SetText(FText::FromString(Label));
	Text->SetJustification(ETextJustify::Center);
	FPSJanggiUI::StyleText(Text, 19, FPSJanggiUI::Ivory(), true);
	Button->AddChild(Text);
	if (UHorizontalBoxSlot* Slot = Row->AddChildToHorizontalBox(Button))
	{
		Slot->SetPadding(FMargin(8.0f));
		Slot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		Slot->SetHorizontalAlignment(HAlign_Fill);
		Slot->SetVerticalAlignment(VAlign_Fill);
	}
	return Button;
}
}

TSharedRef<SWidget> UArenaDebugWidget::RebuildWidget()
{
	if (!WidgetTree)
	{
		WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"));
	}
	if (WidgetTree->RootWidget)
	{
		return Super::RebuildWidget();
	}

	UCanvasPanel* Canvas = WidgetTree->ConstructWidget<UCanvasPanel>();
	WidgetTree->RootWidget = Canvas;
	UBorder* Background = WidgetTree->ConstructWidget<UBorder>();
	FPSJanggiUI::StylePanel(
		Background,
		FLinearColor(0.025f, 0.018f, 0.013f, 0.95f),
		16.0f,
		FLinearColor(0.48f, 0.30f, 0.10f, 1.0f),
		1.5f);
	Background->SetPadding(FMargin(18.0f, 12.0f, 18.0f, 16.0f));
	UCanvasPanelSlot* BackgroundSlot = Canvas->AddChildToCanvas(Background);
	BackgroundSlot->SetAnchors(FAnchors(0.5f, 1.0f));
	BackgroundSlot->SetAlignment(FVector2D(0.5f, 1.0f));
	BackgroundSlot->SetPosition(FVector2D(0.0f, -24.0f));
	BackgroundSlot->SetSize(FVector2D(670.0f, 144.0f));

	UVerticalBox* Panel = WidgetTree->ConstructWidget<UVerticalBox>();
	Background->SetContent(Panel);
	UTextBlock* Title = WidgetTree->ConstructWidget<UTextBlock>();
	Title->SetText(FText::FromString(TEXT("전투 결과 판정")));
	Title->SetJustification(ETextJustify::Center);
	FPSJanggiUI::StyleText(Title, 22, FPSJanggiUI::Ivory(), true);
	if (UVerticalBoxSlot* TitleSlot = Panel->AddChildToVerticalBox(Title))
	{
		TitleSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 4.0f));
		TitleSlot->SetHorizontalAlignment(HAlign_Fill);
	}
	UTextBlock* Subtitle = WidgetTree->ConstructWidget<UTextBlock>();
	Subtitle->SetText(FText::FromString(TEXT("개발용 임시 판정 · 실제 전투 시스템 연동 전용")));
	Subtitle->SetJustification(ETextJustify::Center);
	FPSJanggiUI::StyleText(Subtitle, 13, FPSJanggiUI::MutedIvory());
	if (UVerticalBoxSlot* SubtitleSlot = Panel->AddChildToVerticalBox(Subtitle))
	{
		SubtitleSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 5.0f));
		SubtitleSlot->SetHorizontalAlignment(HAlign_Fill);
	}

	UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>();
	if (UVerticalBoxSlot* RowSlot = Panel->AddChildToVerticalBox(Row))
	{
		RowSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		RowSlot->SetHorizontalAlignment(HAlign_Fill);
	}
	AddWinnerButton(WidgetTree, Row, TEXT("청 승리 · 전투 종료"), FLinearColor(0.035f, 0.16f, 0.52f, 1.0f))->OnClicked.AddDynamic(
		this, &UArenaDebugWidget::ResolveBlueWinner);
	AddWinnerButton(WidgetTree, Row, TEXT("한 승리 · 전투 종료"), FLinearColor(0.50f, 0.055f, 0.035f, 1.0f))->OnClicked.AddDynamic(
		this, &UArenaDebugWidget::ResolveRedWinner);
	return Super::RebuildWidget();
}

void UArenaDebugWidget::ResolveBlueWinner() { ResolveWinner(static_cast<uint8>(EJanggiTeam::Blue)); }
void UArenaDebugWidget::ResolveRedWinner() { ResolveWinner(static_cast<uint8>(EJanggiTeam::Red)); }

void UArenaDebugWidget::ResolveWinner(uint8 TeamValue)
{
	if (ABoardPlayerController* Controller = GetOwningPlayer<ABoardPlayerController>())
	{
		Controller->RequestDebugArenaWinner(static_cast<EJanggiTeam>(TeamValue));
	}
}
