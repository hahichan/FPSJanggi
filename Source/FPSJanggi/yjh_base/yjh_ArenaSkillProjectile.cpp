// Copyright Epic Games, Inc. All Rights Reserved.

#include "yjh_base/yjh_ArenaSkillProjectile.h"

#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "yjh_base/yjh_ArenaCombatantBase.h"
#include "yjh_base/yjh_ArenaHealthComponent.h"

AYJHArenaSkillProjectile::AYJHArenaSkillProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	CollisionComponent->InitSphereRadius(12.0f);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionComponent->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	CollisionComponent->SetNotifyRigidBodyCollision(true);
	SetRootComponent(CollisionComponent);

	VisualMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMeshComponent->SetupAttachment(CollisionComponent);
	VisualMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionComponent;
	ProjectileMovement->InitialSpeed = 1200.0f;
	ProjectileMovement->MaxSpeed = 1200.0f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.0f;

	CollisionComponent->OnComponentHit.AddDynamic(this, &AYJHArenaSkillProjectile::HandleProjectileHit);
}

void AYJHArenaSkillProjectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AYJHArenaSkillProjectile, CombatSessionId);
	DOREPLIFETIME(AYJHArenaSkillProjectile, InstigatorCombatantId);
	DOREPLIFETIME(AYJHArenaSkillProjectile, InstigatorTeam);
	DOREPLIFETIME(AYJHArenaSkillProjectile, Damage);
	DOREPLIFETIME(AYJHArenaSkillProjectile, bInitialized);
}

void AYJHArenaSkillProjectile::InitializeProjectile(FName InCombatSessionId, FName InInstigatorCombatantId, EYJHTeamInfo InInstigatorTeam, float InDamage, float InSpeed, float InGravityScale, float InMassScale, const FVector& InDirection)
{
	CombatSessionId = InCombatSessionId;
	InstigatorCombatantId = InInstigatorCombatantId;
	InstigatorTeam = InInstigatorTeam;
	Damage = FMath::Max(0.0f, InDamage);
	bInitialized = true;

	if (!ProjectileMovement)
	{
		return;
	}

	const FVector SafeDir = InDirection.IsNearlyZero() ? GetActorForwardVector() : InDirection.GetSafeNormal();
	const float Speed = FMath::Max(200.0f, InSpeed);
	ProjectileMovement->InitialSpeed = Speed;
	ProjectileMovement->MaxSpeed = Speed;
	ProjectileMovement->Velocity = SafeDir * Speed;
	ProjectileMovement->ProjectileGravityScale = InGravityScale;

	if (CollisionComponent)
	{
		const float MassScale = FMath::Max(0.01f, InMassScale);
		CollisionComponent->SetMassScale(NAME_None, MassScale);
	}
}

void AYJHArenaSkillProjectile::BeginPlay()
{
	Super::BeginPlay();
	SetLifeSpan(FMath::Max(0.05f, MaxLifeSeconds));
}

void AYJHArenaSkillProjectile::HandleProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!HasAuthority())
	{
		return;
	}

	if (!bInitialized || !OtherActor || OtherActor == GetOwner())
	{
		Destroy();
		return;
	}

	AYJHArenaCombatantBase* TargetCombatant = Cast<AYJHArenaCombatantBase>(OtherActor);
	if (TargetCombatant && TargetCombatant->TeamInfo != InstigatorTeam && TargetCombatant->HealthComponent && !TargetCombatant->HealthComponent->IsDead() && Damage > 0.0f)
	{
		TargetCombatant->HealthComponent->ApplyServerDamage(CombatSessionId, InstigatorCombatantId, Damage);
	}

	Destroy();
}
