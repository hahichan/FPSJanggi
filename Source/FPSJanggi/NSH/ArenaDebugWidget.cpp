// Copyright Epic Games, Inc. All Rights Reserved.

#include "ArenaDebugWidget.h"

#include "AuthoritativeJanggiBoard.h"
#include "BoardPlayerController.h"
#include "JanggiUIStyle.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Blueprint/WidgetTree.h"
#include "Engine/Blueprint.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/ComboBoxString.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "GameFramework/Pawn.h"
#include "Misc/PackageName.h"
#include "Modules/ModuleManager.h"

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
	BackgroundSlot->SetPosition(FVector2D(0.0f, -18.0f));
	BackgroundSlot->SetSize(FVector2D(760.0f, 250.0f));

	UVerticalBox* Panel = WidgetTree->ConstructWidget<UVerticalBox>();
	Background->SetContent(Panel);
	RebuildPieceClassCandidates();
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

	UHorizontalBox* SelectionRow = WidgetTree->ConstructWidget<UHorizontalBox>();
	if (UVerticalBoxSlot* SelectionRowSlot = Panel->AddChildToVerticalBox(SelectionRow))
	{
		SelectionRowSlot->SetPadding(FMargin(0.0f, 4.0f, 0.0f, 6.0f));
		SelectionRowSlot->SetHorizontalAlignment(HAlign_Fill);
	}

	UVerticalBox* BlueColumn = WidgetTree->ConstructWidget<UVerticalBox>();
	if (UHorizontalBoxSlot* BlueColumnSlot = SelectionRow->AddChildToHorizontalBox(BlueColumn))
	{
		BlueColumnSlot->SetPadding(FMargin(6.0f));
		BlueColumnSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	}

	UTextBlock* BlueLabel = WidgetTree->ConstructWidget<UTextBlock>();
	BlueLabel->SetText(FText::FromString(TEXT("Blue Piece (piece folder)")));
	FPSJanggiUI::StyleText(BlueLabel, 12, FPSJanggiUI::MutedIvory(), true);
	BlueColumn->AddChildToVerticalBox(BlueLabel);

	BluePieceCombo = WidgetTree->ConstructWidget<UComboBoxString>();
	if (BluePieceCombo)
	{
		for (const FString& Label : CandidateLabels)
		{
			BluePieceCombo->AddOption(Label);
		}
		if (CandidateLabels.Num() > 0)
		{
			BluePieceCombo->SetSelectedOption(CandidateLabels[0]);
		}
		BlueColumn->AddChildToVerticalBox(BluePieceCombo);
	}

	UVerticalBox* RedColumn = WidgetTree->ConstructWidget<UVerticalBox>();
	if (UHorizontalBoxSlot* RedColumnSlot = SelectionRow->AddChildToHorizontalBox(RedColumn))
	{
		RedColumnSlot->SetPadding(FMargin(6.0f));
		RedColumnSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	}

	UTextBlock* RedLabel = WidgetTree->ConstructWidget<UTextBlock>();
	RedLabel->SetText(FText::FromString(TEXT("Red Piece (piece folder)")));
	FPSJanggiUI::StyleText(RedLabel, 12, FPSJanggiUI::MutedIvory(), true);
	RedColumn->AddChildToVerticalBox(RedLabel);

	RedPieceCombo = WidgetTree->ConstructWidget<UComboBoxString>();
	if (RedPieceCombo)
	{
		for (const FString& Label : CandidateLabels)
		{
			RedPieceCombo->AddOption(Label);
		}
		if (CandidateLabels.Num() > 0)
		{
			RedPieceCombo->SetSelectedOption(CandidateLabels[0]);
		}
		RedColumn->AddChildToVerticalBox(RedPieceCombo);
	}

	SelectionSummaryText = WidgetTree->ConstructWidget<UTextBlock>();
	FPSJanggiUI::StyleText(SelectionSummaryText.Get(), 11, FPSJanggiUI::MutedIvory());
	if (UVerticalBoxSlot* SummarySlot = Panel->AddChildToVerticalBox(SelectionSummaryText))
	{
		SummarySlot->SetPadding(FMargin(0.0f, 2.0f, 0.0f, 4.0f));
		SummarySlot->SetHorizontalAlignment(HAlign_Fill);
	}
	RefreshSelectionSummary();

	UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>();
	if (UVerticalBoxSlot* RowSlot = Panel->AddChildToVerticalBox(Row))
	{
		RowSlot->SetPadding(FMargin(0.0f, 4.0f, 0.0f, 0.0f));
		RowSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
		RowSlot->SetHorizontalAlignment(HAlign_Fill);
	}
	AddWinnerButton(WidgetTree, Row, TEXT("YJH Arena Start"), FLinearColor(0.08f, 0.38f, 0.18f, 1.0f))->OnClicked.AddDynamic(
		this, &UArenaDebugWidget::RequestYJHArenaStart);
	AddWinnerButton(WidgetTree, Row, TEXT("청 승리 · 전투 종료"), FLinearColor(0.035f, 0.16f, 0.52f, 1.0f))->OnClicked.AddDynamic(
		this, &UArenaDebugWidget::ResolveBlueWinner);
	AddWinnerButton(WidgetTree, Row, TEXT("한 승리 · 전투 종료"), FLinearColor(0.50f, 0.055f, 0.035f, 1.0f))->OnClicked.AddDynamic(
		this, &UArenaDebugWidget::ResolveRedWinner);
	return Super::RebuildWidget();
}

