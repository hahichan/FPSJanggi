// Copyright Epic Games, Inc. All Rights Reserved.

#include "yjh_base/yjh_ArenaCombatComponent.h"

#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "yjh_base/yjh_ArenaCombatantBase.h"
#include "yjh_base/yjh_ArenaHealthComponent.h"
#include "yjh_base/yjh_ArenaSkillProjectile.h"
#include "yjh_base/yjh_SkillDataAsset.h"

DEFINE_LOG_CATEGORY_STATIC(LogYJHCombat, Log, All);

UYJHArenaCombatComponent::UYJHArenaCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UYJHArenaCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (DeltaTime <= 0.0f)
	{
		return;
	}

	TickPassiveSkills(DeltaTime);
}

bool UYJHArenaCombatComponent::InitializeSkillMap()
{
	SkillById.Reset();
	SlotToSkillId.Reset();
	CooldownEndTimeBySkillId.Reset();
	PassiveSkills.Reset();
	RestoreMovementDefaults();

	if (!SkillDataAsset)
	{
		UE_LOG(LogYJHCombat, Warning, TEXT("YJH_SKILL_INIT failed: no SkillDataAsset"));
		SetComponentTickEnabled(false);
		return false;
	}

	MaxSkillSlots = FMath::Max(1, SkillDataAsset->MaxSkillSlots);

	FString Error;
	if (!SkillDataAsset->BuildSkillMaps(SkillById, SlotToSkillId, Error))
	{
		UE_LOG(LogYJHCombat, Error, TEXT("YJH_SKILL_INIT failed: %s"), *Error);
		SetComponentTickEnabled(false);
		return false;
	}

	for (const TPair<FName, FYJHSkillDefinition>& Pair : SkillById)
	{
		const FYJHSkillDefinition& Skill = Pair.Value;
		if (Skill.bIsPassive || Skill.ExecType == EYJHSkillExecType::Passive)
		{
			PassiveSkills.Add(Skill);
		}
	}

	SetComponentTickEnabled(PassiveSkills.Num() > 0);

	for (int32 SlotNumber = 1; SlotNumber <= MaxSkillSlots; ++SlotNumber)
	{
		const FName SlotName(*FString::Printf(TEXT("Slot%d"), SlotNumber));
		const FName* SkillId = SlotToSkillId.Find(SlotName);
		if (SkillId)
		{
			const FYJHSkillDefinition* Skill = SkillById.Find(*SkillId);
			const bool bPassive = Skill ? Skill->bIsPassive : false;
			OnSkillSlotMapped.Broadcast(SlotName, *SkillId, bPassive);
		}
		else
		{
			OnSkillSlotMapped.Broadcast(SlotName, NAME_None, false);
		}
		OnCooldownChanged.Broadcast(SlotName, 0.0f);
	}

	return true;
}

void UYJHArenaCombatComponent::SetCombatContext(FName InCombatSessionId, FName InCombatantId)
{
	CombatSessionId = InCombatSessionId;
	CombatantId = InCombatantId;
}

FYJHSkillExecutionResult UYJHArenaCombatComponent::RequestSkillBySlot(FName SlotIndex, const FYJHRuntimeSkillRequestContext& RuntimeContext)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		const FYJHSkillExecutionResult Result = BuildFailure(SlotIndex, NAME_None, EYJHSkillFailCode::SKX_ExecutionBlocked);
		OnSkillRequestFailed.Broadcast(Result.FailCode);
		return Result;
	}

	const FName* SkillIdPtr = SlotToSkillId.Find(SlotIndex);
	if (!SkillIdPtr)
	{
		const FYJHSkillExecutionResult Result = BuildFailure(SlotIndex, NAME_None, EYJHSkillFailCode::SKX_NoSkillMapped);
		OnSkillRequestFailed.Broadcast(Result.FailCode);
		return Result;
	}

	const FYJHSkillDefinition* SkillPtr = SkillById.Find(*SkillIdPtr);
	if (!SkillPtr)
	{
		const FYJHSkillExecutionResult Result = BuildFailure(SlotIndex, *SkillIdPtr, EYJHSkillFailCode::SKX_NoSkillMapped);
		OnSkillRequestFailed.Broadcast(Result.FailCode);
		return Result;
	}

	const FYJHSkillDefinition SkillSnapshot = *SkillPtr;
	EYJHSkillFailCode FailCode = EYJHSkillFailCode::None;
	if (!ValidateRequest(SkillSnapshot, RuntimeContext, FailCode))
	{
		const FYJHSkillExecutionResult Result = BuildFailure(SlotIndex, SkillSnapshot.SkillId, FailCode);
		OnSkillRequestFailed.Broadcast(Result.FailCode);
		return Result;
	}

	FYJHSkillExecutionResult Result;
	Result.bSuccess = true;
	Result.SkillId = SkillSnapshot.SkillId;
	Result.SlotIndex = SlotIndex;
	Result.FailCode = EYJHSkillFailCode::None;

	ExecuteSkill(SkillSnapshot, RuntimeContext, Result);
	if (!Result.bSuccess)
	{
		OnSkillRequestFailed.Broadcast(Result.FailCode);
		return Result;
	}

	StartSkillCooldown(SkillSnapshot);
	return Result;
}

