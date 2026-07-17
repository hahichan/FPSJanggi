// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "yjh_base/yjh_ArenaCombatSkillTypes.h"
#include "yjh_ArenaCombatantBase.generated.h"

class UYJHArenaCombatComponent;
class UYJHArenaHealthComponent;
class UYJHSkillDataAsset;
class UInputAction;
class UInputMappingContext;
class UCameraComponent;
class USkeletalMeshComponent;
struct FInputActionValue;

UCLASS(BlueprintType, Blueprintable)
class FPSJANGGI_API AYJHArenaCombatantBase : public ACharacter
{
	GENERATED_BODY()

public:
	AYJHArenaCombatantBase();
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "YJH|Movement")
	void MoveForward(float Value);

	UFUNCTION(BlueprintCallable, Category = "YJH|Movement")
	void MoveRight(float Value);

	UFUNCTION(BlueprintCallable, Category = "YJH|Movement")
	void TurnYaw(float Value);

	UFUNCTION(BlueprintCallable, Category = "YJH|Movement")
	void LookPitch(float Value);

	UFUNCTION(BlueprintCallable, Category = "YJH|Movement")
	void SetWalkSpeed(float NewWalkSpeed);

	UFUNCTION(BlueprintCallable, Category = "YJH|Movement")
	void SetScaleStats(float InVisualScale, float InHitboxScale);

	UFUNCTION(BlueprintCallable, Category = "YJH|Movement")
	void ApplyScaleStats();

	UFUNCTION(BlueprintCallable, Category = "YJH|Movement")
	void SetMovementEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "YJH|Combat")
	void PrimaryAction();

	UFUNCTION(BlueprintCallable, Category = "YJH|Combat")
	FYJHSkillExecutionResult TriggerSkillSlotByIndex(int32 SlotNumber);

	UFUNCTION(BlueprintCallable, Category = "YJH|Combat")
	void BeginCombatSession(FName InCombatSessionId);

	UFUNCTION(BlueprintCallable, Category = "YJH|Combat")
	void EndCombatSession(FName InCombatSessionId, FName EndReason);

	UFUNCTION(BlueprintCallable, Category = "YJH|Combat")
	void SetCombatEnabled(FName InCombatSessionId, bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "YJH|Combat")
	FYJHSkillExecutionResult TriggerSkillBySlot(FName SlotIndex, FYJHRuntimeSkillRequestContext RuntimeContext);

	UFUNCTION(BlueprintPure, Category = "YJH|Combat")
	FName GetCombatantId() const { return CombatantId; }

	UFUNCTION(BlueprintPure, Category = "YJH|Combat")
	FName GetCombatSessionId() const { return CombatSessionId; }

	UFUNCTION(BlueprintPure, Category = "YJH|Combat")
	bool IsCombatEnabled() const { return bCombatEnabled; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "YJH|Identity")
	FName CombatantId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "YJH|Identity")
	EYJHTeamInfo TeamInfo = EYJHTeamInfo::Unassigned;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "YJH|Identity")
	EYJHPieceType PieceType = EYJHPieceType::Unknown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "YJH|Identity")
	FName PieceInstanceId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "YJH|Movement", meta = (ClampMin = "0.0"))
	float BaseWalkSpeed = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "YJH|View|FirstPerson")
	bool bEnableFirstPersonPresentation = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "YJH|View|FirstPerson")
	bool bHideThirdPersonMeshForOwner = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "YJH|View|FirstPerson")
	FVector FirstPersonCameraOffset = FVector(0.0f, 0.0f, 64.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "YJH|View|FirstPerson", meta = (ClampMin = "60.0", ClampMax = "120.0"))
	float FirstPersonFieldOfView = 95.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_VisualScale, Category = "YJH|Movement", meta = (ClampMin = "0.2", ClampMax = "3.0"))
	float VisualScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_HitboxScale, Category = "YJH|Movement", meta = (ClampMin = "0.2", ClampMax = "3.0"))
	float HitboxScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "YJH|Movement", meta = (ClampMin = "0.0"))
	float TurnRateScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "YJH|Movement", meta = (ClampMin = "0.0"))
	float LookRateScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "YJH|Combat", meta = (ClampMin = "0.0"))
	float BasicAttackDamage = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "YJH|Combat", meta = (ClampMin = "0.0"))
	float BasicAttackRange = 220.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "YJH|Combat", meta = (ClampMin = "0.0"))
	float BasicAttackRadius = 65.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "YJH|Combat", meta = (ClampMin = "0.0", ClampMax = "10.0"))
	float BasicAttackCooldownSeconds = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "YJH|Combat", meta = (ClampMin = "1.0"))
	float MaxHP = 100.0f;

	/** Optional convenience override exposed on Pawn details. If set, copied into CombatComponent->SkillDataAsset at runtime. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YJH|Combat|Setup")
	TObjectPtr<UYJHSkillDataAsset> SkillDataAssetOverride;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YJH|Input|Enhanced")
	bool bUseEnhancedInputBindings = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YJH|Input|Enhanced")
	TObjectPtr<UInputMappingContext> CommonInputMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YJH|Input|Enhanced")
	TObjectPtr<UInputMappingContext> CombatInputMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YJH|Input|Enhanced")
	int32 CommonInputContextPriority = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YJH|Input|Enhanced")
	int32 CombatInputContextPriority = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YJH|Input|Enhanced")
	TObjectPtr<UInputAction> IA_Move;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YJH|Input|Enhanced")
	TObjectPtr<UInputAction> IA_Look;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YJH|Input|Enhanced")
	TObjectPtr<UInputAction> IA_PrimaryAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YJH|Input|Enhanced")
	TObjectPtr<UInputAction> IA_SkillSlot1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YJH|Input|Enhanced")
	TObjectPtr<UInputAction> IA_SkillSlot2;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YJH|Input|Enhanced")
	TObjectPtr<UInputAction> IA_SkillSlot3;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YJH|Input|Enhanced")
	TObjectPtr<UInputAction> IA_SkillSlot4;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YJH|Input|Enhanced")
	TObjectPtr<UInputAction> IA_SkillSlot5;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YJH|Input|Enhanced")
	TObjectPtr<UInputAction> IA_SkillSlot6;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YJH|Input|Enhanced")
	TObjectPtr<UInputAction> IA_SkillSlot7;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YJH|Input|Enhanced")
	TObjectPtr<UInputAction> IA_SkillSlot8;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YJH|Input|Enhanced")
	TObjectPtr<UInputAction> IA_SkillSlot9;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YJH|Input|Enhanced")
	TObjectPtr<UInputAction> IA_SkillSlot10;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "YJH|Components")
	TObjectPtr<UYJHArenaHealthComponent> HealthComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "YJH|Components")
	TObjectPtr<UYJHArenaCombatComponent> CombatComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "YJH|Components")
	TObjectPtr<UCameraComponent> FirstPersonCameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "YJH|Components")
	TObjectPtr<USkeletalMeshComponent> FirstPersonArmsMesh;

	UFUNCTION(BlueprintImplementableEvent, Category = "YJH|Visual")
	void OnDamagedVisualOnly(float DamageAmount);

	UFUNCTION(BlueprintImplementableEvent, Category = "YJH|Visual")
	void OnDeadVisualOnly();

protected:
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_Controller() override;

private:
	UFUNCTION(Server, Reliable)
	void ServerPrimaryAction();

	UFUNCTION(Server, Reliable)
	void ServerTriggerSkillBySlot(FName SlotIndex, FYJHRuntimeSkillRequestContext RuntimeContext);

	UFUNCTION(Client, Reliable)
	void ClientSyncSkillCooldownSnapshot(const TArray<float>& SlotRemainingSeconds);

	void ExecutePrimaryActionServer();
	bool CanUsePrimaryAction() const;
	FName BuildSlotName(int32 SlotNumber) const;
	bool TryBindEnhancedInput(class UInputComponent* PlayerInputComponent);
	void AddEnhancedInputMappingContexts() const;
	void HandleEnhancedMove(const FInputActionValue& Value);
	void HandleEnhancedLook(const FInputActionValue& Value);
	void HandleEnhancedPrimaryAction(const FInputActionValue& Value);
	void HandleEnhancedSkillSlot1(const FInputActionValue& Value);
	void HandleEnhancedSkillSlot2(const FInputActionValue& Value);
	void HandleEnhancedSkillSlot3(const FInputActionValue& Value);
	void HandleEnhancedSkillSlot4(const FInputActionValue& Value);
	void HandleEnhancedSkillSlot5(const FInputActionValue& Value);
	void HandleEnhancedSkillSlot6(const FInputActionValue& Value);
	void HandleEnhancedSkillSlot7(const FInputActionValue& Value);
	void HandleEnhancedSkillSlot8(const FInputActionValue& Value);
	void HandleEnhancedSkillSlot9(const FInputActionValue& Value);
	void HandleEnhancedSkillSlot10(const FInputActionValue& Value);

	UFUNCTION()
	void TriggerSkillSlot1();
	UFUNCTION()
	void TriggerSkillSlot2();
	UFUNCTION()
	void TriggerSkillSlot3();
	UFUNCTION()
	void TriggerSkillSlot4();
	UFUNCTION()
	void TriggerSkillSlot5();
	UFUNCTION()
	void TriggerSkillSlot6();
	UFUNCTION()
	void TriggerSkillSlot7();
	UFUNCTION()
	void TriggerSkillSlot8();
	UFUNCTION()
	void TriggerSkillSlot9();
	UFUNCTION()
	void TriggerSkillSlot10();

	UFUNCTION()
	void HandleDamaged(FName InCombatSessionId, FName InstigatorCombatantId, float Amount);

	UFUNCTION()
	void HandleDead(FName InCombatSessionId, FName DeadCombatantId, FName KillerCombatantId, FName EndReason);

	UFUNCTION()
	void OnRep_VisualScale();

	UFUNCTION()
	void OnRep_HitboxScale();

	void CacheScaleDefaults();
	void RefreshFirstPersonPresentation();

	UPROPERTY(VisibleAnywhere, Category = "YJH|Combat")
	FName CombatSessionId = NAME_None;

	UPROPERTY(VisibleAnywhere, Category = "YJH|Combat")
	bool bCombatEnabled = false;

	UPROPERTY(VisibleAnywhere, Category = "YJH|Movement")
	bool bMovementEnabled = true;

	UPROPERTY(VisibleAnywhere, Category = "YJH|Combat")
	double NextPrimaryActionTimeSeconds = 0.0;

	UPROPERTY(VisibleAnywhere, Category = "YJH|Combat")
	bool bDeathHandledForSession = false;

	UPROPERTY(VisibleAnywhere, Category = "YJH|Movement")
	bool bScaleDefaultsCached = false;

	UPROPERTY(VisibleAnywhere, Category = "YJH|Movement")
	float DefaultCapsuleRadius = 0.0f;

	UPROPERTY(VisibleAnywhere, Category = "YJH|Movement")
	float DefaultCapsuleHalfHeight = 0.0f;

	UPROPERTY(VisibleAnywhere, Category = "YJH|Movement")
	FVector DefaultMeshScale = FVector::OneVector;
};
