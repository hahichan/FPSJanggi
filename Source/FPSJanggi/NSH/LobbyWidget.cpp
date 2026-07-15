// Copyright Epic Games, Inc. All Rights Reserved.

#include "LobbyWidget.h"

#include "BoardPlayerController.h"
#include "JanggiUIStyle.h"
#include "LobbyRoomEntryWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/EditableTextBox.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/GameInstance.h"

namespace
{
UButton* AddLobbyButton(
	UWidgetTree* Tree,
	UHorizontalBox* Row,
	const FString& Label,
	const FLinearColor& Normal,
	const FLinearColor& Hovered,
	const FLinearColor& Pressed)
{
	UButton* Button = Tree->ConstructWidget<UButton>();
	FPSJanggiUI::StyleButton(Button, Normal, Hovered, Pressed, 10.0f, FLinearColor(0.39f, 0.29f, 0.13f, 1.0f));
	UTextBlock* Text = Tree->ConstructWidget<UTextBlock>();
	Text->SetText(FText::FromString(Label));
	Text->SetJustification(ETextJustify::Center);
	FPSJanggiUI::StyleText(Text, 17, FPSJanggiUI::Ivory(), true);
	Button->AddChild(Text);
	if (UHorizontalBoxSlot* LayoutSlot = Row->AddChildToHorizontalBox(Button))
	{
		LayoutSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		LayoutSlot->SetPadding(FMargin(5.0f));
		LayoutSlot->SetVerticalAlignment(VAlign_Fill);
	}
	return Button;
}
}

