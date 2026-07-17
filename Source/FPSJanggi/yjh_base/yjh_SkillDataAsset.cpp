// Copyright Epic Games, Inc. All Rights Reserved.

#include "yjh_base/yjh_SkillDataAsset.h"

namespace
{
	constexpr float MaxCooldownSeconds = 999.0f;
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
			if (Skill.InputSlot.IsNone())
			{
				OutError = FString::Printf(TEXT("SKV_MissingRequiredField: InputSlot for %s"), *Skill.SkillId.ToString());
				return false;
			}
			if (OutSlotToSkillId.Contains(Skill.InputSlot))
			{
				OutError = FString::Printf(TEXT("SKV_DuplicateSlotMapping: %s"), *Skill.InputSlot.ToString());
				return false;
			}
			OutSlotToSkillId.Add(Skill.InputSlot, Skill.SkillId);
		}

		OutBySkillId.Add(Skill.SkillId, Skill);
	}

	return true;
}