void UArenaDebugWidget::RebuildPieceClassCandidates()
{
	CandidateLabels.Reset();
	LabelToClassPath.Reset();

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	FARFilter Filter;
	Filter.PackagePaths.Add(FName(TEXT("/Game/User/piece")));
	Filter.ClassPaths.Add(UBlueprint::StaticClass()->GetClassPathName());
	Filter.bRecursivePaths = true;

	TArray<FAssetData> BlueprintAssets;
	AssetRegistry.GetAssets(Filter, BlueprintAssets);

	TSet<FString> SeenPaths;
	for (const FAssetData& Asset : BlueprintAssets)
	{
		FString GeneratedClassExportPath;
		if (!Asset.GetTagValue(FName(TEXT("GeneratedClass")), GeneratedClassExportPath) || GeneratedClassExportPath.IsEmpty())
		{
			continue;
		}

		const FString ClassObjectPath = FPackageName::ExportTextPathToObjectPath(GeneratedClassExportPath);
		if (SeenPaths.Contains(ClassObjectPath))
		{
			continue;
		}

		UClass* LoadedClass = StaticLoadClass(APawn::StaticClass(), nullptr, *ClassObjectPath);
		if (!LoadedClass || !LoadedClass->IsChildOf(APawn::StaticClass()) || LoadedClass->HasAnyClassFlags(CLASS_Abstract))
		{
			continue;
		}

		SeenPaths.Add(ClassObjectPath);
		FString Label = LoadedClass->GetName();
		if (LabelToClassPath.Contains(Label))
		{
			Label = FString::Printf(TEXT("%s (%s)"), *LoadedClass->GetName(), *Asset.AssetName.ToString());
		}

		CandidateLabels.Add(Label);
		LabelToClassPath.Add(Label, ClassObjectPath);
	}

	CandidateLabels.Sort();
}

void UArenaDebugWidget::RefreshSelectionSummary()
{
	if (!SelectionSummaryText.Get())
	{
		return;
	}

	if (CandidateLabels.Num() == 0)
	{
		SelectionSummaryText.Get()->SetText(FText::FromString(TEXT("piece 폴더에서 Pawn Blueprint를 찾지 못했습니다.")));
		return;
	}

	const FString BlueSelected = BluePieceCombo ? BluePieceCombo->GetSelectedOption() : TEXT("None");
	const FString RedSelected = RedPieceCombo ? RedPieceCombo->GetSelectedOption() : TEXT("None");
	SelectionSummaryText.Get()->SetText(FText::FromString(FString::Printf(
		TEXT("선택됨  Blue: %s  |  Red: %s"),
		*BlueSelected,
		*RedSelected)));
}

void UArenaDebugWidget::ResolveBlueWinner() { ResolveWinner(static_cast<uint8>(EJanggiTeam::Blue)); }
void UArenaDebugWidget::ResolveRedWinner() { ResolveWinner(static_cast<uint8>(EJanggiTeam::Red)); }

void UArenaDebugWidget::RequestYJHArenaStart()
{
	if (ABoardPlayerController* Controller = GetOwningPlayer<ABoardPlayerController>())
	{
		FString BlueClassPath;
		FString RedClassPath;

		if (BluePieceCombo)
		{
			if (const FString* Path = LabelToClassPath.Find(BluePieceCombo->GetSelectedOption()))
			{
				BlueClassPath = *Path;
			}
		}

		if (RedPieceCombo)
		{
			if (const FString* Path = LabelToClassPath.Find(RedPieceCombo->GetSelectedOption()))
			{
				RedClassPath = *Path;
			}
		}

		if (!BlueClassPath.IsEmpty() && !RedClassPath.IsEmpty())
		{
			Controller->RequestYJHArenaStartWithClassPaths(BlueClassPath, RedClassPath);
		}
		else
		{
			Controller->RequestYJHArenaStart();
		}
	}
}

void UArenaDebugWidget::ResolveWinner(uint8 TeamValue)
{
	if (ABoardPlayerController* Controller = GetOwningPlayer<ABoardPlayerController>())
	{
		Controller->RequestDebugArenaWinner(static_cast<EJanggiTeam>(TeamValue));
	}
}
