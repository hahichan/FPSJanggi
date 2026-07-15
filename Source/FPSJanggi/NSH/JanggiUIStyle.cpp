// Copyright Epic Games, Inc. All Rights Reserved.

#include "JanggiUIStyle.h"

#include "Brushes/SlateRoundedBoxBrush.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "Styling/SlateTypes.h"

namespace FPSJanggiUI
{
namespace
{
const FLinearColor InkColor(0.018f, 0.016f, 0.013f, 0.96f);
const FLinearColor PanelColor(0.075f, 0.061f, 0.043f, 0.96f);
const FLinearColor IvoryColor(0.96f, 0.91f, 0.78f, 1.0f);
const FLinearColor MutedIvoryColor(0.72f, 0.67f, 0.56f, 1.0f);
const FLinearColor GoldColor(0.82f, 0.57f, 0.20f, 1.0f);
const FLinearColor JadeColor(0.08f, 0.50f, 0.32f, 1.0f);
const FLinearColor BlueColor(0.06f, 0.25f, 0.67f, 1.0f);
const FLinearColor RedColor(0.65f, 0.08f, 0.055f, 1.0f);
}

const FLinearColor& Ink() { return InkColor; }
const FLinearColor& Panel() { return PanelColor; }
const FLinearColor& Ivory() { return IvoryColor; }
const FLinearColor& MutedIvory() { return MutedIvoryColor; }
const FLinearColor& Gold() { return GoldColor; }
const FLinearColor& Jade() { return JadeColor; }
const FLinearColor& Blue() { return BlueColor; }
const FLinearColor& Red() { return RedColor; }

void StylePanel(UBorder* Border, const FLinearColor& Fill, float Radius, const FLinearColor& Outline, float OutlineWidth)
{
	if (!Border) return;
	Border->SetBrush(FSlateRoundedBoxBrush(Fill, Radius, Outline, OutlineWidth));
}

void StyleButton(
	UButton* Button,
	const FLinearColor& Normal,
	const FLinearColor& Hovered,
	const FLinearColor& Pressed,
	float Radius,
	const FLinearColor& Outline)
{
	if (!Button) return;
	FButtonStyle Style;
	Style.SetNormal(FSlateRoundedBoxBrush(Normal, Radius, Outline, 1.0f));
	Style.SetHovered(FSlateRoundedBoxBrush(Hovered, Radius, GoldColor, 1.5f));
	Style.SetPressed(FSlateRoundedBoxBrush(Pressed, Radius, GoldColor, 1.0f));
	Style.SetDisabled(FSlateRoundedBoxBrush(FLinearColor(0.08f, 0.075f, 0.065f, 0.72f), Radius));
	Style.SetNormalPadding(FMargin(14.0f, 10.0f));
	Style.SetPressedPadding(FMargin(14.0f, 12.0f, 14.0f, 8.0f));
	Button->SetStyle(Style);
	Button->SetBackgroundColor(FLinearColor::White);
}

void StyleText(UTextBlock* Text, int32 Size, const FLinearColor& Color, bool bBold, int32 OutlineSize)
{
	if (!Text) return;
	FSlateFontInfo Font = Text->GetFont();
	Font.Size = Size;
	Font.TypefaceFontName = bBold ? FName(TEXT("Bold")) : FName(TEXT("Regular"));
	Font.OutlineSettings.OutlineSize = OutlineSize;
	Font.OutlineSettings.OutlineColor = FLinearColor(0.01f, 0.008f, 0.006f, 0.9f);
	Text->SetFont(Font);
	Text->SetColorAndOpacity(FSlateColor(Color));
}

void StyleTextInput(UEditableTextBox* TextBox)
{
	if (!TextBox) return;
	FEditableTextBoxStyle Style = TextBox->GetWidgetStyle();
	Style.SetBackgroundImageNormal(FSlateRoundedBoxBrush(FLinearColor(0.025f, 0.022f, 0.018f, 0.94f), 9.0f, FLinearColor(0.22f, 0.18f, 0.11f, 1.0f), 1.0f));
	Style.SetBackgroundImageHovered(FSlateRoundedBoxBrush(FLinearColor(0.04f, 0.035f, 0.027f, 0.98f), 9.0f, GoldColor, 1.0f));
	Style.SetBackgroundImageFocused(FSlateRoundedBoxBrush(FLinearColor(0.04f, 0.035f, 0.027f, 1.0f), 9.0f, GoldColor, 2.0f));
	Style.SetBackgroundImageReadOnly(FSlateRoundedBoxBrush(FLinearColor(0.04f, 0.04f, 0.04f, 0.8f), 9.0f));
	Style.SetForegroundColor(FSlateColor(IvoryColor));
	Style.SetPadding(FMargin(16.0f, 10.0f));
	FSlateFontInfo Font = Style.TextStyle.Font;
	Font.Size = 20;
	Style.TextStyle.SetFont(Font);
	Style.TextStyle.SetColorAndOpacity(FSlateColor(IvoryColor));
	TextBox->SetWidgetStyle(Style);
	TextBox->SetHintText(FText::FromString(TEXT("방 이름을 입력하세요")));
}
}