float UYJHArenaCombatComponent::GetRemainingCooldownBySlot(FName SlotIndex) const
{
	const FName* SkillId = SlotToSkillId.Find(SlotIndex);
	return SkillId ? GetRemainingCooldownBySkillId(*SkillId) : 0.0f;
}

float UYJHArenaCombatComponent::GetRemainingCooldownBySkillId(FName SkillId) const
{
	if (!GetWorld())
	{
		return 0.0f;
	}
	const double* EndTimePtr = CooldownEndTimeBySkillId.Find(SkillId);
	if (!EndTimePtr)
	{
		return 0.0f;
	}
	return static_cast<float>(FMath::Max(0.0, *EndTimePtr - GetWorld()->GetTimeSeconds()));
}

FYJHSkillExecutionResult UYJHArenaCombatComponent::BuildFailure(FName SlotIndex, FName SkillId, EYJHSkillFailCode FailCode) const
{
	FYJHSkillExecutionResult Result;
	Result.bSuccess = false;
	Result.SlotIndex = SlotIndex;
	Result.SkillId = SkillId;
	Result.FailCode = FailCode;
	return Result;
}

bool UYJHArenaCombatComponent::ValidateRequest(const FYJHSkillDefinition& SkillSnapshot, const FYJHRuntimeSkillRequestContext& RuntimeContext, EYJHSkillFailCode& OutFailCode) const
{
	if (CombatSessionId.IsNone() || RuntimeContext.CombatSessionId.IsNone())
	{
		OutFailCode = EYJHSkillFailCode::SKX_InvalidSession;
		return false;
	}

	if (GetRemainingCooldownBySkillId(SkillSnapshot.SkillId) > 0.0f)
	{
		OutFailCode = EYJHSkillFailCode::SKX_CooldownActive;
		return false;
	}

	if (SkillSnapshot.ExecType == EYJHSkillExecType::Teleport && !RuntimeContext.bHasRequestedLocation)
	{
		OutFailCode = EYJHSkillFailCode::SKX_TeleportBlocked;
		return false;
	}

	if (SkillSnapshot.MaxRange > 0.0f && !RuntimeContext.SourceLocation.IsNearlyZero() && RuntimeContext.bHasRequestedLocation)
	{
		const float Distance = FVector::Dist(RuntimeContext.SourceLocation, RuntimeContext.RequestedLocation);
		if (Distance > SkillSnapshot.MaxRange)
		{
			OutFailCode = EYJHSkillFailCode::SKX_OutOfRange;
			return false;
		}
	}

	OutFailCode = EYJHSkillFailCode::None;
	return true;
}

