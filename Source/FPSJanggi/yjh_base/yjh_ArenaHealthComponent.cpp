// Copyright Epic Games, Inc. All Rights Reserved.

#include "yjh_base/yjh_ArenaHealthComponent.h"

#include "Net/UnrealNetwork.h"

UYJHArenaHealthComponent::UYJHArenaHealthComponent()
{
	SetIsReplicatedByDefault(true);
}

void UYJHArenaHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UYJHArenaHealthComponent, MaxHP);
	DOREPLIFETIME(UYJHArenaHealthComponent, CurrentHP);
	DOREPLIFETIME(UYJHArenaHealthComponent, bDead);
}

void UYJHArenaHealthComponent::InitializeHealth(float InMaxHP)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	MaxHP = FMath::Max(1.0f, InMaxHP);
	CurrentHP = MaxHP;
	bDead = false;
}

bool UYJHArenaHealthComponent::ApplyServerDamage(FName CombatSessionId, FName InstigatorCombatantId, float Amount)
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || bDead || Amount <= 0.0f)
	{
		return false;
	}

	CurrentHP = FMath::Max(0.0f, CurrentHP - Amount);
	OnDamaged.Broadcast(CombatSessionId, InstigatorCombatantId, Amount);

	if (CurrentHP <= 0.0f && !bDead)
	{
		bDead = true;
		OnDead.Broadcast(CombatSessionId, NAME_None, InstigatorCombatantId, FName(TEXT("Eliminated")));
	}

	return true;
}

void UYJHArenaHealthComponent::HealServer(float Amount)
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || Amount <= 0.0f || bDead)
	{
		return;
	}
	CurrentHP = FMath::Clamp(CurrentHP + Amount, 0.0f, MaxHP);
}

void UYJHArenaHealthComponent::OnRep_CurrentHP(float PreviousHP)
{
	if (PreviousHP > CurrentHP)
	{
		OnDamaged.Broadcast(NAME_None, NAME_None, PreviousHP - CurrentHP);
	}
}
