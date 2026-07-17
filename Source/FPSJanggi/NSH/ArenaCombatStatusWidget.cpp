// Copyright Epic Games, Inc. All Rights Reserved.

#include "ArenaCombatStatusWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "yjh_base/yjh_ArenaCombatComponent.h"
#include "yjh_base/yjh_ArenaCombatantBase.h"
#include "yjh_base/yjh_ArenaHealthComponent.h"
#include "yjh_base/yjh_SkillDataAsset.h"

TSharedRef<SWidget> UArenaCombatStatusWidget::RebuildWidget()
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

	RootBorder = WidgetTree->ConstructWidget<UBorder>();
	RootBorder->SetBrushColor(FLinearColor(0.02f, 0.03f, 0.06f, 0.92f));
	RootBorder->SetPadding(FMargin(14.0f, 10.0f));
	if (UCanvasPanelSlot* BorderSlot = Canvas->AddChildToCanvas(RootBorder))
	{
		BorderSlot->SetAnchors(FAnchors(1.0f, 0.0f));
		BorderSlot->SetAlignment(FVector2D(1.0f, 0.0f));
		BorderSlot->SetPosition(FVector2D(-22.0f, 18.0f));
		BorderSlot->SetSize(FVector2D(360.0f, 260.0f));
	}

	UVerticalBox* Stack = WidgetTree->ConstructWidget<UVerticalBox>();
	RootBorder->SetContent(Stack);

	HeaderText = WidgetTree->ConstructWidget<UTextBlock>();
	HeaderText->SetText(FText::FromString(TEXT("Arena Combat HUD")));
	HeaderText->SetColorAndOpacity(FSlateColor(FLinearColor(0.90f, 0.85f, 0.70f, 1.0f)));
	if (UVerticalBoxSlot* HeaderSlot = Stack->AddChildToVerticalBox(HeaderText))
	{
		HeaderSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 6.0f));
	}

	HpText = WidgetTree->ConstructWidget<UTextBlock>();
	HpText->SetText(FText::FromString(TEXT("HP: -")));
	HpText->SetColorAndOpacity(FSlateColor(FLinearColor(0.95f, 0.95f, 0.95f, 1.0f)));
	if (UVerticalBoxSlot* HpSlot = Stack->AddChildToVerticalBox(HpText))
	{
		HpSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 8.0f));
	}

	CooldownText = WidgetTree->ConstructWidget<UTextBlock>();
	CooldownText->SetText(FText::FromString(TEXT("Cooldown: -")));
	CooldownText->SetColorAndOpacity(FSlateColor(FLinearColor(0.82f, 0.88f, 1.0f, 1.0f)));
	CooldownText->SetAutoWrapText(true);
	Stack->AddChildToVerticalBox(CooldownText);

	return Super::RebuildWidget();
}

void UArenaCombatStatusWidget::UpdateFromCombatant(const AYJHArenaCombatantBase* Combatant)
{
	if (!HeaderText || !HpText || !CooldownText)
	{
		return;
	}

	if (!Combatant)
	{
		HeaderText->SetText(FText::FromString(TEXT("Arena Combat HUD (waiting pawn)")));
		HpText->SetText(FText::FromString(TEXT("HP: -")));
		CooldownText->SetText(FText::FromString(TEXT("Cooldown: -")));
		return;
	}

	HeaderText->SetText(FText::FromString(FString::Printf(
		TEXT("Arena Combat HUD (%s)"),
		*Combatant->GetName())));

	if (const UYJHArenaHealthComponent* Health = Combatant->HealthComponent)
	{
		HpText->SetText(FText::FromString(FString::Printf(
			TEXT("HP: %.0f / %.0f | CombatEnabled=%s | Session=%s"),
			Health->GetCurrentHP(),
			Health->GetMaxHP(),
			Combatant->IsCombatEnabled() ? TEXT("true") : TEXT("false"),
			*Combatant->GetCombatSessionId().ToString())));
	}
	else
	{
		HpText->SetText(FText::FromString(TEXT("HP: (missing HealthComponent)")));
	}

	if (const UYJHArenaCombatComponent* Combat = Combatant->CombatComponent)
	{
		FString CooldownLines;
		CooldownLines += FString::Printf(TEXT("SkillData: %s\nCombatCtx: %s\n"),
			*GetNameSafe(Combat->SkillDataAsset.Get()),
			*Combat->GetCombatSessionId().ToString());
		for (int32 SlotNumber = 1; SlotNumber <= 10; ++SlotNumber)
		{
			const FName SlotName(*FString::Printf(TEXT("Slot%d"), SlotNumber));
			const float Remaining = Combat->GetRemainingCooldownBySlot(SlotName);
			CooldownLines += FString::Printf(
				TEXT("S%d: %s\n"),
				SlotNumber,
				Remaining > 0.05f ? *FString::Printf(TEXT("%.1fs"), Remaining) : TEXT("Ready"));
		}
		CooldownText->SetText(FText::FromString(CooldownLines));
	}
	else
	{
		CooldownText->SetText(FText::FromString(TEXT("Cooldown: (missing CombatComponent)")));
	}
}