void UYJHArenaCombatComponent::ExecuteSkill(const FYJHSkillDefinition& SkillSnapshot, const FYJHRuntimeSkillRequestContext& RuntimeContext, FYJHSkillExecutionResult& InOutResult)
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		InOutResult = BuildFailure(RuntimeContext.SlotIndex, SkillSnapshot.SkillId, EYJHSkillFailCode::SKX_ExecutionBlocked);
		return;
	}

	switch (SkillSnapshot.ExecType)
	{
	case EYJHSkillExecType::HitScan:
	{
		const FVector Dir = RuntimeContext.RequestedDirection.IsNearlyZero() ? RuntimeContext.SourceForward.GetSafeNormal() : RuntimeContext.RequestedDirection.GetSafeNormal();
		const float TraceRange = FMath::Max(0.0f, SkillSnapshot.HitScanRange);
		const FVector End = RuntimeContext.SourceLocation + Dir * TraceRange;
		FHitResult HitResult;
		FCollisionQueryParams Params(SCENE_QUERY_STAT(YJH_HitScan), false, Owner);
		const bool bHit = Owner->GetWorld()->LineTraceSingleByChannel(HitResult, RuntimeContext.SourceLocation, End, ECC_Pawn, Params);
		if (!bHit)
		{
			break;
		}

		AYJHArenaCombatantBase* TargetCombatant = Cast<AYJHArenaCombatantBase>(HitResult.GetActor());
		AYJHArenaCombatantBase* SourceCombatant = Cast<AYJHArenaCombatantBase>(Owner);
		if (!TargetCombatant || !SourceCombatant || TargetCombatant == SourceCombatant)
		{
			break;
		}

		if (TargetCombatant->TeamInfo == SourceCombatant->TeamInfo || !TargetCombatant->HealthComponent || TargetCombatant->HealthComponent->IsDead())
		{
			break;
		}

		const float Damage = FMath::Max(0.0f, SkillSnapshot.HitScanDamage > 0.0f ? SkillSnapshot.HitScanDamage : SkillSnapshot.BasePower);
		if (Damage > 0.0f)
		{
			TargetCombatant->HealthComponent->ApplyServerDamage(RuntimeContext.CombatSessionId, RuntimeContext.CombatantId, Damage);
		}
		break;
	}
	case EYJHSkillExecType::Projectile:
	{
		const TSubclassOf<AYJHArenaSkillProjectile> ProjectileClass = ResolveProjectileClass(SkillSnapshot.ProjectileClassKey);
		if (!ProjectileClass)
		{
			InOutResult = BuildFailure(RuntimeContext.SlotIndex, SkillSnapshot.SkillId, EYJHSkillFailCode::SKX_ExecutionBlocked);
			return;
		}

		AYJHArenaCombatantBase* SourceCombatant = Cast<AYJHArenaCombatantBase>(Owner);
		const FVector Dir = RuntimeContext.RequestedDirection.IsNearlyZero() ? RuntimeContext.SourceForward.GetSafeNormal() : RuntimeContext.RequestedDirection.GetSafeNormal();
		const FVector SpawnLocation = RuntimeContext.SourceLocation + Dir * 60.0f;
		const FRotator SpawnRotation = Dir.Rotation();

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = Owner;
		SpawnParams.Instigator = Cast<APawn>(Owner);
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		AYJHArenaSkillProjectile* Projectile = Owner->GetWorld()->SpawnActor<AYJHArenaSkillProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);
		if (!Projectile)
		{
			InOutResult = BuildFailure(RuntimeContext.SlotIndex, SkillSnapshot.SkillId, EYJHSkillFailCode::SKX_ExecutionBlocked);
			return;
		}

		const float Damage = FMath::Max(0.0f, SkillSnapshot.ProjectileDamage > 0.0f ? SkillSnapshot.ProjectileDamage : SkillSnapshot.BasePower);
		const EYJHTeamInfo InstigatorTeam = SourceCombatant ? SourceCombatant->TeamInfo : EYJHTeamInfo::Unassigned;
		Projectile->InitializeProjectile(
			RuntimeContext.CombatSessionId,
			RuntimeContext.CombatantId,
			InstigatorTeam,
			Damage,
			SkillSnapshot.ProjectileSpeed,
			SkillSnapshot.ProjectileGravityScale,
			SkillSnapshot.ProjectileMassScale,
			Dir);
		break;
	}
	case EYJHSkillExecType::Dash:
	{
		const FVector DashDir = SkillSnapshot.bDashUseInputDirection ? RuntimeContext.RequestedDirection.GetSafeNormal() : RuntimeContext.SourceForward.GetSafeNormal();
		const FVector Delta = DashDir * FMath::Max(0.0f, SkillSnapshot.DashPower);
		Owner->SetActorLocation(Owner->GetActorLocation() + Delta, true);
		break;
	}
	case EYJHSkillExecType::CooldownControl:
	{
		if (SkillSnapshot.CooldownTargetSkillId.IsNone())
		{
			InOutResult = BuildFailure(RuntimeContext.SlotIndex, SkillSnapshot.SkillId, EYJHSkillFailCode::SKX_InvalidTarget);
			return;
		}

		const float Current = GetRemainingCooldownBySkillId(SkillSnapshot.CooldownTargetSkillId);
		EYJHSkillFailCode GuardCode = EYJHSkillFailCode::None;
		const float Clamped = ClampCooldown(Current + SkillSnapshot.CooldownDeltaSec, GuardCode);
		if (GuardCode == EYJHSkillFailCode::SKX_CooldownOverflowGuard)
		{
			InOutResult = BuildFailure(RuntimeContext.SlotIndex, SkillSnapshot.SkillId, GuardCode);
			return;
		}
		if (GetWorld())
		{
			CooldownEndTimeBySkillId.Add(SkillSnapshot.CooldownTargetSkillId, GetWorld()->GetTimeSeconds() + Clamped);
		}
		break;
	}
	case EYJHSkillExecType::Teleport:
	{
		FVector Destination;
		if (!FindValidTeleportDestination(SkillSnapshot, RuntimeContext, Destination))
		{
			InOutResult = BuildFailure(RuntimeContext.SlotIndex, SkillSnapshot.SkillId, EYJHSkillFailCode::SKX_TeleportNoValidDestination);
			return;
		}
		Owner->SetActorLocation(Destination, true);
		break;
	}
	case EYJHSkillExecType::Passive:
		// Passive skills are applied in TickPassiveSkills while the session is active.
		break;
	default:
		InOutResult = BuildFailure(RuntimeContext.SlotIndex, SkillSnapshot.SkillId, EYJHSkillFailCode::SKX_ExecutionBlocked);
		return;
	}
}

