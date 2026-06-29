// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FPSJanggiAbilityCharacter.h"
#include "FPSJanggiAbilityPlayerController.generated.h"

class UAnimSequence;
class AActor;

UCLASS()
class FPSJANGGI_API AFPSJanggiAbilityPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AFPSJanggiAbilityPlayerController();

	virtual void BeginPlay() override;
	virtual void Tick(float kkw_delta_seconds) override;
	virtual void SetupInputComponent() override;

protected:
	UPROPERTY()
	TArray<TObjectPtr<AFPSJanggiAbilityCharacter>> kkw_controlled_pieces;

	UPROPERTY()
	TArray<TObjectPtr<AActor>> kkw_source_piece_actors;

	TArray<FTransform> kkw_last_source_piece_transforms;
	TArray<FTransform> kkw_last_piece_source_transforms;

	void BuildPlayablePieces();
	void EnsureScriptDirectoryActor();
	void SyncSourcePiecePlacements();
	void ApplyPiecePlacementToSource(AActor* kkw_source_actor, const FTransform& kkw_source_transform) const;
	void SelectPieceIndex(int32 kkw_index);
	void SelectPiece1();
	void SelectPiece2();
	void SelectPiece3();
	void SelectPiece4();
	void SelectPiece5();
	void SelectPiece6();
	void ToggleCameraMode();

	AActor* FindActorByLabels(const TArray<FString>& kkw_labels) const;
	FTransform GetSourcePlacementTransform(AActor* kkw_source_actor) const;
	FTransform GetPieceSourceTransform(AFPSJanggiAbilityCharacter* kkw_piece, const FTransform& kkw_current_source_transform) const;
	FVector FindFallbackCenter() const;
	USkeletalMesh* LoadMesh(const TCHAR* kkw_mesh_path) const;
	UAnimSequence* LoadAnimation(const TCHAR* kkw_animation_path) const;
};
