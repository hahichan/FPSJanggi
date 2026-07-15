// Copyright Epic Games, Inc. All Rights Reserved.

#include "LobbyRoomEntryWidget.h"

#include "JanggiUIStyle.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/GameInstance.h"

void ULobbyRoomEntryWidget::InitializeRoom(const FNSHRoomInfo& InRoom)
{
	RoomInfo = InRoom;
}

TSharedRef<SWidget> ULobbyRoomEntryWidget::RebuildWidget()
{
	if (!WidgetTree) WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"));
	if (WidgetTree->RootWidget) return Super::RebuildWidget();

	UBorder* Card = WidgetTree->ConstructWidget<UBorder>();
	FPSJanggiUI::StylePanel(
		Card,
		FLinearColor(0.055f, 0.045f, 0.032f, 0.96f),
		10.0f,
		FLinearColor(0.19f, 0.15f, 0.09f, 1.0f),
		1.0f);
	Card->SetPadding(FMargin(14.0f, 10.0f));
	WidgetTree->RootWidget = Card;

	UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>();
	Card->SetContent(Row);
	UVerticalBox* Information = WidgetTree->ConstructWidget<UVerticalBox>();
	if (UHorizontalBoxSlot* LayoutSlot = Row->AddChildToHorizontalBox(Information))
	{
		LayoutSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		LayoutSlot->SetVerticalAlignment(VAlign_Center);
	}

	UTextBlock* RoomName = WidgetTree->ConstructWidget<UTextBlock>();
	RoomName->SetText(FText::FromString(RoomInfo.nsh_room_name.IsEmpty() ? TEXT("이름 없는 대국실") : RoomInfo.nsh_room_name));
	FPSJanggiUI::StyleText(RoomName, 18, FPSJanggiUI::Ivory(), true);
	Information->AddChildToVerticalBox(RoomName);

	UTextBlock* RoomDetails = WidgetTree->ConstructWidget<UTextBlock>();
	RoomDetails->SetText(FText::FromString(FString::Printf(
		TEXT("방장 %s  ·  %d/%d명  ·  %d ms"),
		RoomInfo.nsh_host_name.IsEmpty() ? TEXT("알 수 없음") : *RoomInfo.nsh_host_name,
		RoomInfo.nsh_current_players,
		RoomInfo.nsh_max_players,
		RoomInfo.nsh_ping_ms)));
	FPSJanggiUI::StyleText(RoomDetails, 13, FPSJanggiUI::MutedIvory());
	if (UVerticalBoxSlot* LayoutSlot = Information->AddChildToVerticalBox(RoomDetails))
	{
		LayoutSlot->SetPadding(FMargin(0.0f, 3.0f, 0.0f, 0.0f));
	}

	UButton* JoinButton = WidgetTree->ConstructWidget<UButton>();
	FPSJanggiUI::StyleButton(
		JoinButton,
		FLinearColor(0.055f, 0.31f, 0.19f, 1.0f),
		FLinearColor(0.08f, 0.43f, 0.26f, 1.0f),
		FLinearColor(0.035f, 0.22f, 0.13f, 1.0f),
		8.0f,
		FPSJanggiUI::Jade());
	UTextBlock* JoinLabel = WidgetTree->ConstructWidget<UTextBlock>();
	JoinLabel->SetText(FText::FromString(TEXT("합류")));
	JoinLabel->SetJustification(ETextJustify::Center);
	FPSJanggiUI::StyleText(JoinLabel, 16, FPSJanggiUI::Ivory(), true);
	JoinButton->AddChild(JoinLabel);
	JoinButton->OnClicked.AddDynamic(this, &ULobbyRoomEntryWidget::JoinRoom);
	JoinButton->SetIsEnabled(RoomInfo.nsh_current_players < RoomInfo.nsh_max_players);
	if (UHorizontalBoxSlot* LayoutSlot = Row->AddChildToHorizontalBox(JoinButton))
	{
		LayoutSlot->SetPadding(FMargin(12.0f, 0.0f, 0.0f, 0.0f));
		LayoutSlot->SetVerticalAlignment(VAlign_Center);
	}
	return Super::RebuildWidget();
}

void ULobbyRoomEntryWidget::JoinRoom()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (USessionSubsystem* Sessions = GameInstance->GetSubsystem<USessionSubsystem>())
		{
			Sessions->JoinRoom(RoomInfo.nsh_room_index);
		}
	}
}