void UYJHArenaCombatComponent::StartSkillCooldown(const FYJHSkillDefinition& SkillSnapshot)
{
	if (!GetWorld())
	{
		return;
	}

	EYJHSkillFailCode GuardCode = EYJHSkillFailCode::None;
	const float Cooldown = ClampCooldown(SkillSnapshot.CooldownSec, GuardCode);
	if (Cooldown <= 0.0f)
	{
		return;
	}

	CooldownEndTimeBySkillId.Add(SkillSnapshot.SkillId, GetWorld()->GetTimeSeconds() + Cooldown);
	if (!SkillSnapshot.InputSlot.IsNone())
	{
		OnCooldownChanged.Broadcast(SkillSnapshot.InputSlot, Cooldown);
	}
}

float UYJHArenaCombatComponent::ClampCooldown(float CooldownValue, EYJHSkillFailCode& OutGuardCode) const
{
	if (CooldownValue < MinCooldownSeconds)
	{
		OutGuardCode = EYJHSkillFailCode::SKX_CooldownUnderflowGuard;
		return MinCooldownSeconds;
	}
	if (CooldownValue > MaxCooldownSeconds)
	{
		OutGuardCode = EYJHSkillFailCode::SKX_CooldownOverflowGuard;
		return MaxCooldownSeconds;
	}
	OutGuardCode = EYJHSkillFailCode::None;
	return CooldownValue;
}

bool UYJHArenaCombatComponent::FindValidTeleportDestination(const FYJHSkillDefinition& SkillSnapshot, const FYJHRuntimeSkillRequestContext& RuntimeContext, FVector& OutDestination) const
{
	if (!RuntimeContext.bHasRequestedLocation)
	{
		return false;
	}

	const FVector Requested = RuntimeContext.RequestedLocation;
	if (IsTeleportDestinationValid(Requested, SkillSnapshot.TeleportSafetyRadius))
	{
		OutDestination = Requested;
		return true;
	}

	const int32 Tries = FMath::Max(0, SkillSnapshot.TeleportFallbackTries);
	for (int32 Index = 0; Index < Tries; ++Index)
	{
		const float Angle = (2.0f * PI * Index) / FMath::Max(1, Tries);
		const FVector Offset(FMath::Cos(Angle), FMath::Sin(Angle), 0.0f);
		const FVector Candidate = Requested + Offset * SkillSnapshot.TeleportSafetyRadius;
		if (IsTeleportDestinationValid(Candidate, SkillSnapshot.TeleportSafetyRadius))
		{
			OutDestination = Candidate;
			return true;
		}
	}

	return false;
}

bool UYJHArenaCombatComponent::IsTeleportDestinationValid(const FVector& TestLocation, float SafetyRadius) const
{
	const AActor* Owner = GetOwner();
	if (!Owner || !Owner->GetWorld())
	{
		return false;
	}

	FCollisionQueryParams Params(SCENE_QUERY_STAT(YJH_TeleportValidity), false, Owner);
	const FCollisionShape Shape = FCollisionShape::MakeSphere(FMath::Max(1.0f, SafetyRadius));
	const bool bBlocked = Owner->GetWorld()->OverlapBlockingTestByChannel(TestLocation, FQuat::Identity, ECC_Pawn, Shape, Params);
	return !bBlocked;
}

