// Copyright Epic Games, Inc. All Rights Reserved.

#include "FormationSelectionWidget.h"

#include "AuthoritativeJanggiBoard.h"
#include "BoardPlayerController.h"
#include "Blueprint/WidgetTree.h"
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
UButton* AddFormationButton(UWidgetTree* WidgetTree, UHorizontalBox* Row, const FString& Label)
{
	UButton* Button = WidgetTree->ConstructWidget<UButton>();
	UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>();
	Text->SetText(FText::FromString(Label));
	Text->SetJustification(ETextJustify::Center);
	Text->SetFont(FSlateFontInfo(Text->GetFont().FontObject, 24));
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

TSharedRef<SWidget> UFormationSelectionWidget::RebuildWidget()
{
	if (!WidgetTree) WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"));
	if (WidgetTree->RootWidget) return Super::RebuildWidget();

	UCanvasPanel* Canvas = WidgetTree->ConstructWidget<UCanvasPanel>();
	WidgetTree->RootWidget = Canvas;
	UVerticalBox* Panel = WidgetTree->ConstructWidget<UVerticalBox>();
	UCanvasPanelSlot* PanelSlot = Canvas->AddChildToCanvas(Panel);
	PanelSlot->SetAnchors(FAnchors(0.5f, 1.0f));
	PanelSlot->SetAlignment(FVector2D(0.5f, 1.0f));
	PanelSlot->SetPosition(FVector2D(0.0f, -28.0f));
	PanelSlot->SetSize(FVector2D(920.0f, 130.0f));

	UTextBlock* Title = WidgetTree->ConstructWidget<UTextBlock>();
	Title->SetText(FText::FromString(TEXT("진형을 선택하세요")));
	Title->SetJustification(ETextJustify::Center);
	Title->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	Title->SetFont(FSlateFontInfo(Title->GetFont().FontObject, 28));
	if (UVerticalBoxSlot* TitleSlot = Panel->AddChildToVerticalBox(Title))
	{
		TitleSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 6.0f));
		TitleSlot->SetHorizontalAlignment(HAlign_Fill);
	}

	UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>();
	if (UVerticalBoxSlot* RowSlot = Panel->AddChildToVerticalBox(Row))
	{
		RowSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		RowSlot->SetHorizontalAlignment(HAlign_Fill);
	}

	AddFormationButton(WidgetTree, Row, TEXT("마-상-상-마"))->OnClicked.AddDynamic(
		this, &UFormationSelectionWidget::SelectHorseElephantElephantHorse);
	AddFormationButton(WidgetTree, Row, TEXT("마-상-마-상"))->OnClicked.AddDynamic(
		this, &UFormationSelectionWidget::SelectHorseElephantHorseElephant);
	AddFormationButton(WidgetTree, Row, TEXT("상-마-상-마"))->OnClicked.AddDynamic(
		this, &UFormationSelectionWidget::SelectElephantHorseElephantHorse);
	AddFormationButton(WidgetTree, Row, TEXT("상-마-마-상"))->OnClicked.AddDynamic(
		this, &UFormationSelectionWidget::SelectElephantHorseHorseElephant);
	return Super::RebuildWidget();
}

void UFormationSelectionWidget::SelectHorseElephantElephantHorse() { SubmitFormation(0); }
void UFormationSelectionWidget::SelectHorseElephantHorseElephant() { SubmitFormation(1); }
void UFormationSelectionWidget::SelectElephantHorseElephantHorse() { SubmitFormation(2); }
void UFormationSelectionWidget::SelectElephantHorseHorseElephant() { SubmitFormation(3); }

void UFormationSelectionWidget::SubmitFormation(uint8 FormationValue)
{
	if (ABoardPlayerController* Controller = GetOwningPlayer<ABoardPlayerController>())
	{
		Controller->RequestFormation(static_cast<EJanggiFormation>(FormationValue));
		RemoveFromParent();
	}
}
