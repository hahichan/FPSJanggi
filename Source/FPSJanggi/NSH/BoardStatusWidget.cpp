// Copyright Epic Games, Inc. All Rights Reserved.

#include "BoardStatusWidget.h"

#include "BoardPlayerController.h"
#include "JanggiUIStyle.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"

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
	FPSJanggiUI::StylePanel(
		StatusBorder,
		FLinearColor(0.025f, 0.021f, 0.016f, 0.94f),
		15.0f,
		FLinearColor(0.31f, 0.23f, 0.11f, 0.95f),
		1.0f);
	StatusBorder->SetPadding(FMargin(24.0f, 13.0f));
	StatusBorder->SetHorizontalAlignment(HAlign_Center);
	StatusBorder->SetVerticalAlignment(VAlign_Center);
	StatusBorder->SetVisibility(ESlateVisibility::HitTestInvisible);

	UVerticalBox* Stack = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("TurnStatusStack"));
	StatusBorder->SetContent(Stack);

	EyebrowText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TurnEyebrowText"));
	EyebrowText->SetText(FText::FromString(TEXT("FPS 장기 · 대국 진행")));
	EyebrowText->SetJustification(ETextJustify::Center);
	FPSJanggiUI::StyleText(EyebrowText, 13, FPSJanggiUI::Gold(), true);
	if (UVerticalBoxSlot* LayoutSlot = Stack->AddChildToVerticalBox(EyebrowText))
	{
		LayoutSlot->SetHorizontalAlignment(HAlign_Fill);
		LayoutSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 2.0f));
	}

	StatusText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TurnStatusText"));
	StatusText->SetJustification(ETextJustify::Center);
	FPSJanggiUI::StyleText(StatusText, 27, FPSJanggiUI::Ivory(), true, 1);
	if (UVerticalBoxSlot* LayoutSlot = Stack->AddChildToVerticalBox(StatusText))
	{
		LayoutSlot->SetHorizontalAlignment(HAlign_Fill);
	}

	DetailText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TurnDetailText"));
	DetailText->SetText(FText::FromString(TEXT("기물을 선택해 행마를 진행하세요")));
	DetailText->SetJustification(ETextJustify::Center);
	FPSJanggiUI::StyleText(DetailText, 14, FPSJanggiUI::MutedIvory());
	if (UVerticalBoxSlot* LayoutSlot = Stack->AddChildToVerticalBox(DetailText))
	{
		LayoutSlot->SetHorizontalAlignment(HAlign_Fill);
		LayoutSlot->SetPadding(FMargin(0.0f, 2.0f, 0.0f, 0.0f));
	}

	ReturnLobbyButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("ReturnLobbyButton"));
	FPSJanggiUI::StyleButton(
		ReturnLobbyButton,
		FLinearColor(0.34f, 0.22f, 0.075f, 1.0f),
		FLinearColor(0.48f, 0.32f, 0.10f, 1.0f),
		FLinearColor(0.22f, 0.14f, 0.045f, 1.0f),
		8.0f,
		FPSJanggiUI::Gold());
	UTextBlock* ReturnLabel = WidgetTree->ConstructWidget<UTextBlock>();
	ReturnLabel->SetText(FText::FromString(TEXT("로비로 돌아가기")));
	ReturnLabel->SetJustification(ETextJustify::Center);
	FPSJanggiUI::StyleText(ReturnLabel, 17, FPSJanggiUI::Ivory(), true);
	ReturnLobbyButton->AddChild(ReturnLabel);
	ReturnLobbyButton->OnClicked.AddDynamic(this, &UBoardStatusWidget::ReturnToLobby);
	ReturnLobbyButton->SetVisibility(ESlateVisibility::Collapsed);
	if (UVerticalBoxSlot* LayoutSlot = Stack->AddChildToVerticalBox(ReturnLobbyButton))
	{
		LayoutSlot->SetHorizontalAlignment(HAlign_Center);
		LayoutSlot->SetPadding(FMargin(0.0f, 8.0f, 0.0f, 0.0f));
	}

	UCanvasPanelSlot* StatusSlot = StatusCanvas->AddChildToCanvas(StatusBorder);
	StatusSlot->SetAnchors(FAnchors(0.5f, 0.0f));
	StatusSlot->SetAlignment(FVector2D(0.5f, 0.0f));
	StatusSlot->SetPosition(FVector2D(0.0f, 20.0f));
	StatusSlot->SetSize(FVector2D(590.0f, 146.0f));
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
	FText NewDetail = FText::FromString(TEXT("연결 상태를 확인하고 있습니다"));
	FLinearColor NewColor(0.025f, 0.021f, 0.016f, 0.94f);
	if (LocalTeam == EJanggiTeam::Unassigned)
	{
		NewText = FText::FromString(TEXT("플레이어 배정 대기 중"));
	}
	else if (Phase == EBoardMatchPhase::MatchFinished)
	{
		NewText = FText::Format(
			FText::FromString(TEXT("대국 종료 · {0} 승리")),
			TeamDisplayName(WinnerTeam));
		NewDetail = FText::FromString(TEXT("수고하셨습니다. 로비에서 다음 대국을 준비할 수 있습니다"));
		NewColor = WinnerTeam == EJanggiTeam::Blue
			? FLinearColor(0.025f, 0.09f, 0.23f, 0.97f)
			: FLinearColor(0.24f, 0.035f, 0.025f, 0.97f);
	}
	else if (Phase == EBoardMatchPhase::ArenaTransition || Phase == EBoardMatchPhase::ArenaBattle ||
		Phase == EBoardMatchPhase::BattleResolution)
	{
		NewText = FText::FromString(TEXT("기물 충돌 · 전투 진행 중"));
		NewDetail = FText::FromString(TEXT("전투 결과가 장기판에 반영됩니다"));
		NewColor = FLinearColor(0.22f, 0.105f, 0.025f, 0.97f);
	}
	else if (bInputPaused)
	{
		NewText = FText::Format(
			FText::FromString(TEXT("{0} 진영 · 진형 선택 대기")),
			TeamDisplayName(LocalTeam));
		NewDetail = FText::FromString(TEXT("양쪽 플레이어가 진형을 고르면 대국이 시작됩니다"));
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
		NewDetail = FText::FromString(bMyTurn
			? TEXT("기물을 선택해 이동할 교차점을 지정하세요")
			: TEXT("상대의 행마를 기다리는 중입니다"));
		NewColor = TurnTeam == EJanggiTeam::Blue
			? FLinearColor(0.025f, 0.085f, 0.22f, 0.97f)
			: FLinearColor(0.23f, 0.032f, 0.023f, 0.97f);
	}

	if (!NewText.EqualTo(LastStatusText))
	{
		StatusText->SetText(NewText);
		LastStatusText = NewText;
	}
	if (DetailText)
	{
		DetailText->SetText(NewDetail);
	}
	if (EyebrowText)
	{
		EyebrowText->SetText(FText::FromString(
			LocalTeam == EJanggiTeam::Blue ? TEXT("청 진영 · FPS 장기") :
			LocalTeam == EJanggiTeam::Red ? TEXT("한 진영 · FPS 장기") : TEXT("FPS 장기 · 대국 준비")));
	}
	if (ReturnLobbyButton)
	{
		ReturnLobbyButton->SetVisibility(
			Phase == EBoardMatchPhase::MatchFinished ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		StatusBorder->SetVisibility(
			Phase == EBoardMatchPhase::MatchFinished ? ESlateVisibility::Visible : ESlateVisibility::HitTestInvisible);
	}
	if (!NewColor.Equals(LastStatusColor))
	{
		FPSJanggiUI::StylePanel(StatusBorder, NewColor, 15.0f, FPSJanggiUI::Gold(), 1.0f);
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
	const FLinearColor NoticeColor = bError
		? FLinearColor(0.42f, 0.045f, 0.025f, 0.97f)
		: FLinearColor(0.025f, 0.25f, 0.14f, 0.97f);
	FPSJanggiUI::StylePanel(StatusBorder, NoticeColor, 15.0f, bError ? FPSJanggiUI::Red() : FPSJanggiUI::Jade(), 1.5f);
	if (DetailText) DetailText->SetText(FText::GetEmpty());
	LastStatusText = Message;
	LastStatusColor = NoticeColor;
}

void UBoardStatusWidget::ReturnToLobby()
{
	if (ABoardPlayerController* Controller = GetOwningPlayer<ABoardPlayerController>())
	{
		Controller->RequestReturnToLobby();
	}
}
