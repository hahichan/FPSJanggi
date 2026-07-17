// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "yjh_base/yjh_ArenaCombatSkillTypes.h"
#include "yjh_ArenaSkillProjectile.generated.h"

class UProjectileMovementComponent;
class USphereComponent;
class UStaticMeshComponent;
class UPrimitiveComponent;
struct FHitResult;

UCLASS(BlueprintType, Blueprintable)
class FPSJANGGI_API AYJHArenaSkillProjectile : public AActor
{
	GENERATED_BODY()

public:
	AYJHArenaSkillProjectile();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "YJH|Projectile")
	void InitializeProjectile(FName InCombatSessionId, FName InInstigatorCombatantId, EYJHTeamInfo InInstigatorTeam, float InDamage, float InSpeed, float InGravityScale, float InMassScale, const FVector& InDirection);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "YJH|Projectile")
	TObjectPtr<USphereComponent> CollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "YJH|Projectile")
	TObjectPtr<UStaticMeshComponent> VisualMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "YJH|Projectile")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "YJH|Projectile", meta = (ClampMin = "0.05", ClampMax = "30.0"))
	float MaxLifeSeconds = 5.0f;

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void HandleProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY(Replicated, VisibleAnywhere, Category = "YJH|Projectile")
	FName CombatSessionId = NAME_None;

	UPROPERTY(Replicated, VisibleAnywhere, Category = "YJH|Projectile")
	FName InstigatorCombatantId = NAME_None;

	UPROPERTY(Replicated, VisibleAnywhere, Category = "YJH|Projectile")
	EYJHTeamInfo InstigatorTeam = EYJHTeamInfo::Unassigned;

	UPROPERTY(Replicated, VisibleAnywhere, Category = "YJH|Projectile")
	float Damage = 0.0f;

	UPROPERTY(Replicated, VisibleAnywhere, Category = "YJH|Projectile")
	bool bInitialized = false;
};
