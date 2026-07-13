// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ArenaPlaceholderCharacter.generated.h"

class USkeletalMesh;
class USkeletalMeshComponent;

/** Replicated visual-only character used to prove the board-to-arena handoff. */
UCLASS()
class FPSJANGGI_API AArenaPlaceholderCharacter : public AActor
{
	GENERATED_BODY()

public:
	AArenaPlaceholderCharacter();
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void SetCharacterMesh(USkeletalMesh* Mesh);

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USkeletalMeshComponent> MeshComponent;

	UPROPERTY(ReplicatedUsing = OnRep_CharacterMesh)
	TObjectPtr<USkeletalMesh> CharacterMesh;

	UFUNCTION()
	void OnRep_CharacterMesh();
};
