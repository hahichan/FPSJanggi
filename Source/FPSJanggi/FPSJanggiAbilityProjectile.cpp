// Copyright Epic Games, Inc. All Rights Reserved.

#include "FPSJanggiAbilityProjectile.h"

#include "Components/SphereComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

AFPSJanggiAbilityProjectile::AFPSJanggiAbilityProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	kkw_collision_component = CreateDefaultSubobject<USphereComponent>(TEXT("kkw_collision_component"));
	kkw_collision_component->InitSphereRadius(18.0f);
	kkw_collision_component->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	kkw_collision_component->SetNotifyRigidBodyCollision(true);
	RootComponent = kkw_collision_component;

	kkw_visual_component = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("kkw_visual_component"));
	kkw_visual_component->SetupAttachment(RootComponent);
	kkw_visual_component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	kkw_visual_component->SetRelativeScale3D(FVector(0.35f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> kkw_sphere_mesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (kkw_sphere_mesh.Succeeded())
	{
		kkw_visual_component->SetStaticMesh(kkw_sphere_mesh.Object);
	}

	kkw_projectile_movement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("kkw_projectile_movement"));
	kkw_projectile_movement->bRotationFollowsVelocity = true;
	kkw_projectile_movement->InitialSpeed = 2300.0f;
	kkw_projectile_movement->MaxSpeed = 3200.0f;
	kkw_projectile_movement->ProjectileGravityScale = 0.18f;
}

void AFPSJanggiAbilityProjectile::BeginPlay()
{
	Super::BeginPlay();
	kkw_collision_component->OnComponentHit.AddDynamic(this, &AFPSJanggiAbilityProjectile::OnProjectileHit);
	SetLifeSpan(4.0f);
}

void AFPSJanggiAbilityProjectile::ConfigureProjectile(float kkw_in_damage, float kkw_in_blast_radius, const FColor& kkw_in_debug_color)
{
	kkw_damage = kkw_in_damage;
	kkw_blast_radius = kkw_in_blast_radius;
	kkw_debug_color = kkw_in_debug_color;
}

void AFPSJanggiAbilityProjectile::FireInDirection(const FVector& kkw_direction, float kkw_speed)
{
	const FVector kkw_safe_direction = kkw_direction.GetSafeNormal();
	kkw_projectile_movement->Velocity = kkw_safe_direction * kkw_speed;
	kkw_projectile_movement->InitialSpeed = kkw_speed;
	kkw_projectile_movement->MaxSpeed = FMath::Max(kkw_speed, kkw_projectile_movement->MaxSpeed);
}

void AFPSJanggiAbilityProjectile::OnProjectileHit(UPrimitiveComponent* kkw_hit_component, AActor* kkw_other_actor, UPrimitiveComponent* kkw_other_comp, FVector kkw_normal_impulse, const FHitResult& kkw_hit)
{
	if (kkw_other_actor == GetOwner())
	{
		return;
	}

	Explode();
}

void AFPSJanggiAbilityProjectile::Explode()
{
	TArray<AActor*> kkw_ignored_actors;
	kkw_ignored_actors.Add(this);
	if (AActor* kkw_projectile_owner = GetOwner())
	{
		kkw_ignored_actors.Add(kkw_projectile_owner);
	}

	UGameplayStatics::ApplyRadialDamage(this, kkw_damage, GetActorLocation(), kkw_blast_radius, UDamageType::StaticClass(), kkw_ignored_actors, this, GetInstigatorController(), true);
	DrawDebugSphere(GetWorld(), GetActorLocation(), kkw_blast_radius, 24, kkw_debug_color, false, 1.0f, 0, 4.0f);
	Destroy();
}
