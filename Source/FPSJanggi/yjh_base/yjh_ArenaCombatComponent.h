// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "yjh_base/yjh_ArenaCombatSkillTypes.h"
#include "yjh_ArenaCombatComponent.generated.h"

class UYJHSkillDataAsset;
class AYJHArenaSkillProjectile;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FYJHOnSkillSlotMapped, FName, SlotIndex, FName, SkillId, bool, bIsPassive);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FYJHOnCooldownChanged, FName, SlotIndex, float, RemainingSec);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FYJHOnSkillRequestFailed, EYJHSkillFailCode, FailCode);

UCLASS(ClassGroup = (Custom), BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class FPSJANGGI_API UYJHArenaCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UYJHArenaCombatComponent();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "YJH|Combat")
	bool InitializeSkillMap();

	UFUNCTION(BlueprintCallable, Category = "YJH|Combat")
	void SetCombatContext(FName InCombatSessionId, FName InCombatantId);

	UFUNCTION(BlueprintCallable, Category = "YJH|Combat")
	FYJHSkillExecutionResult RequestSkillBySlot(FName SlotIndex, const FYJHRuntimeSkillRequestContext& RuntimeContext);

	UFUNCTION(BlueprintCallable, Category = "YJH|Combat")
	float GetRemainingCooldownBySlot(FName SlotIndex) const;

	UFUNCTION(BlueprintCallable, Category = "YJH|Combat")
	float GetRemainingCooldownBySkillId(FName SkillId) const;

	void ApplyClientCooldownSnapshot(const TArray<float>& SlotRemainingSeconds);

	UFUNCTION(BlueprintPure, Category = "YJH|Combat")
	FName GetCombatSessionId() const { return CombatSessionId; }

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YJH|Combat")
	TObjectPtr<UYJHSkillDataAsset> SkillDataAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "YJH|Combat", meta = (ClampMin = "1", ClampMax = "64"))
	int32 MaxSkillSlots = 10;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YJH|Combat|Projectile")
	TSubclassOf<AYJHArenaSkillProjectile> DefaultProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YJH|Combat|Projectile")
	TMap<FName, TSubclassOf<AYJHArenaSkillProjectile>> ProjectileClassByKey;

	UPROPERTY(BlueprintAssignable, Category = "YJH|Combat")
	FYJHOnSkillSlotMapped OnSkillSlotMapped;

	UPROPERTY(BlueprintAssignable, Category = "YJH|Combat")
	FYJHOnCooldownChanged OnCooldownChanged;

	UPROPERTY(BlueprintAssignable, Category = "YJH|Combat")
	FYJHOnSkillRequestFailed OnSkillRequestFailed;

private:
	static constexpr float MinCooldownSeconds = 0.0f;
	static constexpr float MaxCooldownSeconds = 999.0f;

	FYJHSkillExecutionResult BuildFailure(FName SlotIndex, FName SkillId, EYJHSkillFailCode FailCode) const;
	bool ValidateRequest(const FYJHSkillDefinition& SkillSnapshot, const FYJHRuntimeSkillRequestContext& RuntimeContext, EYJHSkillFailCode& OutFailCode) const;
	void ExecuteSkill(const FYJHSkillDefinition& SkillSnapshot, const FYJHRuntimeSkillRequestContext& RuntimeContext, FYJHSkillExecutionResult& InOutResult);
	void StartSkillCooldown(const FYJHSkillDefinition& SkillSnapshot);
	float ClampCooldown(float CooldownValue, EYJHSkillFailCode& OutGuardCode) const;
	bool FindValidTeleportDestination(const FYJHSkillDefinition& SkillSnapshot, const FYJHRuntimeSkillRequestContext& RuntimeContext, FVector& OutDestination) const;
	bool IsTeleportDestinationValid(const FVector& TestLocation, float SafetyRadius) const;
	TSubclassOf<AYJHArenaSkillProjectile> ResolveProjectileClass(FName ProjectileClassKey) const;
	void TickPassiveSkills(float DeltaTime);
	void ApplyRegenPassive(const FYJHSkillDefinition& SkillSnapshot, float DeltaTime);
	void ApplyWallRunPassive(const FYJHSkillDefinition& SkillSnapshot, float DeltaTime);
	void RestoreMovementDefaults();

	UPROPERTY(VisibleAnywhere, Replicated, Category = "YJH|Combat")
	FName CombatSessionId = NAME_None;

	UPROPERTY(VisibleAnywhere, Replicated, Category = "YJH|Combat")
	FName CombatantId = NAME_None;

	UPROPERTY(VisibleAnywhere, Category = "YJH|Combat")
	TMap<FName, FYJHSkillDefinition> SkillById;

	UPROPERTY(VisibleAnywhere, Category = "YJH|Combat")
	TMap<FName, FName> SlotToSkillId;

	UPROPERTY(VisibleAnywhere, Category = "YJH|Combat")
	TMap<FName, double> CooldownEndTimeBySkillId;

	UPROPERTY(VisibleAnywhere, Category = "YJH|Combat")
	TMap<FName, double> ClientCooldownEndTimeBySlot;

	UPROPERTY(VisibleAnywhere, Category = "YJH|Combat")
	TArray<FYJHSkillDefinition> PassiveSkills;

	UPROPERTY(VisibleAnywhere, Category = "YJH|Combat")
	float DefaultGravityScale = 1.0f;

	UPROPERTY(VisibleAnywhere, Category = "YJH|Combat")
	float DefaultAirControl = 0.05f;

	UPROPERTY(VisibleAnywhere, Category = "YJH|Combat")
	bool bMovementDefaultsCached = false;

	UPROPERTY(VisibleAnywhere, Category = "YJH|Combat")
	bool bWallRunModifierApplied = false;
};