TSharedRef<SWidget> ULobbyWidget::RebuildWidget()
{
	if (!WidgetTree) WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"));
	if (WidgetTree->RootWidget) return Super::RebuildWidget();

	UCanvasPanel* Canvas = WidgetTree->ConstructWidget<UCanvasPanel>();
	WidgetTree->RootWidget = Canvas;

	UBorder* Scrim = WidgetTree->ConstructWidget<UBorder>();
	Scrim->SetBrushColor(FLinearColor(0.005f, 0.004f, 0.003f, 0.38f));
	Scrim->SetVisibility(ESlateVisibility::HitTestInvisible);
	if (UCanvasPanelSlot* LayoutSlot = Canvas->AddChildToCanvas(Scrim))
	{
		LayoutSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
		LayoutSlot->SetOffsets(FMargin(0.0f));
	}

	UBorder* BrandCard = WidgetTree->ConstructWidget<UBorder>();
	FPSJanggiUI::StylePanel(
		BrandCard,
		FLinearColor(0.018f, 0.014f, 0.010f, 0.82f),
		18.0f,
		FLinearColor(0.44f, 0.31f, 0.12f, 0.85f),
		1.0f);
	BrandCard->SetPadding(FMargin(30.0f, 24.0f));
	if (UCanvasPanelSlot* LayoutSlot = Canvas->AddChildToCanvas(BrandCard))
	{
		LayoutSlot->SetAnchors(FAnchors(0.0f, 0.5f));
		LayoutSlot->SetAlignment(FVector2D(0.0f, 0.5f));
		LayoutSlot->SetPosition(FVector2D(54.0f, 0.0f));
		LayoutSlot->SetSize(FVector2D(500.0f, 270.0f));
	}
	UVerticalBox* BrandStack = WidgetTree->ConstructWidget<UVerticalBox>();
	BrandCard->SetContent(BrandStack);
	UTextBlock* SmallTitle = WidgetTree->ConstructWidget<UTextBlock>();
	SmallTitle->SetText(FText::FromString(TEXT("전통 장기 × 실시간 전투")));
	FPSJanggiUI::StyleText(SmallTitle, 15, FPSJanggiUI::Gold(), true);
	BrandStack->AddChildToVerticalBox(SmallTitle);
	UTextBlock* GameTitle = WidgetTree->ConstructWidget<UTextBlock>();
	GameTitle->SetText(FText::FromString(TEXT("FPS 장기")));
	FPSJanggiUI::StyleText(GameTitle, 52, FPSJanggiUI::Ivory(), true, 1);
	if (UVerticalBoxSlot* LayoutSlot = BrandStack->AddChildToVerticalBox(GameTitle))
	{
		LayoutSlot->SetPadding(FMargin(0.0f, 7.0f, 0.0f, 2.0f));
	}
	UTextBlock* Tagline = WidgetTree->ConstructWidget<UTextBlock>();
	Tagline->SetText(FText::FromString(TEXT("한 수의 선택이 전장의 승부가 됩니다")));
	FPSJanggiUI::StyleText(Tagline, 18, FPSJanggiUI::MutedIvory());
	BrandStack->AddChildToVerticalBox(Tagline);
	UBorder* MainCard = WidgetTree->ConstructWidget<UBorder>();
	FPSJanggiUI::StylePanel(
		MainCard,
		FLinearColor(0.022f, 0.018f, 0.014f, 0.97f),
		20.0f,
		FLinearColor(0.48f, 0.34f, 0.14f, 0.95f),
		1.5f);
	MainCard->SetPadding(FMargin(24.0f, 22.0f));
	if (UCanvasPanelSlot* LayoutSlot = Canvas->AddChildToCanvas(MainCard))
	{
		LayoutSlot->SetAnchors(FAnchors(1.0f, 0.5f));
		LayoutSlot->SetAlignment(FVector2D(1.0f, 0.5f));
		LayoutSlot->SetPosition(FVector2D(-54.0f, 0.0f));
		LayoutSlot->SetSize(FVector2D(620.0f, 780.0f));
	}

	UVerticalBox* MainStack = WidgetTree->ConstructWidget<UVerticalBox>();
	MainCard->SetContent(MainStack);
	UTextBlock* LobbyTitle = WidgetTree->ConstructWidget<UTextBlock>();
	LobbyTitle->SetText(FText::FromString(TEXT("대국실 로비")));
	LobbyTitle->SetJustification(ETextJustify::Center);
	FPSJanggiUI::StyleText(LobbyTitle, 30, FPSJanggiUI::Ivory(), true);
	MainStack->AddChildToVerticalBox(LobbyTitle);
	UTextBlock* LobbySubtitle = WidgetTree->ConstructWidget<UTextBlock>();
	LobbySubtitle->SetText(FText::FromString(TEXT("방을 만들거나 다른 플레이어의 대국실에 합류하세요")));
	LobbySubtitle->SetJustification(ETextJustify::Center);
	FPSJanggiUI::StyleText(LobbySubtitle, 14, FPSJanggiUI::MutedIvory());
	if (UVerticalBoxSlot* LayoutSlot = MainStack->AddChildToVerticalBox(LobbySubtitle))
	{
		LayoutSlot->SetPadding(FMargin(0.0f, 2.0f, 0.0f, 16.0f));
	}

	RoomNameInput = WidgetTree->ConstructWidget<UEditableTextBox>();
	FPSJanggiUI::StyleTextInput(RoomNameInput);
	RoomNameInput->SetText(FText::FromString(TEXT("FPS 장기 대국실")));
	if (UVerticalBoxSlot* LayoutSlot = MainStack->AddChildToVerticalBox(RoomNameInput))
	{
		LayoutSlot->SetPadding(FMargin(5.0f, 0.0f, 5.0f, 7.0f));
	}

	UHorizontalBox* PrimaryActions = WidgetTree->ConstructWidget<UHorizontalBox>();
	if (UVerticalBoxSlot* LayoutSlot = MainStack->AddChildToVerticalBox(PrimaryActions))
	{
		LayoutSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 5.0f));
	}
	UButton* CreateButton = AddLobbyButton(
		WidgetTree, PrimaryActions, TEXT("방 만들기"),
		FLinearColor(0.40f, 0.255f, 0.065f, 1.0f),
		FLinearColor(0.55f, 0.37f, 0.10f, 1.0f),
		FLinearColor(0.28f, 0.17f, 0.04f, 1.0f));
	CreateButton->OnClicked.AddDynamic(this, &ULobbyWidget::CreateRoom);
	UButton* QuickButton = AddLobbyButton(
		WidgetTree, PrimaryActions, TEXT("빠른 대전"),
		FLinearColor(0.045f, 0.30f, 0.18f, 1.0f),
		FLinearColor(0.07f, 0.43f, 0.25f, 1.0f),
		FLinearColor(0.03f, 0.21f, 0.12f, 1.0f));
	QuickButton->OnClicked.AddDynamic(this, &ULobbyWidget::QuickMatch);

	UHorizontalBox* RoomHeader = WidgetTree->ConstructWidget<UHorizontalBox>();
	if (UVerticalBoxSlot* LayoutSlot = MainStack->AddChildToVerticalBox(RoomHeader))
	{
		LayoutSlot->SetPadding(FMargin(5.0f, 12.0f, 5.0f, 6.0f));
		LayoutSlot->SetHorizontalAlignment(HAlign_Fill);
	}
	UTextBlock* ListTitle = WidgetTree->ConstructWidget<UTextBlock>();
	ListTitle->SetText(FText::FromString(TEXT("참가 가능한 방")));
	FPSJanggiUI::StyleText(ListTitle, 18, FPSJanggiUI::Ivory(), true);
	if (UHorizontalBoxSlot* LayoutSlot = RoomHeader->AddChildToHorizontalBox(ListTitle))
	{
		LayoutSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		LayoutSlot->SetVerticalAlignment(VAlign_Center);
	}
	UButton* RefreshButton = WidgetTree->ConstructWidget<UButton>();
	FPSJanggiUI::StyleButton(
		RefreshButton,
		FLinearColor(0.10f, 0.085f, 0.06f, 1.0f),
		FLinearColor(0.17f, 0.13f, 0.075f, 1.0f),
		FLinearColor(0.07f, 0.055f, 0.04f, 1.0f),
		8.0f);
	UTextBlock* RefreshLabel = WidgetTree->ConstructWidget<UTextBlock>();
	RefreshLabel->SetText(FText::FromString(TEXT("새로고침")));
	FPSJanggiUI::StyleText(RefreshLabel, 14, FPSJanggiUI::Gold(), true);
	RefreshButton->AddChild(RefreshLabel);
	RefreshButton->OnClicked.AddDynamic(this, &ULobbyWidget::RefreshRooms);
	RoomHeader->AddChildToHorizontalBox(RefreshButton);

	UBorder* RoomListBorder = WidgetTree->ConstructWidget<UBorder>();
	FPSJanggiUI::StylePanel(RoomListBorder, FLinearColor(0.012f, 0.011f, 0.009f, 0.78f), 12.0f);
	RoomListBorder->SetPadding(FMargin(8.0f));
	if (UVerticalBoxSlot* LayoutSlot = MainStack->AddChildToVerticalBox(RoomListBorder))
	{
		LayoutSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		LayoutSlot->SetPadding(FMargin(5.0f));
	}
	UScrollBox* Scroll = WidgetTree->ConstructWidget<UScrollBox>();
	Scroll->SetScrollBarVisibility(ESlateVisibility::Visible);
	RoomListBorder->SetContent(Scroll);
	RoomList = WidgetTree->ConstructWidget<UVerticalBox>();
	Scroll->AddChild(RoomList);
	PopulateRooms({});

	StatusText = WidgetTree->ConstructWidget<UTextBlock>();
	StatusText->SetText(FText::FromString(TEXT("Steam을 실행한 뒤 방 목록을 새로고침하세요")));
	StatusText->SetJustification(ETextJustify::Center);
	StatusText->SetAutoWrapText(true);
	FPSJanggiUI::StyleText(StatusText, 13, FPSJanggiUI::MutedIvory());
	if (UVerticalBoxSlot* LayoutSlot = MainStack->AddChildToVerticalBox(StatusText))
	{
		LayoutSlot->SetPadding(FMargin(5.0f, 7.0f, 5.0f, 5.0f));
		LayoutSlot->SetHorizontalAlignment(HAlign_Fill);
	}

	return Super::RebuildWidget();
}

void ULobbyWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (USessionSubsystem* Sessions = GetSessions())
	{
		Sessions->nsh_on_session_operation_complete.AddUniqueDynamic(this, &ULobbyWidget::HandleSessionOperation);
		Sessions->nsh_on_room_search_complete.AddUniqueDynamic(this, &ULobbyWidget::HandleRoomSearch);
		bDelegatesBound = true;
		PopulateRooms(Sessions->GetCachedRooms());
	}
}

void ULobbyWidget::NativeDestruct()
{
	if (bDelegatesBound)
	{
		if (USessionSubsystem* Sessions = GetSessions())
		{
			Sessions->nsh_on_session_operation_complete.RemoveDynamic(this, &ULobbyWidget::HandleSessionOperation);
			Sessions->nsh_on_room_search_complete.RemoveDynamic(this, &ULobbyWidget::HandleRoomSearch);
		}
	}
	bDelegatesBound = false;
	Super::NativeDestruct();
}

void ULobbyWidget::CreateRoom()
{
	if (USessionSubsystem* Sessions = GetSessions())
	{
		SetStatus(TEXT("대국실을 만들고 있습니다..."));
		Sessions->CreateRoom(RoomNameInput ? RoomNameInput->GetText().ToString() : TEXT("FPS 장기 대국실"), 2, false);
	}
}

