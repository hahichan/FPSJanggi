// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "yjh_ArenaCombatSkillTypes.generated.h"

UENUM(BlueprintType)
enum class EYJHTeamInfo : uint8
{
	Blue,
	Red,
	Unassigned
};

UENUM(BlueprintType)
enum class EYJHPieceType : uint8
{
	King,
	Chariot,
	Cannon,
	Horse,
	Elephant,
	Guard,
	Pawn,
	Unknown
};

UENUM(BlueprintType)
enum class EYJHSkillExecType : uint8
{
	HitScan,
	Projectile,
	Dash,
	CooldownControl,
	Teleport,
	Passive,
	Reserved
};

UENUM(BlueprintType)
enum class EYJHPassiveType : uint8
{
	None,
	Regen,
	WallRun,
	AutoShield,
	MoveSpeedAura
};

UENUM(BlueprintType)
enum class EYJHSkillFailCode : uint8
{
	None,
	SKX_InvalidSession,
	SKX_InvalidState,
	SKX_NoSkillMapped,
	SKX_CooldownActive,
	SKX_NotEnoughResource,
	SKX_OutOfRange,
	SKX_InvalidTarget,
	SKX_ExecutionBlocked,
	SKX_TeleportBlocked,
	SKX_TeleportNoValidDestination,
	SKX_CooldownUnderflowGuard,
	SKX_CooldownOverflowGuard
};

UENUM(BlueprintType)
enum class EYJHSkillSlot : uint8
{
	None,
	Slot1,
	Slot2,
	Slot3,
	Slot4,
	Slot5,
	Slot6,
	Slot7,
	Slot8,
	Slot9,
	Slot10
};

inline FName YJHSkillSlotToName(EYJHSkillSlot Slot)
{
	switch (Slot)
	{
	case EYJHSkillSlot::Slot1: return FName(TEXT("Slot1"));
	case EYJHSkillSlot::Slot2: return FName(TEXT("Slot2"));
	case EYJHSkillSlot::Slot3: return FName(TEXT("Slot3"));
	case EYJHSkillSlot::Slot4: return FName(TEXT("Slot4"));
	case EYJHSkillSlot::Slot5: return FName(TEXT("Slot5"));
	case EYJHSkillSlot::Slot6: return FName(TEXT("Slot6"));
	case EYJHSkillSlot::Slot7: return FName(TEXT("Slot7"));
	case EYJHSkillSlot::Slot8: return FName(TEXT("Slot8"));
	case EYJHSkillSlot::Slot9: return FName(TEXT("Slot9"));
	case EYJHSkillSlot::Slot10: return FName(TEXT("Slot10"));
	default: return NAME_None;
	}
}

USTRUCT(BlueprintType)
struct FPSJANGGI_API FYJHSkillExecutionResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "YJH|Skill")
	bool bSuccess = false;

	UPROPERTY(BlueprintReadOnly, Category = "YJH|Skill")
	FName SkillId = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category = "YJH|Skill")
	FName SlotIndex = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category = "YJH|Skill")
	EYJHSkillFailCode FailCode = EYJHSkillFailCode::None;
};

USTRUCT(BlueprintType)
struct FPSJANGGI_API FYJHRuntimeSkillRequestContext
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "YJH|Skill")
	FName CombatSessionId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "YJH|Skill")
	FName CombatantId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "YJH|Skill")
	FName SkillId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "YJH|Skill")
	FName SlotIndex = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "YJH|Skill")
	FVector SourceLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "YJH|Skill")
	FVector SourceForward = FVector::ForwardVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "YJH|Skill")
	FVector RequestedDirection = FVector::ForwardVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "YJH|Skill")
	FVector RequestedLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "YJH|Skill")
	bool bHasRequestedLocation = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "YJH|Skill")
	float ClientRequestTimeSec = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "YJH|Skill")
	FVector AimHitLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "YJH|Skill")
	FVector AimHitNormal = FVector::UpVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "YJH|Skill")
	FString AimHitActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "YJH|Skill")
	FName OptionalTargetId = NAME_None;
};

