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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YJH|Skill")
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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YJH|Skill")
	EYJHPassiveType PassiveType = EYJHPassiveType::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YJH|Skill")
	FName SkillIconKey = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YJH|Skill")
	FText CooldownDisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YJH|Skill")
	float HitScanRange = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YJH|Skill")
	float HitScanRadius = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YJH|Skill")
	float HitScanDamage = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YJH|Skill")
	FName ProjectileClassKey = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YJH|Skill")
	float ProjectileSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YJH|Skill")
	float ProjectileGravityScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YJH|Skill")
	float ProjectileMassScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YJH|Skill")
	float ProjectileDamage = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YJH|Skill")
	float DashPower = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YJH|Skill")
	bool bDashUseInputDirection = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YJH|Skill")
	FName CooldownTargetSkillId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YJH|Skill")
	float CooldownDeltaSec = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YJH|Skill")
	float TeleportMaxDistance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YJH|Skill")
	float TeleportSafetyRadius = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YJH|Skill")
	int32 TeleportFallbackTries = 0;
};
