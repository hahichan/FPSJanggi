// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class UBorder;
class UButton;
class UEditableTextBox;
class UTextBlock;

/** Shared code-only visual language for the FPS Janggi front end and match HUD. */
namespace FPSJanggiUI
{
	FPSJANGGI_API const FLinearColor& Ink();
	FPSJANGGI_API const FLinearColor& Panel();
	FPSJANGGI_API const FLinearColor& Ivory();
	FPSJANGGI_API const FLinearColor& MutedIvory();
	FPSJANGGI_API const FLinearColor& Gold();
	FPSJANGGI_API const FLinearColor& Jade();
	FPSJANGGI_API const FLinearColor& Blue();
	FPSJANGGI_API const FLinearColor& Red();

	FPSJANGGI_API void StylePanel(
		UBorder* Border,
		const FLinearColor& Fill,
		float Radius = 14.0f,
		const FLinearColor& Outline = FLinearColor::Transparent,
		float OutlineWidth = 0.0f);

	FPSJANGGI_API void StyleButton(
		UButton* Button,
		const FLinearColor& Normal,
		const FLinearColor& Hovered,
		const FLinearColor& Pressed,
		float Radius = 10.0f,
		const FLinearColor& Outline = FLinearColor::Transparent);

	FPSJANGGI_API void StyleText(
		UTextBlock* Text,
		int32 Size,
		const FLinearColor& Color,
		bool bBold = false,
		int32 OutlineSize = 0);

	FPSJANGGI_API void StyleTextInput(UEditableTextBox* TextBox);
}
