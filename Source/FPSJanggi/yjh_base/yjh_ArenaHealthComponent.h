// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "yjh_ArenaHealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FYJHOnDamaged, FName, CombatSessionId, FName, InstigatorCombatantId, float, Amount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FYJHOnDead, FName, CombatSessionId, FName, DeadCombatantId, FName, KillerCombatantId, FName, EndReason);

UCLASS(ClassGroup = (Custom), BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class FPSJANGGI_API UYJHArenaHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UYJHArenaHealthComponent();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "YJH|Health")
	void InitializeHealth(float InMaxHP);

	UFUNCTION(BlueprintCallable, Category = "YJH|Health")
	bool ApplyServerDamage(FName CombatSessionId, FName InstigatorCombatantId, float Amount);

	UFUNCTION(BlueprintCallable, Category = "YJH|Health")
	void HealServer(float Amount);

	UFUNCTION(BlueprintPure, Category = "YJH|Health")
	bool IsDead() const { return bDead; }

	UFUNCTION(BlueprintPure, Category = "YJH|Health")
	float GetCurrentHP() const { return CurrentHP; }

	UFUNCTION(BlueprintPure, Category = "YJH|Health")
	float GetMaxHP() const { return MaxHP; }

	UPROPERTY(BlueprintAssignable, Category = "YJH|Health")
	FYJHOnDamaged OnDamaged;

	UPROPERTY(BlueprintAssignable, Category = "YJH|Health")
	FYJHOnDead OnDead;

private:
	UFUNCTION()
	void OnRep_CurrentHP(float PreviousHP);

	UPROPERTY(Replicated)
	float MaxHP = 100.0f;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentHP)
	float CurrentHP = 100.0f;

	UPROPERTY(Replicated)
	bool bDead = false;
};