void UYJHArenaCombatComponent::TickPassiveSkills(float DeltaTime)
{
	AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority() || PassiveSkills.Num() == 0)
	{
		return;
	}

	for (const FYJHSkillDefinition& PassiveSkill : PassiveSkills)
	{
		switch (PassiveSkill.PassiveType)
		{
		case EYJHPassiveType::Regen:
			ApplyRegenPassive(PassiveSkill, DeltaTime);
			break;
		case EYJHPassiveType::WallRun:
			ApplyWallRunPassive(PassiveSkill, DeltaTime);
			break;
		default:
			break;
		}
	}
}

void UYJHArenaCombatComponent::ApplyRegenPassive(const FYJHSkillDefinition& SkillSnapshot, float DeltaTime)
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	UYJHArenaHealthComponent* HealthComponent = Owner->FindComponentByClass<UYJHArenaHealthComponent>();
	if (!HealthComponent || HealthComponent->IsDead())
	{
		return;
	}

	const float RegenPerSecond = FMath::Max(0.0f, SkillSnapshot.BasePower);
	if (RegenPerSecond <= 0.0f)
	{
		return;
	}

	HealthComponent->HealServer(RegenPerSecond * DeltaTime);
}

void UYJHArenaCombatComponent::ApplyWallRunPassive(const FYJHSkillDefinition& SkillSnapshot, float DeltaTime)
{
	ACharacter* CharacterOwner = Cast<ACharacter>(GetOwner());
	if (!CharacterOwner)
	{
		return;
	}

	UCharacterMovementComponent* MovementComponent = CharacterOwner->GetCharacterMovement();
	if (!MovementComponent)
	{
		return;
	}

	if (!bMovementDefaultsCached)
	{
		DefaultGravityScale = MovementComponent->GravityScale;
		DefaultAirControl = MovementComponent->AirControl;
		bMovementDefaultsCached = true;
	}

	const float ForwardTraceDistance = FMath::Max(30.0f, SkillSnapshot.MaxRange > 0.0f ? SkillSnapshot.MaxRange : 80.0f);
	const FVector TraceStart = CharacterOwner->GetActorLocation();
	const FVector TraceEnd = TraceStart + CharacterOwner->GetActorForwardVector() * ForwardTraceDistance;

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(YJH_WallRunPassive), false, CharacterOwner);
	FHitResult WallHit;
	const bool bHitWallAhead = GetWorld() && GetWorld()->LineTraceSingleByChannel(WallHit, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams);
	const bool bFalling = MovementComponent->IsFalling();

	if (bFalling && bHitWallAhead)
	{
		const float ClimbSpeed = FMath::Max(150.0f, SkillSnapshot.BasePower);
		MovementComponent->Velocity.Z = FMath::Max(MovementComponent->Velocity.Z, ClimbSpeed);
		MovementComponent->GravityScale = 0.35f;
		MovementComponent->AirControl = 1.0f;
		bWallRunModifierApplied = true;
		return;
	}

	if (bWallRunModifierApplied)
	{
		RestoreMovementDefaults();
	}
}

void UYJHArenaCombatComponent::RestoreMovementDefaults()
{
	ACharacter* CharacterOwner = Cast<ACharacter>(GetOwner());
	if (!CharacterOwner)
	{
		bWallRunModifierApplied = false;
		return;
	}

	UCharacterMovementComponent* MovementComponent = CharacterOwner->GetCharacterMovement();
	if (!MovementComponent)
	{
		bWallRunModifierApplied = false;
		return;
	}

	if (bMovementDefaultsCached)
	{
		MovementComponent->GravityScale = DefaultGravityScale;
		MovementComponent->AirControl = DefaultAirControl;
	}

	bWallRunModifierApplied = false;
}

TSubclassOf<AYJHArenaSkillProjectile> UYJHArenaCombatComponent::ResolveProjectileClass(FName ProjectileClassKey) const
{
	if (!ProjectileClassKey.IsNone())
	{
		if (const TSubclassOf<AYJHArenaSkillProjectile>* FoundClass = ProjectileClassByKey.Find(ProjectileClassKey))
		{
			if (*FoundClass)
			{
				return *FoundClass;
			}
		}
	}

	return DefaultProjectileClass;
}
