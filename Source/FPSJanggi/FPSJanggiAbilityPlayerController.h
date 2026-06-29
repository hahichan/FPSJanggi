// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FPSJanggiAbilityCharacter.h"
#include "FPSJanggiAbilityPlayerController.generated.h"

class UAnimSequence;

UCLASS()
class FPSJANGGI_API AFPSJanggiAbilityPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AFPSJanggiAbilityPlayerController();

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

protected:
	UPROPERTY()
	TArray<TObjectPtr<AFPSJanggiAbilityCharacter>> kkw_controlled_pieces;

	void BuildPlayablePieces();
	void EnsureScriptDirectoryActor();
	void SelectPieceIndex(int32 kkw_index);
	void SelectPiece1();
	void SelectPiece2();
	void SelectPiece3();
	void SelectPiece4();
	void SelectPiece5();
	void SelectPiece6();
	void ToggleCameraMode();

	AActor* FindActorByLabels(const TArray<FString>& kkw_labels) const;
	FVector FindFallbackCenter() const;
	USkeletalMesh* LoadMesh(const TCHAR* kkw_mesh_path) const;
	UAnimSequence* LoadAnimation(const TCHAR* kkw_animation_path) const;
};
