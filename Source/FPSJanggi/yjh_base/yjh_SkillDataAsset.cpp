// Copyright Epic Games, Inc. All Rights Reserved.

#include "yjh_base/yjh_SkillDataAsset.h"

namespace
{
	constexpr float MaxCooldownSeconds = 999.0f;

	bool TryParseSlotNumber(const FName SlotName, int32& OutSlotNumber)
	{
		OutSlotNumber = INDEX_NONE;
		if (SlotName.IsNone())
		{
			return false;
		}

		const FString SlotString = SlotName.ToString();
		if (!SlotString.StartsWith(TEXT("Slot")))
		{
			return false;
		}

		const FString NumberText = SlotString.RightChop(4);
		if (NumberText.IsEmpty())
		{
			return false;
		}

		OutSlotNumber = FCString::Atoi(*NumberText);
		return OutSlotNumber >= 1 && OutSlotNumber <= 10;
	}
}

bool UYJHSkillDataAsset::BuildSkillMaps(TMap<FName, FYJHSkillDefinition>& OutBySkillId, TMap<FName, FName>& OutSlotToSkillId, FString& OutError) const
{
	OutBySkillId.Reset();
	OutSlotToSkillId.Reset();
	OutError.Empty();

	if (MaxSkillSlots <= 0)
	{
		OutError = TEXT("MaxSkillSlots must be > 0.");
		return false;
	}

	for (const FYJHSkillDefinition& Skill : Skills)
	{
		if (Skill.SkillId.IsNone())
		{
			OutError = TEXT("SKV_MissingRequiredField: SkillId is required.");
			return false;
		}

		if (OutBySkillId.Contains(Skill.SkillId))
		{
			OutError = FString::Printf(TEXT("SKV_DuplicateSkillId: %s"), *Skill.SkillId.ToString());
			return false;
		}

		if (Skill.CooldownSec < 0.0f || Skill.CooldownSec > MaxCooldownSeconds)
		{
			OutError = FString::Printf(TEXT("SKV_InvalidCooldown: %s"), *Skill.SkillId.ToString());
			return false;
		}

		if (!Skill.bIsPassive)
		{
			const FName ResolvedInputSlot = Skill.InputSlotEnum != EYJHSkillSlot::None
				? YJHSkillSlotToName(Skill.InputSlotEnum)
				: Skill.InputSlot;

			if (ResolvedInputSlot.IsNone())
			{
				OutError = FString::Printf(TEXT("SKV_MissingRequiredField: InputSlot for %s"), *Skill.SkillId.ToString());
				return false;
			}

			int32 ParsedSlotNumber = INDEX_NONE;
			if (!TryParseSlotNumber(ResolvedInputSlot, ParsedSlotNumber))
			{
				OutError = FString::Printf(TEXT("SKV_InvalidSlotMapping: %s uses invalid slot %s"), *Skill.SkillId.ToString(), *ResolvedInputSlot.ToString());
				return false;
			}

			if (ParsedSlotNumber > MaxSkillSlots)
			{
				OutError = FString::Printf(TEXT("SKV_SlotOutOfRange: %s mapped to %s but MaxSkillSlots=%d"), *Skill.SkillId.ToString(), *ResolvedInputSlot.ToString(), MaxSkillSlots);
				return false;
			}

			if (OutSlotToSkillId.Contains(ResolvedInputSlot))
			{
				OutError = FString::Printf(TEXT("SKV_DuplicateSlotMapping: %s"), *ResolvedInputSlot.ToString());
				return false;
			}
			OutSlotToSkillId.Add(ResolvedInputSlot, Skill.SkillId);
		}

		OutBySkillId.Add(Skill.SkillId, Skill);
	}

	return true;
}
