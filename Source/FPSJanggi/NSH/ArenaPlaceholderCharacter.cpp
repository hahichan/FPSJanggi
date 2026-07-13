// Copyright Epic Games, Inc. All Rights Reserved.

#include "ArenaPlaceholderCharacter.h"

#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Net/UnrealNetwork.h"

AArenaPlaceholderCharacter::AArenaPlaceholderCharacter()
{
	bReplicates = true;
	SetReplicateMovement(true);
	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh"));
	SetRootComponent(MeshComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComponent->SetGenerateOverlapEvents(false);
}

void AArenaPlaceholderCharacter::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Display, TEXT("BOARD_ARENA_PLACEHOLDER_READY role=%s mesh=%s location=%s"),
		*UEnum::GetValueAsString(GetLocalRole()),
		CharacterMesh ? *CharacterMesh->GetName() : TEXT("None"),
		*GetActorLocation().ToCompactString());
}

void AArenaPlaceholderCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AArenaPlaceholderCharacter, CharacterMesh);
}

void AArenaPlaceholderCharacter::SetCharacterMesh(USkeletalMesh* Mesh)
{
	if (!HasAuthority()) return;
	CharacterMesh = Mesh;
	OnRep_CharacterMesh();
	ForceNetUpdate();
}

void AArenaPlaceholderCharacter::OnRep_CharacterMesh()
{
	if (MeshComponent)
	{
		MeshComponent->SetSkeletalMesh(CharacterMesh);
	}
	UE_LOG(LogTemp, Display, TEXT("BOARD_ARENA_PLACEHOLDER_MESH role=%s mesh=%s"),
		*UEnum::GetValueAsString(GetLocalRole()), CharacterMesh ? *CharacterMesh->GetName() : TEXT("None"));
}
