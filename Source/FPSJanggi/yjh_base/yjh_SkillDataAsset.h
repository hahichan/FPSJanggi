// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "yjh_base/yjh_ArenaCombatSkillTypes.h"
#include "yjh_SkillDataAsset.generated.h"

UCLASS(BlueprintType)
class FPSJANGGI_API UYJHSkillDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YJH|Skill")
	int32 MaxSkillSlots = 10;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YJH|Skill")
	TArray<FYJHSkillDefinition> Skills;

	UFUNCTION(BlueprintCallable, Category = "YJH|Skill")
	bool BuildSkillMaps(TMap<FName, FYJHSkillDefinition>& OutBySkillId, TMap<FName, FName>& OutSlotToSkillId, FString& OutError) const;
};