void ULobbyWidget::RefreshRooms()
{
	if (USessionSubsystem* Sessions = GetSessions())
	{
		SetStatus(TEXT("참가 가능한 대국실을 찾고 있습니다..."));
		Sessions->FindRooms(50, false);
	}
}

void ULobbyWidget::QuickMatch()
{
	if (USessionSubsystem* Sessions = GetSessions())
	{
		SetStatus(TEXT("빠른 대전 상대를 찾고 있습니다..."));
		Sessions->QuickMatch(2, false);
	}
}

void ULobbyWidget::OpenLocalPreview()
{
	if (ABoardPlayerController* Controller = GetOwningPlayer<ABoardPlayerController>())
	{
		Controller->ExitLobbyForLocalPreview();
	}
}

void ULobbyWidget::HandleSessionOperation(FName Operation, bool bSuccess, const FString& Message)
{
	FString FriendlyMessage = Message;
	if (Operation == TEXT("CreateRoom")) FriendlyMessage = bSuccess ? TEXT("대국실 생성 완료 · 게임 화면으로 이동합니다") : TEXT("대국실을 만들지 못했습니다. Steam 연결을 확인하세요");
	else if (Operation == TEXT("FindRooms")) FriendlyMessage = bSuccess ? TEXT("방 목록을 갱신했습니다") : TEXT("방 목록을 불러오지 못했습니다. Steam 연결을 확인하세요");
	else if (Operation == TEXT("JoinRoom")) FriendlyMessage = bSuccess ? TEXT("대국실에 합류했습니다 · 게임 화면으로 이동합니다") : TEXT("선택한 대국실에 합류하지 못했습니다");
	SetStatus(FriendlyMessage, !bSuccess);
}

void ULobbyWidget::HandleRoomSearch(bool bSuccess, const TArray<FNSHRoomInfo>& Rooms)
{
	PopulateRooms(Rooms);
	if (bSuccess)
	{
		SetStatus(Rooms.IsEmpty() ? TEXT("현재 참가 가능한 대국실이 없습니다") : FString::Printf(TEXT("대국실 %d개를 찾았습니다"), Rooms.Num()));
	}
}

USessionSubsystem* ULobbyWidget::GetSessions() const
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		return GameInstance->GetSubsystem<USessionSubsystem>();
	}
	return nullptr;
}

void ULobbyWidget::PopulateRooms(const TArray<FNSHRoomInfo>& Rooms)
{
	if (!RoomList) return;
	RoomList->ClearChildren();
	if (Rooms.IsEmpty())
	{
		UTextBlock* EmptyText = WidgetTree->ConstructWidget<UTextBlock>();
		EmptyText->SetText(FText::FromString(TEXT("표시할 대국실이 없습니다\n새로고침하거나 직접 방을 만들어 보세요")));
		EmptyText->SetJustification(ETextJustify::Center);
		FPSJanggiUI::StyleText(EmptyText, 15, FLinearColor(0.50f, 0.46f, 0.38f, 1.0f));
		if (UVerticalBoxSlot* LayoutSlot = RoomList->AddChildToVerticalBox(EmptyText))
		{
			LayoutSlot->SetPadding(FMargin(20.0f, 70.0f));
			LayoutSlot->SetHorizontalAlignment(HAlign_Fill);
		}
		return;
	}
	for (const FNSHRoomInfo& Room : Rooms)
	{
		ULobbyRoomEntryWidget* Entry = CreateWidget<ULobbyRoomEntryWidget>(GetOwningPlayer(), ULobbyRoomEntryWidget::StaticClass());
		if (!Entry) continue;
		Entry->InitializeRoom(Room);
		if (UVerticalBoxSlot* LayoutSlot = RoomList->AddChildToVerticalBox(Entry))
		{
			LayoutSlot->SetPadding(FMargin(2.0f, 3.0f));
			LayoutSlot->SetHorizontalAlignment(HAlign_Fill);
		}
	}
}

void ULobbyWidget::SetStatus(const FString& Message, bool bError)
{
	if (!StatusText) return;
	StatusText->SetText(FText::FromString(Message));
	FPSJanggiUI::StyleText(
		StatusText,
		13,
		bError ? FLinearColor(0.95f, 0.32f, 0.22f, 1.0f) : FPSJanggiUI::MutedIvory(),
		bError);
}