USTRUCT(BlueprintType)
struct FPSJANGGI_API FYJHSkillDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YJH|Skill")
	FName SkillId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YJH|Skill", meta = (EditCondition = "!bIsPassive", EditConditionHides, DisplayName = "Input Slot"))
	EYJHSkillSlot InputSlotEnum = EYJHSkillSlot::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YJH|Skill", meta = (EditCondition = "!bIsPassive && InputSlotEnum == EYJHSkillSlot::None", EditConditionHides, DisplayName = "Legacy InputSlot (Deprecated)", AdvancedDisplay))
	FName InputSlot = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YJH|Skill")
	EYJHSkillExecType ExecType = EYJHSkillExecType::Reserved;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YJH|Skill", meta = (ClampMin = "0.0", ClampMax = "999.0"))
	float CooldownSec = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YJH|Skill", meta = (ClampMin = "0.0"))
	float MaxRange = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YJH|Skill")
	float BasePower = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YJH|Skill")
	bool bServerAuthorityOnly = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YJH|Skill")
	bool bIsPassive = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YJH|Skill", meta = (EditCondition = "bIsPassive || ExecType == EYJHSkillExecType::Passive", EditConditionHides))
	EYJHPassiveType PassiveType = EYJHPassiveType::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YJH|Skill")
	FName SkillIconKey = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YJH|Skill")
	FText CooldownDisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YJH|Skill", meta = (EditCondition = "ExecType == EYJHSkillExecType::HitScan", EditConditionHides))
	float HitScanRange = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YJH|Skill", meta = (EditCondition = "ExecType == EYJHSkillExecType::HitScan", EditConditionHides))
	float HitScanRadius = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YJH|Skill", meta = (EditCondition = "ExecType == EYJHSkillExecType::HitScan", EditConditionHides))
	float HitScanDamage = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YJH|Skill", meta = (EditCondition = "ExecType == EYJHSkillExecType::Projectile", EditConditionHides))
	FName ProjectileClassKey = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YJH|Skill", meta = (EditCondition = "ExecType == EYJHSkillExecType::Projectile", EditConditionHides))
	float ProjectileSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YJH|Skill", meta = (EditCondition = "ExecType == EYJHSkillExecType::Projectile", EditConditionHides))
	float ProjectileGravityScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YJH|Skill", meta = (EditCondition = "ExecType == EYJHSkillExecType::Projectile", EditConditionHides))
	float ProjectileMassScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YJH|Skill", meta = (EditCondition = "ExecType == EYJHSkillExecType::Projectile", EditConditionHides))
	float ProjectileDamage = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YJH|Skill", meta = (EditCondition = "ExecType == EYJHSkillExecType::Dash", EditConditionHides))
	float DashPower = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YJH|Skill", meta = (EditCondition = "ExecType == EYJHSkillExecType::Dash", EditConditionHides))
	bool bDashUseInputDirection = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YJH|Skill", meta = (EditCondition = "ExecType == EYJHSkillExecType::CooldownControl", EditConditionHides))
	FName CooldownTargetSkillId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YJH|Skill", meta = (EditCondition = "ExecType == EYJHSkillExecType::CooldownControl", EditConditionHides))
	float CooldownDeltaSec = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YJH|Skill", meta = (EditCondition = "ExecType == EYJHSkillExecType::Teleport", EditConditionHides))
	float TeleportMaxDistance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YJH|Skill", meta = (EditCondition = "ExecType == EYJHSkillExecType::Teleport", EditConditionHides))
	float TeleportSafetyRadius = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YJH|Skill", meta = (EditCondition = "ExecType == EYJHSkillExecType::Teleport", EditConditionHides))
	int32 TeleportFallbackTries = 0;
};
