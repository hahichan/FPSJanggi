// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FPSJanggiAbilityProjectile.generated.h"

class UProjectileMovementComponent;
class USphereComponent;

UCLASS()
class FPSJANGGI_API AFPSJanggiAbilityProjectile : public AActor
{
	GENERATED_BODY()

public:
	AFPSJanggiAbilityProjectile();

	void ConfigureProjectile(float kkw_in_damage, float kkw_in_blast_radius, const FColor& kkw_in_debug_color);
	void FireInDirection(const FVector& kkw_direction, float kkw_speed);

protected:
	UPROPERTY(VisibleAnywhere, Category = "kkw_projectile")
	TObjectPtr<USphereComponent> kkw_collision_component;

	UPROPERTY(VisibleAnywhere, Category = "kkw_projectile")
	TObjectPtr<UStaticMeshComponent> kkw_visual_component;

	UPROPERTY(VisibleAnywhere, Category = "kkw_projectile")
	TObjectPtr<UProjectileMovementComponent> kkw_projectile_movement;

	UPROPERTY(EditDefaultsOnly, Category = "kkw_projectile")
	float kkw_damage = 45.0f;

	UPROPERTY(EditDefaultsOnly, Category = "kkw_projectile")
	float kkw_blast_radius = 220.0f;

	FColor kkw_debug_color = FColor::Orange;

	virtual void BeginPlay() override;

	UFUNCTION()
	void OnProjectileHit(UPrimitiveComponent* kkw_hit_component, AActor* kkw_other_actor, UPrimitiveComponent* kkw_other_comp, FVector kkw_normal_impulse, const FHitResult& kkw_hit);

	void Explode();
};
