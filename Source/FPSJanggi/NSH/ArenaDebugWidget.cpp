// Copyright Epic Games, Inc. All Rights Reserved.

#include "ArenaDebugWidget.h"

#include "AuthoritativeJanggiBoard.h"
#include "BoardPlayerController.h"
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
	Button->SetBackgroundColor(Color);
	UTextBlock* Text = Tree->ConstructWidget<UTextBlock>();
	Text->SetText(FText::FromString(Label));
	Text->SetJustification(ETextJustify::Center);
	Text->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	Text->SetFont(FSlateFontInfo(Text->GetFont().FontObject, 22));
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
	Background->SetBrushColor(FLinearColor(0.02f, 0.02f, 0.02f, 0.88f));
	Background->SetPadding(FMargin(12.0f));
	UCanvasPanelSlot* BackgroundSlot = Canvas->AddChildToCanvas(Background);
	BackgroundSlot->SetAnchors(FAnchors(0.5f, 1.0f));
	BackgroundSlot->SetAlignment(FVector2D(0.5f, 1.0f));
	BackgroundSlot->SetPosition(FVector2D(0.0f, -28.0f));
	BackgroundSlot->SetSize(FVector2D(620.0f, 118.0f));

	UVerticalBox* Panel = WidgetTree->ConstructWidget<UVerticalBox>();
	Background->SetContent(Panel);
	UTextBlock* Title = WidgetTree->ConstructWidget<UTextBlock>();
	Title->SetText(FText::FromString(TEXT("전투 판정 디버그 · 임시 기능")));
	Title->SetJustification(ETextJustify::Center);
	Title->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	Title->SetFont(FSlateFontInfo(Title->GetFont().FontObject, 20));
	if (UVerticalBoxSlot* TitleSlot = Panel->AddChildToVerticalBox(Title))
	{
		TitleSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 4.0f));
		TitleSlot->SetHorizontalAlignment(HAlign_Fill);
	}

	UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>();
	if (UVerticalBoxSlot* RowSlot = Panel->AddChildToVerticalBox(Row))
	{
		RowSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		RowSlot->SetHorizontalAlignment(HAlign_Fill);
	}
	AddWinnerButton(WidgetTree, Row, TEXT("청 승리로 전투 종료"), FLinearColor(0.03f, 0.18f, 0.75f, 1.0f))->OnClicked.AddDynamic(
		this, &UArenaDebugWidget::ResolveBlueWinner);
	AddWinnerButton(WidgetTree, Row, TEXT("한 승리로 전투 종료"), FLinearColor(0.75f, 0.04f, 0.03f, 1.0f))->OnClicked.AddDynamic(
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
