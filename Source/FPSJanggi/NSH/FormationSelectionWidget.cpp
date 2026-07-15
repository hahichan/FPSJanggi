// Copyright Epic Games, Inc. All Rights Reserved.

#include "FormationSelectionWidget.h"

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
UButton* AddFormationButton(
	UWidgetTree* WidgetTree,
	UHorizontalBox* Row,
	const FString& Label,
	const FString& Description,
	int32 Number)
{
	UButton* Button = WidgetTree->ConstructWidget<UButton>();
	FPSJanggiUI::StyleButton(
		Button,
		FLinearColor(0.11f, 0.085f, 0.052f, 1.0f),
		FLinearColor(0.19f, 0.135f, 0.065f, 1.0f),
		FLinearColor(0.075f, 0.055f, 0.035f, 1.0f),
		11.0f,
		FLinearColor(0.32f, 0.24f, 0.12f, 1.0f));
	UVerticalBox* Content = WidgetTree->ConstructWidget<UVerticalBox>();
	Button->AddChild(Content);
	UTextBlock* NumberText = WidgetTree->ConstructWidget<UTextBlock>();
	NumberText->SetText(FText::FromString(FString::Printf(TEXT("진형 %d"), Number)));
	NumberText->SetJustification(ETextJustify::Center);
	FPSJanggiUI::StyleText(NumberText, 12, FPSJanggiUI::Gold(), true);
	Content->AddChildToVerticalBox(NumberText);
	UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>();
	Text->SetText(FText::FromString(Label));
	Text->SetJustification(ETextJustify::Center);
	FPSJanggiUI::StyleText(Text, 23, FPSJanggiUI::Ivory(), true);
	if (UVerticalBoxSlot* TextSlot = Content->AddChildToVerticalBox(Text))
	{
		TextSlot->SetPadding(FMargin(0.0f, 3.0f));
	}
	UTextBlock* Hint = WidgetTree->ConstructWidget<UTextBlock>();
	Hint->SetText(FText::FromString(Description));
	Hint->SetJustification(ETextJustify::Center);
	FPSJanggiUI::StyleText(Hint, 12, FPSJanggiUI::MutedIvory());
	Content->AddChildToVerticalBox(Hint);
	if (UHorizontalBoxSlot* Slot = Row->AddChildToHorizontalBox(Button))
	{
		Slot->SetPadding(FMargin(7.0f));
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
	UBorder* Background = WidgetTree->ConstructWidget<UBorder>();
	FPSJanggiUI::StylePanel(
		Background,
		FLinearColor(0.022f, 0.018f, 0.014f, 0.95f),
		18.0f,
		FLinearColor(0.42f, 0.30f, 0.13f, 0.95f),
		1.5f);
	Background->SetPadding(FMargin(20.0f, 14.0f, 20.0f, 18.0f));
	UCanvasPanelSlot* BackgroundSlot = Canvas->AddChildToCanvas(Background);
	BackgroundSlot->SetAnchors(FAnchors(0.5f, 1.0f));
	BackgroundSlot->SetAlignment(FVector2D(0.5f, 1.0f));
	BackgroundSlot->SetPosition(FVector2D(0.0f, -24.0f));
	BackgroundSlot->SetSize(FVector2D(1050.0f, 210.0f));
	UVerticalBox* Panel = WidgetTree->ConstructWidget<UVerticalBox>();
	Background->SetContent(Panel);

	UTextBlock* Title = WidgetTree->ConstructWidget<UTextBlock>();
	Title->SetText(FText::FromString(TEXT("초기 진형 선택")));
	Title->SetJustification(ETextJustify::Center);
	FPSJanggiUI::StyleText(Title, 27, FPSJanggiUI::Ivory(), true);
	if (UVerticalBoxSlot* TitleSlot = Panel->AddChildToVerticalBox(Title))
	{
		TitleSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 1.0f));
		TitleSlot->SetHorizontalAlignment(HAlign_Fill);
	}
	UTextBlock* Subtitle = WidgetTree->ConstructWidget<UTextBlock>();
	Subtitle->SetText(FText::FromString(TEXT("양쪽 끝의 마(馬)와 상(象) 배치를 선택하세요 · 진영 방향은 바뀌지 않습니다")));
	Subtitle->SetJustification(ETextJustify::Center);
	FPSJanggiUI::StyleText(Subtitle, 14, FPSJanggiUI::MutedIvory());
	if (UVerticalBoxSlot* SubtitleSlot = Panel->AddChildToVerticalBox(Subtitle))
	{
		SubtitleSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 7.0f));
		SubtitleSlot->SetHorizontalAlignment(HAlign_Fill);
	}

	UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>();
	if (UVerticalBoxSlot* RowSlot = Panel->AddChildToVerticalBox(Row))
	{
		RowSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		RowSlot->SetHorizontalAlignment(HAlign_Fill);
	}

	AddFormationButton(WidgetTree, Row, TEXT("마 · 상 │ 상 · 마"), TEXT("양끝에 마 배치"), 1)->OnClicked.AddDynamic(
		this, &UFormationSelectionWidget::SelectHorseElephantElephantHorse);
	AddFormationButton(WidgetTree, Row, TEXT("마 · 상 │ 마 · 상"), TEXT("좌우 교차 배치"), 2)->OnClicked.AddDynamic(
		this, &UFormationSelectionWidget::SelectHorseElephantHorseElephant);
	AddFormationButton(WidgetTree, Row, TEXT("상 · 마 │ 상 · 마"), TEXT("반대 교차 배치"), 3)->OnClicked.AddDynamic(
		this, &UFormationSelectionWidget::SelectElephantHorseElephantHorse);
	AddFormationButton(WidgetTree, Row, TEXT("상 · 마 │ 마 · 상"), TEXT("양끝에 상 배치"), 4)->OnClicked.AddDynamic(
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
